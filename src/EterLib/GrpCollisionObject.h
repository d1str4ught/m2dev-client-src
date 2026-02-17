#pragma once

#include "GrpBase.h"

class CGraphicCollisionObject : public CGraphicBase {
public:
  CGraphicCollisionObject();
  virtual ~CGraphicCollisionObject();

protected:
  bool IntersectTriangle(const D3DXVECTOR3 &c_orig, const D3DXVECTOR3 &c_dir,
                         const D3DXVECTOR3 &c_v0, const D3DXVECTOR3 &c_v1,
                         const D3DXVECTOR3 &c_v2, float *pu, float *pv,
                         float *pt);
  bool IntersectBoundBox(const D3DXMATRIX *c_pmatWorld,
                         const TBoundBox &c_rboundBox, float *pu, float *pv,
                         float *pt);
  bool IntersectCube(const D3DXMATRIX *c_pmatWorld, float sx, float sy,
                     float sz, float ex, float ey, float ez,
                     D3DXVECTOR3 &RayOriginal, D3DXVECTOR3 &RayDirection,
                     float *pu, float *pv, float *pt);
  bool IntersectIndexedMesh(const D3DXMATRIX *c_pmatWorld, const void *vertices,
                            int step, int vtxCount, const void *indices,
                            int idxCount, D3DXVECTOR3 &RayOriginal,
                            D3DXVECTOR3 &RayDirection, float *pu, float *pv,
                            float *pt);
  bool IntersectMesh(const D3DXMATRIX *c_pmatWorld, const void *vertices,
                     DWORD dwStep, DWORD dwvtxCount, D3DXVECTOR3 &RayOriginal,
                     D3DXVECTOR3 &RayDirection, float *pu, float *pv,
                     float *pt);

  bool IntersectSphere(const D3DXVECTOR3 &c_rv3Position, float fRadius,
                       const D3DXVECTOR3 &c_rv3RayOriginal,
                       const D3DXVECTOR3 &c_rv3RayDirection);
  bool IntersectCylinder(const D3DXVECTOR3 &c_rv3Position, float fRadius,
                         float fHeight, const D3DXVECTOR3 &c_rv3RayOriginal,
                         const D3DXVECTOR3 &c_rv3RayDirection);

  // NOTE : ms_vtPickRayOrig와 ms_vtPickRayDir를 CGraphicBGase가 가지고 있는데
  //        굳이 인자로 넣어줘야 하는 이유가 있는가? Customize를 위해서? -
  //        [levites]
  bool IntersectSphere(const D3DXVECTOR3 &c_rv3Position, float fRadius);
  bool IntersectCylinder(const D3DXVECTOR3 &c_rv3Position, float fRadius,
                         float fHeight);
};
