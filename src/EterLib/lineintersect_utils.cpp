/**************************************************************************************
 |           File: lineintersect_utils.cpp
 |        Purpose: Implementation of line segment intersection utility functions
 |     Book Title: Game Programming Gems II
 |  Chapter Title: Fast, Robust Intersection of 3D Line Segments
 |         Author: Graham Rhodes
 |      Revisions: 05-Apr-2001 - GSR. Original.
 **************************************************************************************/
#include "Stdafx.h"
#include <math.h>
#include "lineintersect_utils.h"
#include <assert.h>

// uncomment the following line to have the code check intermediate results
// #define CHECK_ANSWERS

// uncomment the following line to use Cramer's rule instead of Gaussian
// elimination
// #define USE_CRAMERS_RULE

#define FMAX(a, b) ((a) > (b) ? (a) : (b))
#define FMIN(a, b) ((a) > (b) ? (b) : (a))
#define FABS(a) ((a) < 0.0f ? -(a) : (a))
#define OUT_OF_RANGE(a) ((a) < 0.0f || (a) > 1.f)

#define MY_EPSILON 0.1f

__forceinline void FindNearestPointOnLineSegment(const D3DXVECTOR3 &A1,
                                                 const D3DXVECTOR3 &L,
                                                 const D3DXVECTOR3 &B,
                                                 D3DXVECTOR3 &Nearest,
                                                 float &parameter) {
  // Line/Segment is degenerate --- special case #1
  float D = D3DXVec3LengthSq(&L);
  if (D < MY_EPSILON * MY_EPSILON) {
    Nearest = A1;
    return;
  }

  D3DXVECTOR3 AB = B - A1;

  // parameter is computed from Equation (20).
  parameter = (D3DXVec3Dot(&AB, &L)) / D;

  // if (false == infinite_line)
  parameter = FMAX(0.0f, FMIN(1.0f, parameter));

  Nearest = A1 + parameter * L;
  return;
}

/**************************************************************************
|
|     Method: FindNearestPointOfParallelLineSegments
|
|    Purpose: Given two lines (segments) that are known to be parallel, find
|             a representative point on each that is nearest to the other. If
|             the lines are considered to be finite then it is possible that
there |             is one true point on each line that is nearest to the other.
This |             code properly handles this case.
|
|             This is the most difficult line intersection case to handle, since
|             there is potentially a family, or locus of points on each
line/segment |             that are nearest to the other. | Parameters: Input:
|             ------
|             A1x, A1y, A1z   - Coordinates of first defining point of
line/segment A |             A2x, A2y, A2z   - Coordinates of second defining
point of line/segment A |             Lax, Lay, Laz   - Vector from (A1x, A1y,
A1z) to the (A2x, A2y, A2z). |             B1x, B1y, B1z   - Coordinates of
first defining point of line/segment B |             B2x, B2y, B2z   -
Coordinates of second defining point of line/segment B |             Lbx, Lby,
Lbz   - Vector from (B1x, B1y, B1z) to the (B2x, B2y, B2z). | infinite_lines  -
set to true if lines are to be treated as infinite |             epsilon_squared
- tolerance value to be used to check for degenerate | and parallel lines, and
to check for true intersection.
|
|             Output:
|             -------
|             PointOnSegAx,   - Coordinates of the point on segment A that are
nearest |             PointOnSegAy,     to segment B. This corresponds to point
C in the text. |             PointOnSegAz |             PointOnSegBx,   -
Coordinates of the point on segment B that are nearest | PointOnSegBy,     to
segment A. This corresponds to point D in the text. |             PointOnSegBz

**************************************************************************/
__forceinline void FindNearestPointOfParallelLineSegments(
    const D3DXVECTOR3 &A1, const D3DXVECTOR3 &A2, const D3DXVECTOR3 &La,
    const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2, const D3DXVECTOR3 &Lb,
    // bool infinite_lines, float epsilon_squared,
    D3DXVECTOR3 &OutA, D3DXVECTOR3 &OutB) {
  float s[2], temp;
  FindNearestPointOnLineSegment(A1, La, B1, OutA, s[0]);
  /*if (true == infinite_lines)
  {
      PointOnSegBx = B1x;
      PointOnSegBy = B1y;
      PointOnSegBz = B1z;
  }
  else*/
  {
    // float tp[3];
    D3DXVECTOR3 tp;
    FindNearestPointOnLineSegment(A1, La, B2, tp, s[1]);
    if (s[0] < 0.f && s[1] < 0.f) {
      OutA = A1;
      if (s[0] < s[1]) {
        OutB = B2;
      } else {
        OutB = B1;
      }
    } else if (s[0] > 1.f && s[1] > 1.f) {
      OutA = A2;
      if (s[0] < s[1]) {
        OutB = B1;
      } else {
        OutB = B2;
      }
    } else {
      temp =
          0.5f * (FMAX(0.0f, FMIN(1.0f, s[0])) + FMAX(0.0f, FMIN(1.0f, s[1])));
      OutA = A1 + temp * La;
      FindNearestPointOnLineSegment(B1, Lb, OutA, OutB, temp);
    }
  }
}

