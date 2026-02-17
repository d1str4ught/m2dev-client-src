#pragma once

//   Collision Detection
// Dynamic VS Dynamic
bool DetectCollisionDynamicSphereVSDynamicSphere(
    const CDynamicSphereInstance &c_rSphere1,
    const CDynamicSphereInstance &c_rSphere2);
bool DetectCollisionDynamicZCylinderVSDynamicZCylinder(
    const CDynamicSphereInstance &c_rSphere1,
    const CDynamicSphereInstance &c_rSphere2);
// Dynamic VS Static
// bool DetectCollisionDynamicSphereVSStaticPlane(const CDynamicSphereInstance &
// c_rSphere, const TPlaneData & c_rPlaneData); bool
// DetectCollisionDynamicSphereVSStaticSphere(const CDynamicSphereInstance &
// c_rSphere, const TSphereData & c_rSphereData); bool
// DetectCollisionDynamicSphereVSStaticCylinder(const CDynamicSphereInstance &
// c_rSphere, const TCylinderData & c_rCylinderData); bool
// DetectCollisionDynamicSphereVSStaticBox(const TSphereInstance & c_rSphere,
// const TBoxData & c_rBoxData);
// Static VS Static
// bool DetectCollisionStaticSphereVSStaticSphere(const CDynamicSphereInstance &
// c_rSphere1, const TSphereData & c_rSphere2); bool
// DetectCollisionStaticSphereVSStaticCylinder(const CDynamicSphereInstance &
// c_rSphere, const TCylinderData & c_rCylinder); bool
// DetectCollisionStaticSphereVSStaticBox(const TSphereData & c_rSphere, const
// TBoxData & c_rBox);

// Rotation
float GetDegreeFromPosition(float x, float y);
float GetDegreeFromPosition2(float sx, float sy, float ex, float ey);

float GetInterpolatedRotation(float begin, float end, float curRate);
bool IsSameDirectionRotation();
bool IsCWRotation(float begin, float end);
bool IsCCWRotation(float begin, float end);

bool IsCWAcuteAngle(float begin, float end);
bool IsCCWAcuteAngle(float begin, float end);

// NOTE - Finally, this code is same with upper rotating direction code.
enum EDegree_Direction {
  DEGREE_DIRECTION_SAME = 0,
  DEGREE_DIRECTION_RIGHT = 1,
  DEGREE_DIRECTION_LEFT = 2,
};
float GetDegreeDifference(float fSource, float fTarget);
int GetRotatingDirection(float fSource, float fTarget);

// Converting Rotation
float CameraRotationToCharacterRotation(float fCameraRotation);
float CharacterRotationToCameraRotation(float fCharacterRotation);
