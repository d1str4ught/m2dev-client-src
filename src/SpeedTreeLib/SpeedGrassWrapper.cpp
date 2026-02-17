///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper Class
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

#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

#ifdef USE_SPEEDGRASS

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::CSpeedGrassWrapper

CSpeedGrassWrapper::CSpeedGrassWrapper()
    : m_pMapOutdoor(NULL), m_lpD3DTexure8(NULL) // m_uiTexture(0)
{}

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::~CSpeedGrassWrapper

CSpeedGrassWrapper::~CSpeedGrassWrapper() {}

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::Draw

int CSpeedGrassWrapper::Draw(float fDensity) {
  int nTriangleCount = 0;

  //	// determine which regions are visible
  //	Cull( );
  //
  //	// setup opengl state
  //	glPushAttrib(GL_ENABLE_BIT);
  //	glDisable(GL_CULL_FACE);
  //	glDisable(GL_BLEND);
  //
  //	glEnable(GL_TEXTURE_2D);
  //	glBindTexture(GL_TEXTURE_2D, m_uiTexture);
  //	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  //	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  //
  //	glEnable(GL_ALPHA_TEST);
  //	glAlphaFunc(GL_GREATER, 0.4f);
  //	glDisable(GL_LIGHTING);
  //
  //	unsigned int uiCount = 0;
  //	unsigned int uiNumRegions = 0;
  //	const SRegion* pRegions = GetRegions(uiNumRegions);
  //
  //	// setup for vertex buffer rendering (enable client buffers)
  //	CIdvVertexBuffer::Enable(true);
  //	if (uiNumRegions > 0)
  //		pRegions[0].m_pVertexBuffer->EnableClientStates( );
  //
  //	// run through the regions and render those that aren't culled
  //	for (unsigned int i = 0; i < uiNumRegions; ++i)
  //	{
  //		if (!pRegions[i].m_bCulled)
  //		{
  //			pRegions[i].m_pVertexBuffer->Bind( );
  //			unsigned int uiNumBlades = int(fDensity *
  // pRegions[i].m_vBlades.size( )); 			glDrawArrays(GL_QUADS,
  // 0, uiNumBlades * 4); 			nTriangleCount += uiNumBlades * 2;
  //		}
  //	}
  //
  //	// disable client buffers
  //	if (uiNumRegions > 0)
  //		pRegions[0].m_pVertexBuffer->DisableClientStates( );
  //	CIdvVertexBuffer::Disable(true);
  //
  //	// restore opengl state
  //	glPopAttrib( );

  return nTriangleCount;
}

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::InitFromBsfFile

bool CSpeedGrassWrapper::InitFromBsfFile(const char *pFilename,
                                         unsigned int nNumBlades,
                                         unsigned int uiRows,
                                         unsigned int uiCols,
                                         float fCollisionDistance) {
  bool bSuccess = false;

  if (pFilename) {
    // use SpeedGrass's built-in parse function
    if (ParseBsfFile(pFilename, nNumBlades, uiRows, uiCols, fCollisionDistance))
      bSuccess = true;
  }
  InitGraphics();

  return bSuccess;
}

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::Color

