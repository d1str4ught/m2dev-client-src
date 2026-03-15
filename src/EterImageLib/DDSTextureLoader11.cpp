//--------------------------------------------------------------------------------------
// DDSTextureLoader11.cpp — Lean DX11 DDS texture loader
// Creates ID3D11Texture2D + ID3D11ShaderResourceView from DDS memory
// Based on DirectXTK DDSTextureLoader (MIT License, Microsoft Corporation)
//--------------------------------------------------------------------------------------

#include "DDSTextureLoader11.h"
#include <algorithm>
#include <cstring>


#pragma pack(push, 1)

constexpr uint32_t DDS_MAGIC = 0x20534444; // "DDS "

#define DDS_FOURCC 0x00000004
#define DDS_RGB 0x00000040
#define DDS_LUMINANCE 0x00020000
#define DDS_ALPHA 0x00000002
#define DDS_BUMPDUDV 0x00080000

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                         \
  ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |                \
   ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif

struct DDS_PIXELFORMAT {
  uint32_t size;
  uint32_t flags;
  uint32_t fourCC;
  uint32_t RGBBitCount;
  uint32_t RBitMask;
  uint32_t GBitMask;
  uint32_t BBitMask;
  uint32_t ABitMask;
};

struct DDS_HEADER {
  uint32_t size;
  uint32_t flags;
  uint32_t height;
  uint32_t width;
  uint32_t pitchOrLinearSize;
  uint32_t depth;
  uint32_t mipMapCount;
  uint32_t reserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t caps;
  uint32_t caps2;
  uint32_t caps3;
  uint32_t caps4;
  uint32_t reserved2;
};

#pragma pack(pop)

#define ISBITMASK(r, g, b, a)                                                  \
  (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b &&           \
   ddpf.ABitMask == a)

