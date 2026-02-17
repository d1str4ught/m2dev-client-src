/* Copyright (C) Graham Rhodes, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Graham Rhodes, 2001"
 */
/**************************************************************************************
|
|           File: lineintersect_utils.h
|
|        Purpose: Function prototypes for line segment intersection utility
functions
|
|     Book Title: Game Programming Gems II
|
|  Chapter Title: Fast, Robust Intersection of 3D Line Segments
|
|         Author: Graham Rhodes
|
|      Revisions: 05-Apr-2001 - GSR. Original.
|
**************************************************************************************/
#ifndef _lineintersect_utils_h
#define _lineintersect_utils_h

void IntersectLineSegments(const D3DXVECTOR3 &A1, const D3DXVECTOR3 &A2,
                           const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2,
                           // bool infinite_lines, /*float epsilon,*/
                           D3DXVECTOR3 &OutA, D3DXVECTOR3 &OutB);

void IntersectLineSegments(
    const float A1x, const float A1y, const float A1z, const float A2x,
    const float A2y, const float A2z, const float B1x, const float B1y,
    const float B1z, const float B2x, const float B2y, const float B2z,
    bool infinite_lines, float epsilon, float &PointOnSegAx,
    float &PointOnSegAy, float &PointOnSegAz, float &PointOnSegBx,
    float &PointOnSegBy, float &PointOnSegBz, float &NearestPointX,
    float &NearestPointY, float &NearestPointZ, float &NearestVectorX,
    float &NearestVectorY, float &NearestVectorZ, bool &true_intersection);

void IntersectLineSegments(const float A1x, const float A1y, const float A1z,
                           const float A2x, const float A2y, const float A2z,
                           const float B1x, const float B1y, const float B1z,
                           const float B2x, const float B2y, const float B2z,
                           bool infinite_lines, float epsilon,
                           float &PointOnSegAx, float &PointOnSegAy,
                           float &PointOnSegAz, float &PointOnSegBx,
                           float &PointOnSegBy, float &PointOnSegBz);

void FindNearestPointOnLineSegment(const float A1x, const float A1y,
                                   const float A1z, const float Lx,
                                   const float Ly, const float Lz,
                                   const float Bx, const float By,
                                   const float Bz, bool infinite_line,
                                   float epsilon_squared, float &NearestPointX,
                                   float &NearestPointY, float &NearestPointZ,
                                   float &parameter);

void FindNearestPointOfParallelLineSegments(
    float A1x, float A1y, float A1z, float A2x, float A2y, float A2z, float Lax,
    float Lay, float Laz, float B1x, float B1y, float B1z, float B2x, float B2y,
    float B2z, float Lbx, float Lby, float Lbz, bool infinite_lines,
    float epsilon_squared, float &PointOnSegAx, float &PointOnSegAy,
    float &PointOnSegAz, float &PointOnSegBx, float &PointOnSegBy,
    float &PointOnSegBz);

void AdjustNearestPoints(float A1x, float A1y, float A1z, float Lax, float Lay,
                         float Laz, float B1x, float B1y, float B1z, float Lbx,
                         float Lby, float Lbz, float epsilon_squared, float s,
                         float t, float &PointOnSegAx, float &PointOnSegAy,
                         float &PointOnSegAz, float &PointOnSegBx,
                         float &PointOnSegBy, float &PointOnSegBz);

#endif // _lineintersect_utils_h