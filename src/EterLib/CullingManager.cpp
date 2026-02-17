#include "StdAfx.h"
#include "CullingManager.h"
#include "GrpObjectInstance.h"

// #define COUNT_SHOWING_SPHERE

#ifdef COUNT_SHOWING_SPHERE
int showingcount = 0;
#endif

void CCullingManager::RayTraceCallback(
    const Vector3d & /*p1*/,  // source pos of ray
    const Vector3d & /*dir*/, // dest pos of ray
    float distance, const Vector3d & /*sect*/, SpherePack *sphere) {
  // if (state!=VS_OUTSIDE)
  //{
  if (m_RayFarDistance <= 0.0f || m_RayFarDistance >= distance) {
#ifdef SPHERELIB_STRICT
    if (sphere->IS_SPHERE)
      puts("CCullingManager::RayTraceCallback");
#endif
    m_list.push_back((CGraphicObjectInstance *)sphere->GetUserData());
  }
  // f((CGraphicObjectInstance *)sphere->GetUserData());
  //}
}

void CCullingManager::VisibilityCallback(const Frustum & /*f*/,
                                         SpherePack *sphere, ViewState state) {
#ifdef SPHERELIB_STRICT
  if (sphere->IS_SPHERE)
    puts("CCullingManager::VisibilityCallback");
#endif

  CGraphicObjectInstance *pInstance =
      (CGraphicObjectInstance *)sphere->GetUserData();
  /*if (state == VS_PARTIAL)
  {
          Vector3d v;
          float r;
          pInstance->GetBoundingSphere(v,r);
          state = f.ViewVolumeTest(v,r);
  }*/
  if (state == VS_OUTSIDE) {
#ifdef COUNT_SHOWING_SPHERE
    if (pInstance->isShow()) {
      Tracef("SH : %p  ", sphere->GetUserData());
      showingcount--;
      Tracef("show size : %5d\n", showingcount);
    }

#endif
    pInstance->Hide();
  } else {
#ifdef COUNT_SHOWING_SPHERE
    if (!pInstance->isShow()) {
      Tracef("HS : %p  ", sphere->GetUserData());
      showingcount++;
      Tracef("show size : %5d\n", showingcount);
    }
#endif
    pInstance->Show();
  }
}

void CCullingManager::RangeTestCallback(const Vector3d & /*p*/,
                                        float /*distance*/, SpherePack *sphere,
                                        ViewState state) {
#ifdef SPHERELIB_STRICT
  if (sphere->IS_SPHERE)
    puts("CCullingManager::RangeTestCallback");
#endif
  if (state != VS_OUTSIDE) {
    m_list.push_back((CGraphicObjectInstance *)sphere->GetUserData());
    // f((CGraphicObjectInstance *)sphere->GetUserData());
  }
  // assert(false && "NOT REACHED");
}

void CCullingManager::Reset() { m_Factory->Reset(); }

void CCullingManager::Update() {
  // TODO : update each object
  // 하지말고 각자 하게 해보자

  // DWORD time = ELTimer_GetMSec();
  // Reset();

  m_Factory->Process();
  // Tracef("cull update : %3d  ",ELTimer_GetMSec()-time);
}

void CCullingManager::Process() {
  // DWORD time = ELTimer_GetMSec();
  // Frustum f;
  UpdateViewMatrix();
  UpdateProjMatrix();
  BuildViewFrustum();
  m_Factory->FrustumTest(GetFrustum(), this);
  // Tracef("cull process : %3d  ",ELTimer_GetMSec()-time);
}

CCullingManager::CullingHandle
CCullingManager::Register(CGraphicObjectInstance *obj) {
  assert(obj);
#ifdef COUNT_SHOWING_SPHERE
  Tracef("CR : %p  ", obj);
  showingcount++;
  Tracef("show size : %5d\n", showingcount);
#endif
  Vector3d center;
  float radius;
  obj->GetBoundingSphere(center, radius);
  return m_Factory->AddSphere_(center, radius, obj, false);
}

void CCullingManager::Unregister(CullingHandle h) {
#ifdef COUNT_SHOWING_SPHERE
  if (((CGraphicObjectInstance *)h->GetUserData())->isShow()) {
    Tracef("DE : %p  ", h->GetUserData());
    showingcount--;
    Tracef("show size : %5d\n", showingcount);
  }
#endif
  m_Factory->Remove(h);
}

CCullingManager::CCullingManager() {
  m_Factory = new SpherePackFactory(10000, // maximum count
                                    6400,  // root radius
                                    1600,  // leaf radius
                                    400    // extra radius
  );
}

CCullingManager::~CCullingManager() { delete m_Factory; }

void CCullingManager::FindRange(const Vector3d &p, float radius) {
  m_list.clear();
  m_Factory->RangeTest(p, radius, this);
}

void CCullingManager::FindRay(const Vector3d &p1, const Vector3d &dir) {
  m_RayFarDistance = -1;
  m_list.clear();
  m_Factory->RayTrace(p1, dir, this);
}

void CCullingManager::FindRayDistance(const Vector3d &p1, const Vector3d &dir,
                                      float distance) {
  m_RayFarDistance = distance;
  m_list.clear();
  m_Factory->RayTrace(p1, dir, this);
}