#include "StdAfx.h"
#include "GrpText.h"
#include "FontManager.h"
#include "EterBase/Stl.h"

#include "Util.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cmath>

// Gamma LUT to sharpen grayscale anti-aliasing edges.
static struct SAlphaGammaLUT {
  unsigned char table[256];
  SAlphaGammaLUT() {
    table[0] = 0;
    for (int i = 1; i < 256; ++i)
      table[i] = (unsigned char)(pow(i / 255.0, 0.85) * 255.0 + 0.5);
  }
} s_alphaGammaLUT;

CGraphicFontTexture::CGraphicFontTexture() { Initialize(); }

CGraphicFontTexture::~CGraphicFontTexture() { Destroy(); }

void CGraphicFontTexture::Initialize() {
  CGraphicTexture::Initialize();
  m_ftFace = nullptr;
  m_pAtlasBuffer = nullptr;
  m_atlasWidth = 0;
  m_atlasHeight = 0;
  m_isDirty = false;
  m_bItalic = false;
  m_ascender = 0;
  m_lineHeight = 0;
  m_hasKerning = false;
  m_x = 0;
  m_y = 0;
  m_step = 0;
  m_fontSize = 0;
}

bool CGraphicFontTexture::IsEmpty() const { return m_ftFace == nullptr; }

void CGraphicFontTexture::Destroy() {
  delete[] m_pAtlasBuffer;
  m_pAtlasBuffer = nullptr;

  m_lpd3dTexture = NULL;
  CGraphicTexture::Destroy();
  stl_wipe(m_pFontTextureVector);
  m_charInfoMap.clear();

  if (m_ftFace) {
    FT_Done_Face(m_ftFace);
    m_ftFace = nullptr;
  }

  Initialize();
}

bool CGraphicFontTexture::CreateDeviceObjects() {
  if (!m_ftFace)
    return true;

  // After device reset: wipe GPU textures, clear atlas state, and
  // re-render all previously cached characters on demand.
  // We keep m_charInfoMap keys but clear the entries so glyphs get
  // re-rasterized.
  std::vector<TCharacterKey> cachedKeys;
  cachedKeys.reserve(m_charInfoMap.size());
  for (const auto &pair : m_charInfoMap)
    cachedKeys.push_back(pair.first);

  stl_wipe(m_pFontTextureVector);
  m_charInfoMap.clear();
  m_x = 0;
  m_y = 0;
  m_step = 0;
  m_isDirty = false;

  // Reset CPU atlas buffer
  if (m_pAtlasBuffer)
    memset(m_pAtlasBuffer, 0, m_atlasWidth * m_atlasHeight * sizeof(DWORD));

  // Create first GPU texture page
  if (!AppendTexture())
    return false;

  // Re-rasterize all previously cached glyphs
  for (TCharacterKey key : cachedKeys)
    UpdateCharacterInfomation(key);

  UpdateTexture();
  return true;
}

void CGraphicFontTexture::DestroyDeviceObjects() {
  m_lpd3dTexture = NULL;
  stl_wipe(m_pFontTextureVector);
}

bool CGraphicFontTexture::Create(const char *c_szFontName, int fontSize,
                                 bool bItalic) {
  Destroy();

  m_fontSize = fontSize;
  m_bItalic = bItalic;

  m_x = 0;
  m_y = 0;
  m_step = 0;

  // Determine atlas dimensions
  DWORD width = 256, height = 256;
  if (GetMaxTextureWidth() > 512)
    width = 512;
  if (GetMaxTextureHeight() > 512)
    height = 512;

  m_atlasWidth = width;
  m_atlasHeight = height;

  // Allocate CPU-side atlas buffer
  m_pAtlasBuffer = new DWORD[width * height];
  memset(m_pAtlasBuffer, 0, width * height * sizeof(DWORD));

  // Create a per-instance FT_Face (this instance owns it)
  m_ftFace = CFontManager::Instance().CreateFace(c_szFontName);
  if (!m_ftFace) {
    TraceError("CGraphicFontTexture::Create - Failed to create face for '%s'",
               c_szFontName ? c_szFontName : "(null)");
    return false;
  }

  int pixelSize = (fontSize < 0) ? -fontSize : fontSize;
  if (pixelSize == 0)
    pixelSize = 12;

  FT_Set_Pixel_Sizes(m_ftFace, 0, pixelSize);

  m_hasKerning = FT_HAS_KERNING(m_ftFace) != 0;

  // Apply italic via shear matrix if needed
  if (bItalic) {
    FT_Matrix matrix;
    matrix.xx = 0x10000L;
    matrix.xy = 0x5800L; // ~0.34 shear for synthetic italic
    matrix.yx = 0;
    matrix.yy = 0x10000L;
    FT_Set_Transform(m_ftFace, &matrix, NULL);
  } else {
    FT_Set_Transform(m_ftFace, NULL, NULL);
  }

  // Cache font metrics
  m_ascender = (int)(m_ftFace->size->metrics.ascender >> 6);
  m_lineHeight = (int)(m_ftFace->size->metrics.height >> 6);

  if (!AppendTexture())
    return false;

  return true;
}

