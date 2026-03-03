#include "GrpImageTexture.h"
#include "DecodedImageData.h"
#include "EterImageLib/DDSTextureLoader11.h"
#include "EterImageLib/DDSTextureLoader9.h"
#include "PackLib/PackManager.h"
#include "StdAfx.h"

#include <stb_image.h>

#if defined(_M_IX86) || defined(_M_X64)
#include <emmintrin.h> // SSE2
#include <tmmintrin.h> // SSSE3 (for _mm_shuffle_epi8)
#endif

bool CGraphicImageTexture::Lock(int *pRetPitch, void **ppRetPixels, int level) {
  D3DLOCKED_RECT lockedRect;
  if (FAILED(m_lpd3dTexture->LockRect(level, &lockedRect, NULL, 0)))
    return false;

  *pRetPitch = lockedRect.Pitch;
  *ppRetPixels = (void *)lockedRect.pBits;
  return true;
}

void CGraphicImageTexture::Unlock(int level) {
  assert(m_lpd3dTexture != NULL);
  m_lpd3dTexture->UnlockRect(level);
}

void CGraphicImageTexture::Initialize() {
  CGraphicTexture::Initialize();

  m_stFileName = "";

  m_d3dFmt = D3DFMT_UNKNOWN;
  m_dwFilter = 0;
}

void CGraphicImageTexture::Destroy() {
  if (m_pDX11SRV) {
    m_pDX11SRV->Release();
    m_pDX11SRV = nullptr;
  }
  CGraphicTexture::Destroy();

  Initialize();
}

bool CGraphicImageTexture::CreateDeviceObjects() {
  assert(ms_lpd3dDevice != NULL);
  assert(m_lpd3dTexture == NULL);

  if (m_stFileName.empty()) {
    // Font texture — create DX9 dynamic
    if (FAILED(ms_lpd3dDevice->CreateTexture(
            m_width, m_height, 1, D3DUSAGE_DYNAMIC, m_d3dFmt, D3DPOOL_DEFAULT,
            &m_lpd3dTexture, nullptr)))
      return false;

    // DX11: Create matching dynamic texture
    if (ms_pD3D11Device) {
      DXGI_FORMAT dxgiFmt = DXGI_FORMAT_B8G8R8A8_UNORM; // default
      if (m_d3dFmt == D3DFMT_A8R8G8B8)
        dxgiFmt = DXGI_FORMAT_B8G8R8A8_UNORM;
      else if (m_d3dFmt == D3DFMT_A4R4G4B4)
        dxgiFmt = DXGI_FORMAT_B4G4R4A4_UNORM;
      else if (m_d3dFmt == D3DFMT_A8)
        dxgiFmt = DXGI_FORMAT_A8_UNORM;
      DX11Tex::CreateDynamicTexture(ms_pD3D11Device, m_width, m_height, dxgiFmt,
                                    &m_pDX11SRV);
    }
  } else {
    TPackFile mappedFile;
    if (!CPackManager::Instance().GetFile(m_stFileName, mappedFile))
      return false;

    return CreateFromMemoryFile(mappedFile.size(), mappedFile.data(), m_d3dFmt,
                                m_dwFilter);
  }

  m_bEmpty = false;
  return true;
}

bool CGraphicImageTexture::Create(UINT width, UINT height, D3DFORMAT d3dFmt,
                                  DWORD dwFilter) {
  assert(ms_lpd3dDevice != NULL);
  Destroy();

  m_width = width;
  m_height = height;
  m_d3dFmt = d3dFmt;
  m_dwFilter = dwFilter;

  return CreateDeviceObjects();
}

void CGraphicImageTexture::CreateFromTexturePointer(
    const CGraphicTexture *c_pSrcTexture) {
  if (m_lpd3dTexture)
    m_lpd3dTexture->Release();

  m_width = c_pSrcTexture->GetWidth();
  m_height = c_pSrcTexture->GetHeight();
  m_lpd3dTexture = c_pSrcTexture->GetD3DTexture();

  if (m_lpd3dTexture)
    m_lpd3dTexture->AddRef();

  m_bEmpty = false;
}

bool CGraphicImageTexture::CreateFromDDSTexture(UINT bufSize,
                                                const void *c_pvBuf) {
  if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(
          ms_lpd3dDevice, reinterpret_cast<const uint8_t *>(c_pvBuf), bufSize,
          0, D3DPOOL_DEFAULT, false, &m_lpd3dTexture)))
    return false;

  D3DSURFACE_DESC desc;
  m_lpd3dTexture->GetLevelDesc(0, &desc);
  m_width = desc.Width;
  m_height = desc.Height;
  m_bEmpty = false;

  // DX11: Create SRV from same DDS data
  if (ms_pD3D11Device)
    DX11Tex::CreateDDSTextureFromMemory(
        ms_pD3D11Device, reinterpret_cast<const uint8_t *>(c_pvBuf), bufSize,
        &m_pDX11SRV);

  // Register in the DX9→DX11 texture registry
  if (m_lpd3dTexture && m_pDX11SRV)
    RegisterDX11SRV(m_lpd3dTexture, m_pDX11SRV);

  return true;
}

