#ifndef __INC_ETERLIB_DECODEDIMAGEDATA_H__
#define __INC_ETERLIB_DECODEDIMAGEDATA_H__

#include <vector>
#include <cstdint>
#include <d3d9.h>

// Decoded image data for GPU upload
struct TDecodedImageData {
  enum EFormat {
    FORMAT_UNKNOWN = 0,
    FORMAT_RGBA8,
    FORMAT_RGB8,
    FORMAT_DDS,
  };

  std::vector<uint8_t> pixels;
  int width;
  int height;
  EFormat format;
  D3DFORMAT d3dFormat;
  bool isDDS;
  int mipLevels;

  TDecodedImageData()
      : width(0), height(0), format(FORMAT_UNKNOWN), d3dFormat(D3DFMT_UNKNOWN),
        isDDS(false), mipLevels(1) {}

  void Clear() {
    pixels.clear();
    width = 0;
    height = 0;
    format = FORMAT_UNKNOWN;
    d3dFormat = D3DFMT_UNKNOWN;
    isDDS = false;
    mipLevels = 1;
  }

  bool IsValid() const { return width > 0 && height > 0 && !pixels.empty(); }

  size_t GetDataSize() const { return pixels.size(); }
};

#endif // __INC_ETERLIB_DECODEDIMAGEDATA_H__
