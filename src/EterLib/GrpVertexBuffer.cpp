#include "GrpVertexBuffer.h"
#include "EterBase/Stl.h"
#include "StateManager.h"
#include "StdAfx.h"

#include <d3d11.h>

int CGraphicVertexBuffer::GetVertexStride() const {
  int retSize = D3DXGetFVFVertexSize(m_dwFVF);
  return retSize;
}

DWORD CGraphicVertexBuffer::GetFlexibleVertexFormat() const { return m_dwFVF; }

int CGraphicVertexBuffer::GetVertexCount() const { return m_vtxCount; }

void CGraphicVertexBuffer::SetStream(int stride, int layer) const {
  assert(ms_lpd3dDevice != NULL);
  STATEMANAGER.SetStreamSource(layer, m_lpd3dVB, stride);

  // DX11: Bind vertex buffer to matching slot
  if (ms_pD3D11Context && m_pDX11Buffer) {
    UINT dx11Stride = (UINT)stride;
    UINT dx11Offset = 0;
    ms_pD3D11Context->IASetVertexBuffers((UINT)layer, 1, &m_pDX11Buffer,
                                         &dx11Stride, &dx11Offset);
  }
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

  // DX11: Create equivalent vertex buffer
  if (ms_pD3D11Device && !m_pDX11Buffer) {
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = m_dwBufferSize;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    if (m_dwUsage & D3DUSAGE_DYNAMIC) {
      bd.Usage = D3D11_USAGE_DYNAMIC;
      bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    } else {
      bd.Usage = D3D11_USAGE_DEFAULT;
      bd.CPUAccessFlags = 0;
    }
    ms_pD3D11Device->CreateBuffer(&bd, nullptr, &m_pDX11Buffer);
  }

  return true;
}

void CGraphicVertexBuffer::DestroyDeviceObjects() {
  safe_release(m_lpd3dVB);
  if (m_pDX11Buffer) {
    m_pDX11Buffer->Release();
    m_pDX11Buffer = nullptr;
  }
}

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
  m_pDX11Buffer = nullptr;
  m_vtxCount = 0;
  m_dwBufferSize = 0;
}

CGraphicVertexBuffer::CGraphicVertexBuffer() { Initialize(); }

CGraphicVertexBuffer::~CGraphicVertexBuffer() { Destroy(); }