bool CGraphicImageTexture::CreateFromSTB(UINT bufSize, const void *c_pvBuf) {
  int width, height, channels;
  unsigned char *data = stbi_load_from_memory(
      (stbi_uc *)c_pvBuf, bufSize, &width, &height, &channels, 4); // force RGBA
  if (data) {
    LPDIRECT3DTEXTURE9 texture;
    if (SUCCEEDED(ms_lpd3dDevice->CreateTexture(
            width, height, 1, 0,
            channels == 4 ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
            &texture, nullptr))) {
      D3DLOCKED_RECT rect;
      if (SUCCEEDED(texture->LockRect(0, &rect, nullptr, 0))) {
        uint8_t *dstData = (uint8_t *)rect.pBits;
        uint8_t *srcData = (uint8_t *)data;
        size_t pixelCount = width * height;

#if defined(_M_IX86) || defined(_M_X64)
        {
          size_t simdPixels = pixelCount & ~3;
          __m128i shuffle_mask = _mm_setr_epi8(2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8,
                                               11, 14, 13, 12, 15);

          for (size_t i = 0; i < simdPixels; i += 4) {
            __m128i pixels = _mm_loadu_si128((__m128i *)(srcData + i * 4));
            pixels = _mm_shuffle_epi8(pixels, shuffle_mask);
            _mm_storeu_si128((__m128i *)(dstData + i * 4), pixels);
          }

          for (size_t i = simdPixels; i < pixelCount; ++i) {
            size_t idx = i * 4;
            dstData[idx + 0] = srcData[idx + 2];
            dstData[idx + 1] = srcData[idx + 1];
            dstData[idx + 2] = srcData[idx + 0];
            dstData[idx + 3] = srcData[idx + 3];
          }
        }
#else
        for (size_t i = 0; i < pixelCount; ++i) {
          size_t idx = i * 4;
          dstData[idx + 0] = srcData[idx + 2];
          dstData[idx + 1] = srcData[idx + 1];
          dstData[idx + 2] = srcData[idx + 0];
          dstData[idx + 3] = srcData[idx + 3];
        }
#endif

        texture->UnlockRect(0);
        m_width = width;
        m_height = height;
        m_bEmpty = false;
        m_lpd3dTexture = texture;

        // DX11: Create SRV from BGRA data (dstData is already swizzled)
        if (ms_pD3D11Device)
          DX11Tex::CreateTextureFromRGBA(ms_pD3D11Device, width, height,
                                         (const uint8_t *)rect.pBits, true,
                                         &m_pDX11SRV);

        // Register in the DX9→DX11 texture registry
        if (m_lpd3dTexture && m_pDX11SRV)
          RegisterDX11SRV(m_lpd3dTexture, m_pDX11SRV);
      } else {
        texture->Release();
      }
    }
    stbi_image_free(data);
  }

  return !m_bEmpty;
}

bool CGraphicImageTexture::CreateFromMemoryFile(UINT bufSize,
                                                const void *c_pvBuf,
                                                D3DFORMAT d3dFmt,
                                                DWORD dwFilter) {
  assert(ms_lpd3dDevice != NULL);
  assert(m_lpd3dTexture == NULL);

  m_bEmpty = true;

  if (!CreateFromDDSTexture(bufSize, c_pvBuf)) {
    if (!CreateFromSTB(bufSize, c_pvBuf)) {

      D3DXIMAGE_INFO imageInfo;
      if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
              ms_lpd3dDevice, c_pvBuf, bufSize, D3DX_DEFAULT_NONPOW2,
              D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT, 0, d3dFmt, D3DPOOL_DEFAULT,
              dwFilter, dwFilter, 0xffff00ff, &imageInfo, NULL,
              &m_lpd3dTexture))) {
        TraceError("CreateFromMemoryFile: Cannot create texture (%s, %u bytes)",
                   m_stFileName.c_str(), bufSize);
        return false;
      }

      m_width = imageInfo.Width;
      m_height = imageInfo.Height;

      D3DFORMAT format = imageInfo.Format;
      switch (imageInfo.Format) {
      case D3DFMT_A8R8G8B8:
        format = D3DFMT_A4R4G4B4;
        break;

      case D3DFMT_X8R8G8B8:
      case D3DFMT_R8G8B8:
        format = D3DFMT_A1R5G5B5;
        break;
      }

      UINT uTexBias = 0;

      extern bool GRAPHICS_CAPS_HALF_SIZE_IMAGE;
      if (GRAPHICS_CAPS_HALF_SIZE_IMAGE)
        uTexBias = 1;

      if (IsLowTextureMemory()) {
        if (uTexBias || format != imageInfo.Format) {
          IDirect3DTexture9 *pkTexSrc = m_lpd3dTexture;
          IDirect3DTexture9 *pkTexDst;

          if (SUCCEEDED(D3DXCreateTexture(
                  ms_lpd3dDevice, imageInfo.Width >> uTexBias,
                  imageInfo.Height >> uTexBias, imageInfo.MipLevels, 0, format,
                  D3DPOOL_DEFAULT, &pkTexDst))) {
            m_lpd3dTexture = pkTexDst;
            for (int i = 0; i < imageInfo.MipLevels; ++i) {

              IDirect3DSurface9 *ppsSrc = NULL;
              IDirect3DSurface9 *ppsDst = NULL;

              if (SUCCEEDED(pkTexSrc->GetSurfaceLevel(i, &ppsSrc))) {
                if (SUCCEEDED(pkTexDst->GetSurfaceLevel(i, &ppsDst))) {
                  D3DXLoadSurfaceFromSurface(ppsDst, NULL, NULL, ppsSrc, NULL,
                                             NULL, D3DX_FILTER_LINEAR, 0);
                  ppsDst->Release();
                }
                ppsSrc->Release();
              }
            }

            pkTexSrc->Release();
          }
        }
      }
    }
  }

  m_bEmpty = false;
  return true;
}

