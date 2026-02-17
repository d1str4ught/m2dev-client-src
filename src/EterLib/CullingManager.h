#pragma once

#include "GrpScreen.h"

#include "Eterbase/Singleton.h"
#include "SphereLib/spherepack.h"

class CGraphicObjectInstance;
template <class T> struct RangeTester : public SpherePackCallback {
  T *f;
  float dist;
  RangeTester(T *fn, float distance = -1) : f(fn), dist(distance) {}

  virtual ~RangeTester() {}

  virtual void RayTraceCallback(const Vector3d &p1,  // source pos of ray
                                const Vector3d &dir, // dest pos of ray
                                float distance, const Vector3d &sect,
                                SpherePack *sphere) {
#ifdef SPHERELIB_STRICT
    if (sphere->IS_SPHERE)
      puts("RangeTester::RayTraceCallback");
#endif
    if (dist <= 0.0f || dist >= distance)
      (*f)((CGraphicObjectInstance *)sphere->GetUserData());
  };

  virtual void VisibilityCallback(const Frustum &f, SpherePack *sphere,
                                  ViewState state) {};

  virtual void RangeTestCallback(const Vector3d &p, float distance,
                                 SpherePack *sphere, ViewState state) {
#ifdef SPHERELIB_STRICT
    if (sphere->IS_SPHERE)
      puts("RangeTester::RangeTestCallback");
#endif
    if (state != VS_OUTSIDE)
      (*f)((CGraphicObjectInstance *)sphere->GetUserData());
  }

  virtual void PointTest2dCallback(const Vector3d &p, SpherePack *sphere,
                                   ViewState state) {
#ifdef SPHERELIB_STRICT
    if (sphere->IS_SPHERE)
      puts("RangeTester::PointTest2dCallback");
#endif
    if (state != VS_OUTSIDE) {
#ifdef SPHERELIB_STRICT
      puts("FIND!!");
#endif
      (*f)((CGraphicObjectInstance *)sphere->GetUserData());
    }
  }
};

class CCullingManager : public CSingleton<CCullingManager>,
                        public SpherePackCallback,
                        private CScreen {
public:
  typedef SpherePack *CullingHandle;
  typedef std::vector<CGraphicObjectInstance *> TRangeList;

  CCullingManager();
  virtual ~CCullingManager();

  virtual void RayTraceCallback(const Vector3d &p1,  // source pos of ray
                                const Vector3d &dir, // dest pos of ray
                                float distance, const Vector3d &sect,
                                SpherePack *sphere);

  virtual void VisibilityCallback(const Frustum &f, SpherePack *sphere,
                                  ViewState state);

  void RangeTestCallback(const Vector3d &p, float distance, SpherePack *sphere,
                         ViewState state);

  void Reset();
  void Update();
  void Process();

  void FindRange(const Vector3d &p, float radius);
  void FindRay(const Vector3d &p1, const Vector3d &dir);
  void FindRayDistance(const Vector3d &p1, const Vector3d &dir, float distance);

  void RangeTest(const Vector3d &p, float radius,
                 SpherePackCallback *callback) {
    m_Factory->RangeTest(p, radius, callback);
  }

  void PointTest2d(const Vector3d &p, SpherePackCallback *callback) {
    m_Factory->PointTest2d(p, callback);
  }

  template <class T> void ForInRange2d(const Vector3d &p, T *pFunc) {
    RangeTester<T> r(pFunc);
    m_Factory->PointTest2d(p, &r);
  }

  template <class T>
  void ForInRange(const Vector3d &p, float radius, T *pFunc) {
    RangeTester<T> r(pFunc);
    m_Factory->RangeTest(p, radius, &r /*this*/);
  }

  template <class T>
  void ForInRay(const Vector3d &p1, const Vector3d &dir, T *pFunc) {
    RangeTester<T> r(pFunc);
    /*Vector3d p2;
    //p2.Set(p.x+(dir.x*50000.0f),p.y+(dir.y*50000.0f),p.z+(dir.z*50000.0f));
    p2.x = p.x+50000.0f*dir.x;
    p2.y = p.y+50000.0f*dir.y;
    p2.z = p.z+50000.0f*dir.z;
            // p +
    (50000.0f*dir);//(p.x+(dir.x*50000.0f),p.y+(dir.y*50000.0f),p.z+(dir.z*50000.0f));*/
    m_Factory->RayTrace(p1, dir, &r /*this*/);
  }

  template <class T>
  void ForInRayDistance(const Vector3d &p, const Vector3d &dir, float distance,
                        T *pFunc) {
    RangeTester<T> r(pFunc, distance);
    m_Factory->RayTrace(p, dir, &r /*this*/);
  }

  CullingHandle Register(CGraphicObjectInstance *ob);
  void Unregister(CullingHandle h);

  TRangeList::iterator begin() { return m_list.begin(); }
  TRangeList::iterator end() { return m_list.end(); }

protected:
  TRangeList m_list;

  float m_RayFarDistance;

  SpherePackFactory *m_Factory;
};