float CSpeedGrassWrapper::Color(float fX, float fY, const float *pNormal,
                                float *pTopColor, float *pBottomColor) const {
  const float c_fColorAdjust = 0.3f; // controls how much the color of the top
                                     // vertices of each grass blade can vary
  const float c_fColorThrow =
      1.0f; // controls how much the r, g, and b components can vary
  const float c_fColorRandomness =
      0.01f; // controls how much the r, g, and b components can vary
  const float c_TopLight = 0.75f;

  float afLowColor[4] = {0.0f}, afHighColor[4] = {0.0f};
  if (m_pMapOutdoor->GetBrushColor(fX, fY, afLowColor, afHighColor)) {
    pBottomColor[0] = afLowColor[2];
    pBottomColor[1] = afLowColor[1];
    pBottomColor[2] = afLowColor[0];

    float fColorThrow = GetRandom(0.0f, c_fColorThrow);
    pTopColor[0] =
        VecInterpolate(pBottomColor[0], afHighColor[2], fColorThrow) +
        GetRandom(-c_fColorRandomness, c_fColorRandomness);
    pTopColor[1] =
        VecInterpolate(pBottomColor[1], afHighColor[1], fColorThrow) +
        GetRandom(-c_fColorRandomness, c_fColorRandomness);
    pTopColor[2] =
        VecInterpolate(pBottomColor[2], afHighColor[0], fColorThrow) +
        GetRandom(-c_fColorRandomness, c_fColorRandomness);

    float fLargest = pTopColor[0];
    if (pTopColor[1] > fLargest)
      fLargest = pTopColor[1];
    if (pTopColor[2] > fLargest)
      fLargest = pTopColor[2];
    if (fLargest > 1.0f) {
      pTopColor[0] /= fLargest;
      pTopColor[1] /= fLargest;
      pTopColor[2] /= fLargest;
    }
    pTopColor[0] = max(0.0f, pTopColor[0]);
    pTopColor[1] = max(0.0f, pTopColor[1]);
    pTopColor[2] = max(0.0f, pTopColor[2]);
  }

  return afLowColor[3];
}

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::Height

float CSpeedGrassWrapper::Height(float fX, float fY, float *pNormal) const {
  float fHeight = 0.0f;
  float afPos[3] = {fX, fY, 0.0f};
  fHeight = m_pMapOutdoor->GetHeight(afPos);

  pNormal[0] = 0.0f;
  pNormal[1] = 0.0f;
  pNormal[2] = 1.0f;

  return fHeight;
}

///////////////////////////////////////////////////////////////////////
//	CSpeedGrassWrapper::InitGraphics

