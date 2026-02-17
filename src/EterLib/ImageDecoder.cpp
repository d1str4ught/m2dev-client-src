#include "StdAfx.h"
#include "ImageDecoder.h"
#include "EterImageLib/DDSTextureLoader9.h"
#include <stb_image.h>

bool CImageDecoder::DecodeImage(const void *pData, size_t dataSize,
                                TDecodedImageData &outImage) {
  if (!pData || dataSize == 0)
    return false;

  outImage.Clear();

  if (DecodeDDS(pData, dataSize, outImage))
    return true;

  if (DecodeSTB(pData, dataSize, outImage))
    return true;

  return false;
}

bool CImageDecoder::DecodeDDS(const void *pData, size_t dataSize,
                              TDecodedImageData &outImage) {
  if (dataSize < 4)
    return false;

  const uint32_t DDS_MAGIC = 0x20534444;
  uint32_t magic = *(const uint32_t *)pData;

  if (magic != DDS_MAGIC)
    return false;

  if (dataSize < 128)
    return false;

  struct DDSHeader {
    uint32_t magic;
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
  };

  const DDSHeader *header = (const DDSHeader *)pData;

  outImage.width = header->width;
  outImage.height = header->height;
  outImage.mipLevels = (header->mipMapCount > 0) ? header->mipMapCount : 1;
  outImage.isDDS = true;
  outImage.format = TDecodedImageData::FORMAT_DDS;

  outImage.pixels.resize(dataSize);
  memcpy(outImage.pixels.data(), pData, dataSize);

  return true;
}

bool CImageDecoder::DecodeSTB(const void *pData, size_t dataSize,
                              TDecodedImageData &outImage) {
  int width, height, channels;

  unsigned char *imageData = stbi_load_from_memory(
      (const stbi_uc *)pData, (int)dataSize, &width, &height, &channels, 4);

  if (!imageData)
    return false;

  outImage.width = width;
  outImage.height = height;
  outImage.format = TDecodedImageData::FORMAT_RGBA8;
  outImage.isDDS = false;
  outImage.mipLevels = 1;

  size_t pixelDataSize = width * height * 4;
  outImage.pixels.resize(pixelDataSize);
  memcpy(outImage.pixels.data(), imageData, pixelDataSize);

  stbi_image_free(imageData);

  return true;
}
