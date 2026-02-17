///////////////////////////////////////////////////////////////////////
//  Name: BoundaryShapeManager.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2002-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not
//  be copied or disclosed except in accordance with the terms of that
//  agreement.

///////////////////////////////////////////////////////////////////////
//	Preprocessor
#include "StdAfx.h"

#include <windows.h>
#include "EterBase/Random.h"
#include "BoundaryShapeManager.h"

using namespace std;

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::CBoundaryShapeManager
CBoundaryShapeManager::CBoundaryShapeManager() {}

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::~CBoundaryShapeManager

CBoundaryShapeManager::~CBoundaryShapeManager() {}

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::LoadBsfFile

bool CBoundaryShapeManager::LoadBsfFile(const char *pszFilename) {
  bool bSuccess = true;
  try {
    FILE *pFile = fopen(pszFilename, "rb");
    if (pFile) {
      // number of boundary shapes
      unsigned int nNumBoundaries;
      if (fread(&nNumBoundaries, sizeof(unsigned int), 1, pFile) == 1) {
        for (unsigned int i = 0; i < nNumBoundaries && bSuccess; ++i) {
          SBoundaryShape sShape;

          // number of contours for this shape
          unsigned int nNumContours;
          if (fread(&nNumContours, sizeof(unsigned int), 1, pFile) == 1) {
            for (unsigned int j = 0; j < nNumContours && bSuccess; ++j) {
              // number of points in this contour
              vector<SPoint> vPoints;
              unsigned int nNumPoints;
              if (fread(&nNumPoints, sizeof(unsigned int), 1, pFile) == 1) {
                // read the points
                for (unsigned int k = 0; k < nNumPoints && bSuccess; ++k) {
                  SPoint sPoint;

                  if (fread(sPoint.m_afData, sizeof(float), 3, pFile) == 3) {
                    vPoints.push_back(sPoint);

                    // update extents
                    if (j == 0 && k == 0) {
                      // first point of this shape
                      memcpy(sShape.m_afMin, sPoint.m_afData,
                             3 * sizeof(float));
                      memcpy(sShape.m_afMax, sPoint.m_afData,
                             3 * sizeof(float));
                    } else {
                      // check extents
                      for (int l = 0; l < 3; ++l) {
                        if (sPoint.m_afData[l] < sShape.m_afMin[l])
                          sShape.m_afMin[l] = sPoint.m_afData[l];
                        else if (sPoint.m_afData[l] > sShape.m_afMax[l])
                          sShape.m_afMax[l] = sPoint.m_afData[l];
                      }
                    }
                  } else {
                    m_strCurrentError =
                        "Error in CBoundaryShapeManager::LoadBsfFile(): Failed "
                        "to read point";
                    bSuccess = false;
                  }
                }
                sShape.m_vContours.push_back(vPoints);
              } else {
                m_strCurrentError =
                    "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to "
                    "read number of points";
                bSuccess = false;
              }
            }
            m_vBoundaries.push_back(sShape);
          } else {
            m_strCurrentError =
                "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read "
                "number of contours";
            bSuccess = false;
          }
        }
      } else {
        m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): "
                            "Failed to read number of boundaries";
        bSuccess = false;
      }
      fclose(pFile);
    } else {
      m_strCurrentError =
          string("Error in CBoundaryShapeManager::LoadBsfFile(): Could not "
                 "open ") +
          string(pszFilename);
      bSuccess = false;
    }
  }

  catch (...) {
    m_strCurrentError =
        "Error in CBoundaryShapeManager::LoadBsfFile(): Unknown exception";
  }

  return bSuccess;
}

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::PointInside

bool CBoundaryShapeManager::PointInside(float fX, float fY) {
  bool bInside = false;

  for (DWORD i = 0; i < m_vBoundaries.size() && !bInside; ++i)
    bInside = PointInShape(m_vBoundaries[i], fX, fY);

  return bInside;
}

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::PointInShape

bool CBoundaryShapeManager::PointInShape(SBoundaryShape &sShape, float fX,
                                         float fY) {
  bool bInside = false;

  for (DWORD k = 0; k < sShape.m_vContours.size(); ++k) {
    for (DWORD i = 0, j = sShape.m_vContours[k].size() - 1;
         i < sShape.m_vContours[k].size(); j = i++) {
      if ((((sShape.m_vContours[k][i][1] <= fY) &&
            (fY < sShape.m_vContours[k][j][1])) ||
           ((sShape.m_vContours[k][j][1] <= fY) &&
            (fY < sShape.m_vContours[k][i][1]))) &&
          (fX <
           (sShape.m_vContours[k][i][0] - sShape.m_vContours[k][i][0]) *
                   (fY - sShape.m_vContours[k][i][1]) /
                   (sShape.m_vContours[k][j][1] - sShape.m_vContours[k][i][1]) +
               sShape.m_vContours[k][i][0])) {
        bInside = !bInside;
      }
    }
  }

  return bInside;
}

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::RandomPoint

bool CBoundaryShapeManager::RandomPoint(float &fX, float &fY) {
  bool bSuccess = false;

  if (m_vBoundaries.size() > 0) {
    // pick a random boundary shape
    int nIndex = random_range(0, m_vBoundaries.size() - 1);
    SBoundaryShape &sShape = m_vBoundaries[nIndex];

    // pick a point at random within its extents
    fX = frandom(sShape.m_afMin[0], sShape.m_afMax[0]);
    fY = frandom(sShape.m_afMin[1], sShape.m_afMax[1]);

    // try it
    bSuccess = PointInShape(sShape, fX, fY);
  }

  return bSuccess;
}
