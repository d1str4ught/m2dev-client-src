/* Copyright (C) John W. Ratcliff, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) John W. Ratcliff, 2001"
 */

#include "Stdafx.h"
#include "sphere.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

bool Vector3d::IsInStaticRange() const {
  const float LIMIT = 3276700.0f;
  if (x < LIMIT && x > -LIMIT)
    if (y < LIMIT && y > -LIMIT)
      if (z < LIMIT && z > -LIMIT)
        return true;

  return false;
}

void Sphere::Set(const Vector3d &center, float radius) {
#ifdef __STATIC_RANGE__
  assert(center.IsInStaticRange());
#endif
  mCenter = center;
  mRadius = radius;
  mRadius2 = radius * radius;
}

// ray-sphere intersection test from Graphics Gems p.388
//  **NOTE** There is a bug in this Graphics Gem.  If the origin
//  of the ray is *inside* the sphere being tested, it reports the
//  wrong intersection location.  This code has a fix for the bug.
bool Sphere::RayIntersection(const Vector3d &rayOrigin, const Vector3d &dir,
                             Vector3d *intersect) {
  // notation:
  // point E  = rayOrigin
  // point O  = sphere center

  Vector3d EO = mCenter - rayOrigin;
  Vector3d V = dir;
  float dist2 = EO.x * EO.x + EO.y * EO.y + EO.z * EO.z;
  // Bug Fix For Gem, if origin is *inside* the sphere, invert the
  // direction vector so that we get a valid intersection location.
  if (dist2 < mRadius2)
    V *= -1;

  float v = EO.Dot(V);

  float disc = mRadius2 - (EO.Length2() - v * v);

  if (disc > 0.0f) {

    if (intersect) {

      float d = sqrtf(disc);

      // float dist2 = rayOrigin.DistanceSq(mCenter);

      *intersect = rayOrigin + V * (v - d);
    }

    return true;
  }
  return false;
}

//
bool Sphere::RayIntersection(const Vector3d &rayOrigin, const Vector3d &V,
                             float distance, Vector3d *intersect) {
  Vector3d sect;
  bool hit = RayIntersectionInFront(rayOrigin, V, &sect);

  if (hit) {
    float d = rayOrigin.DistanceSq(sect);
    if (d > (distance * distance))
      return false;
    if (intersect)
      *intersect = sect;
    return true;
  }
  return false;
}

bool Sphere::RayIntersectionInFront(const Vector3d &rayOrigin,
                                    const Vector3d &V, Vector3d *intersect) {
  Vector3d sect;
  bool hit = RayIntersection(rayOrigin, V, &sect);

  if (hit) {

    Vector3d dir = sect - rayOrigin;

    float dot = dir.Dot(V);

    if (dot >= 0) // then it's in front!
    {
      if (intersect)
        *intersect = sect;
      return true;
    }
  }
  return false;
}

void Sphere::Report(void) {}

/*
An Efficient Bounding Sphere
by Jack Ritter
from "Graphics Gems", Academic Press, 1990
*/

/* Routine to calculate tight bounding sphere over    */
/* a set of points in 3D */
/* This contains the routine find_bounding_sphere(), */
/* the struct definition, and the globals used for parameters. */
/* The abs() of all coordinates must be < BIGNUMBER */
/* Code written by Jack Ritter and Lyle Rains. */

#define BIGNUMBER 100000000.0 /* hundred million */

