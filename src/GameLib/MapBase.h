#pragma once

class CMapBase : public CScreen {
public:
  enum EMAPTYPE {
    MAPTYPE_INVALID,
    MAPTYPE_INDOOR,
    MAPTYPE_OUTDOOR,
  };

public:
  CMapBase();
  virtual ~CMapBase();

  virtual void Clear();
  virtual bool Initialize() = 0;
  virtual bool Destroy() = 0;
  virtual bool Load(float x, float y, float z) = 0;

  virtual bool Update(float fX, float fY, float fZ) = 0;
  virtual void UpdateAroundAmbience(float fX, float fY, float fZ) = 0;
  virtual float GetHeight(float fx, float fy) = 0;
  virtual void
  OnBeginEnvironment() = 0; // 렌더링 할 때 불려지며 여기서 Environment에 관련
                            // 있는 것들을 셋팅 한다.

  virtual void ApplyLight(DWORD dwVersion, const D3DLIGHT9 &c_rkLight) = 0;

protected:
  virtual void OnRender() = 0;
  virtual void
  OnSetEnvironmentDataPtr() = 0; // Environment 데이터가 설정될 때 불려진다.
  virtual void OnResetEnvironmentDataPtr() = 0;

public:
  void Render();
  void SetEnvironmentDataPtr(const TEnvironmentData *c_pEnvironmentData);
  void ResetEnvironmentDataPtr(const TEnvironmentData *c_pEnvironmentData);
  bool Enter();
  bool Leave();
  bool IsReady() { return m_bReady; }

  bool LoadProperty();

  //////////////////////////////////////////////////////////////////////////
  // Setting
  //////////////////////////////////////////////////////////////////////////
  EMAPTYPE GetType() const { return m_eType; }
  void SetType(EMAPTYPE eType) { m_eType = eType; }

  const std::string &GetName() const { return m_strName; }
  void SetName(const std::string &cr_strName) { m_strName = cr_strName; }

  bool IsCopiedMap() const {
    return m_strParentMapName.length() > 0;
  } // "원본 데이터는 따로 있는" 복사 맵인가?
  const std::string &GetParentMapName() const {
    return m_strParentMapName;
  } // 원본 데이터를 갖고있는 맵의 이름을 리턴 (복사맵의 경우 필수로 세팅되어야
    // 함)
  const std::string &GetMapDataDirectory() const {
    return IsCopiedMap() ? m_strParentMapName : m_strName;
  }

protected:
  EMAPTYPE m_eType;      // 맵 종류... 현재는 Indoor와 Ourdoor가 있다.
  std::string m_strName; // 맵 이름. 맵 이름이 맵 식별자가 될 수 있을까?
  std::string
      m_strParentMapName; // 원본 맵의 이름. 이 값이 세팅되어 있다면 실제 모든
                          // 맵 데이터는 ParentMap/* 경로에서 읽어온다. 맵의
                          // 일부만 읽어오는 기능은 필요없대서 패스.
  bool m_bReady;

  const TEnvironmentData *mc_pEnvironmentData;
};
