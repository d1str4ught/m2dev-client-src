#pragma once

#include "GrpImage.h"
#include "Pool.h"

class CGraphicMarkInstance {
public:
  static DWORD Type();
  BOOL IsType(DWORD dwType);

  void SetImageFileName(const char *c_szFileName);
  const std::string &GetImageFileName();

public:
  CGraphicMarkInstance();
  virtual ~CGraphicMarkInstance();

  void Destroy();

  void Render();

  void SetDepth(float fDepth);
  void SetDiffuseColor(float fr, float fg, float fb, float fa);
  void SetPosition(float fx, float fy);
  void SetIndex(UINT uIndex);
  void SetScale(float fScale);

  void Load();
  bool IsEmpty() const;

  int GetWidth();
  int GetHeight();

  CGraphicTexture *GetTexturePointer();
  const CGraphicTexture &GetTextureReference() const;
  CGraphicImage *GetGraphicImagePointer();

  bool operator==(const CGraphicMarkInstance &rhs) const;

protected:
  enum {
    MARK_WIDTH = 16,
    MARK_HEIGHT = 12,
  };

  void Initialize();

  virtual void OnRender();
  virtual void OnSetImagePointer();

  virtual BOOL OnIsType(DWORD dwType);

  void SetImagePointer(CGraphicImage *pImage);

protected:
  D3DXCOLOR m_DiffuseColor;
  D3DXVECTOR2 m_v2Position;

  UINT m_uIndex;

  FLOAT m_fScale;
  FLOAT m_fDepth;

  CGraphicImage::TRef m_roImage;
  std::string m_stImageFileName;

public:
  static void CreateSystem(UINT uCapacity);
  static void DestroySystem();

  static CGraphicMarkInstance *New();
  static void Delete(CGraphicMarkInstance *pkImgInst);

  static CDynamicPool<CGraphicMarkInstance> ms_kPool;
};
