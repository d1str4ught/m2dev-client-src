#include "StdAfx.h"
#include "EterLib/lineintersect_utils.h"

#include "GameUtil.h"

bool DetectCollisionDynamicZCylinderVSDynamicZCylinder(
    const CDynamicSphereInstance &c_rSphere1,
    const CDynamicSphereInstance &c_rSphere2) {
  CDynamicSphereInstance c_rCylinder1 = c_rSphere1;
  CDynamicSphereInstance c_rCylinder2 = c_rSphere2;

  c_rCylinder1.v3Position.z = 0;
  c_rCylinder1.v3LastPosition.z = 0;

  c_rCylinder2.v3Position.z = 0;
  c_rCylinder2.v3LastPosition.z = 0;

  float r = c_rCylinder1.fRadius + c_rCylinder2.fRadius;
  float rsq = r * r;

  // AABB check
  D3DXVECTOR3 mi1 = c_rCylinder1.v3LastPosition, mi2 = c_rCylinder1.v3Position;
  D3DXVECTOR3 mi3 = c_rCylinder2.v3LastPosition, mi4 = c_rCylinder2.v3Position;
  if (mi1.x > mi2.x)
    std::swap(mi1.x, mi2.x);
  if (mi1.y > mi2.y)
    std::swap(mi1.y, mi2.y);
  if (mi1.z > mi2.z)
    std::swap(mi1.z, mi2.z);
  if (mi3.x > mi4.x)
    std::swap(mi3.x, mi4.x);
  if (mi3.y > mi4.y)
    std::swap(mi3.y, mi4.y);
  if (mi3.z > mi4.z)
    std::swap(mi3.z, mi4.z);
  mi1.x -= c_rCylinder1.fRadius;
  mi1.y -= c_rCylinder1.fRadius;
  mi1.z -= c_rCylinder1.fRadius;
  mi2.x += c_rCylinder1.fRadius;
  mi2.y += c_rCylinder1.fRadius;
  mi2.z += c_rCylinder1.fRadius;
  mi3.x -= c_rCylinder2.fRadius;
  mi3.y -= c_rCylinder2.fRadius;
  mi3.z -= c_rCylinder2.fRadius;
  mi4.x += c_rCylinder2.fRadius;
  mi4.y += c_rCylinder2.fRadius;
  mi4.z += c_rCylinder2.fRadius;
  if (mi4.x < mi1.x || mi2.x < mi3.x)
    return false;
  if (mi4.y < mi1.y || mi2.y < mi3.y)
    return false;
  if (mi4.z < mi1.z || mi2.z < mi3.z)
    return false;

  D3DXVECTOR3 vA, vB;
  IntersectLineSegments(c_rCylinder1.v3LastPosition, c_rCylinder1.v3Position,
                        c_rCylinder2.v3LastPosition, c_rCylinder2.v3Position,
                        vA, vB);
  const auto vv = (vA - vB);
  return (D3DXVec3LengthSq(&vv) <= rsq);
}