bool CGraphicFontTexture::AppendTexture() {
  CGraphicImageTexture *pNewTexture = new CGraphicImageTexture;

  if (!pNewTexture->Create(m_atlasWidth, m_atlasHeight, D3DFMT_A8R8G8B8)) {
    delete pNewTexture;
    return false;
  }

  m_pFontTextureVector.push_back(pNewTexture);
  return true;
}

bool CGraphicFontTexture::UpdateTexture() {
  if (!m_isDirty)
    return true;

  m_isDirty = false;

  CGraphicImageTexture *pFontTexture = m_pFontTextureVector.back();

  if (!pFontTexture)
    return false;

  DWORD *pdwDst;
  int pitch;

  if (!pFontTexture->Lock(&pitch, (void **)&pdwDst))
    return false;

  pitch /= 4; // pitch in DWORDs (A8R8G8B8 = 4 bytes per pixel)

  DWORD *pdwSrc = m_pAtlasBuffer;

  for (int y = 0; y < m_atlasHeight;
       ++y, pdwDst += pitch, pdwSrc += m_atlasWidth) {
    memcpy(pdwDst, pdwSrc, m_atlasWidth * sizeof(DWORD));
  }

  pFontTexture->Unlock();
  return true;
}

float CGraphicFontTexture::GetKerning(wchar_t prev, wchar_t cur) {
  if (!m_hasKerning || !m_ftFace || prev == 0)
    return 0.0f;

  FT_UInt prevIndex = FT_Get_Char_Index(m_ftFace, prev);
  FT_UInt curIndex = FT_Get_Char_Index(m_ftFace, cur);

  if (prevIndex == 0 || curIndex == 0)
    return 0.0f;

  FT_Vector delta;
  if (FT_Get_Kerning(m_ftFace, prevIndex, curIndex, FT_KERNING_DEFAULT,
                     &delta) != 0)
    return 0.0f;

  return (float)(delta.x) / 64.0f;
}

CGraphicFontTexture::TCharacterInfomation *
CGraphicFontTexture::GetCharacterInfomation(wchar_t keyValue) {
  TCharacterKey code = keyValue;

  TCharacterInfomationMap::iterator f = m_charInfoMap.find(code);

  if (m_charInfoMap.end() == f) {
    return UpdateCharacterInfomation(code);
  } else {
    return &f->second;
  }
}