/**************************************************************************
|
|     Method: AdjustNearestPoints
|
|    Purpose: Given nearest point information for two infinite lines, adjust
|             to model finite line segments.
|
| Parameters: Input:
|             ------
|             A1x, A1y, A1z   - Coordinates of first defining point of
line/segment A |             Lax, Lay, Laz   - Vector from (A1x, A1y, A1z) to
the (A2x, A2y, A2z). |             B1x, B1y, B1z   - Coordinates of first
defining point of line/segment B |             Lbx, Lby, Lbz   - Vector from
(B1x, B1y, B1z) to the (B2x, B2y, B2z). |             epsilon_squared -
tolerance value to be used to check for degenerate | and parallel lines, and to
check for true intersection. |             s               - parameter
representing nearest point on infinite line A |             t               -
parameter representing nearest point on infinite line B
|
|             Output:
|             -------
|             PointOnSegAx,   - Coordinates of the point on segment A that are
nearest |             PointOnSegAy,     to segment B. This corresponds to point
C in the text. |             PointOnSegAz |             PointOnSegBx,   -
Coordinates of the point on segment B that are nearest | PointOnSegBy,     to
segment A. This corresponds to point D in the text. |             PointOnSegBz
**************************************************************************/
__forceinline void AdjustNearestPoints(const D3DXVECTOR3 &A1,
                                       const D3DXVECTOR3 &La,
                                       const D3DXVECTOR3 &B1,
                                       const D3DXVECTOR3 &Lb, float s, float t,
                                       D3DXVECTOR3 &OutA, D3DXVECTOR3 &OutB) {
  // handle the case where both parameter s and t are out of range
  if (OUT_OF_RANGE(s) && OUT_OF_RANGE(t)) {
    s = FMAX(0.0f, FMIN(1.0f, s));
    OutA = A1 + s * La;
    FindNearestPointOnLineSegment(B1, Lb, OutA, OutB, t);
    if (OUT_OF_RANGE(t)) {
      t = FMAX(0.0f, FMIN(1.0f, t));
      OutB = B1 + t * Lb;
      FindNearestPointOnLineSegment(A1, La, OutB, OutA, s);
      FindNearestPointOnLineSegment(B1, Lb, OutA, OutB, t);
    }
  }
  // otherwise, handle the case where the parameter for only one segment is
  // out of range
  else if (OUT_OF_RANGE(s)) {
    s = FMAX(0.0f, FMIN(1.0f, s));
    OutA = A1 + s * La;
    FindNearestPointOnLineSegment(B1, Lb, OutA, OutB, t);
  } else if (OUT_OF_RANGE(t)) {
    t = FMAX(0.0f, FMIN(1.0f, t));
    OutB = B1 + t * Lb;
    FindNearestPointOnLineSegment(A1, La, OutB, OutA, s);
  } else {
    assert(0);
  }
}

