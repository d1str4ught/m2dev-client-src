#include "StdAfx.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"

#include "EterBase/Random.h"
#include "EterLib/Camera.h"
#include "EterLib/StateManager.h"

CDynamicPool<CParticleInstance> CParticleInstance::ms_kPool;

void CParticleInstance::DestroySystem() { ms_kPool.Destroy(); }

CParticleInstance *CParticleInstance::New() { return ms_kPool.Alloc(); }

void CParticleInstance::DeleteThis() {
  Destroy();

  ms_kPool.Free(this);
}

float CParticleInstance::GetRadiusApproximation() {
  return m_v2HalfSize.y * m_v2Scale.y + m_v2HalfSize.x * m_v2Scale.x;
}

BOOL CParticleInstance::Update(float fElapsedTime, float fAngle) {
  m_fLastLifeTime -= fElapsedTime;
  if (m_fLastLifeTime < 0.0f)
    return FALSE;

  float fLifePercentage = (m_fLifeTime - m_fLastLifeTime) / m_fLifeTime;

  UpdateRotation(fLifePercentage, fElapsedTime);
  UpdateTextureAnimation(fLifePercentage, fElapsedTime);
  UpdateScale(fLifePercentage, fElapsedTime);
  UpdateColor(fLifePercentage, fElapsedTime);
  UpdateGravity(fLifePercentage, fElapsedTime);
  UpdateAirResistance(fLifePercentage, fElapsedTime);

  m_v3LastPosition = m_v3Position;
  m_v3Position += m_v3Velocity * fElapsedTime;

  if (fAngle) {
    if (m_pParticleProperty->m_bAttachFlag) {
      float fCos, fSin;
      fAngle = D3DXToRadian(fAngle);
      fCos = cos(fAngle);
      fSin = sin(fAngle);

      float rx = m_v3Position.x - m_v3StartPosition.x;
      float ry = m_v3Position.y - m_v3StartPosition.y;

      m_v3Position.x = fCos * rx + fSin * ry + m_v3StartPosition.x;
      m_v3Position.y = -fSin * rx + fCos * ry + m_v3StartPosition.y;
    } else {
      D3DXQUATERNION q, qc;
      D3DXQuaternionRotationAxis(&q, &m_pParticleProperty->m_v3ZAxis,
                                 D3DXToRadian(fAngle));
      D3DXQuaternionConjugate(&qc, &q);

      D3DXQUATERNION qr(m_v3Position.x - m_v3StartPosition.x,
                        m_v3Position.y - m_v3StartPosition.y,
                        m_v3Position.z - m_v3StartPosition.z, 0.0f);
      D3DXQuaternionMultiply(&qr, &q, &qr);
      D3DXQuaternionMultiply(&qr, &qr, &qc);

      m_v3Position.x = qr.x;
      m_v3Position.y = qr.y;
      m_v3Position.z = qr.z;

      m_v3Position += m_v3StartPosition;
    }
  }

  return TRUE;
}

void CParticleInstance::UpdateRotation(float time, float elapsedTime) {
  if (m_rotationType == CParticleProperty::ROTATION_TYPE_NONE)
    return;

  if (m_rotationType == CParticleProperty::ROTATION_TYPE_TIME_EVENT)
    m_fRotationSpeed =
        GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventRotation);

  m_fRotation += m_fRotationSpeed * elapsedTime;
}

void CParticleInstance::UpdateTextureAnimation(float time, float elapsedTime) {
  if (m_byTextureAnimationType ==
      CParticleProperty::TEXTURE_ANIMATION_TYPE_NONE)
    return;

  const float frameDelay = m_pParticleProperty->GetTextureAnimationFrameDelay();
  const DWORD frameCount = m_pParticleProperty->GetTextureAnimationFrameCount();

  m_fFrameTime += elapsedTime;

  const uint64_t elapsedFrames =
      static_cast<uint64_t>(m_fFrameTime / frameDelay);

  if (0 == elapsedFrames)
    return;

  m_fFrameTime -= elapsedFrames * frameDelay;

  switch (m_byTextureAnimationType) {
  case CParticleProperty::TEXTURE_ANIMATION_TYPE_CW:
    m_byFrameIndex += elapsedFrames;
    if (m_byFrameIndex >= frameCount)
      m_byFrameIndex = 0;
    break;

  case CParticleProperty::TEXTURE_ANIMATION_TYPE_CCW:
    m_byFrameIndex =
        std::min<uint8_t>(m_byFrameIndex - elapsedFrames, frameCount - 1);
    break;

  case CParticleProperty::TEXTURE_ANIMATION_TYPE_RANDOM_FRAME:
    if (frameCount != 0)
      m_byFrameIndex = random_range(0, frameCount - 1);
    break;

  default:
    break;
  }
}

