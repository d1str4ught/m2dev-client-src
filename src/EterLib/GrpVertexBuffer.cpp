#include "GrpVertexBuffer.h"
#include "EterBase/Stl.h"
#include "StateManager.h"
#include "StdAfx.h"

#include <d3d11.h>

// ============================================================================
// ComputeFVFVertexSize — replaces D3DXGetFVFVertexSize
// Calculates vertex stride from DX9 FVF flags.
// ============================================================================
UINT CGraphicVertexBuffer::ComputeFVFVertexSize(DWORD dwFVF) {
  UINT size = 0;

  // Position components
  switch (dwFVF & D3DFVF_POSITION_MASK) {
  case D3DFVF_XYZ:
    size += 12;
    break; // float3
  case D3DFVF_XYZRHW:
    size += 16;
    break; // float4 (pre-transformed)
  case D3DFVF_XYZW:
    size += 16;
    break; // float4
  case D3DFVF_XYZB1:
    size += 16;
    break; // float3 + 1 blend weight
  case D3DFVF_XYZB2:
    size += 20;
    break; // float3 + 2 blend weights
  case D3DFVF_XYZB3:
    size += 24;
    break; // float3 + 3 blend weights
  case D3DFVF_XYZB4:
    size += 28;
    break; // float3 + 4 blend weights
  case D3DFVF_XYZB5:
    size += 32;
    break; // float3 + 5 blend weights
  }

  // Normal
  if (dwFVF & D3DFVF_NORMAL)
    size += 12; // float3

  // Point size
  if (dwFVF & D3DFVF_PSIZE)
    size += 4; // float

  // Diffuse color
  if (dwFVF & D3DFVF_DIFFUSE)
    size += 4; // DWORD (ARGB)

  // Specular color
  if (dwFVF & D3DFVF_SPECULAR)
    size += 4; // DWORD (ARGB)

  // Texture coordinates
  UINT numTexCoords = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  for (UINT i = 0; i < numTexCoords; ++i) {
    // Each tex coord set can be 1-4 floats, encoded in bits 16+
    UINT texCoordSize = (dwFVF >> (16 + i * 2)) & 0x3;
    switch (texCoordSize) {
    case D3DFVF_TEXTUREFORMAT1:
      size += 4;
      break; // 1 float (1D)
    case D3DFVF_TEXTUREFORMAT2:
      size += 8;
      break; // 2 floats (2D) — most common
    case D3DFVF_TEXTUREFORMAT3:
      size += 12;
      break; // 3 floats (3D)
    case D3DFVF_TEXTUREFORMAT4:
      size += 16;
      break; // 4 floats (4D)
    }
  }

  return size;
}

int CGraphicVertexBuffer::GetVertexStride() const {
  return (int)ComputeFVFVertexSize(m_dwFVF);
}

DWORD CGraphicVertexBuffer::GetFlexibleVertexFormat() const { return m_dwFVF; }

int CGraphicVertexBuffer::GetVertexCount() const { return m_vtxCount; }

void CGraphicVertexBuffer::SetStream(int stride, int layer) const {
  // DX9: bind for backward compatibility during transition
  if (ms_lpd3dDevice && m_lpd3dVB)
    STATEMANAGER.SetStreamSource(layer, m_lpd3dVB, stride);

  // DX11: Bind vertex buffer
  if (ms_pD3D11Context && m_pDX11Buffer) {
    UINT dx11Stride = (UINT)stride;
    UINT dx11Offset = 0;
    ms_pD3D11Context->IASetVertexBuffers((UINT)layer, 1, &m_pDX11Buffer,
                                         &dx11Stride, &dx11Offset);
  }
}

// ============================================================================
// Lock/Unlock — CPU staging buffer pattern
// Lock returns a pointer to a CPU staging buffer.
// Unlock syncs the staging buffer to the DX11 GPU buffer.
// ============================================================================

bool CGraphicVertexBuffer::LockRange(unsigned count,
                                     void **pretVertices) const {
  if (!m_pStagingData)
    return false;

  // For the const version, allow reading from staging buffer
  *pretVertices = m_pStagingData;
  return true;
}

bool CGraphicVertexBuffer::Lock(void **pretVertices) const {
  if (!m_pStagingData)
    return false;

  *pretVertices = m_pStagingData;
  return true;
}

bool CGraphicVertexBuffer::Unlock() const {
  // Const unlock — sync staging data to DX11 buffer
  if (ms_pD3D11Context && m_pDX11Buffer && m_pStagingData) {
    if (m_bDynamic) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      if (SUCCEEDED(ms_pD3D11Context->Map(
              m_pDX11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, m_pStagingData, m_dwBufferSize);
        ms_pD3D11Context->Unmap(m_pDX11Buffer, 0);
      }
    } else {
      ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                          m_pStagingData, 0, 0);
    }
  }

  // DX9: Also unlock if DX9 buffer is still alive
  if (m_lpd3dVB)
    m_lpd3dVB->Unlock();

  return true;
}

bool CGraphicVertexBuffer::IsEmpty() const {
  return m_pDX11Buffer == nullptr && m_lpd3dVB == nullptr;
}

bool CGraphicVertexBuffer::LockDynamic(void **pretVertices) {
  if (!m_pStagingData)
    return false;

  *pretVertices = m_pStagingData;
  return true;
}