void CGraphicImageTexture::SetFileName(const char *c_szFileName) {
  m_stFileName = c_szFileName;
}

bool CGraphicImageTexture::CreateFromDiskFile(const char *c_szFileName,
                                              D3DFORMAT d3dFmt,
                                              DWORD dwFilter) {
  Destroy();

  SetFileName(c_szFileName);

  m_d3dFmt = d3dFmt;
  m_dwFilter = dwFilter;
  return CreateDeviceObjects();
}

bool CGraphicImageTexture::CreateFromDecodedData(
    const TDecodedImageData &decodedImage, D3DFORMAT d3dFmt, DWORD dwFilter) {
  assert(ms_lpd3dDevice != NULL);
  assert(m_lpd3dTexture == NULL);

  if (!decodedImage.IsValid())
    return false;

  m_bEmpty = true;

  if (decodedImage.isDDS) {
    // DDS format - use DirectX loader
    if (!CreateFromDDSTexture(decodedImage.pixels.size(),
                              decodedImage.pixels.data()))
      return false;
  } else if (decodedImage.format == TDecodedImageData::FORMAT_RGBA8) {
    LPDIRECT3DTEXTURE9 texture;
    D3DFORMAT format = D3DFMT_A8R8G8B8;

    if (FAILED(ms_lpd3dDevice->CreateTexture(
            decodedImage.width, decodedImage.height, 1, 0, format,
            D3DPOOL_MANAGED, &texture, nullptr))) {
      return false;
    }

    D3DLOCKED_RECT rect;
    if (SUCCEEDED(texture->LockRect(0, &rect, nullptr, 0))) {
      uint8_t *dstData = (uint8_t *)rect.pBits;
      const uint8_t *srcData = decodedImage.pixels.data();
      size_t pixelCount = decodedImage.width * decodedImage.height;

#if defined(_M_IX86) || defined(_M_X64)
      {
        size_t simdPixels = pixelCount & ~3;
        __m128i shuffle_mask =
            _mm_setr_epi8(2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15);

        for (size_t i = 0; i < simdPixels; i += 4) {
          __m128i pixels = _mm_loadu_si128((__m128i *)(srcData + i * 4));
          pixels = _mm_shuffle_epi8(pixels, shuffle_mask);
          _mm_storeu_si128((__m128i *)(dstData + i * 4), pixels);
        }

        for (size_t i = simdPixels; i < pixelCount; ++i) {
          size_t idx = i * 4;
          dstData[idx + 0] = srcData[idx + 2];
          dstData[idx + 1] = srcData[idx + 1];
          dstData[idx + 2] = srcData[idx + 0];
          dstData[idx + 3] = srcData[idx + 3];
        }
      }
#else
      for (size_t i = 0; i < pixelCount; ++i) {
        size_t idx = i * 4;
        dstData[idx + 0] = srcData[idx + 2];
        dstData[idx + 1] = srcData[idx + 1];
        dstData[idx + 2] = srcData[idx + 0];
        dstData[idx + 3] = srcData[idx + 3];
      }
#endif

      texture->UnlockRect(0);

      m_width = decodedImage.width;
      m_height = decodedImage.height;
      m_lpd3dTexture = texture;
      m_bEmpty = false;
    } else {
      texture->Release();
      return false;
    }
  } else {
    TraceError("CreateFromDecodedData: Unsupported decoded image format");
    return false;
  }

  return !m_bEmpty;
}

CGraphicImageTexture::CGraphicImageTexture() { Initialize(); }

CGraphicImageTexture::~CGraphicImageTexture() { Destroy(); }
