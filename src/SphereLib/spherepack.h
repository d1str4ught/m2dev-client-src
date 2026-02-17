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

/***********************************************************************/
/** SPHEREPACK.H: Contains all of the support code for Sphere Trees.   */
/**                                                                    */
/**               Written by John W. Ratcliff jratcliff@att.net        */
/***********************************************************************/

#include <assert.h>

#include "vector.h"  // need 3d vector support.
#include "pool.h"    // need memory pool template class.
#include "sphere.h"  // Need sphere base class.
#include "frustum.h" // Need to know what a frustum is.

#define DEMO 0 // true if running in windows test app

enum SpherePackFlag {

  SPF_SUPERSPHERE =
      (1 << 0), // this is a supersphere, allocated and deleted by us
  SPF_ROOT_TREE = (1 << 1), // member of the root tree
  SPF_LEAF_TREE = (1 << 2), // member of the leaf node tree
  SPF_ROOTNODE = (1 << 3),  // this is the root node
  SPF_RECOMPUTE = (1 << 4), // needs recomputed bounding sphere
  SPF_INTEGRATE = (1 << 5), // needs to be reintegrated into tree
  // Frame-to-frame view frustum status.  Only does callbacks when a
  // state change occurs.
  SPF_HIDDEN = (1 << 6),  // outside of view frustum
  SPF_PARTIAL = (1 << 7), // partially inside view frustum
  SPF_INSIDE = (1 << 8)   // completely inside view frustum
};

class SpherePackFactory; // forward reference the factory.
class SpherePack;        // forward reference the sphere package

#pragma warning(push)
#pragma warning(disable : 4100)

// Virtual base class, used to implement callbacks for RayTracing,
// range testing, and frustum culling.
class SpherePackCallback {
public:
  SpherePackCallback() {}
  virtual ~SpherePackCallback() {}

  virtual void
  VisibilityCallback(const Frustum &f,   // frustum clipped against
                     SpherePack *sphere, // leaf node sphere in question
                     ViewState state)    // new state it is in.
  {};

  virtual void RayTraceCallback(const Vector3d &p1,   // source pos of ray
                                const Vector3d &dir,  // direction of ray
                                float distance,       // distance of ray
                                const Vector3d &sect, // intersection location
                                SpherePack *sphere)   // sphere ray hit
  {};

  virtual void RangeTestCallback(
      const Vector3d
          &searchpos, // position we are performing range test against.
      float distance, // squared distance we are range searching against.
      SpherePack *sphere,
      ViewState state) // sphere within range, VS_PARTIAL if sphere straddles
                       // range test
  {};

  virtual void
  PointTest2dCallback(const Vector3d &searchpos, // position we are performing
                                                 // range test against.
                      SpherePack *sphere,
                      ViewState state) // sphere within range, VS_PARTIAL if
                                       // sphere straddles range test
  {};

private:
};

#pragma warning(pop)

class SpherePack : public Sphere {
public:
  SpherePack(void) {
    mUserData = 0; // default user data is null
    mFactory = 0;  // factory we are a member of
    mNext = 0;     // linked list pointers
    mPrevious = 0;
    mParent = 0;
    mNextSibling = 0; // our brothers and sisters at this level.
    mPrevSibling = 0;
    mChildren = 0;        // our children.
    mChildCount = 0;      // number of children we have.
    mFifo1 = 0;           // our FIFO1 location if we have one.
    mFifo2 = 0;           // our FIFO2 location if we have one.
    SetRadius(0);         // default radius
    mCenter.Set(0, 0, 0); // default center position.
    IS_SPHERE = false;
  };
  virtual ~SpherePack() {}

  void Init(SpherePackFactory *factory,    // factory we belong to
            const Vector3d &pos,           // center of sphere
            float radius,                  // radius of sphere
            void *userdata, bool isSphere) // user data
  {
    IS_SPHERE = isSphere;
    mUserData = userdata;
    mParent = 0;
    mNextSibling = 0;
    mPrevSibling = 0;
    mFlags = 0;
    mFifo1 = 0;
    mFifo2 = 0;
    mFactory = factory;
    mCenter = pos;
    SetRadius(radius);
  };

  // Access to SpherePack bit flags.
  void SetSpherePackFlag(SpherePackFlag flag) { mFlags |= flag; };
  void ClearSpherePackFlag(SpherePackFlag flag) { mFlags &= ~flag; };
  bool HasSpherePackFlag(SpherePackFlag flag) const {
    if (mFlags & flag)
      return true;
    return false;
  };

  void SetParent(SpherePack *pack) { mParent = pack; };
  SpherePack *GetParent(void) const { return mParent; };

  // Sphere has a new position.
  inline void NewPos(const Vector3d &pos);
  // Sphere has a new position and radius
  inline void NewPosRadius(const Vector3d &pos, float radius);

