#pragma once

#include "GrpCollisionObject.h"
#include "SphereLib/frustum.h"

class CScreen : public CGraphicCollisionObject {
public:
  CScreen();
  virtual ~CScreen();

  void ClearDepthBuffer();
  void Clear();
  bool Begin();
  void End();
  void Show(HWND hWnd = NULL);
  void Show(RECT *pSrcRect);
  void Show(RECT *pSrcRect, HWND hWnd);

  void RenderLine2d(float sx, float sy, float ex, float ey, float z = 0.0f);
  void RenderBox2d(float sx, float sy, float ex, float ey, float z = 0.0f);
  void RenderBar2d(float sx, float sy, float ex, float ey, float z = 0.0f);
  void RenderGradationBar2d(float sx, float sy, float ex, float ey,
                            DWORD dwStartColor, DWORD dwEndColor,
                            float ez = 0.0f);
  void RenderCircle2d(float fx, float fy, float fz, float fRadius,
                      int iStep = 50);
  void RenderCircle3d(float fx, float fy, float fz, float fRadius,
                      int iStep = 50);

  void RenderLine3d(float sx, float sy, float sz, float ex, float ey, float ez);
  void RenderBox3d(float sx, float sy, float sz, float ex, float ey, float ez);
  void RenderBar3d(float sx, float sy, float sz, float ex, float ey, float ez);
  void RenderBar3d(const D3DXVECTOR3 *c_pv3Positions);
  void RenderGradationBar3d(float sx, float sy, float sz, float ex, float ey,
                            float ez, DWORD dwStartColor, DWORD dwEndColor);

  void RenderLineCube(float sx, float sy, float sz, float ex, float ey,
                      float ez);
  void RenderCube(float sx, float sy, float sz, float ex, float ey, float ez);
  void RenderCube(float sx, float sy, float sz, float ex, float ey, float ez,
                  D3DXMATRIX matRotation);
  void RenderTextureBox(float sx, float sy, float ex, float ey, float z = 0.0f,
                        float su = 0.0f, float sv = 0.0f, float eu = 1.0f,
                        float ev = 1.0f);
  void RenderBillboard(D3DXVECTOR3 *Position, D3DXCOLOR &Color);

  void DrawMinorGrid(float xMin, float yMin, float xMax, float yMax,
                     float xminorStep, float yminorStep, float zPos = 0);
  void DrawGrid(float xMin, float yMin, float xMax, float yMax,
                float xmajorStep, float ymajorStep, float xminorStep,
                float yminorStep, float zPos = 0);

  void RenderD3DXMesh(LPD3DXMESH lpMesh, const D3DXMATRIX *c_pmatWorld,
                      float fx, float fy, float fz, float fRadius,
                      D3DFILLMODE d3dFillMode);
  void RenderSphere(const D3DXMATRIX *c_pmatWorld, float fx, float fy, float fz,
                    float fRadius, D3DFILLMODE d3dFillMode = D3DFILL_SOLID);
  void RenderCylinder(const D3DXMATRIX *c_pmatWorld, float fx, float fy,
                      float fz, float fRadius, float fLength,
                      D3DFILLMODE d3dFillMode = D3DFILL_SOLID);

  void SetColorOperation();
  void SetDiffuseOperation();
  void SetBlendOperation();
  void SetOneColorOperation(D3DXCOLOR &rColor);
  void SetAddColorOperation(D3DXCOLOR &rColor);
  void SetDiffuseColor(DWORD diffuseColor);
  void SetDiffuseColor(float r, float g, float b, float a = 1.0f);
  void SetClearColor(float r, float g, float b, float a = 1.0f);
  void SetClearDepth(float depth);
  void SetClearStencil(DWORD stencil);

  void SetCursorPosition(int x, int y, int hres,
                         int vres); // creates picking ray
  bool GetCursorPosition(float *px, float *py, float *pz);
  bool GetCursorXYPosition(float *px, float *py);
  bool GetCursorZPosition(float *pz);
  void GetPickingPosition(float t, float *x, float *y, float *z);
  void ProjectPosition(float x, float y, float z, float *pfX, float *pfY);
  void ProjectPosition(float x, float y, float z, float *pfX, float *pfY,
                       float *pfZ);
  void UnprojectPosition(float x, float y, float z, float *pfX, float *pfY,
                         float *pfZ);

  BOOL IsLostDevice();
  BOOL RestoreDevice();

  void BuildViewFrustum();

  static void Identity();
  static Frustum &GetFrustum() { return ms_frustum; }

protected:
  static DWORD ms_diffuseColor;
  static DWORD ms_clearColor;
  static DWORD ms_clearStencil;
  static float ms_clearDepth;

  static Frustum ms_frustum;
};