void CParticleInstance::UpdateScale(float time, float elapsedTime) {
  if (!m_pParticleProperty->m_TimeEventScaleX.empty())
    m_v2Scale.x =
        GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventScaleX);

  if (!m_pParticleProperty->m_TimeEventScaleY.empty())
    m_v2Scale.y =
        GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventScaleY);
}

void CParticleInstance::UpdateColor(float time, float elapsedTime) {
  if (m_pParticleProperty->m_TimeEventColor.empty())
    return;

  m_Color = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventColor);
}

void CParticleInstance::UpdateGravity(float time, float elapsedTime) {
  if (m_pParticleProperty->m_TimeEventGravity.empty())
    return;

  float fGravity;
  fGravity =
      GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventGravity);

  m_v3Velocity.z -= fGravity * elapsedTime;
}

void CParticleInstance::UpdateAirResistance(float time, float elapsedTime) {
  if (m_pParticleProperty->m_TimeEventAirResistance.empty())
    return;

  m_v3Velocity *=
      1.0f - GetTimeEventBlendValue(
                 time, m_pParticleProperty->m_TimeEventAirResistance);
}

void CParticleInstance::Transform(const D3DXMATRIX *c_matLocal) {
  STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);

  D3DXVECTOR3 v3Up;
  D3DXVECTOR3 v3Cross;

  if (!m_pParticleProperty->m_bStretchFlag) {
    CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
    const D3DXVECTOR3 &c_rv3Up = pCurrentCamera->GetUp();
    const D3DXVECTOR3 &c_rv3Cross = pCurrentCamera->GetCross();

    D3DXVECTOR3 v3Rotation;

    switch (m_pParticleProperty->m_byBillboardType) {
    case BILLBOARD_TYPE_LIE: {
      float fCos = cosf(D3DXToRadian(m_fRotation)),
            fSin = sinf(D3DXToRadian(m_fRotation));
      v3Up.x = fCos;
      v3Up.y = -fSin;
      v3Up.z = 0;
      v3Cross.x = fSin;
      v3Cross.y = fCos;
      v3Cross.z = 0;
    } break;
    case BILLBOARD_TYPE_2FACE:
    case BILLBOARD_TYPE_3FACE:
      // using setting with y, and local rotation at render
    case BILLBOARD_TYPE_Y: {
      v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
      // v3Up =
      // D3DXVECTOR3(cosf(D3DXToRadian(m_fRotation)),0.0f,-sinf(D3DXToRadian(m_fRotation)));
      const D3DXVECTOR3 &c_rv3View = pCurrentCamera->GetView();
      if (v3Up.x * c_rv3View.y - v3Up.y * c_rv3View.x < 0)
        v3Up *= -1;
      auto d3dd = D3DXVECTOR3(c_rv3View.x, c_rv3View.y, 0);
      D3DXVec3Cross(&v3Cross, &v3Up, &d3dd);
      D3DXVec3Normalize(&v3Cross, &v3Cross);

      if (m_fRotation) {
        float fCos = -sinf(D3DXToRadian(m_fRotation)); // + 90
        float fSin = cosf(D3DXToRadian(m_fRotation));

        D3DXVECTOR3 v3Temp = v3Up * fCos - v3Cross * fSin;
        v3Cross = v3Cross * fCos + v3Up * fSin;
        v3Up = v3Temp;
      }

      // D3DXVECTOR3 v3Rotation;
      // D3DXVec3Cross(&v3Rotation, &v3Up, &v3Cross);

    } break;
    case BILLBOARD_TYPE_ALL:
    default: {
      // NOTE : Rotation Routine. Camera의 Up Vector와 Cross Vector 자체를 View
      // Vector 기준으로
      //        Rotation 시킨다.
      // FIXME : 반드시 최적화 할 것!
      if (m_fRotation == 0.0f) {
        v3Up = -c_rv3Cross;
        v3Cross = c_rv3Up;
      } else {
        const D3DXVECTOR3 &c_rv3View = pCurrentCamera->GetView();
        D3DXQUATERNION q, qc;
        D3DXQuaternionRotationAxis(&q, &c_rv3View, D3DXToRadian(m_fRotation));
        D3DXQuaternionConjugate(&qc, &q);

        {
          D3DXQUATERNION qr(-c_rv3Cross.x, -c_rv3Cross.y, -c_rv3Cross.z, 0);
          D3DXQuaternionMultiply(&qr, &qc, &qr);
          D3DXQuaternionMultiply(&qr, &qr, &q);
          v3Up.x = qr.x;
          v3Up.y = qr.y;
          v3Up.z = qr.z;
        }
        {
          D3DXQUATERNION qr(c_rv3Up.x, c_rv3Up.y, c_rv3Up.z, 0);
          D3DXQuaternionMultiply(&qr, &qc, &qr);
          D3DXQuaternionMultiply(&qr, &qr, &q);
          v3Cross.x = qr.x;
          v3Cross.y = qr.y;
          v3Cross.z = qr.z;
        }
      }
      // D3DXMATRIX matRotation;

      // D3DXMatrixRotationAxis(&matRotation, &c_rv3View,
      // D3DXToRadian(m_fRotation));

      // D3DXVec3TransformCoord(&v3Up, &(-c_rv3Cross), &matRotation);
      // D3DXVec3TransformCoord(&v3Cross, &c_rv3Up, &matRotation);
    } break;
    }

  } else {
    v3Up = m_v3Position - m_v3LastPosition;

    if (c_matLocal) {
      // if (!m_pParticleProperty->m_bAttachFlag)
      D3DXVec3TransformNormal(&v3Up, &v3Up, c_matLocal);
    }

    // NOTE: 속도가 길이에 주는 영향 : log(velocity)만큼 늘어난다.
    float length = D3DXVec3Length(&v3Up);
    if (length == 0.0f) {
      v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    } else
      v3Up *= (1 + log(1 + length)) / length;

    CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
    const D3DXVECTOR3 &c_rv3View = pCurrentCamera->GetView();
    D3DXVec3Cross(&v3Cross, &v3Up, &c_rv3View);
    D3DXVec3Normalize(&v3Cross, &v3Cross);
  }

  v3Cross = -(m_v2HalfSize.x * m_v2Scale.x) * v3Cross;
  v3Up = (m_v2HalfSize.y * m_v2Scale.y) * v3Up;

  if (c_matLocal && m_pParticleProperty->m_bAttachFlag) {
    D3DXVECTOR3 v3Position;
    D3DXVec3TransformCoord(&v3Position, &m_v3Position, c_matLocal);
    m_ParticleMesh[0].position = v3Position - v3Up + v3Cross;
    m_ParticleMesh[1].position = v3Position - v3Up - v3Cross;
    m_ParticleMesh[2].position = v3Position + v3Up + v3Cross;
    m_ParticleMesh[3].position = v3Position + v3Up - v3Cross;
  } else {
    m_ParticleMesh[0].position = m_v3Position - v3Up + v3Cross;
    m_ParticleMesh[1].position = m_v3Position - v3Up - v3Cross;
    m_ParticleMesh[2].position = m_v3Position + v3Up + v3Cross;
    m_ParticleMesh[3].position = m_v3Position + v3Up - v3Cross;
  }
}

