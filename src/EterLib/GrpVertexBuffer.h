#pragma once

#include "GrpBase.h"

// Forward declaration for DX11 buffer
struct ID3D11Buffer;

class CGraphicVertexBuffer : public CGraphicBase {
public:
  CGraphicVertexBuffer();
  virtual ~CGraphicVertexBuffer();

  void Destroy();
  virtual bool Create(int vtxCount, DWORD fvf, DWORD usage, D3DPOOL d3dPool);

  bool CreateDeviceObjects();
  void DestroyDeviceObjects();

  bool Copy(int bufSize, const void *srcVertices);

  bool LockRange(unsigned count, void **pretVertices) const;
  bool Lock(void **pretVertices) const;
  bool Unlock() const;

  bool LockDynamic(void **pretVertices);
  virtual bool Lock(void **pretVertices);
  bool Unlock();

  void SetStream(int stride, int layer = 0) const;

  int GetVertexCount() const;
  int GetVertexStride() const;
  DWORD GetFlexibleVertexFormat() const;

  inline LPDIRECT3DVERTEXBUFFER9 GetD3DVertexBuffer() const {
    return m_lpd3dVB;
  }
  inline ID3D11Buffer *GetDX11Buffer() const { return m_pDX11Buffer; }
  inline DWORD GetBufferSize() const { return m_dwBufferSize; }

  bool IsEmpty() const;

  // Compute vertex stride from FVF flags (replaces D3DXGetFVFVertexSize)
  static UINT ComputeFVFVertexSize(DWORD dwFVF);

protected:
  void Initialize();

protected:
  LPDIRECT3DVERTEXBUFFER9 m_lpd3dVB; // DX9 buffer (kept for transition)
  ID3D11Buffer *m_pDX11Buffer;       // DX11 vertex buffer (primary)

  // CPU staging buffer for Lock/Unlock pattern
  BYTE *m_pStagingData;  // CPU-side staging allocation
  DWORD m_dwStagingSize; // Size of staging allocation

  DWORD m_dwBufferSize;
  DWORD m_dwFVF;
  DWORD m_dwUsage;
  D3DPOOL m_d3dPool;
  int m_vtxCount;
  DWORD m_dwLockFlag;
  bool m_bDynamic; // true if buffer was created with dynamic usage
};
