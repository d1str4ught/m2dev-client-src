#include "StdAfx.h"
#include "PythonPlayer.h"

#include "InstanceBase.h"

void CPythonPlayer::SetAttackKeyState(bool isPress) {
  if (isPress) {
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (pkInstMain)
      if (pkInstMain->IsFishingMode()) {
        NEW_Fishing();
        return;
      }
  }

  m_isAtkKey = isPress;
}

void CPythonPlayer::NEW_SetSingleDIKKeyState(int eDIKKey, bool isPress) {
  if (NEW_CancelFishing())
    return;

  switch (eDIKKey) {
  case DIK_UP:
    NEW_SetSingleDirKeyState(DIR_UP, isPress);
    break;
  case DIK_DOWN:
    NEW_SetSingleDirKeyState(DIR_DOWN, isPress);
    break;
  case DIK_LEFT:
    NEW_SetSingleDirKeyState(DIR_LEFT, isPress);
    break;
  case DIK_RIGHT:
    NEW_SetSingleDirKeyState(DIR_RIGHT, isPress);
    break;
  }
}

void CPythonPlayer::NEW_SetSingleDirKeyState(int eDirKey, bool isPress) {
  switch (eDirKey) {
  case DIR_UP:
    m_isUp = isPress;
    break;
  case DIR_DOWN:
    m_isDown = isPress;
    break;
  case DIR_LEFT:
    m_isLeft = isPress;
    break;
  case DIR_RIGHT:
    m_isRight = isPress;
    break;
  }

  m_isDirKey = (m_isUp || m_isDown || m_isLeft || m_isRight);

  NEW_SetMultiDirKeyState(m_isLeft, m_isRight, m_isUp, m_isDown);
}

void CPythonPlayer::NEW_SetMultiDirKeyState(bool isLeft, bool isRight,
                                            bool isUp, bool isDown) {
  if (!__CanMove())
    return;

  bool isAny = (isLeft || isRight || isUp || isDown);

  if (isAny) {
    float fDirRot = 0.0f;
    NEW_GetMultiKeyDirRotation(isLeft, isRight, isUp, isDown, &fDirRot);

    if (!NEW_MoveToDirection(fDirRot)) {
      Tracen("CPythonPlayer::NEW_SetMultiKeyState - NEW_Move -> ERROR");
      return;
    }
  } else {
    NEW_Stop();
  }
}

float CPythonPlayer::GetDegreeFromDirection(int iUD, int iLR) {
  switch (iUD) {
  case KEYBOARD_UD_UP:
    if (KEYBOARD_LR_LEFT == iLR) {
      return +45.0f;
    } else if (KEYBOARD_LR_RIGHT == iLR) {
      return -45.0f;
    }

    return 0.0f;
    break;

  case KEYBOARD_UD_DOWN:
    if (KEYBOARD_LR_LEFT == iLR) {
      return +135.0f;
    } else if (KEYBOARD_LR_RIGHT == iLR) {
      return -135.0f;
    }

    return +180.0f;
    break;

  case KEYBOARD_UD_NONE:
    if (KEYBOARD_LR_LEFT == iLR) {
      return +90.0f;
    } else if (KEYBOARD_LR_RIGHT == iLR) {
      return -90.0f;
    }
    break;
  }

  return 0.0f;
}
