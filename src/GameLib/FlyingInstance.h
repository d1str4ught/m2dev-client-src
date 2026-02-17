#pragma once

#include "FlyTarget.h"
#include "ActorInstanceInterface.h"

class CFlyingData;
class CFlyTrace;
class IFlyEventHandler;
class CActorInstance;

class CFlyingInstance {
public:
  // 2004. 3. 26. myevan. 적절한 네이밍이 필요. 게임에서 사용하지 않는다면
  // 툴에서 툴 전용으로 상속받아 만들도록 하자
  void Clear(); // Destroy와 같다
  void SetDataPointer(CFlyingData *pData, const D3DXVECTOR3 &v3StartPosition);
  void SetFlyTarget(const CFlyTarget &cr_Target); // Shot at Target

public:
  friend class CSceneFly;

  struct TAttachEffectInstance {
    int iAttachIndex;
    DWORD dwEffectInstanceIndex;

    CFlyTrace *pFlyTrace;
  };

  CFlyingInstance();
  virtual ~CFlyingInstance();

  void Destroy();
  void Create(CFlyingData *pData, const D3DXVECTOR3 &c_rv3StartPos,
              const CFlyTarget &c_rkTarget, bool canAttack);

  bool Update();
  void Render();

  bool IsAlive() { return m_bAlive; }

  const D3DXVECTOR3 &GetPosition() { return m_v3Position; }

  void AdjustDirectionForHoming(const D3DXVECTOR3 &v3TargetPosition);

  typedef std::vector<TAttachEffectInstance> TAttachEffectInstanceVector;

  void BuildAttachInstance();
  void UpdateAttachInstance();
  void RenderAttachInstance();
  void ClearAttachInstance();

  void SetEventHandler(IFlyEventHandler *pHandler) { m_pHandler = pHandler; }
  void ClearEventHandler() { m_pHandler = 0; }

  void SetPierceCount(int iCount) { m_iPierceCount = iCount; }
  void SetOwner(IActorInstance *pOwner) { m_pOwner = pOwner; }
  void SetSkillIndex(DWORD dwIndex) { m_dwSkillIndex = dwIndex; }

  // FIXME : 툴에서 사용하고 있습니다. 임시로 위로.. - [levites]
  void __Explode(bool bBomb = true);
  void __Bomb();

  DWORD ID;

protected:
  void __Initialize();

  void __SetDataPointer(CFlyingData *pData, const D3DXVECTOR3 &v3StartPosition);
  void __SetTargetDirection(const CFlyTarget &c_rkTarget);
  void __SetTargetNormalizedDirection(
      const D3DXVECTOR3 &v3NormalizedDirection); // 시작 타겟 방향 설정

protected:
  CFlyingData *m_pData;
  D3DXQUATERNION m_qRot;

  float m_fStartTime;

  bool m_bAlive;
  bool m_canAttack;

  int m_iPierceCount;
  DWORD m_dwSkillIndex;

  D3DXVECTOR3 m_v3Position;
  D3DXVECTOR3 m_v3Velocity;
  D3DXVECTOR3 m_v3LocalVelocity;
  D3DXVECTOR3 m_v3Accel;

  float m_fRemainRange;

  CFlyTarget m_FlyTarget;

  D3DXQUATERNION m_qAttachRotation;
  TAttachEffectInstanceVector m_vecAttachEffectInstance;

  IFlyEventHandler *m_pHandler;

  IActorInstance *m_pOwner;

  BOOL m_bTargetHitted;
  std::set<IActorInstance *> m_HittedObjectSet;

public:
  static CFlyingInstance *New() { return ms_kPool.Alloc(); }
  static void Delete(CFlyingInstance *pInstance) {
    pInstance->Destroy();
    ms_kPool.Free(pInstance);
  }

  static void DestroySystem() { ms_kPool.Destroy(); }

  static CDynamicPool<CFlyingInstance> ms_kPool;
};
