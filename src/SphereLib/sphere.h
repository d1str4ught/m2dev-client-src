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

#include "vector.h"

/***********************************************************************/
/** SPHERE.H    : Base class to represent a Sphere in 3 space.         */
/**                                                                    */
/**               Written by John W. Ratcliff jratcliff@att.net        */
/***********************************************************************/

class SphereInterface {
public:
  SphereInterface();
  virtual ~SphereInterface();
  virtual int GetVertexCount(void) const = 0;
  virtual bool GetVertex(int i, Vector3d &vect) const = 0;

private:
};

class Sphere {
public:
  Sphere();
  Sphere(const Vector3d &center, float radius);

  virtual ~Sphere() {}

  void Set(const Vector3d &center, float radius);

  void Compute(const SphereInterface &source);

  float GetRadius(void) const { return mRadius; };
  float GetRadius2(void) const { return mRadius2; };
  const Vector3d &GetCenter(void) const { return mCenter; };

  bool RayIntersection(const Vector3d &rayOrigin, const Vector3d &V,
                       float distance, Vector3d *intersect);

  bool RayIntersection(const Vector3d &rayOrigin, const Vector3d &rayDirection,
                       Vector3d *intersect);

  bool RayIntersectionInFront(const Vector3d &rayOrigin,
                              const Vector3d &rayDirection,
                              Vector3d *intersect);

  void Report(void);

  void SetRadius(float radius) {
    mRadius = radius;
    mRadius2 = radius * radius;
  }

  bool InSphereXY(const Vector3d &pos, float distance) const {
    float dx = pos.x - mCenter.x;
    float dy = pos.y - mCenter.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < (mRadius + distance))
      return true;
    return false;
  };

  bool InSphere(const Vector3d &pos, float distance) const {
    float dx = pos.x - mCenter.x;
    float dy = pos.y - mCenter.y;
    float dz = pos.z - mCenter.z;

    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist < (mRadius + distance))
      return true;
    return false;
  }

protected:
  Vector3d mCenter;

private:
  float mRadius;
  float mRadius2; // radius squared.
};

inline Sphere::Sphere()
    : mCenter(Vector3d(0.0f, 0.0f, 0.0f)), mRadius(0.0f), mRadius2(0.0f) {};

inline Sphere::Sphere(const Vector3d &center, float radius)
    : mCenter(center), mRadius(radius), mRadius2(radius * radius) {};
