#include "StdAfx.h"
#include "EterBase/Stl.h"
#include "GrpVertexBuffer.h"
#include "StateManager.h"

int CGraphicVertexBuffer::GetVertexStride() const {
  int retSize = D3DXGetFVFVertexSize(m_dwFVF);
  return retSize;
}

DWORD CGraphicVertexBuffer::GetFlexibleVertexFormat() const { return m_dwFVF; }

int CGraphicVertexBuffer::GetVertexCount() const { return m_vtxCount; }

void CGraphicVertexBuffer::SetStream(int stride, int layer) const {
  assert(ms_lpd3dDevice != NULL);
  STATEMANAGER.SetStreamSource(layer, m_lpd3dVB, stride);
}

bool CGraphicVertexBuffer::LockRange(unsigned count,
                                     void **pretVertices) const {
  if (!m_lpd3dVB)
    return false;

  DWORD dwLockSize = GetVertexStride() * count;
  if (FAILED(
          m_lpd3dVB->Lock(0, dwLockSize, (void **)pretVertices, m_dwLockFlag)))
    return false;

  return true;
}

bool CGraphicVertexBuffer::Lock(void **pretVertices) const {
  if (!m_lpd3dVB)
    return false;

  DWORD dwLockSize = GetVertexStride() * GetVertexCount();
  if (FAILED(
          m_lpd3dVB->Lock(0, dwLockSize, (void **)pretVertices, m_dwLockFlag)))
    return false;

  return true;
}

bool CGraphicVertexBuffer::Unlock() const {
  if (!m_lpd3dVB)
    return false;

  if (FAILED(m_lpd3dVB->Unlock()))
    return false;
  return true;
}

bool CGraphicVertexBuffer::IsEmpty() const { return m_lpd3dVB == nullptr; }

bool CGraphicVertexBuffer::LockDynamic(void **pretVertices) {
  if (!m_lpd3dVB)
    return false;

  if (FAILED(m_lpd3dVB->Lock(0, 0, (void **)pretVertices, 0)))
    return false;

  return true;
}

bool CGraphicVertexBuffer::Lock(void **pretVertices) {
  if (!m_lpd3dVB)
    return false;

  if (FAILED(m_lpd3dVB->Lock(0, 0, (void **)pretVertices, m_dwLockFlag)))
    return false;

  return true;
}

bool CGraphicVertexBuffer::Unlock() {
  if (!m_lpd3dVB)
    return false;

  if (FAILED(m_lpd3dVB->Unlock()))
    return false;
  return true;
}

bool CGraphicVertexBuffer::Copy(int bufSize, const void *srcVertices) {
  void *dstVertices;

  if (!Lock(&dstVertices))
    return false;

  memcpy(dstVertices, srcVertices, bufSize);

  Unlock();
  return true;
}

bool CGraphicVertexBuffer::CreateDeviceObjects() {
  assert(ms_lpd3dDevice != NULL);
  assert(m_lpd3dVB == NULL);

  if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(
          m_dwBufferSize, m_dwUsage, m_dwFVF, m_d3dPool, &m_lpd3dVB, nullptr)))
    return false;

  return true;
}

void CGraphicVertexBuffer::DestroyDeviceObjects() { safe_release(m_lpd3dVB); }

bool CGraphicVertexBuffer::Create(int vtxCount, DWORD fvf, DWORD usage,
                                  D3DPOOL d3dPool) {
  assert(ms_lpd3dDevice != NULL);
  assert(vtxCount > 0);

  Destroy();

  m_vtxCount = vtxCount;
  m_dwBufferSize = D3DXGetFVFVertexSize(fvf) * m_vtxCount;
  m_d3dPool = d3dPool;
  m_dwUsage = usage;
  m_dwFVF = fvf;

  if (usage == D3DUSAGE_WRITEONLY || usage == D3DUSAGE_DYNAMIC)
    m_dwLockFlag = 0;
  else
    m_dwLockFlag = D3DLOCK_READONLY;

  return CreateDeviceObjects();
}

void CGraphicVertexBuffer::Destroy() { DestroyDeviceObjects(); }

void CGraphicVertexBuffer::Initialize() {
  m_lpd3dVB = NULL;
  m_vtxCount = 0;
  m_dwBufferSize = 0;
}

CGraphicVertexBuffer::CGraphicVertexBuffer() { Initialize(); }

CGraphicVertexBuffer::~CGraphicVertexBuffer() { Destroy(); }
