#ifndef __INC_ETERLIB_IMAGEDECODER_H__
#define __INC_ETERLIB_IMAGEDECODER_H__

#include "DecodedImageData.h"

// Image decoder for worker threads
class CImageDecoder {
public:
  // Decode image from memory (DDS, PNG, JPG, TGA, BMP)
  static bool DecodeImage(const void *pData, size_t dataSize,
                          TDecodedImageData &outImage);

private:
  static bool DecodeDDS(const void *pData, size_t dataSize,
                        TDecodedImageData &outImage);
  static bool DecodeSTB(const void *pData, size_t dataSize,
                        TDecodedImageData &outImage);
};

#endif // __INC_ETERLIB_IMAGEDECODER_H__
