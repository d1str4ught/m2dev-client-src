#ifndef __INC_ETERIMAGELIB_IMAGE_H__
#define __INC_ETERIMAGELIB_IMAGE_H__

#include <windows.h>
#include <string>

#pragma pack(push)
#pragma pack(1)
struct TGA_HEADER {
  char idLen;        // 0
  char palType;      // 파레트있으면 1, 없음 0
  char imgType;      // 파레트있으면 1, 없음 2
  WORD colorBegin;   // 0
  WORD colorCount;   // 파레트 있으면 256, 없음 0
  char palEntrySize; // 파레트 있으면 24, 없음 0
  WORD left;
  WORD top;
  WORD width;
  WORD height;
  char colorBits;
  char desc;
};
#define IMAGEDESC_ORIGIN_MASK 0x30
#define IMAGEDESC_TOPLEFT 0x20
#define IMAGEDESC_BOTLEFT 0x00
#define IMAGEDESC_BOTRIGHT 0x10
#define IMAGEDESC_TOPRIGHT 0x30
#pragma pack(pop)

class CImage {
public:
  CImage();
  CImage(CImage &image);

  virtual ~CImage();

  void Destroy();

  void Create(int width, int height);

  void Clear(DWORD color = 0);

  int GetWidth() const;
  int GetHeight() const;

  DWORD *GetBasePointer();
  DWORD *GetLinePointer(int line);

  void PutImage(int x, int y, CImage *pImage);
  void FlipTopToBottom();

  void SetFileName(const char *c_szFileName);

  const std::string &GetFileNameString();

  bool IsEmpty() const;

protected:
  void Initialize();

protected:
  DWORD *m_pdwColors;
  int m_width;
  int m_height;

  std::string m_stFileName;
};

#endif
