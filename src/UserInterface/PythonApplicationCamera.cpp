#include "StdAfx.h"
#include "PythonApplication.h"

#include "EterBase/timer.h"
#include "EterLib/Camera.h"

#include <utf8.h>

float BlendValueByLinear(float fElapsedTime, float fDuration, float fBeginValue,
                         float fEndValue) {
  if (fElapsedTime >= fDuration)
    return fEndValue;

  return (fEndValue - fBeginValue) * (fElapsedTime / fDuration) + fBeginValue;
}

void CPythonApplication::__UpdateCamera() {
  //////////////////////
  // Camera Setting
  CCamera *pMainCamera = CCameraManager::Instance().GetCurrentCamera();
  if (!pMainCamera)
    return;

  if (CAMERA_MODE_BLEND == m_iCameraMode) {
    float fcurTime = CTimer::Instance().GetCurrentSecond();
    float fElapsedTime = fcurTime - m_fBlendCameraStartTime;

    float fxCenter =
        BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                           m_kEventCameraSetting.v3CenterPosition.x,
                           m_kEndBlendCameraSetting.v3CenterPosition.x);
    float fyCenter =
        BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                           m_kEventCameraSetting.v3CenterPosition.y,
                           m_kEndBlendCameraSetting.v3CenterPosition.y);
    float fzCenter =
        BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                           m_kEventCameraSetting.v3CenterPosition.z,
                           m_kEndBlendCameraSetting.v3CenterPosition.z);

    float fDistance = BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                                         m_kEventCameraSetting.fZoom,
                                         m_kEndBlendCameraSetting.fZoom);
    float fPitch = BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                                      m_kEventCameraSetting.fPitch,
                                      m_kEndBlendCameraSetting.fPitch);
    float fRotation = BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                                         m_kEventCameraSetting.fRotation,
                                         m_kEndBlendCameraSetting.fRotation);

    float fUpDir =
        BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                           m_kEventCameraSetting.kCmrPos.m_fUpDir,
                           m_kEndBlendCameraSetting.kCmrPos.m_fUpDir);
    float fViewDir =
        BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                           m_kEventCameraSetting.kCmrPos.m_fViewDir,
                           m_kEndBlendCameraSetting.kCmrPos.m_fViewDir);
    float fCrossDir =
        BlendValueByLinear(fElapsedTime, m_fBlendCameraBlendTime,
                           m_kEventCameraSetting.kCmrPos.m_fCrossDir,
                           m_kEndBlendCameraSetting.kCmrPos.m_fCrossDir);

    // Temporary. Have to fix that this work in camera class. - [levites]
    pMainCamera->Unlock();
    m_pyGraphic.SetPositionCamera(fxCenter, fyCenter, fzCenter, fDistance,
                                  fPitch, fRotation);
    pMainCamera->MoveVertical(fUpDir);
    pMainCamera->MoveFront(fViewDir);
    pMainCamera->MoveAlongCross(fCrossDir);
    pMainCamera->Lock();
  } else if (CAMERA_MODE_STAND == m_iCameraMode) {
    float fDistance, fPitch, fRotation, fHeight;
    GetCamera(&fDistance, &fPitch, &fRotation, &fHeight);
    m_pyGraphic.SetPositionCamera(m_kEventCameraSetting.v3CenterPosition.x,
                                  m_kEventCameraSetting.v3CenterPosition.y,
                                  m_kEventCameraSetting.v3CenterPosition.z +
                                      pMainCamera->GetTargetHeight(),
                                  fDistance, fPitch, fRotation);
  } else if (CAMERA_MODE_NORMAL == m_iCameraMode) {
    float fDistance, fPitch, fRotation, fHeight;
    GetCamera(&fDistance, &fPitch, &fRotation, &fHeight);
    m_pyGraphic.SetPositionCamera(m_v3CenterPosition.x, m_v3CenterPosition.y,
                                  m_v3CenterPosition.z +
                                      pMainCamera->GetTargetHeight(),
                                  fDistance, fPitch, fRotation);
  }

  if (0.0f != m_fRotationSpeed)
    pMainCamera->Roll(m_fRotationSpeed);
  if (0.0f != m_fPitchSpeed)
    pMainCamera->Pitch(m_fPitchSpeed);
  if (0.0f != m_fZoomSpeed)
    pMainCamera->Zoom(m_fZoomSpeed);

  if (0.0f != m_kCmrSpd.m_fViewDir)
    m_kCmrPos.m_fViewDir += m_kCmrSpd.m_fViewDir;
  if (0.0f != m_kCmrSpd.m_fCrossDir)
    m_kCmrPos.m_fCrossDir += m_kCmrSpd.m_fCrossDir;
  if (0.0f != m_kCmrSpd.m_fUpDir)
    m_kCmrPos.m_fUpDir += m_kCmrSpd.m_fUpDir;

  if (0.0f != m_kCmrPos.m_fViewDir)
    pMainCamera->MoveFront(m_kCmrPos.m_fViewDir);
  if (0.0f != m_kCmrPos.m_fCrossDir)
    pMainCamera->MoveAlongCross(m_kCmrPos.m_fCrossDir);
  if (0.0f != m_kCmrPos.m_fUpDir)
    pMainCamera->MoveVertical(m_kCmrPos.m_fUpDir);

  // Sound Setting
  const D3DXVECTOR3 &c_rv3CameraDirection = pMainCamera->GetView();
  const D3DXVECTOR3 &c_rv3CameraUp = pMainCamera->GetUp();
  m_SoundEngine.SetListenerPosition(
      m_v3CenterPosition.x, m_v3CenterPosition.y,
      m_v3CenterPosition.z); // Listener - 캐릭터 위치
  m_SoundEngine.SetListenerOrientation(
      c_rv3CameraDirection.x, c_rv3CameraDirection.y, c_rv3CameraDirection.z,
      c_rv3CameraUp.x, c_rv3CameraUp.y, c_rv3CameraUp.z);
  m_SoundEngine.Update();
  //////////////////////
}