bool DetectCollisionDynamicSphereVSDynamicSphere(
    const CDynamicSphereInstance &c_rSphere1,
    const CDynamicSphereInstance &c_rSphere2) {
  float r = c_rSphere1.fRadius + c_rSphere2.fRadius;
  float rsq = r * r;

  // using gpg line-collision

  // AABB check

  D3DXVECTOR3 mi1 = c_rSphere1.v3LastPosition, mi2 = c_rSphere1.v3Position;
  D3DXVECTOR3 mi3 = c_rSphere2.v3LastPosition, mi4 = c_rSphere2.v3Position;
  if (mi1.x > mi2.x)
    std::swap(mi1.x, mi2.x);
  if (mi1.y > mi2.y)
    std::swap(mi1.y, mi2.y);
  if (mi1.z > mi2.z)
    std::swap(mi1.z, mi2.z);
  if (mi3.x > mi4.x)
    std::swap(mi3.x, mi4.x);
  if (mi3.y > mi4.y)
    std::swap(mi3.y, mi4.y);
  if (mi3.z > mi4.z)
    std::swap(mi3.z, mi4.z);
  mi1.x -= c_rSphere1.fRadius;
  mi1.y -= c_rSphere1.fRadius;
  mi1.z -= c_rSphere1.fRadius;
  mi2.x += c_rSphere1.fRadius;
  mi2.y += c_rSphere1.fRadius;
  mi2.z += c_rSphere1.fRadius;
  mi3.x -= c_rSphere2.fRadius;
  mi3.y -= c_rSphere2.fRadius;
  mi3.z -= c_rSphere2.fRadius;
  mi4.x += c_rSphere2.fRadius;
  mi4.y += c_rSphere2.fRadius;
  mi4.z += c_rSphere2.fRadius;
  if (mi4.x < mi1.x || mi2.x < mi3.x)
    return false;
  if (mi4.y < mi1.y || mi2.y < mi3.y)
    return false;
  if (mi4.z < mi1.z || mi2.z < mi3.z)
    return false;

  D3DXVECTOR3 vA, vB;
  IntersectLineSegments(c_rSphere1.v3LastPosition, c_rSphere1.v3Position,
                        c_rSphere2.v3LastPosition, c_rSphere2.v3Position, vA,
                        vB);
  const auto vvv = (vA - vB);
  return (D3DXVec3LengthSq(&vvv) <= rsq);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool IsCWAcuteAngle(float begin, float end) {
  // 360 - src + dest		// 시계 반대 방향
  // src - dest			// 시계 방향
  return ((360.0f - begin + end) > (begin - end));
}

bool IsCCWAcuteAngle(float begin, float end) {
  // abs(360 - dest + src) 	// 시계 방향
  // dest - src				// 시계 반대 방향
  int fValue = abs((int)(360.0f - end + begin));
  return fValue >= (end - begin) ? true : false;
}

bool IsCWRotation(float begin, float end) { return !IsCCWRotation(begin, end); }

bool IsCCWRotation(float begin, float end) {
  //     180
  // 270      90
  //      0
  //
  // 시계 반대
  return (begin - end < 0);
}

float GetInterpolatedRotation(float begin, float end, float curRate) {
  if (IsCCWRotation(begin, end)) {
    if (IsCCWAcuteAngle(begin, end))
      return GetLinearInterpolation(begin, end, curRate);

    return (360.0f + GetLinearInterpolation(begin, end - 360.0f, curRate));
  }

  if (IsCWAcuteAngle(begin, end))
    return GetLinearInterpolation(begin, end, curRate);

  return GetLinearInterpolation(begin, end + 360.0f, curRate);
}

float GetDegreeFromPosition(float x, float y) {
  D3DXVECTOR3 vtDir(floor(x), floor(y), 0.0f);
  D3DXVec3Normalize(&vtDir, &vtDir);

  D3DXVECTOR3 vtStan(0, -1, 0);
  float ret = D3DXToDegree(acosf(D3DXVec3Dot(&vtDir, &vtStan)));

  if (vtDir.x < 0.0f)
    ret = 360.0f - ret;

  return ret;
}

float GetDegreeFromPosition2(float sx, float sy, float ex, float ey) {
  return GetDegreeFromPosition(ex - sx, ey - sy);
}

float GetDegreeDifference(float fSource, float fTarget) {
  if (fSource < 180.0f) {
    if (fTarget < fSource)
      return fSource - fTarget;

    else if (fTarget - fSource > 180.0f)
      return (360.0f - (fTarget - fSource));

    return fTarget - fSource;
  } else {
    if (fTarget > fSource)
      return fTarget - fSource;

    else if (fSource - fTarget > 180.0f) {
      return (360.0f - (fSource - fTarget));
    }

    return fSource - fTarget;
  }
}
int GetRotatingDirection(float fSource, float fTarget) {
  if (fSource < 180.0f) {
    if (fTarget < fSource)
      return DEGREE_DIRECTION_RIGHT;

    else if ((360.0f - fTarget) + fSource < 180.0f)
      return DEGREE_DIRECTION_RIGHT;

    return DEGREE_DIRECTION_LEFT;
  } else {
    if (fTarget > fSource) {
      return DEGREE_DIRECTION_LEFT;
    } else if ((360.0f - fSource) + fTarget < 180.0f) {
      return DEGREE_DIRECTION_LEFT;
    }

    return DEGREE_DIRECTION_RIGHT;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

float CameraRotationToCharacterRotation(float fCameraRotation) {
  return fmod((540.0f - fCameraRotation), 360.0f);
}

float CharacterRotationToCameraRotation(float fCharacterRotation) {
  return fmod((540.0f - fCharacterRotation), 360.0f);
}