void IntersectLineSegments(const D3DXVECTOR3 &A1, const D3DXVECTOR3 &A2,
                           const D3DXVECTOR3 &B1, const D3DXVECTOR3 &B2,
                           // bool infinite_lines, /*float epsilon,*/
                           D3DXVECTOR3 &OutA, D3DXVECTOR3 &OutB) {
  float temp = 0.f;
  const float epsilon = MY_EPSILON;
  const float epsilon_squared = MY_EPSILON * MY_EPSILON;

  // Compute parameters from Equations (1) and (2) in the text
  D3DXVECTOR3 La = A2 - A1;
  D3DXVECTOR3 Lb = B2 - B1;
  // From Equation (15)
  float L11 = D3DXVec3LengthSq(&La);
  float L22 = D3DXVec3LengthSq(&Lb);

  // Line/Segment A is degenerate ---- Special Case #1
  if (L11 < epsilon_squared) {
    OutA = A1;
    FindNearestPointOnLineSegment(B1, Lb, A1, OutB, temp);
  }
  // Line/Segment B is degenerate ---- Special Case #1
  else if (L22 < epsilon_squared) {
    OutB = B1;
    FindNearestPointOnLineSegment(A1, La, B1, OutA, temp);
  }
  // Neither line/segment is degenerate
  else {
    // Compute more parameters from Equation (3) in the text.
    D3DXVECTOR3 AB = B1 - A1;

    // and from Equation (15).
    float L12 = -D3DXVec3Dot(&La, &Lb);

    float DetL = L11 * L22 - L12 * L12;
    // Lines/Segments A and B are parallel ---- special case #2.
    if (FABS(DetL) < epsilon) {
      FindNearestPointOfParallelLineSegments(A1, A2, La, B1, B2, Lb, OutA,
                                             OutB);
    }
    // The general case
    else {
      // from Equation (15)
      float ra = D3DXVec3Dot(&La, &AB); // Lax * ABx + Lay * ABy + Laz * ABz;
      float rb = D3DXVec3Dot(&Lb, &AB); //-Lbx * ABx - Lby * ABy - Lbz * ABz;

      float t = (L11 * rb - ra * L12) / DetL; // Equation (12)

#ifdef USE_CRAMERS_RULE
      float s = (L22 * ra - rb * L12) / DetL;
#else
      float s = (ra - L12 * t) / L11; // Equation (13)
#endif // USE_CRAMERS_RULE

#ifdef CHECK_ANSWERS
      float check_ra = s * L11 + t * L12;
      float check_rb = s * L12 + t * L22;
      assert(FABS(check_ra - ra) < epsilon);
      assert(FABS(check_rb - rb) < epsilon);
#endif // CHECK_ANSWERS

      // if we are dealing with infinite lines or if parameters s and t both
      // lie in the range [0,1] then just compute the points using Equations
      // (1) and (2) from the text.
      OutA = (A1 + s * La);
      OutB = (B1 + t * Lb);
      // otherwise, at least one of s and t is outside of [0,1] and we have to
      // handle this case.
      if ((OUT_OF_RANGE(s) || OUT_OF_RANGE(t))) {
        AdjustNearestPoints(A1, La, B1, Lb, s, t, OutA, OutB);
      }
    }
  }
}

