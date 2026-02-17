#include "StdAfx.h"
#include "EmitterProperty.h"

BYTE CEmitterProperty::GetEmitterShape() { return m_byEmitterShape; }
BYTE CEmitterProperty::GetEmitterAdvancedType() {
  return m_byEmitterAdvancedType;
}
BOOL CEmitterProperty::isEmitFromEdge() { return m_bEmitFromEdgeFlag; }

void CEmitterProperty::GetEmittingSize(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingSize);
}
void CEmitterProperty::GetEmittingAngularVelocity(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingAngularVelocity);
}
void CEmitterProperty::GetEmittingDirectionX(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingDirectionX);
}
void CEmitterProperty::GetEmittingDirectionY(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingDirectionY);
}
void CEmitterProperty::GetEmittingDirectionZ(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingDirectionZ);
}
void CEmitterProperty::GetEmittingVelocity(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingVelocity);
}
void CEmitterProperty::GetEmissionCountPerSecond(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmissionCountPerSecond);
}
void CEmitterProperty::GetParticleLifeTime(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventLifeTime);
}
void CEmitterProperty::GetParticleSizeX(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventSizeX);
}
void CEmitterProperty::GetParticleSizeY(float fTime, float *pfValue) {
  *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventSizeY);
}

void CEmitterProperty::Clear() {
  m_dwMaxEmissionCount = 0;

  m_fCycleLength = 0.0f;
  m_bCycleLoopFlag = FALSE;
  m_iLoopCount = 0;

  m_byEmitterShape = EMITTER_SHAPE_POINT;
  m_byEmitterAdvancedType = EMITTER_ADVANCED_TYPE_FREE;
  m_bEmitFromEdgeFlag = FALSE;
  m_v3EmittingSize = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_fEmittingRadius = 0.0f;

  m_v3EmittingDirection = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

  m_TimeEventEmittingSize.clear();
  m_TimeEventEmittingDirectionX.clear();
  m_TimeEventEmittingDirectionY.clear();
  m_TimeEventEmittingDirectionZ.clear();
  m_TimeEventEmittingVelocity.clear();
  m_TimeEventEmissionCountPerSecond.clear();
  m_TimeEventLifeTime.clear();
  m_TimeEventSizeX.clear();
  m_TimeEventSizeY.clear();
  m_TimeEventEmittingAngularVelocity.clear();
}

CEmitterProperty::CEmitterProperty() { Clear(); }
CEmitterProperty::~CEmitterProperty() {}
