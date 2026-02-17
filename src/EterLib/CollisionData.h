#pragma once

// Collision Detection
typedef struct SSphereData {
  D3DXVECTOR3 v3Position;
  float fRadius;
} TSphereData;

typedef struct SPlaneData {
  D3DXVECTOR3 v3Position;
  D3DXVECTOR3 v3Normal;

  D3DXVECTOR3 v3QuadPosition[4];
  D3DXVECTOR3 v3InsideVector[4];
} TPlaneData;

typedef struct SAABBData {
  D3DXVECTOR3 v3Min;
  D3DXVECTOR3 v3Max;

} TAABBData;

typedef struct SOBBData {
  D3DXVECTOR3 v3Min;
  D3DXVECTOR3 v3Max;
  D3DXMATRIX matRot;

} TOBBData;

typedef struct SCylinderData {
  D3DXVECTOR3 v3Position;
  float fRadius;
  float fHeight;
} TCylinderData;

enum ECollisionType {
  COLLISION_TYPE_PLANE,
  COLLISION_TYPE_BOX,
  COLLISION_TYPE_SPHERE,
  COLLISION_TYPE_CYLINDER,
  COLLISION_TYPE_AABB,
  COLLISION_TYPE_OBB,
};

struct CDynamicSphereInstance {
  D3DXVECTOR3 v3Position;
  D3DXVECTOR3 v3LastPosition;

  float fRadius;
};

class CStaticCollisionData {
public:
  DWORD dwType;
  char szName[32 + 1];

  D3DXVECTOR3 v3Position;
  float fDimensions[3];
  D3DXQUATERNION quatRotation;
};

void DestroyCollisionInstanceSystem();

typedef std::vector<CStaticCollisionData> CStaticCollisionDataVector;

/////////////////////////////////////////////
// Base
class CBaseCollisionInstance {
public:
  virtual void Render(D3DFILLMODE d3dFillMode = D3DFILL_SOLID) = 0;

  bool MovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const {
    return OnMovementCollisionDynamicSphere(s);
  }
  bool CollisionDynamicSphere(const CDynamicSphereInstance &s) const {
    return OnCollisionDynamicSphere(s);
  }

  D3DXVECTOR3
  GetCollisionMovementAdjust(const CDynamicSphereInstance &s) const {
    return OnGetCollisionMovementAdjust(s);
  }

  void Destroy();

  static CBaseCollisionInstance *
  BuildCollisionInstance(const CStaticCollisionData *c_pCollisionData,
                         const D3DXMATRIX *pMat);

protected:
  virtual D3DXVECTOR3
  OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const = 0;
  virtual bool
  OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const = 0;
  virtual bool
  OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const = 0;
  virtual void OnDestroy() = 0;
};

/////////////////////////////////////////////
// Sphere
class CSphereCollisionInstance : public CBaseCollisionInstance {
public:
  TSphereData &GetAttribute();
  const TSphereData &GetAttribute() const;
  virtual void Render(D3DFILLMODE d3dFillMode = D3DFILL_SOLID);

protected:
  void OnDestroy();
  bool OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual bool OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual D3DXVECTOR3
  OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const;

protected:
  TSphereData m_attribute;
};

/////////////////////////////////////////////
// Plane
class CPlaneCollisionInstance : public CBaseCollisionInstance {
public:
  TPlaneData &GetAttribute();
  const TPlaneData &GetAttribute() const;
  virtual void Render(D3DFILLMODE d3dFillMode = D3DFILL_SOLID);

protected:
  void OnDestroy();
  bool OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual bool OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual D3DXVECTOR3
  OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const;

protected:
  TPlaneData m_attribute;
};

/////////////////////////////////////////////
// AABB (Aligned Axis Bounding Box)
class CAABBCollisionInstance : public CBaseCollisionInstance {
public:
  TAABBData &GetAttribute();
  const TAABBData &GetAttribute() const;
  virtual void Render(D3DFILLMODE d3dFillMode = D3DFILL_SOLID);

protected:
  void OnDestroy();
  bool OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual bool OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual D3DXVECTOR3
  OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const;

protected:
  TAABBData m_attribute;
};

/////////////////////////////////////////////
// OBB
class COBBCollisionInstance : public CBaseCollisionInstance {
public:
  TOBBData &GetAttribute();
  const TOBBData &GetAttribute() const;
  virtual void Render(D3DFILLMODE d3dFillMode = D3DFILL_SOLID);

protected:
  void OnDestroy();
  bool OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual bool OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual D3DXVECTOR3
  OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const;

protected:
  TOBBData m_attribute;
};

/////////////////////////////////////////////
// Cylinder
class CCylinderCollisionInstance : public CBaseCollisionInstance {
public:
  TCylinderData &GetAttribute();
  const TCylinderData &GetAttribute() const;
  virtual void Render(D3DFILLMODE d3dFillMode = D3DFILL_SOLID);

protected:
  void OnDestroy();
  bool OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual bool OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
  virtual D3DXVECTOR3
  OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const;

  bool CollideCylinderVSDynamicSphere(const TCylinderData &c_rattribute,
                                      const CDynamicSphereInstance &s) const;

protected:
  TCylinderData m_attribute;
};

typedef std::vector<CSphereCollisionInstance> CSphereCollisionInstanceVector;
typedef std::vector<CDynamicSphereInstance> CDynamicSphereInstanceVector;
typedef std::vector<CBaseCollisionInstance *> CCollisionInstanceVector;