void IntersectLineSegments(const float A1x, const float A1y, const float A1z,
                           const float A2x, const float A2y, const float A2z,
                           const float B1x, const float B1y, const float B1z,
                           const float B2x, const float B2y, const float B2z,
                           bool infinite_lines, float epsilon,
                           float &PointOnSegAx, float &PointOnSegAy,
                           float &PointOnSegAz, float &PointOnSegBx,
                           float &PointOnSegBy, float &PointOnSegBz) {
  float temp = 0.f;
  float epsilon_squared = epsilon * epsilon;

  // Compute parameters from Equations (1) and (2) in the text
  float Lax = A2x - A1x;
  float Lay = A2y - A1y;
  float Laz = A2z - A1z;
  float Lbx = B2x - B1x;
  float Lby = B2y - B1y;
  float Lbz = B2z - B1z;
  // From Equation (15)
  float L11 = (Lax * Lax) + (Lay * Lay) + (Laz * Laz);
  float L22 = (Lbx * Lbx) + (Lby * Lby) + (Lbz * Lbz);

  // Line/Segment A is degenerate ---- Special Case #1
  if (L11 < epsilon_squared) {
    PointOnSegAx = A1x;
    PointOnSegAy = A1y;
    PointOnSegAz = A1z;
    FindNearestPointOnLineSegment(B1x, B1y, B1z, Lbx, Lby, Lbz, A1x, A1y, A1z,
                                  infinite_lines, epsilon, PointOnSegBx,
                                  PointOnSegBy, PointOnSegBz, temp);
  }
  // Line/Segment B is degenerate ---- Special Case #1
  else if (L22 < epsilon_squared) {
    PointOnSegBx = B1x;
    PointOnSegBy = B1y;
    PointOnSegBz = B1z;
    FindNearestPointOnLineSegment(A1x, A1y, A1z, Lax, Lay, Laz, B1x, B1y, B1z,
                                  infinite_lines, epsilon, PointOnSegAx,
                                  PointOnSegAy, PointOnSegAz, temp);
  }
  // Neither line/segment is degenerate
  else {
    // Compute more parameters from Equation (3) in the text.
    float ABx = B1x - A1x;
    float ABy = B1y - A1y;
    float ABz = B1z - A1z;

    // and from Equation (15).
    float L12 = -(Lax * Lbx) - (Lay * Lby) - (Laz * Lbz);

    float DetL = L11 * L22 - L12 * L12;
    // Lines/Segments A and B are parallel ---- special case #2.
    if (FABS(DetL) < epsilon) {
      FindNearestPointOfParallelLineSegments(
          A1x, A1y, A1z, A2x, A2y, A2z, Lax, Lay, Laz, B1x, B1y, B1z, B2x, B2y,
          B2z, Lbx, Lby, Lbz, infinite_lines, epsilon, PointOnSegAx,
          PointOnSegAy, PointOnSegAz, PointOnSegBx, PointOnSegBy, PointOnSegBz);
    }
    // The general case
    else {
      // from Equation (15)
      float ra = Lax * ABx + Lay * ABy + Laz * ABz;
      float rb = -Lbx * ABx - Lby * ABy - Lbz * ABz;

      float t = (L11 * rb - ra * L12) / DetL; // Equation (12)

#ifdef USE_CRAMERS_RULE
      float s = (L22 * ra - rb * L12) / DetL;
#else
      float s = (ra - L12 * t) / L11; // Equation (13)
#endif // USE_CRAMERS_RULE

#ifdef CHECK_ANSWERS
      float check_ra = s * L11 + t * L12;
      float check_rb = s * L12 + t * L22;
      assert(FABS(check_ra - ra) < epsilon);
      assert(FABS(check_rb - rb) < epsilon);
#endif // CHECK_ANSWERS

      // if we are dealing with infinite lines or if parameters s and t both
      // lie in the range [0,1] then just compute the points using Equations
      // (1) and (2) from the text.
      PointOnSegAx = (A1x + s * Lax);
      PointOnSegAy = (A1y + s * Lay);
      PointOnSegAz = (A1z + s * Laz);
      PointOnSegBx = (B1x + t * Lbx);
      PointOnSegBy = (B1y + t * Lby);
      PointOnSegBz = (B1z + t * Lbz);
      // otherwise, at least one of s and t is outside of [0,1] and we have to
      // handle this case.
      if (false == infinite_lines && (OUT_OF_RANGE(s) || OUT_OF_RANGE(t))) {
        AdjustNearestPoints(A1x, A1y, A1z, Lax, Lay, Laz, B1x, B1y, B1z, Lbx,
                            Lby, Lbz, epsilon, s, t, PointOnSegAx, PointOnSegAy,
                            PointOnSegAz, PointOnSegBx, PointOnSegBy,
                            PointOnSegBz);
      }
    }
  }
}

// pragma to get rid of math.h inline function removal warnings.
#pragma warning(disable : 4514)

