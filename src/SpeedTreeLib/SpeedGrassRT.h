///////////////////////////////////////////////////////////////////////
//	CSpeedGrassRT Class
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com

#pragma once

#ifdef USE_SPEEDGRASS

// forward reference
class CIdvTerrain;

///////////////////////////////////////////////////////////////////////
//	class CSpeedGrassRT declaration

class CSpeedGrassRT {
public:
  CSpeedGrassRT();
  virtual ~CSpeedGrassRT();

  ///////////////////////////////////////////////////////////////////////
  //  struct SBlade

  struct SBlade {
    SBlade();

    // geometry
    float m_afPos[3];
    float m_afNormal[3];
    float m_fSize;
    unsigned char m_ucWhichTexture;

    // wind
    float m_fNoise;
    float m_fThrow;

    // color
    float m_afBottomColor[3];
    float m_afTopColor[3];
  };

  ///////////////////////////////////////////////////////////////////////
  //  struct SRegion

  struct SRegion {
    SRegion();

    // dimensions
    float m_afCenter[3];
    float m_afMin[3];
    float m_afMax[3];

    // culling
    bool m_bCulled;
    float m_fCullingRadius;

    // grass/brush blades
    std::vector<SBlade> m_vBlades;
    // 			CIdvVertexBuffer*	m_pVertexBuffer;
    CGraphicVertexBuffer m_VertexBuffer;
  };

  void DeleteRegions(void);
  const SRegion *GetRegions(unsigned int &uiNumRegions);
  bool ParseBsfFile(const char *pFilename, unsigned int nNumBlades,
                    unsigned int uiRows, unsigned int uiCols,
                    float fCollisionDistance = 0.0f);
  bool CustomPlacement(unsigned int uiRows, unsigned int uiCols);

  //////////////////////////////////////////////////////////////////////////
  // Utility
  static void RotateAxisFromIdentity(D3DXMATRIX *pMat, const float &c_fAngle,
                                     const D3DXVECTOR3 &c_rv3Axis);

  ///////////////////////////////////////////////////////////////////////
  //  Geometry

  static const float *GetUnitBillboard(void) { return m_afUnitBillboard; }

  ///////////////////////////////////////////////////////////////////////
  //  LOD

  static void GetLodParams(float &fFarDistance, float &fTransitionLength);
  static void SetLodParams(float fFarDistance, float fTransitionLength);

  ///////////////////////////////////////////////////////////////////////
  //  Culling

  bool AllRegionsAreCulled(void) const { return m_bAllRegionsCulled; }
  void Cull(void);

  ///////////////////////////////////////////////////////////////////////
  //  Wind

  static void SetWindDirection(const float *pWindDir);
  static const float *GetWindDirection(void);

  ///////////////////////////////////////////////////////////////////////
  //  Camera

  static const float *GetCameraPos(void);
  static void SetCamera(const float *pPosition, const double *pModelviewMatrix);
  static void SetPerspective(float fAspectRatio, float fFieldOfView);

  ///////////////////////////////////////////////////////////////////////
  //  Terrain hugging

  virtual float Color(float fX, float fY, const float *pNormal,
                      float *pTopColor, float *pBottomColor) const {
    return 0.0f;
  }
  virtual float Height(float fX, float fY, float *pNormal) const {
    return 0.0f;
  }

protected:
  void CreateRegions(const std::vector<SBlade> &vSceneBlades,
                     float fCollisionDistance = 0.0f);
  static void ComputeFrustum(void);
  static void ComputeUnitBillboard(void);
  void ConvertCoordsToCell(const float *pCoords, int *pGridCoords) const;
  unsigned int GetRegionIndex(unsigned int uiRow, unsigned int uiCol) const {
    return uiRow * m_nNumRegionCols + uiCol;
  }
  static bool OutsideFrustum(SRegion *pRegion);

  // general
  static float m_fLodFarDistance;
  static float m_fLodTransitionLength;
  static float m_afUnitBillboard[12];
  static float m_afWindDir[4];

  // regions
  int m_nNumRegions;
  int m_nNumRegionCols;
  int m_nNumRegionRows;
  SRegion *m_pRegions;

  // camera
  static float m_afCameraOut[3];
  static float m_afCameraRight[3];
  static float m_afCameraUp[3];
  static float m_afCameraPos[3];
  static float m_fFieldOfView;
  static float m_fAspectRatio;

  // culling
  static float m_afFrustumBox[6];
  static float m_afFrustumMin[2];
  static float m_afFrustumMax[2];
  static float m_afFrustumPlanes[5][4];
  float m_afBoundingBox[6];
  bool m_bAllRegionsCulled;
};

extern float VecInterpolate(float fStart, float fEnd, float fPercent);
#endif // USE_SPEEDGRASS