CGraphicFontTexture::TCharacterInfomation *
CGraphicFontTexture::UpdateCharacterInfomation(TCharacterKey keyValue) {
  if (!m_ftFace)
    return NULL;

  if (keyValue == 0x08)
    keyValue = L' ';

  // Load and render the glyph
  FT_UInt glyphIndex = FT_Get_Char_Index(m_ftFace, keyValue);
  if (glyphIndex == 0 && keyValue != L' ') {
    // Try space as fallback for unknown characters
    glyphIndex = FT_Get_Char_Index(m_ftFace, L' ');
    if (glyphIndex == 0)
      return NULL;
  }

  if (FT_Load_Glyph(m_ftFace, glyphIndex, FT_LOAD_TARGET_LCD) != 0)
    return NULL;

  if (FT_Render_Glyph(m_ftFace->glyph, FT_RENDER_MODE_LCD) != 0)
    return NULL;

  FT_GlyphSlot slot = m_ftFace->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  int glyphBitmapWidth =
      bitmap.width / 3; // LCD bitmap is 3x wider (R,G,B per pixel)
  int glyphBitmapHeight = bitmap.rows;
  int bearingX = slot->bitmap_left;
  int bearingY = slot->bitmap_top;
  float advance = ceilf((float)(slot->advance.x) / 64.0f);

  // Normalize glyph placement to common baseline
  // yOffset = distance from atlas row top to where the glyph bitmap starts
  int yOffset = m_ascender - bearingY;
  if (yOffset < 0)
    yOffset = 0;

  // The effective cell height is the full line height
  int cellHeight = m_lineHeight;
  int cellWidth = glyphBitmapWidth;

  // For spacing characters (space, etc.)
  if (glyphBitmapWidth == 0 || glyphBitmapHeight == 0) {
    TCharacterInfomation &rNewCharInfo = m_charInfoMap[keyValue];
    rNewCharInfo.index = static_cast<short>(m_pFontTextureVector.size() - 1);
    rNewCharInfo.width = 0;
    rNewCharInfo.height = (short)cellHeight;
    rNewCharInfo.left = 0;
    rNewCharInfo.top = 0;
    rNewCharInfo.right = 0;
    rNewCharInfo.bottom = 0;
    rNewCharInfo.advance = advance;
    rNewCharInfo.bearingX = 0.0f;
    return &rNewCharInfo;
  }

  // Make sure cell fits the glyph including offset
  int requiredHeight = yOffset + glyphBitmapHeight;
  if (requiredHeight > cellHeight)
    cellHeight = requiredHeight;

  int width = m_atlasWidth;
  int height = m_atlasHeight;

  // Atlas packing (row-based)
  if (m_x + cellWidth >= (width - 1)) {
    m_y += (m_step + 1);
    m_step = 0;
    m_x = 0;

    if (m_y + cellHeight >= (height - 1)) {
      if (!UpdateTexture())
        return NULL;

      if (!AppendTexture())
        return NULL;

      // Reset atlas buffer for new texture
      memset(m_pAtlasBuffer, 0, m_atlasWidth * m_atlasHeight * sizeof(DWORD));
      m_y = 0;
    }
  }

  // Copy LCD subpixel FreeType bitmap into atlas buffer (R,G,B per-channel
  // coverage)
  for (int row = 0; row < glyphBitmapHeight; ++row) {
    int atlasY = m_y + yOffset + row;
    if (atlasY < 0 || atlasY >= height)
      continue;

    unsigned char *srcRow = bitmap.buffer + row * bitmap.pitch;
    DWORD *dstRow = m_pAtlasBuffer + atlasY * m_atlasWidth + m_x;

    for (int col = 0; col < glyphBitmapWidth; ++col) {
      unsigned char r = srcRow[col * 3 + 0];
      unsigned char g = srcRow[col * 3 + 1];
      unsigned char b = srcRow[col * 3 + 2];
      if (r | g | b) {
        unsigned char a = (r > g) ? r : g;
        if (b > a)
          a = b;
        // A8R8G8B8: A=max_coverage, R=r_cov, G=g_cov, B=b_cov
        dstRow[col] =
            ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
      }
    }
  }

  float rhwidth = 1.0f / float(width);
  float rhheight = 1.0f / float(height);

  TCharacterInfomation &rNewCharInfo = m_charInfoMap[keyValue];

  rNewCharInfo.index = static_cast<short>(m_pFontTextureVector.size() - 1);
  rNewCharInfo.width = (short)cellWidth;
  rNewCharInfo.height = (short)cellHeight;
  rNewCharInfo.left = float(m_x) * rhwidth;
  rNewCharInfo.top = float(m_y) * rhheight;
  rNewCharInfo.right = float(m_x + cellWidth) * rhwidth;
  rNewCharInfo.bottom = float(m_y + cellHeight) * rhheight;
  rNewCharInfo.advance = advance;
  rNewCharInfo.bearingX = (float)bearingX;

  m_x += cellWidth + 1; // +1 horizontal padding to prevent bilinear bleed

  if (m_step < cellHeight)
    m_step = cellHeight;

  m_isDirty = true;

  return &rNewCharInfo;
}

bool CGraphicFontTexture::CheckTextureIndex(DWORD dwTexture) {
  if (dwTexture >= m_pFontTextureVector.size())
    return false;

  return true;
}

void CGraphicFontTexture::SelectTexture(DWORD dwTexture) {
  assert(CheckTextureIndex(dwTexture));
  m_lpd3dTexture = m_pFontTextureVector[dwTexture]->GetD3DTexture();
}