void CPythonApplication::SetViewDirCameraSpeed(float fSpeed) {
  if (IsLockCurrentCamera())
    return;

  m_kCmrSpd.m_fViewDir = fSpeed;
}

void CPythonApplication::SetCrossDirCameraSpeed(float fSpeed) {
  if (IsLockCurrentCamera())
    return;

  m_kCmrSpd.m_fCrossDir = fSpeed;
}

void CPythonApplication::SetUpDirCameraSpeed(float fSpeed) {
  if (IsLockCurrentCamera())
    return;

  m_kCmrSpd.m_fUpDir = fSpeed;
}

void CPythonApplication::SetCamera(float Distance, float Pitch, float Rotation,
                                   float fDestinationHeight) {
  if (IsLockCurrentCamera())
    return;

  CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
  if (!pCurrentCamera)
    return;

  D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();
  m_pyGraphic.SetPositionCamera(v3Target.x, v3Target.y, v3Target.z, Distance,
                                Pitch, Rotation);

  CCamera *pMainCamera = CCameraManager::Instance().GetCurrentCamera();
  if (pMainCamera)
    pMainCamera->SetTargetHeight(fDestinationHeight);
}

void CPythonApplication::GetCamera(float *Distance, float *Pitch,
                                   float *Rotation, float *DestinationHeight) {
  CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
  *Distance = pCurrentCamera->GetDistance();
  *Pitch = pCurrentCamera->GetPitch();
  *Rotation = pCurrentCamera->GetRoll();
  *DestinationHeight = pCurrentCamera->GetTarget().z;
}

void CPythonApplication::RotateCamera(int iDirection) {
  if (IsLockCurrentCamera())
    return;

  float fDegree = m_fCameraRotateSpeed * float(iDirection);
  m_fRotationSpeed = fDegree;
}

void CPythonApplication::PitchCamera(int iDirection) {
  if (IsLockCurrentCamera())
    return;

  float fDegree = m_fCameraPitchSpeed * float(iDirection);
  m_fPitchSpeed = fDegree;
}

void CPythonApplication::ZoomCamera(int iDirection) {
  if (IsLockCurrentCamera())
    return;

  float fRatio = 1.0f + m_fCameraZoomSpeed * float(iDirection);
  m_fZoomSpeed = fRatio;
}

void CPythonApplication::MovieRotateCamera(int iDirection) {
  if (IsLockCurrentCamera())
    return;

  float fDegree = m_fCameraRotateSpeed * float(iDirection);
  if (m_isSpecialCameraMode) {
    if (GetKeyState(VK_SCROLL) & 1) {
      SetCrossDirCameraSpeed(-fDegree * 6.0f);
      return;
    }
  }
  m_fRotationSpeed = fDegree;
}