void Sphere::Compute(const SphereInterface &source) {

  Vector3d xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2;

  /* FIRST PASS: find 6 minima/maxima points */
  xmin.Set(BIGNUMBER, BIGNUMBER, BIGNUMBER);
  xmax.Set(-BIGNUMBER, -BIGNUMBER, -BIGNUMBER);
  ymin.Set(BIGNUMBER, BIGNUMBER, BIGNUMBER);
  ymax.Set(-BIGNUMBER, -BIGNUMBER, -BIGNUMBER);
  zmin.Set(BIGNUMBER, BIGNUMBER, BIGNUMBER);
  zmax.Set(-BIGNUMBER, -BIGNUMBER, -BIGNUMBER);

  int count = source.GetVertexCount();

  for (int i = 0; i < count; i++) {
    Vector3d caller_p;
    source.GetVertex(i, caller_p);

    if (caller_p.GetX() < xmin.GetX())
      xmin = caller_p; /* New xminimum point */
    if (caller_p.GetX() > xmax.GetX())
      xmax = caller_p;
    if (caller_p.GetY() < ymin.GetY())
      ymin = caller_p;
    if (caller_p.GetY() > ymax.GetY())
      ymax = caller_p;
    if (caller_p.GetZ() < zmin.GetZ())
      zmin = caller_p;
    if (caller_p.GetZ() > zmax.GetZ())
      zmax = caller_p;
  }

  /* Set xspan = distance between the 2 points xmin & xmax (squared) */
  float dx = xmax.GetX() - xmin.GetX();
  float dy = xmax.GetY() - xmin.GetY();
  float dz = xmax.GetZ() - xmin.GetZ();
  float xspan = dx * dx + dy * dy + dz * dz;

  /* Same for y & z spans */
  dx = ymax.GetX() - ymin.GetX();
  dy = ymax.GetY() - ymin.GetY();
  dz = ymax.GetZ() - ymin.GetZ();
  float yspan = dx * dx + dy * dy + dz * dz;

  dx = zmax.GetX() - zmin.GetX();
  dy = zmax.GetY() - zmin.GetY();
  dz = zmax.GetZ() - zmin.GetZ();
  float zspan = dx * dx + dy * dy + dz * dz;

  /* Set points dia1 & dia2 to the maximally separated pair */
  dia1 = xmin;
  dia2 = xmax; /* assume xspan biggest */
  float maxspan = xspan;

  if (yspan > maxspan) {
    maxspan = yspan;
    dia1 = ymin;
    dia2 = ymax;
  }

  if (zspan > maxspan) {
    dia1 = zmin;
    dia2 = zmax;
  }

  /* dia1,dia2 is a diameter of initial sphere */
  /* calc initial center */
  mCenter.SetX((dia1.GetX() + dia2.GetX()) * 0.5f);
  mCenter.SetY((dia1.GetY() + dia2.GetY()) * 0.5f);
  mCenter.SetZ((dia1.GetZ() + dia2.GetZ()) * 0.5f);
  /* calculate initial radius**2 and radius */
  dx = dia2.GetX() - mCenter.GetX(); /* x component of radius vector */
  dy = dia2.GetY() - mCenter.GetY(); /* y component of radius vector */
  dz = dia2.GetZ() - mCenter.GetZ(); /* z component of radius vector */
  mRadius2 = dx * dx + dy * dy + dz * dz;
  mRadius = float(sqrt(mRadius2));

  /* SECOND PASS: increment current sphere */

  for (int j = 0; j < count; j++) {
    Vector3d caller_p;
    source.GetVertex(j, caller_p);
    dx = caller_p.GetX() - mCenter.GetX();
    dy = caller_p.GetY() - mCenter.GetY();
    dz = caller_p.GetZ() - mCenter.GetZ();
    float old_to_p_sq = dx * dx + dy * dy + dz * dz;
    if (old_to_p_sq > mRadius2) /* do r**2 test first */
    {                           /* this point is outside of current sphere */
      float old_to_p = float(sqrt(old_to_p_sq));
      /* calc radius of new sphere */
      mRadius = (mRadius + old_to_p) * 0.5f;
      mRadius2 = mRadius * mRadius; /* for next r**2 compare */
      float old_to_new = old_to_p - mRadius;
      /* calc center of new sphere */
      float recip = 1.0f / old_to_p;

      float cx =
          (mRadius * mCenter.GetX() + old_to_new * caller_p.GetX()) * recip;
      float cy =
          (mRadius * mCenter.GetY() + old_to_new * caller_p.GetY()) * recip;
      float cz =
          (mRadius * mCenter.GetZ() + old_to_new * caller_p.GetZ()) * recip;

      mCenter.Set(cx, cy, cz);
    }
  }
}
