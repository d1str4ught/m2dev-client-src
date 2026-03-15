#include "GrpIndexBuffer.h"
#include "EterBase/Stl.h"
#include "StateManager.h"
#include "StdAfx.h"

#include <d3d11.h>

LPDIRECT3DINDEXBUFFER9 CGraphicIndexBuffer::GetD3DIndexBuffer() const {
  return m_lpd3dIdxBuf;
}

void CGraphicIndexBuffer::SetIndices(int startIndex) const {
  // DX9: bind for backward compatibility during transition
  if (ms_lpd3dDevice && m_lpd3dIdxBuf)
    STATEMANAGER.SetIndices(m_lpd3dIdxBuf, startIndex);

  // DX11: Bind index buffer
  if (ms_pD3D11Context && m_pDX11Buffer) {
    DXGI_FORMAT dxgiFmt = (m_d3dFmt == D3DFMT_INDEX32) ? DXGI_FORMAT_R32_UINT
                                                       : DXGI_FORMAT_R16_UINT;
    ms_pD3D11Context->IASetIndexBuffer(m_pDX11Buffer, dxgiFmt, 0);
  }
}

// ============================================================================
// Lock/Unlock — CPU staging buffer pattern
// ============================================================================

bool CGraphicIndexBuffer::Lock(void **pretIndices) const {
  if (!m_pStagingData)
    return false;

  *pretIndices = m_pStagingData;
  return true;
}

void CGraphicIndexBuffer::Unlock() const {
  // Sync staging data to DX11 buffer
  if (ms_pD3D11Context && m_pDX11Buffer && m_pStagingData) {
    ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                        m_pStagingData, 0, 0);
  }

  // DX9: Also unlock if DX9 buffer is still alive
  if (m_lpd3dIdxBuf)
    m_lpd3dIdxBuf->Unlock();
}

bool CGraphicIndexBuffer::Lock(void **pretIndices) {
  if (!m_pStagingData)
    return false;

  // Also lock DX9 buffer if it exists (for transition period)
  if (m_lpd3dIdxBuf) {
    void *pDummy;
    m_lpd3dIdxBuf->Lock(0, 0, &pDummy, 0);
  }

  *pretIndices = m_pStagingData;
  return true;
}

void CGraphicIndexBuffer::Unlock() {
  // Sync staging data to DX11 buffer
  if (ms_pD3D11Context && m_pDX11Buffer && m_pStagingData) {
    ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                        m_pStagingData, 0, 0);
  }

  // DX9: Also unlock if DX9 buffer is still alive
  if (m_lpd3dIdxBuf)
    m_lpd3dIdxBuf->Unlock();
}

bool CGraphicIndexBuffer::Copy(int bufSize, const void *srcIndices) {
  if (!m_pStagingData)
    return false;

  DWORD copySize =
      ((DWORD)bufSize < m_dwBufferSize) ? (DWORD)bufSize : m_dwBufferSize;
  memcpy(m_pStagingData, srcIndices, copySize);

  // Sync to DX11 buffer
  if (ms_pD3D11Context && m_pDX11Buffer)
    ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                        m_pStagingData, 0, 0);

  // DX9: Also copy for transition
  if (m_lpd3dIdxBuf) {
    BYTE *dstIndices;
    if (SUCCEEDED(m_lpd3dIdxBuf->Lock(0, 0, (void **)&dstIndices, 0))) {
      memcpy(dstIndices, srcIndices, copySize);
      m_lpd3dIdxBuf->Unlock();
    }
  }

  return true;
}

bool CGraphicIndexBuffer::Create(int faceCount, TFace *faces) {
  int idxCount = faceCount * 3;
  m_iidxCount = idxCount;
  if (!Create(idxCount, D3DFMT_INDEX16))
    return false;

  // Write face data to staging buffer
  WORD *dstIndices = (WORD *)m_pStagingData;
  for (int i = 0; i < faceCount; ++i, dstIndices += 3) {
    TFace *curFace = faces + i;
    dstIndices[0] = curFace->indices[0];
    dstIndices[1] = curFace->indices[1];
    dstIndices[2] = curFace->indices[2];
  }

  // Sync to DX11
  if (ms_pD3D11Context && m_pDX11Buffer)
    ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                        m_pStagingData, 0, 0);

  // DX9: Also sync for transition
  if (m_lpd3dIdxBuf) {
    BYTE *pData;
    if (SUCCEEDED(m_lpd3dIdxBuf->Lock(0, 0, (void **)&pData, 0))) {
      memcpy(pData, m_pStagingData, m_dwBufferSize);
      m_lpd3dIdxBuf->Unlock();
    }
  }

  return true;
}

bool CGraphicIndexBuffer::CreateDeviceObjects() {
  // Create DX11 buffer as primary
  if (ms_pD3D11Device && !m_pDX11Buffer) {
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = m_dwBufferSize;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    if (m_pStagingData) {
      initData.pSysMem = m_pStagingData;
      ms_pD3D11Device->CreateBuffer(&bd, &initData, &m_pDX11Buffer);
    } else {
      ms_pD3D11Device->CreateBuffer(&bd, nullptr, &m_pDX11Buffer);
    }
  }

  // DX9: Create for backward compatibility during transition
  if (ms_lpd3dDevice && !m_lpd3dIdxBuf) {
    ms_lpd3dDevice->CreateIndexBuffer(m_dwBufferSize, D3DUSAGE_WRITEONLY,
                                      m_d3dFmt, D3DPOOL_MANAGED, &m_lpd3dIdxBuf,
                                      NULL);
    // Not fatal if DX9 creation fails — DX11 is primary
  }

  return m_pDX11Buffer != nullptr;
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

  // Allocate CPU staging buffer
  m_dwStagingSize = m_dwBufferSize;
  m_pStagingData = new BYTE[m_dwStagingSize];
  memset(m_pStagingData, 0, m_dwStagingSize);

  return CreateDeviceObjects();
}

void CGraphicIndexBuffer::Destroy() {
  DestroyDeviceObjects();

  if (m_pStagingData) {
    delete[] m_pStagingData;
    m_pStagingData = nullptr;
  }
  m_dwStagingSize = 0;
}

void CGraphicIndexBuffer::Initialize() {
  m_lpd3dIdxBuf = NULL;
  m_pDX11Buffer = nullptr;
  m_pStagingData = nullptr;
  m_dwStagingSize = 0;
}

CGraphicIndexBuffer::CGraphicIndexBuffer() { Initialize(); }

CGraphicIndexBuffer::~CGraphicIndexBuffer() { Destroy(); }
