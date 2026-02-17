///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest Class
//
//	(c) 2003 IDV, Inc.
//
//	This class is provided to illustrate one way to incorporate
//	SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//	calls that must be made on a per tree basis are done by this class.
//	Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//	are made in the functions in main.cpp.
//
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

///////////////////////////////////////////////////////////////////////
//	Include Files
#include <SpeedTreeRT.h>
#include "SpeedTreeWrapper.h"

#include <vector>
#include <map>

#include "EterBase/CRC32.h"

///////////////////////////////////////////////////////////////////////
//	Render bit vector

#define Forest_RenderBranches (1 << 0)
#define Forest_RenderLeaves (1 << 1)
#define Forest_RenderFronds (1 << 2)
#define Forest_RenderBillboards (1 << 3)
#define Forest_RenderAll ((1 << 4) - 1)
#define Forest_RenderToShadow (1 << 5)
#define Forest_RenderToMiniMap (1 << 6)

///////////////////////////////////////////////////////////////////////
//	class CSpeedTreeForest declaration

class CSpeedTreeForest {
public:
  using SpeedTreeWrapperPtr = std::shared_ptr<CSpeedTreeWrapper>;
  using TTreeMap = std::map<DWORD, SpeedTreeWrapperPtr>;

public:
  CSpeedTreeForest();
  virtual ~CSpeedTreeForest();

  void ClearMainTree();

  BOOL GetMainTree(DWORD dwCRC, SpeedTreeWrapperPtr &ppMainTree,
                   const char *c_pszFileName);
  SpeedTreeWrapperPtr GetMainTree(DWORD dwCRC);
  void DeleteMainTree(DWORD dwCRC);

  SpeedTreeWrapperPtr CreateInstance(float x, float y, float z, DWORD dwTreeCRC,
                                     const char *c_pszTreeName);
  void DeleteInstance(SpeedTreeWrapperPtr pTree);

  // void						SetLodLimits(void);

  void UpdateSystem(float fCurrentTime);

  void Clear();

  void SetLight(const float *afDirection, const float *afAmbient,
                const float *afDiffuse);
  void SetFog(float fFogNear, float fFogFar);
  //////////////////////////////////////////////////////////////////////////

  const float *GetExtents(void) const { return m_afForestExtents; }

  // wind management
  float GetWindStrength(void) const { return m_fWindStrength; }
  void SetWindStrength(float fStrength);
  void SetupWindMatrices(float fTimeInSecs);

  // overridden by specific graphics API
  virtual void UploadWindMatrix(unsigned int uiLocation,
                                const float *pMatrix) const = 0;
  virtual void Render(unsigned long ulRenderBitVector) = 0;

protected:
  TTreeMap m_pMainTreeMap;

  float m_afLighting[12];
  float m_afFog[4];

private:
  void AdjustExtents(float x, float y, float z);

  float m_afForestExtents[6]; // [0] = min x, [1] = min y..., [3] = max x, [4] =
                              // max y...
  float m_fWindStrength;      // 0.0 = no wind, 1.0 = full strength

  float m_fAccumTime;
};