/**************************************************************************
|
|     Method: IntersectLineSegments
|
|    Purpose: Find the nearest point between two finite length line segments
|             or two infinite lines in 3-dimensional space. The function
calculates |             the point on each line/line segment that is closest to
the other |             line/line segment, the midpoint between the nearest
points, and |             the vector between these two points. If the two
nearest points |             are close within a tolerance, a flag is set
indicating the lines |             have a "true" intersection.
|
| Parameters: Input:
|             ------
|             A1x, A1y, A1z   - Coordinates of first defining point of
line/segment A |             A2x, A2y, A2z   - Coordinates of second defining
point of line/segment A |             B1x, B1y, B1z   - Coordinates of first
defining point of line/segment B |             B2x, B2y, B2z   - Coordinates of
second defining point of line/segment B |             infinite_lines  - set to
true if lines are to be treated as infinite |             epsilon         -
tolerance value to be used to check for degenerate | and parallel lines, and to
check for true intersection.
|
|             Output:
|             -------
|             PointOnSegAx,   - Coordinates of the point on segment A that are
nearest |             PointOnSegAy,     to segment B. This corresponds to point
C in the text. |             PointOnSegAz |             PointOnSegBx,   -
Coordinates of the point on segment B that are nearest | PointOnSegBy,     to
segment A. This corresponds to point D in the text. |             PointOnSegBz
|             NearestPointX,  - Midpoint between the two nearest points. This
can be |             NearestPointY,    treated as *the* intersection point if
nearest points |             NearestPointZ     are sufficiently close. This
corresponds to point P |                               in the text. |
NearestVectorX, - Vector between the nearest point on A to the nearest | point
on segment B. This vector is normal to both | lines if the lines are infinite,
but is not guaranteed |                               to be normal to both lines
if both lines are finite |                               length. |
true_intersection - true if the nearest points are close within a small |
tolerance.
**************************************************************************/
void IntersectLineSegments(
    const float A1x, const float A1y, const float A1z, const float A2x,
    const float A2y, const float A2z, const float B1x, const float B1y,
    const float B1z, const float B2x, const float B2y, const float B2z,
    bool infinite_lines, float epsilon, float &PointOnSegAx,
    float &PointOnSegAy, float &PointOnSegAz, float &PointOnSegBx,
    float &PointOnSegBy, float &PointOnSegBz, float &NearestPointX,
    float &NearestPointY, float &NearestPointZ, float &NearestVectorX,
    float &NearestVectorY, float &NearestVectorZ, bool &true_intersection) {
  float temp = 0.f;
  float epsilon_squared = epsilon * epsilon;

  // Compute parameters from Equations (1) and (2) in the text
  float Lax = A2x - A1x;
  float Lay = A2y - A1y;
  float Laz = A2z - A1z;
  float Lbx = B2x - B1x;
  float Lby = B2y - B1y;
  float Lbz = B2z - B1z;
  // From Equation (15)
  float L11 = (Lax * Lax) + (Lay * Lay) + (Laz * Laz);
  float L22 = (Lbx * Lbx) + (Lby * Lby) + (Lbz * Lbz);

  // Line/Segment A is degenerate ---- Special Case #1
  if (L11 < epsilon_squared) {
    PointOnSegAx = A1x;
    PointOnSegAy = A1y;
    PointOnSegAz = A1z;
    FindNearestPointOnLineSegment(B1x, B1y, B1z, Lbx, Lby, Lbz, A1x, A1y, A1z,
                                  infinite_lines, epsilon, PointOnSegBx,
                                  PointOnSegBy, PointOnSegBz, temp);
  }
  // Line/Segment B is degenerate ---- Special Case #1
  else if (L22 < epsilon_squared) {
    PointOnSegBx = B1x;
    PointOnSegBy = B1y;
    PointOnSegBz = B1z;
    FindNearestPointOnLineSegment(A1x, A1y, A1z, Lax, Lay, Laz, B1x, B1y, B1z,
                                  infinite_lines, epsilon, PointOnSegAx,
                                  PointOnSegAy, PointOnSegAz, temp);
  }
  // Neither line/segment is degenerate
  else {
    // Compute more parameters from Equation (3) in the text.
    float ABx = B1x - A1x;
    float ABy = B1y - A1y;
    float ABz = B1z - A1z;

    // and from Equation (15).
    float L12 = -(Lax * Lbx) - (Lay * Lby) - (Laz * Lbz);

    float DetL = L11 * L22 - L12 * L12;
    // Lines/Segments A and B are parallel ---- special case #2.
    if (FABS(DetL) < epsilon) {
      FindNearestPointOfParallelLineSegments(
          A1x, A1y, A1z, A2x, A2y, A2z, Lax, Lay, Laz, B1x, B1y, B1z, B2x, B2y,
          B2z, Lbx, Lby, Lbz, infinite_lines, epsilon, PointOnSegAx,
          PointOnSegAy, PointOnSegAz, PointOnSegBx, PointOnSegBy, PointOnSegBz);
    }
    // The general case
    else {
      // from Equation (15)
      float ra = Lax * ABx + Lay * ABy + Laz * ABz;
      float rb = -Lbx * ABx - Lby * ABy - Lbz * ABz;

      float t = (L11 * rb - ra * L12) / DetL; // Equation (12)

#ifdef USE_CRAMERS_RULE
      float s = (L22 * ra - rb * L12) / DetL;
#else
      float s = (ra - L12 * t) / L11; // Equation (13)
#endif // USE_CRAMERS_RULE

#ifdef CHECK_ANSWERS
      float check_ra = s * L11 + t * L12;
      float check_rb = s * L12 + t * L22;
      assert(FABS(check_ra - ra) < epsilon);
      assert(FABS(check_rb - rb) < epsilon);
#endif // CHECK_ANSWERS

      // if we are dealing with infinite lines or if parameters s and t both
      // lie in the range [0,1] then just compute the points using Equations
      // (1) and (2) from the text.
      PointOnSegAx = (A1x + s * Lax);
      PointOnSegAy = (A1y + s * Lay);
      PointOnSegAz = (A1z + s * Laz);
      PointOnSegBx = (B1x + t * Lbx);
      PointOnSegBy = (B1y + t * Lby);
      PointOnSegBz = (B1z + t * Lbz);
      // otherwise, at least one of s and t is outside of [0,1] and we have to
      // handle this case.
      if (false == infinite_lines && (OUT_OF_RANGE(s) || OUT_OF_RANGE(t))) {
        AdjustNearestPoints(A1x, A1y, A1z, Lax, Lay, Laz, B1x, B1y, B1z, Lbx,
                            Lby, Lbz, epsilon, s, t, PointOnSegAx, PointOnSegAy,
                            PointOnSegAz, PointOnSegBx, PointOnSegBy,
                            PointOnSegBz);
      }
    }
  }

  NearestPointX = 0.5f * (PointOnSegAx + PointOnSegBx);
  NearestPointY = 0.5f * (PointOnSegAy + PointOnSegBy);
  NearestPointZ = 0.5f * (PointOnSegAz + PointOnSegBz);

  NearestVectorX = PointOnSegBx - PointOnSegAx;
  NearestVectorY = PointOnSegBy - PointOnSegAy;
  NearestVectorZ = PointOnSegBz - PointOnSegAz;

  // optional check to indicate if the lines truly intersect
  true_intersection = (FABS(NearestVectorX) + FABS(NearestVectorY) +
                       FABS(NearestVectorZ)) < epsilon
                          ? true
                          : false;
}

