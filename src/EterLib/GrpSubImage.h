#pragma once

#include "GrpImage.h"

class CGraphicSubImage : public CGraphicImage {
public:
  typedef CRef<CGraphicImage> TRef;

public:
  static TType Type();
  static char m_SearchPath[256];

public:
  CGraphicSubImage(const char *c_szFileName);
  virtual ~CGraphicSubImage();

  bool CreateDeviceObjects();

  bool SetImageFileName(const char *c_szFileName);

  void SetRectPosition(int left, int top, int right, int bottom);

  void SetRectReference(const RECT &c_rRect);

  static void SetSearchPath(const char *c_szFileName);

protected:
  void SetImagePointer(CGraphicImage *pImage);

  bool OnLoad(int iSize, const void *c_pvBuf);
  void OnClear();
  bool OnIsEmpty() const;
  bool OnIsType(TType type);

protected:
  CGraphicImage::TRef m_roImage;
};
