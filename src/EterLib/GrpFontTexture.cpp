#include "StdAfx.h"
#include "GrpText.h"
#include "FontManager.h"
#include "EterBase/Stl.h"

#include "Util.h"
#include <utf8.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

CGraphicFontTexture::CGraphicFontTexture()
{
	Initialize();
}

CGraphicFontTexture::~CGraphicFontTexture()
{
	Destroy();
}

void CGraphicFontTexture::Initialize()
{
	CGraphicTexture::Initialize();
	m_ftFace = nullptr;
	m_pAtlasBuffer = nullptr;
	m_atlasWidth = 0;
	m_atlasHeight = 0;
	m_isDirty = false;
	m_bItalic = false;
	m_ascender = 0;
	m_lineHeight = 0;
	m_x = 0;
	m_y = 0;
	m_step = 0;
	m_fontSize = 0;
	memset(m_fontName, 0, sizeof(m_fontName));
}

bool CGraphicFontTexture::IsEmpty() const
{
	return m_ftFace == nullptr;
}

void CGraphicFontTexture::Destroy()
{
	delete[] m_pAtlasBuffer;
	m_pAtlasBuffer = nullptr;

	m_lpd3dTexture = NULL;
	CGraphicTexture::Destroy();
	stl_wipe(m_pFontTextureVector);
	m_charInfoMap.clear();

	// FT_Face is owned by CFontManager, do NOT free it here
	m_ftFace = nullptr;

	Initialize();
}

bool CGraphicFontTexture::CreateDeviceObjects()
{
	return true;
}

void CGraphicFontTexture::DestroyDeviceObjects()
{
}

bool CGraphicFontTexture::Create(const char* c_szFontName, int fontSize, bool bItalic)
{
	Destroy();

	// UTF-8 -> UTF-16 for font name storage
	std::wstring wFontName = Utf8ToWide(c_szFontName ? c_szFontName : "");
	wcsncpy_s(m_fontName, wFontName.c_str(), _TRUNCATE);

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

	// Get FT_Face from FontManager
	m_ftFace = CFontManager::Instance().GetFace(c_szFontName);
	if (!m_ftFace)
	{
		TraceError("CGraphicFontTexture::Create - Failed to get face for '%s'", c_szFontName ? c_szFontName : "(null)");
		return false;
	}

	Tracef(" FontTexture: loaded '%s' size=%d family='%s' style='%s'\n",
		c_szFontName ? c_szFontName : "(null)", fontSize,
		m_ftFace->family_name ? m_ftFace->family_name : "?",
		m_ftFace->style_name ? m_ftFace->style_name : "?");

	// Set pixel size
	int pixelSize = (fontSize < 0) ? -fontSize : fontSize;
	if (pixelSize == 0)
		pixelSize = 12;
	FT_Set_Pixel_Sizes(m_ftFace, 0, pixelSize);

	// Apply italic via shear matrix if needed
	if (bItalic)
	{
		FT_Matrix matrix;
		matrix.xx = 0x10000L;
		matrix.xy = 0x5800L;  // ~0.34 shear for synthetic italic
		matrix.yx = 0;
		matrix.yy = 0x10000L;
		FT_Set_Transform(m_ftFace, &matrix, NULL);
	}
	else
	{
		FT_Set_Transform(m_ftFace, NULL, NULL);
	}

	// Cache font metrics
	m_ascender = (int)(m_ftFace->size->metrics.ascender >> 6);
	m_lineHeight = (int)(m_ftFace->size->metrics.height >> 6);

	if (!AppendTexture())
		return false;

	return true;
}

bool CGraphicFontTexture::AppendTexture()
{
	CGraphicImageTexture* pNewTexture = new CGraphicImageTexture;

	if (!pNewTexture->Create(m_atlasWidth, m_atlasHeight, D3DFMT_A8R8G8B8))
	{
		delete pNewTexture;
		return false;
	}

	m_pFontTextureVector.push_back(pNewTexture);
	return true;
}