/**************************************************************************
|
|     Method: FindNearestPointOnLineSegment
|
|    Purpose: Given a line (segment) and a point in 3-dimensional space,
|             find the point on the line (segment) that is closest to the
|             point.
|
| Parameters: Input:
|             ------
|             A1x, A1y, A1z   - Coordinates of first defining point of the
line/segment |             Lx, Ly, Lz      - Vector from (A1x, A1y, A1z) to the
second defining point |                               of the line/segment. | Bx,
By, Bz      - Coordinates of the point |             infinite_lines  - set to
true if lines are to be treated as infinite |             epsilon_squared -
tolerance value to be used to check for degenerate | and parallel lines, and to
check for true intersection.
|
|             Output:
|             -------
|             NearestPointX,  - Point on line/segment that is closest to (Bx,
By, Bz) |             NearestPointY, |             NearestPointZ | parameter -
Parametric coordinate of the nearest point along the | line/segment. parameter =
0 at (A1x, A1y, A1z) and |                               parameter = 1 at the
second defining point of the line/ |                               segmetn
**************************************************************************/
void FindNearestPointOnLineSegment(const float A1x, const float A1y,
                                   const float A1z, const float Lx,
                                   const float Ly, const float Lz,
                                   const float Bx, const float By,
                                   const float Bz, bool infinite_line,
                                   float epsilon_squared, float &NearestPointX,
                                   float &NearestPointY, float &NearestPointZ,
                                   float &parameter) {
  // Line/Segment is degenerate --- special case #1
  float D = Lx * Lx + Ly * Ly + Lz * Lz;
  if (D < epsilon_squared) {
    NearestPointX = A1x;
    NearestPointY = A1y;
    NearestPointZ = A1z;
    return;
  }

  float ABx = Bx - A1x;
  float ABy = By - A1y;
  float ABz = Bz - A1z;

  // parameter is computed from Equation (20).
  parameter = (Lx * ABx + Ly * ABy + Lz * ABz) / D;

  if (false == infinite_line)
    parameter = FMAX(0.0f, FMIN(1.0f, parameter));

  NearestPointX = A1x + parameter * Lx;
  NearestPointY = A1y + parameter * Ly;
  NearestPointZ = A1z + parameter * Lz;
  return;
}