static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT &ddpf) {
  if (ddpf.flags & DDS_RGB) {
    switch (ddpf.RGBBitCount) {
    case 32:
      if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
        return DXGI_FORMAT_R8G8B8A8_UNORM;
      if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
        return DXGI_FORMAT_B8G8R8A8_UNORM;
      if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
        return DXGI_FORMAT_B8G8R8X8_UNORM;
      if (ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000))
        return DXGI_FORMAT_R10G10B10A2_UNORM;
      if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
        return DXGI_FORMAT_R16G16_UNORM;
      if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
        return DXGI_FORMAT_R32_FLOAT; // D3DX writes this as D3DFMT_R32F
      break;

    case 24:
      // No 24bpp DXGI format — will need conversion
      break;

    case 16:
      if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
        return DXGI_FORMAT_B5G5R5A1_UNORM;
      if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
        return DXGI_FORMAT_B5G6R5_UNORM;
      if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
        return DXGI_FORMAT_B4G4R4A4_UNORM;
      if (ISBITMASK(0x00ff, 0x0000, 0x0000, 0xff00))
        return DXGI_FORMAT_R8G8_UNORM;
      if (ISBITMASK(0xffff, 0x0000, 0x0000, 0x0000))
        return DXGI_FORMAT_R16_UNORM;
      break;

    case 8:
      if (ISBITMASK(0xff, 0x00, 0x00, 0x00))
        return DXGI_FORMAT_R8_UNORM;
      break;
    }
  } else if (ddpf.flags & DDS_LUMINANCE) {
    if (ddpf.RGBBitCount == 16 && ISBITMASK(0x00ff, 0x0000, 0x0000, 0xff00))
      return DXGI_FORMAT_R8G8_UNORM;
    if (ddpf.RGBBitCount == 8 && ISBITMASK(0xff, 0x00, 0x00, 0x00))
      return DXGI_FORMAT_R8_UNORM;
  } else if (ddpf.flags & DDS_ALPHA) {
    if (ddpf.RGBBitCount == 8)
      return DXGI_FORMAT_A8_UNORM;
  } else if (ddpf.flags & DDS_FOURCC) {
    if (ddpf.fourCC == MAKEFOURCC('D', 'X', 'T', '1'))
      return DXGI_FORMAT_BC1_UNORM;
    if (ddpf.fourCC == MAKEFOURCC('D', 'X', 'T', '2') ||
        ddpf.fourCC == MAKEFOURCC('D', 'X', 'T', '3'))
      return DXGI_FORMAT_BC2_UNORM;
    if (ddpf.fourCC == MAKEFOURCC('D', 'X', 'T', '4') ||
        ddpf.fourCC == MAKEFOURCC('D', 'X', 'T', '5'))
      return DXGI_FORMAT_BC3_UNORM;
    if (ddpf.fourCC == MAKEFOURCC('A', 'T', 'I', '1') ||
        ddpf.fourCC == MAKEFOURCC('B', 'C', '4', 'U'))
      return DXGI_FORMAT_BC4_UNORM;
    if (ddpf.fourCC == MAKEFOURCC('B', 'C', '4', 'S'))
      return DXGI_FORMAT_BC4_SNORM;
    if (ddpf.fourCC == MAKEFOURCC('A', 'T', 'I', '2') ||
        ddpf.fourCC == MAKEFOURCC('B', 'C', '5', 'U'))
      return DXGI_FORMAT_BC5_UNORM;
    if (ddpf.fourCC == MAKEFOURCC('B', 'C', '5', 'S'))
      return DXGI_FORMAT_BC5_SNORM;

    // D3DFMT_R16G16B16A16
    if (ddpf.fourCC == 36)
      return DXGI_FORMAT_R16G16B16A16_UNORM;
    // D3DFMT_Q16W16V16U16
    if (ddpf.fourCC == 110)
      return DXGI_FORMAT_R16G16B16A16_SNORM;
    // D3DFMT_R16F
    if (ddpf.fourCC == 111)
      return DXGI_FORMAT_R16_FLOAT;
    // D3DFMT_G16R16F
    if (ddpf.fourCC == 112)
      return DXGI_FORMAT_R16G16_FLOAT;
    // D3DFMT_A16B16G16R16F
    if (ddpf.fourCC == 113)
      return DXGI_FORMAT_R16G16B16A16_FLOAT;
    // D3DFMT_R32F
    if (ddpf.fourCC == 114)
      return DXGI_FORMAT_R32_FLOAT;
    // D3DFMT_G32R32F
    if (ddpf.fourCC == 115)
      return DXGI_FORMAT_R32G32_FLOAT;
    // D3DFMT_A32B32G32R32F
    if (ddpf.fourCC == 116)
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
  } else if (ddpf.flags & DDS_BUMPDUDV) {
    if (ddpf.RGBBitCount == 32 &&
        ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
      return DXGI_FORMAT_R8G8B8A8_SNORM;
    if (ddpf.RGBBitCount == 16 && ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
      return DXGI_FORMAT_R8G8_SNORM;
  }

  return DXGI_FORMAT_UNKNOWN;
}

#undef ISBITMASK

static size_t BitsPerPixel(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_R32G32B32A32_FLOAT:
    return 128;
  case DXGI_FORMAT_R16G16B16A16_FLOAT:
  case DXGI_FORMAT_R16G16B16A16_UNORM:
  case DXGI_FORMAT_R16G16B16A16_SNORM:
    return 64;
  case DXGI_FORMAT_R32G32_FLOAT:
    return 64;
  case DXGI_FORMAT_R10G10B10A2_UNORM:
    return 32;
  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_R8G8B8A8_SNORM:
  case DXGI_FORMAT_B8G8R8A8_UNORM:
  case DXGI_FORMAT_B8G8R8X8_UNORM:
  case DXGI_FORMAT_R32_FLOAT:
    return 32;
  case DXGI_FORMAT_R16G16_UNORM:
  case DXGI_FORMAT_R16G16_FLOAT:
  case DXGI_FORMAT_B5G5R5A1_UNORM:
  case DXGI_FORMAT_B5G6R5_UNORM:
  case DXGI_FORMAT_B4G4R4A4_UNORM:
  case DXGI_FORMAT_R16_UNORM:
  case DXGI_FORMAT_R16_FLOAT:
  case DXGI_FORMAT_R8G8_UNORM:
  case DXGI_FORMAT_R8G8_SNORM:
    return 16;
  case DXGI_FORMAT_R8_UNORM:
  case DXGI_FORMAT_A8_UNORM:
    return 8;
  case DXGI_FORMAT_BC1_UNORM:
    return 4;
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
    return 8; // per 4x4 block
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
    return 16; // per 4x4 block (wait, BC5 is 128 bits per block = 8bpp)
  default:
    return 0;
  }
}

static bool IsCompressed(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_BC1_UNORM:
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
    return true;
  default:
    return false;
  }
}

// Get row pitch and slice pitch for a given format
static void GetSurfaceInfo(size_t width, size_t height, DXGI_FORMAT fmt,
                           size_t *outRowPitch, size_t *outSlicePitch) {
  size_t rowPitch = 0;
  size_t slicePitch = 0;

  if (IsCompressed(fmt)) {
    size_t bw = (std::max)((size_t)1, (width + 3) / 4);
    size_t bh = (std::max)((size_t)1, (height + 3) / 4);
    size_t blockSize =
        (fmt == DXGI_FORMAT_BC1_UNORM || fmt == DXGI_FORMAT_BC4_UNORM ||
         fmt == DXGI_FORMAT_BC4_SNORM)
            ? 8
            : 16;
    rowPitch = bw * blockSize;
    slicePitch = rowPitch * bh;
  } else {
    size_t bpp = BitsPerPixel(fmt);
    rowPitch = (width * bpp + 7) / 8;
    slicePitch = rowPitch * height;
  }

  if (outRowPitch)
    *outRowPitch = rowPitch;
  if (outSlicePitch)
    *outSlicePitch = slicePitch;
}

