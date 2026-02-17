#pragma once

class IFlyTargetableObject {
  friend class CFlyTarget;

public:
  IFlyTargetableObject() {}
  virtual ~IFlyTargetableObject() {}

  virtual D3DXVECTOR3 OnGetFlyTargetPosition() = 0;
  virtual void OnShootDamage() = 0;

protected:
  inline void ClearFlyTargeter();

private:
  std::set<CFlyTarget *> m_FlyTargeterSet;
  inline void AddFlyTargeter(CFlyTarget *pTargeter) {
    // if (m_FlyTargeterSet.find(pTargeter)!=m_FlyTargeterSet.end())
    m_FlyTargeterSet.insert(pTargeter);
  }
  inline void RemoveFlyTargeter(CFlyTarget *pTargeter) {
    // if (m_FlyTargeterSet.find(pTargeter)!=m_FlyTargeterSet.end())
    m_FlyTargeterSet.erase(pTargeter);
  }
};

class CFlyTarget // final
{
public:
  enum EType {
    TYPE_NONE,
    TYPE_OBJECT,
    TYPE_POSITION,
  };

public:
  CFlyTarget();
  CFlyTarget(IFlyTargetableObject *pFlyTarget);
  CFlyTarget(const D3DXVECTOR3 &v3FlyTargetPosition);
  CFlyTarget(const CFlyTarget &rhs);
  virtual ~CFlyTarget();

  void Clear();
  bool IsObject();
  bool IsPosition();
  bool IsValidTarget();
  void NotifyTargetClear();

  const D3DXVECTOR3 &GetFlyTargetPosition() const;
  EType GetType();

  IFlyTargetableObject *GetFlyTarget();

  CFlyTarget &operator=(const CFlyTarget &rhs);
  void GetFlyTargetData(CFlyTarget *pFlyTarget);

private:
  void __Initialize();

private:
  mutable D3DXVECTOR3 m_v3FlyTargetPosition;
  IFlyTargetableObject *m_pFlyTarget;

  EType m_eType;
};

inline void IFlyTargetableObject::ClearFlyTargeter() {
  std::set<CFlyTarget *>::iterator it;
  for (it = m_FlyTargeterSet.begin(); it != m_FlyTargeterSet.end(); ++it) {
    (*it)->NotifyTargetClear();
  }
  m_FlyTargeterSet.clear();
}