  void Unlink(void) {
    if (mFifo1) // if we belong to fifo1, null us out
    {
      *mFifo1 = 0;
      mFifo1 = 0;
    }

    if (mFifo2) // if we belong to fifo2, null us out
    {
      *mFifo2 = 0;
      mFifo2 = 0;
    }

    if (mParent)
      mParent->LostChild(this);

    assert(!mChildren); // can't unlink guys with children!

    mParent = 0; // got no father anymore
  }

  void AddChild(SpherePack *pack) {

    SpherePack *my_child = mChildren;
    mChildren = pack; // new head of list

    pack->SetNextSibling(my_child); // his next is my old next
    pack->SetPrevSibling(0);        // at head of list, no previous
    pack->SetParent(this);

    if (my_child)
      my_child->SetPrevSibling(pack); // previous now this..

    mChildCount++;

#if defined(_DEBUG)
    float dist = DistanceSquared(pack);
    float radius = sqrtf(dist) + pack->GetRadius();

    assert(radius <= GetRadius() + 0.0001f);
#endif
  }

  void SetNextSibling(SpherePack *child) { mNextSibling = child; }
  void SetPrevSibling(SpherePack *child) { mPrevSibling = child; }

  SpherePack *_GetNextSibling(void) const { return mNextSibling; }
  SpherePack *_GetPrevSibling(void) const { return mPrevSibling; }
  SpherePack *GetChildren(void) const { return mChildren; }

  SpherePack *GetNext(void) const { return mNext; };
  SpherePack *GetPrevious(void) const { return mPrevious; };

  void SetNext(SpherePack *pack) { mNext = pack; };
  void SetPrevious(SpherePack *pack) { mPrevious = pack; };

  void *GetUserData(void) const { return mUserData; };
  void SetUserData(void *data, bool isSphere) {
    mUserData = data;
    IS_SPHERE = isSphere;
  };

  float DistanceSquared(const SpherePack *pack) const {
    return mCenter.DistanceSq(pack->mCenter);
  };

  void LostChild(SpherePack *pack);

  const Vector3d &GetPos(void) const { return mCenter; };

  inline void Render(unsigned int color);

  inline bool Recompute(float gravy);

  int GetChildCount(void) const { return mChildCount; };

#if DEMO
  void SetColor(unsigned int color) { mColor = color; };
  unsigned int GetColor(void) const { return mColor; };
#endif

  void SetFifo1(SpherePack **fifo) { mFifo1 = fifo; };

  void SetFifo2(SpherePack **fifo) { mFifo2 = fifo; };

  void ComputeBindingDistance(SpherePack *parent) {
    mBindingDistance = parent->GetRadius() - GetRadius();
    if (mBindingDistance <= 0)
      mBindingDistance = 0;
    else
      mBindingDistance *= mBindingDistance;
  }

  void VisibilityTest(const Frustum &f, SpherePackCallback *callback,
                      ViewState state);

  void RayTrace(const Vector3d &p1,  // origin of Ray
                const Vector3d &dir, // direction of Ray
                float distance,      // length of ray.
                SpherePackCallback *callback);

  void RangeTest(const Vector3d &p, float distance,
                 SpherePackCallback *callback, ViewState state);

  void PointTest2d(const Vector3d &p, SpherePackCallback *callback,
                   ViewState state);

  void Reset(void);

private:
  SpherePack *mNext;
  SpherePack *mPrevious; // used by pool memory management linked list code

  SpherePack *mParent;
  SpherePack *mChildren; // *my* children

  SpherePack *mNextSibling; // doubly linked list of my brothers
  SpherePack *mPrevSibling; // and sisters

  SpherePack **mFifo1; // address of location inside of fifo1
  SpherePack **mFifo2; // address of location inside of fifo2

  long mFlags;      // my bit flags.
  long mChildCount; // number of children

  float mBindingDistance;

  void *mUserData;

  SpherePackFactory *mFactory; // the factory we are a member of.
#if DEMO
  unsigned long mColor;
#endif

public:
  bool IS_SPHERE;
};

class SpherePackFifo {
public:
  SpherePackFifo(int fifosize) {
    mCount = 0;
    mSP = 0;
    mBottom = 0;
    mFifoSize = fifosize;
    mFifo = new SpherePack *[mFifoSize];
  };

  virtual ~SpherePackFifo(void) { delete[] mFifo; };

  SpherePack **Push(SpherePack *sphere) {
    mCount++;
    SpherePack **ret = &mFifo[mSP];
    mFifo[mSP] = sphere;
    mSP++;
    if (mSP == mFifoSize)
      mSP = 0;
    return ret;
  };