void CSpeedGrassWrapper::InitGraphics(void) {
  // load texture
  //	m_uiTexture = LoadDDS((c_strDataPath + string("brush_2.dds")).c_str( ));
  CGraphicImage *pImage =
      (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
          "D:/ymir work/special/brush_2.dds");
  m_GrassImageInstance.SetImagePointer(pImage);
  m_lpD3DTexure8 = m_GrassImageInstance.GetTexturePointer()->GetD3DTexture();

  // prepare static vertex buffers
  for (int i = 0; i < m_nNumRegions; ++i) {
    SRegion *pRegion = m_pRegions + i;

    //		pRegion->m_pVertexBuffer = new CIdvVertexBuffer;

    // setup up temporary buffer to copy later
    const int c_nNumCorners = 4;
    unsigned int uiNumBlades = pRegion->m_vBlades.size();
    unsigned int uiBufferSize =
        uiNumBlades * c_nNumCorners * c_nGrassVertexTotalSize;
    unsigned char *pBuffer = new unsigned char[uiBufferSize];

    // setup initial pointers for individual attribute copying
    float *pTexCoords0 = reinterpret_cast<float *>(pBuffer + 0);
    float *pTexCoords1 = reinterpret_cast<float *>(
        pTexCoords0 + c_nGrassVertexTexture0Size * uiNumBlades * c_nNumCorners /
                          sizeof(float));
    unsigned char *pColors =
        (unsigned char *)pTexCoords1 +
        c_nGrassVertexTexture1Size * uiNumBlades * c_nNumCorners;
    float *pPositions = reinterpret_cast<float *>(
        pColors + c_nGrassVertexColorSize * uiNumBlades * c_nNumCorners);

    for (vector<SBlade>::const_iterator iBlade = pRegion->m_vBlades.begin();
         iBlade != pRegion->m_vBlades.end(); ++iBlade) {
      float fS1 = float(iBlade->m_ucWhichTexture) / c_nNumBladeMaps;
      float fS2 = float(iBlade->m_ucWhichTexture + 1) / c_nNumBladeMaps;

      for (int nCorner = 0; nCorner < c_nNumCorners; ++nCorner) {
        // texcoord 0
        switch (nCorner) {
        case 0:
          pTexCoords0[0] = fS2;
          pTexCoords0[1] = 1.0f;
          break;
        case 1:
          pTexCoords0[0] = fS1;
          pTexCoords0[1] = 1.0f;
          break;
        case 2:
          pTexCoords0[0] = fS1;
          pTexCoords0[1] = 0.0f;
          break;
        case 3:
          pTexCoords0[0] = fS2;
          pTexCoords0[1] = 0.0f;
          break;
        default:
          assert(false);
        }
        pTexCoords0 += c_nGrassVertexTexture0Size / sizeof(float);

        // texcoord 1
        switch (nCorner) {
        case 0:
          pTexCoords1[0] = c_nShaderGrassBillboard;
          pTexCoords1[2] = iBlade->m_fThrow;
          break;
        case 1:
          pTexCoords1[0] = c_nShaderGrassBillboard + 1;
          pTexCoords1[2] = iBlade->m_fThrow;
          break;
        case 2:
          pTexCoords1[0] = c_nShaderGrassBillboard + 2;
          pTexCoords1[2] = 0.0f;
          break;
        case 3:
          pTexCoords1[0] = c_nShaderGrassBillboard + 3;
          pTexCoords1[2] = 0.0f;
          break;
        default:
          assert(false);
        }
        // same for all corners
        pTexCoords1[1] = iBlade->m_fSize;
        pTexCoords1[3] = iBlade->m_fNoise;
        pTexCoords1 += c_nGrassVertexTexture1Size / sizeof(float);

        // color
        unsigned long ulColor = 0;
        if (nCorner == 0 || nCorner == 1)
          ulColor = (int(iBlade->m_afTopColor[0] * 255.0f) << 0) +
                    (int(iBlade->m_afTopColor[1] * 255.0f) << 8) +
                    (int(iBlade->m_afTopColor[2] * 255.0f) << 16) + 0xff000000;
        else
          ulColor = (int(iBlade->m_afBottomColor[0] * 255.0f) << 0) +
                    (int(iBlade->m_afBottomColor[1] * 255.0f) << 8) +
                    (int(iBlade->m_afBottomColor[2] * 255.0f) << 16) +
                    0xff000000;
        memcpy(pColors, &ulColor, c_nGrassVertexColorSize);
        pColors += c_nGrassVertexColorSize;

        // position
        memcpy(pPositions, iBlade->m_afPos, c_nGrassVertexPositionSize);
        pPositions += c_nGrassVertexPositionSize / sizeof(float);
      }
    }

    //		assert((unsigned char*) pTexCoords0 - pBuffer ==
    // c_nGrassVertexTexture0Size * uiNumBlades * c_nNumCorners);
    //		assert(pTexCoords1 - pTexCoords0 == (c_nGrassVertexTexture1Size
    //* uiNumBlades * c_nNumCorners) / sizeof(float)); 		assert(pColors -
    //(unsigned char*) pTexCoords1 == c_nGrassVertexColorSize * uiNumBlades *
    // c_nNumCorners); 		assert((unsigned char*) pPositions - pColors ==
    // c_nGrassVertexPositionSize * uiNumBlades * c_nNumCorners);

    //		pRegion->m_pVertexBuffer->SetBuffer(pBuffer, uiBufferSize,
    // true);
    //		pRegion->m_pVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD0,
    // 2, GL_FLOAT, 0, 0);
    //		pRegion->m_pVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD1,
    // 4, GL_FLOAT, 0, (unsigned char*) pTexCoords0 - pBuffer);
    //		pRegion->m_pVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_COLOR,
    // 4, GL_UNSIGNED_BYTE, 0, (unsigned char*) pTexCoords1 - pBuffer);
    //		pRegion->m_pVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_POSITION,
    // 3, GL_FLOAT, 0, pColors - pBuffer);

    DWORD dwFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
    // 		pRegion->m_VertexBuffer.Create();

    delete[] pBuffer;
  }
}

#endif // USE_SPEEDGRASS