/**************************************************************************
|
|     Method: FindNearestPointOfParallelLineSegments
|
|    Purpose: Given two lines (segments) that are known to be parallel, find
|             a representative point on each that is nearest to the other. If
|             the lines are considered to be finite then it is possible that
there |             is one true point on each line that is nearest to the other.
This |             code properly handles this case.
|
|             This is the most difficult line intersection case to handle, since
|             there is potentially a family, or locus of points on each
line/segment |             that are nearest to the other. | Parameters: Input:
|             ------
|             A1x, A1y, A1z   - Coordinates of first defining point of
line/segment A |             A2x, A2y, A2z   - Coordinates of second defining
point of line/segment A |             Lax, Lay, Laz   - Vector from (A1x, A1y,
A1z) to the (A2x, A2y, A2z). |             B1x, B1y, B1z   - Coordinates of
first defining point of line/segment B |             B2x, B2y, B2z   -
Coordinates of second defining point of line/segment B |             Lbx, Lby,
Lbz   - Vector from (B1x, B1y, B1z) to the (B2x, B2y, B2z). | infinite_lines  -
set to true if lines are to be treated as infinite |             epsilon_squared
- tolerance value to be used to check for degenerate | and parallel lines, and
to check for true intersection.
|
|             Output:
|             -------
|             PointOnSegAx,   - Coordinates of the point on segment A that are
nearest |             PointOnSegAy,     to segment B. This corresponds to point
C in the text. |             PointOnSegAz |             PointOnSegBx,   -
Coordinates of the point on segment B that are nearest | PointOnSegBy,     to
segment A. This corresponds to point D in the text. |             PointOnSegBz

**************************************************************************/
void FindNearestPointOfParallelLineSegments(
    float A1x, float A1y, float A1z, float A2x, float A2y, float A2z, float Lax,
    float Lay, float Laz, float B1x, float B1y, float B1z, float B2x, float B2y,
    float B2z, float Lbx, float Lby, float Lbz, bool infinite_lines,
    float epsilon_squared, float &PointOnSegAx, float &PointOnSegAy,
    float &PointOnSegAz, float &PointOnSegBx, float &PointOnSegBy,
    float &PointOnSegBz) {
  float s[2], temp;
  FindNearestPointOnLineSegment(A1x, A1y, A1z, Lax, Lay, Laz, B1x, B1y, B1z,
                                true, epsilon_squared, PointOnSegAx,
                                PointOnSegAy, PointOnSegAz, s[0]);
  if (true == infinite_lines) {
    PointOnSegBx = B1x;
    PointOnSegBy = B1y;
    PointOnSegBz = B1z;
  } else {
    float tp[3];
    FindNearestPointOnLineSegment(A1x, A1y, A1z, Lax, Lay, Laz, B2x, B2y, B2z,
                                  true, epsilon_squared, tp[0], tp[1], tp[2],
                                  s[1]);
    if (s[0] < 0.f && s[1] < 0.f) {
      PointOnSegAx = A1x;
      PointOnSegAy = A1y;
      PointOnSegAz = A1z;
      if (s[0] < s[1]) {
        PointOnSegBx = B2x;
        PointOnSegBy = B2y;
        PointOnSegBz = B2z;
      } else {
        PointOnSegBx = B1x;
        PointOnSegBy = B1y;
        PointOnSegBz = B1z;
      }
    } else if (s[0] > 1.f && s[1] > 1.f) {
      PointOnSegAx = A2x;
      PointOnSegAy = A2y;
      PointOnSegAz = A2z;
      if (s[0] < s[1]) {
        PointOnSegBx = B1x;
        PointOnSegBy = B1y;
        PointOnSegBz = B1z;
      } else {
        PointOnSegBx = B2x;
        PointOnSegBy = B2y;
        PointOnSegBz = B2z;
      }
    } else {
      temp =
          0.5f * (FMAX(0.0f, FMIN(1.0f, s[0])) + FMAX(0.0f, FMIN(1.0f, s[1])));
      PointOnSegAx = (A1x + temp * Lax);
      PointOnSegAy = (A1y + temp * Lay);
      PointOnSegAz = (A1z + temp * Laz);
      FindNearestPointOnLineSegment(B1x, B1y, B1z, Lbx, Lby, Lbz, PointOnSegAx,
                                    PointOnSegAy, PointOnSegAz, true,
                                    epsilon_squared, PointOnSegBx, PointOnSegBy,
                                    PointOnSegBz, temp);
    }
  }
}

