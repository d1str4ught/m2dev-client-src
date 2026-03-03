#include "GrpIndexBuffer.h"
#include "EterBase/Stl.h"
#include "StateManager.h"
#include "StdAfx.h"

#include <d3d11.h>

LPDIRECT3DINDEXBUFFER9 CGraphicIndexBuffer::GetD3DIndexBuffer() const {
  assert(m_lpd3dIdxBuf != NULL);
  return m_lpd3dIdxBuf;
}

void CGraphicIndexBuffer::SetIndices(int startIndex) const {
  assert(ms_lpd3dDevice != NULL);
  STATEMANAGER.SetIndices(m_lpd3dIdxBuf, startIndex);

  // DX11: Bind index buffer
  if (ms_pD3D11Context && m_pDX11Buffer) {
    DXGI_FORMAT dxgiFmt = (m_d3dFmt == D3DFMT_INDEX32) ? DXGI_FORMAT_R32_UINT
                                                       : DXGI_FORMAT_R16_UINT;
    ms_pD3D11Context->IASetIndexBuffer(m_pDX11Buffer, dxgiFmt, 0);
  }
}

bool CGraphicIndexBuffer::Lock(void **pretIndices) const {
  assert(m_lpd3dIdxBuf != NULL);

  if (!m_lpd3dIdxBuf)
    return false;

  if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, pretIndices, 0)))
    return false;

  return true;
}

void CGraphicIndexBuffer::Unlock() const {
  assert(m_lpd3dIdxBuf != NULL);

  if (!m_lpd3dIdxBuf)
    return;

  m_lpd3dIdxBuf->Unlock();
}

bool CGraphicIndexBuffer::Lock(void **pretIndices) {
  assert(m_lpd3dIdxBuf != NULL);

  if (!m_lpd3dIdxBuf)
    return false;

  if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, pretIndices, 0)))
    return false;

  return true;
}

void CGraphicIndexBuffer::Unlock() {
  assert(m_lpd3dIdxBuf != NULL);

  if (!m_lpd3dIdxBuf)
    return;

  m_lpd3dIdxBuf->Unlock();
}

bool CGraphicIndexBuffer::Copy(int bufSize, const void *srcIndices) {
  assert(m_lpd3dIdxBuf != NULL);

  BYTE *dstIndices;
  if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, (void **)&dstIndices, 0)))
    return false;

  memcpy(dstIndices, srcIndices, bufSize);

  m_lpd3dIdxBuf->Unlock();

  return true;
}

bool CGraphicIndexBuffer::Create(int faceCount, TFace *faces) {
  int idxCount = faceCount * 3;
  m_iidxCount = idxCount;
  if (!Create(idxCount, D3DFMT_INDEX16))
    return false;

  WORD *dstIndices;
  if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, (void **)&dstIndices, 0)))
    return false;

  for (int i = 0; i < faceCount; ++i, dstIndices += 3) {
    TFace *curFace = faces + i;
    dstIndices[0] = curFace->indices[0];
    dstIndices[1] = curFace->indices[1];
    dstIndices[2] = curFace->indices[2];
  }

  m_lpd3dIdxBuf->Unlock();
  return true;
}

bool CGraphicIndexBuffer::CreateDeviceObjects() {
  if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(
          m_dwBufferSize, D3DUSAGE_WRITEONLY, m_d3dFmt, D3DPOOL_DEFAULT,
          &m_lpd3dIdxBuf, NULL)))
    return false;

  // DX11: Create equivalent index buffer
  if (ms_pD3D11Device && !m_pDX11Buffer) {
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = m_dwBufferSize;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    ms_pD3D11Device->CreateBuffer(&bd, nullptr, &m_pDX11Buffer);
  }

  return true;
}

void CGraphicIndexBuffer::DestroyDeviceObjects() {
  safe_release(m_lpd3dIdxBuf);
  if (m_pDX11Buffer) {
    m_pDX11Buffer->Release();
    m_pDX11Buffer = nullptr;
  }
}

bool CGraphicIndexBuffer::Create(int idxCount, D3DFORMAT d3dFmt) {
  Destroy();

  m_iidxCount = idxCount;
  UINT bytesPerIndex = (d3dFmt == D3DFMT_INDEX32) ? 4u : 2u;
  m_dwBufferSize = bytesPerIndex * idxCount;
  m_d3dFmt = d3dFmt;

  return CreateDeviceObjects();
}

void CGraphicIndexBuffer::Destroy() { DestroyDeviceObjects(); }

void CGraphicIndexBuffer::Initialize() {
  m_lpd3dIdxBuf = NULL;
  m_pDX11Buffer = nullptr;
}

CGraphicIndexBuffer::CGraphicIndexBuffer() { Initialize(); }

CGraphicIndexBuffer::~CGraphicIndexBuffer() { Destroy(); }