void CPythonApplication::MoviePitchCamera(int iDirection) {
  if (IsLockCurrentCamera())
    return;

  float fDegree = m_fCameraPitchSpeed * float(iDirection);
  if (m_isSpecialCameraMode) {
    if (GetKeyState(VK_SCROLL) & 1) {
      SetViewDirCameraSpeed(-fDegree * 6.0f);
      return;
    }
  }
  m_fPitchSpeed = fDegree;
}

void CPythonApplication::MovieZoomCamera(int iDirection) {
  if (IsLockCurrentCamera())
    return;

  float fRatio = 1.0f + m_fCameraZoomSpeed * float(iDirection);
  if (m_isSpecialCameraMode) {
    if (GetKeyState(VK_SCROLL) & 1) {
      SetUpDirCameraSpeed((1.0f - fRatio) * 200.0f);
      return;
    }
  }
  m_fZoomSpeed = fRatio;
}

void CPythonApplication::MovieResetCamera() {
  if (IsLockCurrentCamera())
    return;

  if (m_isSpecialCameraMode) {
    SetCrossDirCameraSpeed(0.0f);
    SetViewDirCameraSpeed(0.0f);
    SetUpDirCameraSpeed(0.0f);

    m_kCmrPos.m_fViewDir = 0;
    m_kCmrPos.m_fCrossDir = 0;
    m_kCmrPos.m_fUpDir = 0;
  }
}

float CPythonApplication::GetRotation() {
  return CCameraManager::Instance().GetCurrentCamera()->GetRoll();
}

float CPythonApplication::GetPitch() {
  return CCameraManager::Instance().GetCurrentCamera()->GetPitch();
}

bool CPythonApplication::IsLockCurrentCamera() {
  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  if (!pCamera)
    return false;

  return pCamera->IsLock();
}

void CPythonApplication::SetEventCamera(
    const SCameraSetting &c_rCameraSetting) {
  if (CCameraManager::DEFAULT_PERSPECTIVE_CAMERA ==
      CCameraManager::Instance().GetCurrentCameraNum()) {
    GetCameraSetting(&m_DefaultCameraSetting);
  }

  /////

  CCameraManager::Instance().SetCurrentCamera(EVENT_CAMERA_NUMBER);
  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  if (!pCamera)
    return;

  SetCameraSetting(c_rCameraSetting);
  m_kEventCameraSetting = c_rCameraSetting;
  m_iCameraMode = CAMERA_MODE_STAND;
}

void CPythonApplication::BlendEventCamera(
    const SCameraSetting &c_rCameraSetting, float fBlendTime) {
  m_iCameraMode = CAMERA_MODE_BLEND;
  m_fBlendCameraStartTime = CTimer::Instance().GetCurrentSecond();
  m_fBlendCameraBlendTime = fBlendTime;
  m_kEndBlendCameraSetting = c_rCameraSetting;

  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  if (pCamera)
    pCamera->Lock();
}

