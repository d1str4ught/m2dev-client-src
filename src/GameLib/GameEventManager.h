#pragma once

#include "ActorInstance.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CGameEventManager
//
//		Note : Game상에서 일어나는 폭발, 화면 흔들림, Range 무기들의
// 움직임과 타격 등의 Event
//             를 종합적으로 관리하는 클래스. - [levites]
//
//		Note : 게임상의 Cinematic Event 를 컨트롤하는 매니져로 사용한다.
// 2004.07.19 - [levites]
//

class CGameEventManager : public CSingleton<CGameEventManager>, public CScreen {
public:
  CGameEventManager();
  virtual ~CGameEventManager();

  void SetCenterPosition(float fx, float fy, float fz);
  void Update();

  void ProcessEventScreenWaving(
      CActorInstance *pActorInstance,
      const CRaceMotionData::TScreenWavingEventData *c_pData);

protected:
  TPixelPosition m_CenterPosition;
};