void CParticleInstance::Transform(const D3DXMATRIX *c_matLocal,
                                  const float c_fZRotation) {
  STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);

  D3DXVECTOR3 v3Up;
  D3DXVECTOR3 v3Cross;

  if (!m_pParticleProperty->m_bStretchFlag) {
    CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
    const D3DXVECTOR3 &c_rv3Up = pCurrentCamera->GetUp();
    const D3DXVECTOR3 &c_rv3Cross = pCurrentCamera->GetCross();

    D3DXVECTOR3 v3Rotation;

    switch (m_pParticleProperty->m_byBillboardType) {
    case BILLBOARD_TYPE_LIE: {
      float fCos = cosf(D3DXToRadian(m_fRotation)),
            fSin = sinf(D3DXToRadian(m_fRotation));
      v3Up.x = fCos;
      v3Up.y = -fSin;
      v3Up.z = 0;

      v3Cross.x = fSin;
      v3Cross.y = fCos;
      v3Cross.z = 0;
    } break;
    case BILLBOARD_TYPE_2FACE:
    case BILLBOARD_TYPE_3FACE:
      // using setting with y, and local rotation at render
    case BILLBOARD_TYPE_Y: {
      v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
      // v3Up =
      // D3DXVECTOR3(cosf(D3DXToRadian(m_fRotation)),0.0f,-sinf(D3DXToRadian(m_fRotation)));
      const D3DXVECTOR3 &c_rv3View = pCurrentCamera->GetView();
      if (v3Up.x * c_rv3View.y - v3Up.y * c_rv3View.x < 0)
        v3Up *= -1;
      auto d3dd = D3DXVECTOR3(c_rv3View.x, c_rv3View.y, 0);
      D3DXVec3Cross(&v3Cross, &v3Up, &d3dd);
      D3DXVec3Normalize(&v3Cross, &v3Cross);

      if (m_fRotation) {
        float fCos = -sinf(D3DXToRadian(m_fRotation)); // + 90
        float fSin = cosf(D3DXToRadian(m_fRotation));

        D3DXVECTOR3 v3Temp = v3Up * fCos - v3Cross * fSin;
        v3Cross = v3Cross * fCos + v3Up * fSin;
        v3Up = v3Temp;
      }

      // D3DXVECTOR3 v3Rotation;
      // D3DXVec3Cross(&v3Rotation, &v3Up, &v3Cross);

    } break;
    case BILLBOARD_TYPE_ALL:
    default: {
      // NOTE : Rotation Routine. Camera의 Up Vector와 Cross Vector 자체를 View
      // Vector 기준으로
      //        Rotation 시킨다.
      // FIXME : 반드시 최적화 할 것!
      if (m_fRotation == 0.0f) {
        v3Up = -c_rv3Cross;
        v3Cross = c_rv3Up;
      } else {
        const D3DXVECTOR3 &c_rv3View = pCurrentCamera->GetView();
        D3DXMATRIX matRotation;

        D3DXMatrixRotationAxis(&matRotation, &c_rv3View,
                               D3DXToRadian(m_fRotation));
        auto d3dd = (-c_rv3Cross);
        D3DXVec3TransformCoord(&v3Up, &d3dd, &matRotation);
        D3DXVec3TransformCoord(&v3Cross, &c_rv3Up, &matRotation);
      }
    } break;
    }
  } else {
    v3Up = m_v3Position - m_v3LastPosition;

    if (c_matLocal) {
      // if (!m_pParticleProperty->m_bAttachFlag)
      D3DXVec3TransformNormal(&v3Up, &v3Up, c_matLocal);
    }

    // NOTE: 속도가 길이에 주는 영향 : log(velocity)만큼 늘어난다.
    float length = D3DXVec3Length(&v3Up);
    if (length == 0.0f) {
      v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    } else
      v3Up *= (1 + log(1 + length)) / length;
    // D3DXVec3Normalize(&v3Up,&v3Up);
    // v3Up *= 1+log(1+length);

    CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
    const D3DXVECTOR3 &c_rv3View = pCurrentCamera->GetView();
    D3DXVec3Cross(&v3Cross, &v3Up, &c_rv3View);
    D3DXVec3Normalize(&v3Cross, &v3Cross);
  }

  if (c_fZRotation) {
    float x, y;
    float fCos = cosf(c_fZRotation);
    float fSin = sinf(c_fZRotation);

    x = v3Up.x;
    y = v3Up.y;
    v3Up.x = x * fCos - y * fSin;
    v3Up.y = y * fCos + x * fSin;

    x = v3Cross.x;
    y = v3Cross.y;
    v3Cross.x = x * fCos - y * fSin;
    v3Cross.y = y * fCos + x * fSin;
  }

  v3Cross = -(m_v2HalfSize.x * m_v2Scale.x) * v3Cross;
  v3Up = (m_v2HalfSize.y * m_v2Scale.y) * v3Up;

  if (c_matLocal && m_pParticleProperty->m_bAttachFlag) {
    D3DXVECTOR3 v3Position;
    D3DXVec3TransformCoord(&v3Position, &m_v3Position, c_matLocal);
    m_ParticleMesh[0].position = v3Position - v3Up + v3Cross;
    m_ParticleMesh[1].position = v3Position - v3Up - v3Cross;
    m_ParticleMesh[2].position = v3Position + v3Up + v3Cross;
    m_ParticleMesh[3].position = v3Position + v3Up - v3Cross;
  } else {
    m_ParticleMesh[0].position = m_v3Position - v3Up + v3Cross;
    m_ParticleMesh[1].position = m_v3Position - v3Up - v3Cross;
    m_ParticleMesh[2].position = m_v3Position + v3Up + v3Cross;
    m_ParticleMesh[3].position = m_v3Position + v3Up - v3Cross;
  }
}

void CParticleInstance::Destroy() { __Initialize(); }

void CParticleInstance::__Initialize() {
  m_v3Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_v3LastPosition = m_v3Position;
  m_v3Velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

  m_v2Scale = D3DXVECTOR2(1.0f, 1.0f);
  m_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

  m_byFrameIndex = 0;
  m_rotationType = CParticleProperty::ROTATION_TYPE_NONE;
  m_fFrameTime = 0;

  m_ParticleMesh[0].texCoord = D3DXVECTOR2(0.0f, 1.0f);
  m_ParticleMesh[1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
  m_ParticleMesh[2].texCoord = D3DXVECTOR2(1.0f, 1.0f);
  m_ParticleMesh[3].texCoord = D3DXVECTOR2(1.0f, 0.0f);
}

CParticleInstance::CParticleInstance() { __Initialize(); }

CParticleInstance::~CParticleInstance() { Destroy(); }

TPTVertex *CParticleInstance::GetParticleMeshPointer() {
  return m_ParticleMesh;
}