// ============================================================================
// Public API
// ============================================================================

HRESULT DX11Tex::CreateDDSTextureFromMemory(ID3D11Device *device,
                                            const uint8_t *data,
                                            size_t dataSize,
                                            ID3D11ShaderResourceView **srv,
                                            ID3D11Texture2D **texture) {
  if (!device || !data || dataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER)) ||
      !srv)
    return E_INVALIDARG;

  *srv = nullptr;
  if (texture)
    *texture = nullptr;

  // Validate DDS magic
  if (*reinterpret_cast<const uint32_t *>(data) != DDS_MAGIC)
    return E_FAIL;

  const DDS_HEADER *header =
      reinterpret_cast<const DDS_HEADER *>(data + sizeof(uint32_t));
  if (header->size != sizeof(DDS_HEADER) ||
      header->ddspf.size != sizeof(DDS_PIXELFORMAT))
    return E_FAIL;

  const uint8_t *bitData = data + sizeof(uint32_t) + sizeof(DDS_HEADER);
  size_t bitSize = dataSize - sizeof(uint32_t) - sizeof(DDS_HEADER);

  DXGI_FORMAT format = GetDXGIFormat(header->ddspf);
  if (format == DXGI_FORMAT_UNKNOWN)
    return E_FAIL; // Unsupported format

  UINT width = header->width;
  UINT height = header->height;
  UINT mipCount = (std::max)(1u, header->mipMapCount);

  // Create texture
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = mipCount;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  // Fill subresource data for each mip level
  D3D11_SUBRESOURCE_DATA *initData = new D3D11_SUBRESOURCE_DATA[mipCount];
  const uint8_t *pSrcBits = bitData;

  for (UINT i = 0; i < mipCount; i++) {
    size_t mipWidth = (std::max)((UINT)1, width >> i);
    size_t mipHeight = (std::max)((UINT)1, height >> i);

    size_t rowPitch, slicePitch;
    GetSurfaceInfo(mipWidth, mipHeight, format, &rowPitch, &slicePitch);

    initData[i].pSysMem = pSrcBits;
    initData[i].SysMemPitch = (UINT)rowPitch;
    initData[i].SysMemSlicePitch = (UINT)slicePitch;

    pSrcBits += slicePitch;

    if (pSrcBits > data + dataSize) {
      delete[] initData;
      return E_FAIL; // Data too small
    }
  }

  ID3D11Texture2D *tex = nullptr;
  HRESULT hr = device->CreateTexture2D(&desc, initData, &tex);
  delete[] initData;

  if (FAILED(hr))
    return hr;

  // Create SRV
  hr = device->CreateShaderResourceView(tex, nullptr, srv);
  if (FAILED(hr)) {
    tex->Release();
    return hr;
  }

  if (texture)
    *texture = tex;
  else
    tex->Release();

  return S_OK;
}

HRESULT DX11Tex::CreateTextureFromRGBA(ID3D11Device *device, UINT width,
                                       UINT height, const uint8_t *rgbaData,
                                       bool isBGRA,
                                       ID3D11ShaderResourceView **srv,
                                       ID3D11Texture2D **texture) {
  if (!device || !rgbaData || !srv || width == 0 || height == 0)
    return E_INVALIDARG;

  *srv = nullptr;
  if (texture)
    *texture = nullptr;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format =
      isBGRA ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = rgbaData;
  initData.SysMemPitch = width * 4;

  ID3D11Texture2D *tex = nullptr;
  HRESULT hr = device->CreateTexture2D(&desc, &initData, &tex);
  if (FAILED(hr))
    return hr;

  hr = device->CreateShaderResourceView(tex, nullptr, srv);
  if (FAILED(hr)) {
    tex->Release();
    return hr;
  }

  if (texture)
    *texture = tex;
  else
    tex->Release();

  return S_OK;
}

HRESULT DX11Tex::CreateDynamicTexture(ID3D11Device *device, UINT width,
                                      UINT height, DXGI_FORMAT format,
                                      ID3D11ShaderResourceView **srv,
                                      ID3D11Texture2D **texture) {
  if (!device || !srv || width == 0 || height == 0)
    return E_INVALIDARG;

  *srv = nullptr;
  if (texture)
    *texture = nullptr;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  ID3D11Texture2D *tex = nullptr;
  HRESULT hr = device->CreateTexture2D(&desc, nullptr, &tex);
  if (FAILED(hr))
    return hr;

  hr = device->CreateShaderResourceView(tex, nullptr, srv);
  if (FAILED(hr)) {
    tex->Release();
    return hr;
  }

  if (texture)
    *texture = tex;
  else
    tex->Release();

  return S_OK;
}
