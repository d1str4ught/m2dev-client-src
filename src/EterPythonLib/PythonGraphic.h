#pragma once

#include "EterLib/GrpTextInstance.h"
#include "EterLib/GrpMarkInstance.h"
#include "EterLib/GrpImageInstance.h"
#include "EterLib/GrpExpandedImageInstance.h"

#include "EterGrnLib/ThingInstance.h"

class CPythonGraphic : public CScreen, public CSingleton<CPythonGraphic> {
public:
  CPythonGraphic();
  virtual ~CPythonGraphic();

  void Destroy();

  void PushState();
  void PopState();

  LPDIRECT3D9EX GetD3D();

  float GetOrthoDepth();
  void SetInterfaceRenderState();
  void SetGameRenderState();

  void SetCursorPosition(int x, int y);

  void SetOmniLight();

  void SetViewport(float fx, float fy, float fWidth, float fHeight);
  void RestoreViewport();

  long GenerateColor(float r, float g, float b, float a);
  void RenderDownButton(float sx, float sy, float ex, float ey);
  void RenderUpButton(float sx, float sy, float ex, float ey);

  void RenderImage(CGraphicImageInstance *pImageInstance, float x, float y);
  void RenderAlphaImage(CGraphicImageInstance *pImageInstance, float x, float y,
                        float aLeft, float aRight);
  void RenderCoolTimeBox(float fxCenter, float fyCenter, float fRadius,
                         float fTime);

  bool SaveJPEG(const char *pszFileName, LPBYTE pbyBuffer, UINT uWidth,
                UINT uHeight);
  bool SaveScreenShot(const char *szFileName);

  DWORD GetAvailableMemory();
  void SetGamma(float fGammaFactor = 1.0f);

protected:
  typedef struct SState {
    D3DXMATRIX matView;
    D3DXMATRIX matProj;
  } TState;

  DWORD m_lightColor;
  DWORD m_darkColor;

protected:
  std::stack<TState> m_stateStack;

  D3DXMATRIX m_SaveWorldMatrix;

  CCullingManager m_CullingManager;

  D3DVIEWPORT9 m_backupViewport;

  float m_fOrthoDepth;
};