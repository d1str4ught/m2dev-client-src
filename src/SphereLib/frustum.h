/* Copyright (C) John W. Ratcliff, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) John W. Ratcliff, 2001"
 */
#pragma once

/***********************************************************************/
/** FRUSTUM.H   : Represents a clipping frustum.                       */
/**               You should replace this with your own more robust    */
/**               view frustum clipper.                                */
/**                                                                    */
/**               Written by John W. Ratcliff jratcliff@att.net        */
/***********************************************************************/

#include "vector.h"

enum ViewState {
  VS_INSIDE,  // completely inside the frustum.
  VS_PARTIAL, // partially inside and partially outside the frustum.
  VS_OUTSIDE  // completely outside the frustum
};

class Frustum {
public:
  void BuildViewFrustum(D3DXMATRIX &mat);
  void BuildViewFrustum2(D3DXMATRIX &mat, float fNear, float fFar, float fFov,
                         float fAspect, const D3DXVECTOR3 &vCamera,
                         const D3DXVECTOR3 &vLook);
  ViewState ViewVolumeTest(const Vector3d &c_v3Center,
                           const float c_fRadius) const;

private:
  bool m_bUsingSphere;
  D3DXVECTOR3 m_v3Center;
  float m_fRadius;
  D3DXPLANE m_plane[6];
};