/**************************************************************************
|
|     Method: AdjustNearestPoints
|
|    Purpose: Given nearest point information for two infinite lines, adjust
|             to model finite line segments.
|
| Parameters: Input:
|             ------
|             A1x, A1y, A1z   - Coordinates of first defining point of
line/segment A |             Lax, Lay, Laz   - Vector from (A1x, A1y, A1z) to
the (A2x, A2y, A2z). |             B1x, B1y, B1z   - Coordinates of first
defining point of line/segment B |             Lbx, Lby, Lbz   - Vector from
(B1x, B1y, B1z) to the (B2x, B2y, B2z). |             epsilon_squared -
tolerance value to be used to check for degenerate | and parallel lines, and to
check for true intersection. |             s               - parameter
representing nearest point on infinite line A |             t               -
parameter representing nearest point on infinite line B
|
|             Output:
|             -------
|             PointOnSegAx,   - Coordinates of the point on segment A that are
nearest |             PointOnSegAy,     to segment B. This corresponds to point
C in the text. |             PointOnSegAz |             PointOnSegBx,   -
Coordinates of the point on segment B that are nearest | PointOnSegBy,     to
segment A. This corresponds to point D in the text. |             PointOnSegBz
**************************************************************************/
void AdjustNearestPoints(float A1x, float A1y, float A1z, float Lax, float Lay,
                         float Laz, float B1x, float B1y, float B1z, float Lbx,
                         float Lby, float Lbz, float epsilon_squared, float s,
                         float t, float &PointOnSegAx, float &PointOnSegAy,
                         float &PointOnSegAz, float &PointOnSegBx,
                         float &PointOnSegBy, float &PointOnSegBz) {
  // handle the case where both parameter s and t are out of range
  if (OUT_OF_RANGE(s) && OUT_OF_RANGE(t)) {
    s = FMAX(0.0f, FMIN(1.0f, s));
    PointOnSegAx = (A1x + s * Lax);
    PointOnSegAy = (A1y + s * Lay);
    PointOnSegAz = (A1z + s * Laz);
    FindNearestPointOnLineSegment(
        B1x, B1y, B1z, Lbx, Lby, Lbz, PointOnSegAx, PointOnSegAy, PointOnSegAz,
        true, epsilon_squared, PointOnSegBx, PointOnSegBy, PointOnSegBz, t);
    if (OUT_OF_RANGE(t)) {
      t = FMAX(0.0f, FMIN(1.0f, t));
      PointOnSegBx = (B1x + t * Lbx);
      PointOnSegBy = (B1y + t * Lby);
      PointOnSegBz = (B1z + t * Lbz);
      FindNearestPointOnLineSegment(A1x, A1y, A1z, Lax, Lay, Laz, PointOnSegBx,
                                    PointOnSegBy, PointOnSegBz, false,
                                    epsilon_squared, PointOnSegAx, PointOnSegAy,
                                    PointOnSegAz, s);
      FindNearestPointOnLineSegment(B1x, B1y, B1z, Lbx, Lby, Lbz, PointOnSegAx,
                                    PointOnSegAy, PointOnSegAz, false,
                                    epsilon_squared, PointOnSegBx, PointOnSegBy,
                                    PointOnSegBz, t);
    }
  }
  // otherwise, handle the case where the parameter for only one segment is
  // out of range
  else if (OUT_OF_RANGE(s)) {
    s = FMAX(0.0f, FMIN(1.0f, s));
    PointOnSegAx = (A1x + s * Lax);
    PointOnSegAy = (A1y + s * Lay);
    PointOnSegAz = (A1z + s * Laz);
    FindNearestPointOnLineSegment(
        B1x, B1y, B1z, Lbx, Lby, Lbz, PointOnSegAx, PointOnSegAy, PointOnSegAz,
        false, epsilon_squared, PointOnSegBx, PointOnSegBy, PointOnSegBz, t);
  } else if (OUT_OF_RANGE(t)) {
    t = FMAX(0.0f, FMIN(1.0f, t));
    PointOnSegBx = (B1x + t * Lbx);
    PointOnSegBy = (B1y + t * Lby);
    PointOnSegBz = (B1z + t * Lbz);
    FindNearestPointOnLineSegment(
        A1x, A1y, A1z, Lax, Lay, Laz, PointOnSegBx, PointOnSegBy, PointOnSegBz,
        false, epsilon_squared, PointOnSegAx, PointOnSegAy, PointOnSegAz, s);
  } else {
    assert(0);
  }
}