bool CGraphicVertexBuffer::Lock(void **pretVertices) {
  if (!m_pStagingData)
    return false;

  // Also lock DX9 buffer if it exists (for transition period)
  if (m_lpd3dVB)
    m_lpd3dVB->Lock(0, 0, pretVertices, m_dwLockFlag);

  *pretVertices = m_pStagingData;
  return true;
}

bool CGraphicVertexBuffer::Unlock() {
  // Sync staging data to DX11 buffer
  if (ms_pD3D11Context && m_pDX11Buffer && m_pStagingData) {
    if (m_bDynamic) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      if (SUCCEEDED(ms_pD3D11Context->Map(
              m_pDX11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, m_pStagingData, m_dwBufferSize);
        ms_pD3D11Context->Unmap(m_pDX11Buffer, 0);
      }
    } else {
      ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                          m_pStagingData, 0, 0);
    }
  }

  // DX9: Also unlock if DX9 buffer is still alive
  if (m_lpd3dVB)
    m_lpd3dVB->Unlock();

  return true;
}

bool CGraphicVertexBuffer::Copy(int bufSize, const void *srcVertices) {
  if (!m_pStagingData)
    return false;

  // Copy to staging buffer
  DWORD copySize =
      ((DWORD)bufSize < m_dwBufferSize) ? (DWORD)bufSize : m_dwBufferSize;
  memcpy(m_pStagingData, srcVertices, copySize);

  // Sync to DX11 buffer
  if (ms_pD3D11Context && m_pDX11Buffer) {
    if (m_bDynamic) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      if (SUCCEEDED(ms_pD3D11Context->Map(
              m_pDX11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, m_pStagingData, copySize);
        ms_pD3D11Context->Unmap(m_pDX11Buffer, 0);
      }
    } else {
      ms_pD3D11Context->UpdateSubresource(m_pDX11Buffer, 0, nullptr,
                                          m_pStagingData, 0, 0);
    }
  }

  // DX9: Also copy for transition
  if (m_lpd3dVB) {
    void *dstVertices;
    if (SUCCEEDED(m_lpd3dVB->Lock(0, copySize, &dstVertices, m_dwLockFlag))) {
      memcpy(dstVertices, srcVertices, copySize);
      m_lpd3dVB->Unlock();
    }
  }

  return true;
}

bool CGraphicVertexBuffer::CreateDeviceObjects() {
  // Create DX11 buffer as primary
  if (ms_pD3D11Device && !m_pDX11Buffer) {
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = m_dwBufferSize;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    if (m_bDynamic) {
      bd.Usage = D3D11_USAGE_DYNAMIC;
      bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    } else {
      bd.Usage = D3D11_USAGE_DEFAULT;
      bd.CPUAccessFlags = 0;
    }

    // Initialize with staging data if available
    D3D11_SUBRESOURCE_DATA initData = {};
    if (m_pStagingData) {
      initData.pSysMem = m_pStagingData;
      ms_pD3D11Device->CreateBuffer(&bd, &initData, &m_pDX11Buffer);
    } else {
      ms_pD3D11Device->CreateBuffer(&bd, nullptr, &m_pDX11Buffer);
    }
  }

  // DX9: Create for backward compatibility during transition
  if (ms_lpd3dDevice && !m_lpd3dVB) {
    ms_lpd3dDevice->CreateVertexBuffer(m_dwBufferSize, m_dwUsage, m_dwFVF,
                                       m_d3dPool, &m_lpd3dVB, nullptr);
    // Not fatal if DX9 creation fails — DX11 is primary
  }

  return m_pDX11Buffer != nullptr;
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
  assert(vtxCount > 0);

  Destroy();

  m_vtxCount = vtxCount;
  m_dwBufferSize = ComputeFVFVertexSize(fvf) * m_vtxCount;
  m_d3dPool = d3dPool;
  m_dwUsage = usage;
  m_dwFVF = fvf;
  m_bDynamic = (usage & D3DUSAGE_DYNAMIC) != 0;

  if (usage == D3DUSAGE_WRITEONLY || usage == D3DUSAGE_DYNAMIC)
    m_dwLockFlag = 0;
  else
    m_dwLockFlag = D3DLOCK_READONLY;

  // Allocate CPU staging buffer
  m_dwStagingSize = m_dwBufferSize;
  m_pStagingData = new BYTE[m_dwStagingSize];
  memset(m_pStagingData, 0, m_dwStagingSize);

  return CreateDeviceObjects();
}

void CGraphicVertexBuffer::Destroy() {
  DestroyDeviceObjects();

  if (m_pStagingData) {
    delete[] m_pStagingData;
    m_pStagingData = nullptr;
  }
  m_dwStagingSize = 0;
}

void CGraphicVertexBuffer::Initialize() {
  m_lpd3dVB = NULL;
  m_pDX11Buffer = nullptr;
  m_pStagingData = nullptr;
  m_dwStagingSize = 0;
  m_vtxCount = 0;
  m_dwBufferSize = 0;
  m_bDynamic = false;
}

CGraphicVertexBuffer::CGraphicVertexBuffer() { Initialize(); }

CGraphicVertexBuffer::~CGraphicVertexBuffer() { Destroy(); }