void CPythonApplication::SetDefaultCamera() {
  m_iCameraMode = CAMERA_MODE_NORMAL;
  m_fBlendCameraStartTime = 0.0f;
  m_fBlendCameraBlendTime = 0.0f;

  /////

  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  if (pCamera)
    pCamera->Unlock();

  if (CCameraManager::DEFAULT_PERSPECTIVE_CAMERA !=
      CCameraManager::Instance().GetCurrentCameraNum()) {
    CCameraManager::Instance().SetCurrentCamera(
        CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
    SetCameraSetting(m_DefaultCameraSetting);
  }
}

void CPythonApplication::SetCameraSetting(
    const SCameraSetting &c_rCameraSetting) {
  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  if (!pCamera)
    return;

  m_pyGraphic.SetPositionCamera(
      c_rCameraSetting.v3CenterPosition.x, c_rCameraSetting.v3CenterPosition.y,
      c_rCameraSetting.v3CenterPosition.z, c_rCameraSetting.fZoom,
      c_rCameraSetting.fPitch, c_rCameraSetting.fRotation);

  if (0.0f != c_rCameraSetting.kCmrPos.m_fViewDir)
    pCamera->MoveFront(c_rCameraSetting.kCmrPos.m_fViewDir);
  if (0.0f != c_rCameraSetting.kCmrPos.m_fCrossDir)
    pCamera->MoveAlongCross(c_rCameraSetting.kCmrPos.m_fCrossDir);
  if (0.0f != c_rCameraSetting.kCmrPos.m_fUpDir)
    pCamera->MoveVertical(c_rCameraSetting.kCmrPos.m_fUpDir);

  m_kCmrPos.m_fUpDir = 0.0f;
  m_kCmrPos.m_fViewDir = 0.0f;
  m_kCmrPos.m_fCrossDir = 0.0f;
  m_kCmrSpd.m_fUpDir = 0.0f;
  m_kCmrSpd.m_fViewDir = 0.0f;
  m_kCmrSpd.m_fCrossDir = 0.0f;
  m_fZoomSpeed = 0.0f;
  m_fPitchSpeed = 0.0f;
  m_fRotationSpeed = 0.0f;
}

void CPythonApplication::GetCameraSetting(SCameraSetting *pCameraSetting) {
  pCameraSetting->v3CenterPosition = m_v3CenterPosition;
  pCameraSetting->kCmrPos = m_kCmrPos;

  if (CCameraManager::Instance().GetCurrentCamera())
    pCameraSetting->v3CenterPosition.z +=
        CCameraManager::Instance().GetCurrentCamera()->GetTargetHeight();

  float fHeight;
  GetCamera(&pCameraSetting->fZoom, &pCameraSetting->fPitch,
            &pCameraSetting->fRotation, &fHeight);
}

void CPythonApplication::SaveCameraSetting(const char *c_szFileName) {
  SCameraSetting CameraSetting;
  GetCameraSetting(&CameraSetting);

  // UTF-8 → UTF-16
  std::wstring wFileName = Utf8ToWide(c_szFileName);

  FILE *File = _wfopen(wFileName.c_str(), L"w");
  if (!File)
    return;

  SetFileAttributesW(wFileName.c_str(), FILE_ATTRIBUTE_NORMAL);

  PrintfTabs(File, 0, "CenterPos %f %f %f\n", CameraSetting.v3CenterPosition.x,
             CameraSetting.v3CenterPosition.y,
             CameraSetting.v3CenterPosition.z);
  PrintfTabs(File, 0, "CameraSetting %f %f %f\n", CameraSetting.fZoom,
             CameraSetting.fPitch, CameraSetting.fRotation);
  PrintfTabs(File, 0, "CmrPos %f %f %f\n", CameraSetting.kCmrPos.m_fUpDir,
             CameraSetting.kCmrPos.m_fViewDir,
             CameraSetting.kCmrPos.m_fCrossDir);
  PrintfTabs(
      File, 0,
      "Line "
      "\"x;%d|y;%d|z;%d|distance;%d|pitch;%d|rot;%d|up;%d|view;%d|cross;%d\"\n",
      int(CameraSetting.v3CenterPosition.x),
      int(CameraSetting.v3CenterPosition.y),
      int(CameraSetting.v3CenterPosition.z), int(CameraSetting.fZoom),
      int(CameraSetting.fPitch), int(CameraSetting.fRotation),
      int(CameraSetting.kCmrPos.m_fUpDir),
      int(CameraSetting.kCmrPos.m_fViewDir),
      int(CameraSetting.kCmrPos.m_fCrossDir));

  fclose(File);
}

bool CPythonApplication::LoadCameraSetting(const char *c_szFileName) {
  CTextFileLoader TextFileLoader;
  if (!TextFileLoader.Load(c_szFileName))
    return false;

  TextFileLoader.SetTop();

  D3DXVECTOR3 v3CenterPosition;
  CTokenVector *pCameraSetting;
  CTokenVector *pCmrPos;
  if (TextFileLoader.GetTokenVector3("centerpos", &v3CenterPosition))
    if (TextFileLoader.GetTokenVector("camerasetting", &pCameraSetting))
      if (TextFileLoader.GetTokenVector("cmrpos", &pCmrPos))
        if (3 == pCameraSetting->size())
          if (3 == pCmrPos->size()) {
            SCameraSetting CameraSetting;
            CameraSetting.v3CenterPosition = v3CenterPosition;

            CameraSetting.fZoom = atof(pCameraSetting->at(0).c_str());
            CameraSetting.fPitch = atof(pCameraSetting->at(1).c_str());
            CameraSetting.fRotation = atof(pCameraSetting->at(2).c_str());

            CameraSetting.kCmrPos.m_fUpDir = atof(pCmrPos->at(0).c_str());
            CameraSetting.kCmrPos.m_fViewDir = atof(pCmrPos->at(1).c_str());
            CameraSetting.kCmrPos.m_fCrossDir = atof(pCmrPos->at(2).c_str());

            SetEventCamera(CameraSetting);
            return true;
          }

  return false;
}
