///////////////////////////////////////////////////////////////////////
//	CSpeedTreeMaterial Class
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

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>

///////////////////////////////////////////////////////////////////////
//	class CSpeedTreeMaterial declaration/definiton

class CSpeedTreeMaterial {
public:
  CSpeedTreeMaterial() {
    m_cMaterial.Ambient.r = m_cMaterial.Diffuse.r = m_cMaterial.Specular.r =
        m_cMaterial.Emissive.r = 1.0f;
    m_cMaterial.Ambient.g = m_cMaterial.Diffuse.g = m_cMaterial.Specular.g =
        m_cMaterial.Emissive.g = 1.0f;
    m_cMaterial.Ambient.b = m_cMaterial.Diffuse.b = m_cMaterial.Specular.b =
        m_cMaterial.Emissive.b = 1.0f;
    m_cMaterial.Ambient.a = m_cMaterial.Diffuse.a = m_cMaterial.Specular.a =
        m_cMaterial.Emissive.a = 1.0f;
    m_cMaterial.Power = 5.0f;
  }

  void Set(const float *pMaterialArray) {
    memcpy(&m_cMaterial.Diffuse, pMaterialArray, 3 * sizeof(float));
    m_cMaterial.Diffuse.a = 1.0f;

    memcpy(&m_cMaterial.Ambient, pMaterialArray + 3, 3 * sizeof(float));
    m_cMaterial.Ambient.a = 1.0f;

    memcpy(&m_cMaterial.Specular, pMaterialArray + 6, 3 * sizeof(float));
    m_cMaterial.Specular.a = 1.0f;

    memcpy(&m_cMaterial.Emissive, pMaterialArray + 9, 3 * sizeof(float));
    m_cMaterial.Emissive.a = 1.0f;

    m_cMaterial.Power = pMaterialArray[12];
  }

  D3DMATERIAL9 *Get() { return &m_cMaterial; }

private:
  D3DMATERIAL9 m_cMaterial; // the material object
};