bool CGraphicFontTexture::UpdateTexture()
{
	if (!m_isDirty)
		return true;

	m_isDirty = false;

	CGraphicImageTexture* pFontTexture = m_pFontTextureVector.back();

	if (!pFontTexture)
		return false;

	DWORD* pdwDst;
	int pitch;

	if (!pFontTexture->Lock(&pitch, (void**)&pdwDst))
		return false;

	pitch /= 4;  // pitch in DWORDs (A8R8G8B8 = 4 bytes per pixel)

	DWORD* pdwSrc = m_pAtlasBuffer;

	for (int y = 0; y < m_atlasHeight; ++y, pdwDst += pitch, pdwSrc += m_atlasWidth)
	{
		memcpy(pdwDst, pdwSrc, m_atlasWidth * sizeof(DWORD));
	}

	pFontTexture->Unlock();
	return true;
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::GetCharacterInfomation(wchar_t keyValue)
{
	TCharacterKey code = keyValue;

	TCharacterInfomationMap::iterator f = m_charInfoMap.find(code);

	if (m_charInfoMap.end() == f)
	{
		return UpdateCharacterInfomation(code);
	}
	else
	{
		return &f->second;
	}
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::UpdateCharacterInfomation(TCharacterKey keyValue)
{
	if (!m_ftFace)
		return NULL;

	// Re-apply face state (FT_Face is shared across instances via CFontManager)
	int pixelSize = (m_fontSize < 0) ? -m_fontSize : m_fontSize;
	if (pixelSize == 0)
		pixelSize = 12;
	FT_Set_Pixel_Sizes(m_ftFace, 0, pixelSize);

	if (m_bItalic)
	{
		FT_Matrix matrix;
		matrix.xx = 0x10000L;
		matrix.xy = 0x5800L;
		matrix.yx = 0;
		matrix.yy = 0x10000L;
		FT_Set_Transform(m_ftFace, &matrix, NULL);
	}
	else
	{
		FT_Set_Transform(m_ftFace, NULL, NULL);
	}

	if (keyValue == 0x08)
		keyValue = L' ';

	// Load and render the glyph
	FT_UInt glyphIndex = FT_Get_Char_Index(m_ftFace, keyValue);
	if (glyphIndex == 0 && keyValue != L' ')
	{
		// Try space as fallback for unknown characters
		glyphIndex = FT_Get_Char_Index(m_ftFace, L' ');
		if (glyphIndex == 0)
			return NULL;
	}

	if (FT_Load_Glyph(m_ftFace, glyphIndex, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) != 0)
		return NULL;

	FT_GlyphSlot slot = m_ftFace->glyph;
	FT_Bitmap& bitmap = slot->bitmap;

	int glyphBitmapWidth = bitmap.width;
	int glyphBitmapHeight = bitmap.rows;
	int bearingY = slot->bitmap_top;
	float advance = (float)(slot->advance.x >> 6);

	// Normalize glyph placement to common baseline
	// yOffset = distance from atlas row top to where the glyph bitmap starts
	int yOffset = m_ascender - bearingY;
	if (yOffset < 0)
		yOffset = 0;

	// The effective cell height is the full line height
	int cellHeight = m_lineHeight;
	int cellWidth = glyphBitmapWidth;

	// For spacing characters (space, etc.)
	if (glyphBitmapWidth == 0 || glyphBitmapHeight == 0)
	{
		TCharacterInfomation& rNewCharInfo = m_charInfoMap[keyValue];
		rNewCharInfo.index = static_cast<short>(m_pFontTextureVector.size() - 1);
		rNewCharInfo.width = 0;
		rNewCharInfo.height = (short)cellHeight;
		rNewCharInfo.left = 0;
		rNewCharInfo.top = 0;
		rNewCharInfo.right = 0;
		rNewCharInfo.bottom = 0;
		rNewCharInfo.advance = advance;
		return &rNewCharInfo;
	}

	// Make sure cell fits the glyph including offset
	int requiredHeight = yOffset + glyphBitmapHeight;
	if (requiredHeight > cellHeight)
		cellHeight = requiredHeight;

	int width = m_atlasWidth;
	int height = m_atlasHeight;

	// Atlas packing (row-based)
	if (m_x + cellWidth >= (width - 1))
	{
		m_y += (m_step + 1);
		m_step = 0;
		m_x = 0;

		if (m_y + cellHeight >= (height - 1))
		{
			if (!UpdateTexture())
				return NULL;

			if (!AppendTexture())
				return NULL;

			// Reset atlas buffer for new texture
			memset(m_pAtlasBuffer, 0, m_atlasWidth * m_atlasHeight * sizeof(DWORD));
			m_y = 0;
		}
	}

	// Copy FreeType bitmap into atlas buffer at baseline-normalized position
	for (int row = 0; row < glyphBitmapHeight; ++row)
	{
		int atlasY = m_y + yOffset + row;
		if (atlasY < 0 || atlasY >= height)
			continue;

		unsigned char* srcRow = bitmap.buffer + row * bitmap.pitch;
		DWORD* dstRow = m_pAtlasBuffer + atlasY * m_atlasWidth + m_x;

		for (int col = 0; col < glyphBitmapWidth; ++col)
		{
			unsigned char alpha = srcRow[col];
			if (alpha)
				dstRow[col] = ((DWORD)alpha << 24) | 0x00FFFFFF;  // White + alpha
		}
	}

	float rhwidth = 1.0f / float(width);
	float rhheight = 1.0f / float(height);

	TCharacterInfomation& rNewCharInfo = m_charInfoMap[keyValue];

	rNewCharInfo.index = static_cast<short>(m_pFontTextureVector.size() - 1);
	rNewCharInfo.width = (short)cellWidth;
	rNewCharInfo.height = (short)cellHeight;
	rNewCharInfo.left = float(m_x) * rhwidth;
	rNewCharInfo.top = float(m_y) * rhheight;
	rNewCharInfo.right = float(m_x + cellWidth) * rhwidth;
	rNewCharInfo.bottom = float(m_y + cellHeight) * rhheight;
	rNewCharInfo.advance = advance;

	m_x += cellWidth;

	if (m_step < cellHeight)
		m_step = cellHeight;

	m_isDirty = true;

	return &rNewCharInfo;
}

bool CGraphicFontTexture::CheckTextureIndex(DWORD dwTexture)
{
	if (dwTexture >= m_pFontTextureVector.size())
		return false;

	return true;
}

void CGraphicFontTexture::SelectTexture(DWORD dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	m_lpd3dTexture = m_pFontTextureVector[dwTexture]->GetD3DTexture();
}