  SpherePack *Pop(void) {
    while (mSP != mBottom) {
      mCount--;
      SpherePack *ret = mFifo[mBottom];
      mBottom++;
      if (mBottom == mFifoSize)
        mBottom = 0;
      if (ret)
        return ret;
    }
    return 0;
  }

  bool Flush(SpherePack *pack) {
    if (mSP == mBottom)
      return false;
    int i = mBottom;
    while (i != mSP) {
      if (mFifo[i] == pack) {
        mFifo[i] = 0;
        return true;
      }
      i++;
      if (i == mFifoSize)
        i = 0;
    }
    return false;
  };

  int GetCount(void) const { return mCount; };

private:
  int mCount;
  int mSP; // stack pointer
  int mBottom;
  int mFifoSize;
  SpherePack **mFifo;
};

class SpherePackFactory : public SpherePackCallback {
public:
  SpherePackFactory(int maxspheres, float rootsize, float leafsize,
                    float gravy);

  virtual ~SpherePackFactory(void);

  void Process(void);

  SpherePack *AddSphere_(const Vector3d &pos, float radius, void *userdata,
                         bool isSphere, int flags = SPF_LEAF_TREE);

  void AddIntegrate(SpherePack *pack); // add to the integration FIFO
  void AddRecompute(
      SpherePack *recompute); // add to the recomputation (balancing) FIFO.

  void Integrate(SpherePack *pack, SpherePack *supersphere, float node_size);

  void Render(void);

  void Remove(SpherePack *pack);
  // see if any other spheres are contained within this one, if so
  // collapse them and inherit their children.
#if DEMO
  unsigned int GetColor(void);
#endif

  void FrustumTest(const Frustum &f, SpherePackCallback *callback);

  void RayTrace(const Vector3d &p1, // source
                const Vector3d &p2, // dest
                SpherePackCallback *callback);

  void RangeTest(const Vector3d &center, float radius,
                 SpherePackCallback *callback);
  void PointTest2d(const Vector3d &center, SpherePackCallback *callback);

  virtual void RayTraceCallback(const Vector3d &p1,   // source pos of ray
                                const Vector3d &dir,  // direction of ray
                                float distance,       // distance of ray
                                const Vector3d &sect, // intersection location
                                SpherePack *sphere);

  virtual void RangeTestCallback(const Vector3d &p, float distance,
                                 SpherePack *sphere, ViewState state);
  virtual void PointTest2dCallback(const Vector3d &p, SpherePack *sphere,
                                   ViewState state);

  virtual void VisibilityCallback(const Frustum &f, SpherePack *sphere,
                                  ViewState state);

  void Reset(void);

private:
  SpherePack *mRoot; // 1024x1024 root node of all active spheres.
  SpherePack *mLeaf; // 1024x1024 root node of all active spheres.
  SpherePackCallback *mCallback;

  Pool<SpherePack> mSpheres; // all spheres possibly represented.

  SpherePackFifo *mIntegrate; // integration fifo
  SpherePackFifo *mRecompute; // recomputation fifo

#if DEMO
#define MAXCOLORS 12
  int mColorCount;
  unsigned int mColors[MAXCOLORS];
#endif

  float mMaxRootSize;      // maximum size of a root node supersphere
  float mMaxLeafSize;      // maximum size of the leaf node supersphere
  float mSuperSphereGravy; // binding distance gravy.
};

//*** Source code placed in header file so that it will be inlined!
void SpherePack::NewPos(const Vector3d &pos) {
  mCenter = pos; // set our new center position.

  // is we have a parent (meaning we are a valid leaf node) and we have not
  // already been flagged for re-integration, then.....
  if (mParent && !HasSpherePackFlag(SPF_INTEGRATE)) {
    float dist =
        DistanceSquared(mParent); // compute squared distance to our parent.

    if (dist >= mBindingDistance) // if that exceeds our binding distance...
    {
      // If our parent, is not already marked to be recomputed (rebalance the
      // sphere), then add him to the recomputation fifo.
      mFactory->AddRecompute(mParent);

      // Unlink ourselves from the parent sphere and place ourselves into the
      // root node.
      Unlink();
      mFactory->AddIntegrate(this); // add ourselves to the re-integration fifo.
    }
  }
}

void SpherePack::NewPosRadius(const Vector3d &pos, float radius) {
  // New position and, possibly, a new radius.
  mCenter = pos;

  if (mParent && !HasSpherePackFlag(SPF_INTEGRATE)) {
    if (radius != GetRadius()) {
      SetRadius(radius);
      ComputeBindingDistance(mParent);
    }

    mFactory->AddRecompute(mParent);

    float dist = DistanceSquared(mParent);

    if (dist >= mBindingDistance) {
      Unlink();
      mFactory->AddIntegrate(this);
    }
  }
}
