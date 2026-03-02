#pragma once

#include "GrpBase.h"

// Forward declaration for DX11 buffer
struct ID3D11Buffer;

class CGraphicIndexBuffer : public CGraphicBase {
public:
  CGraphicIndexBuffer();
  virtual ~CGraphicIndexBuffer();

  void Destroy();
  bool Create(int idxCount, D3DFORMAT d3dFmt);
  bool Create(int faceCount, TFace *faces);

  bool CreateDeviceObjects();
  void DestroyDeviceObjects();

  bool Copy(int bufSize, const void *srcIndices);

  bool Lock(void **pretIndices) const;
  void Unlock() const;

  bool Lock(void **pretIndices);
  void Unlock();

  void SetIndices(int startIndex = 0) const;

  LPDIRECT3DINDEXBUFFER9 GetD3DIndexBuffer() const;

  int GetIndexCount() const { return m_iidxCount; }
  ID3D11Buffer *GetDX11Buffer() const { return m_pDX11Buffer; }

protected:
  void Initialize();

protected:
  LPDIRECT3DINDEXBUFFER9 m_lpd3dIdxBuf;
  ID3D11Buffer *m_pDX11Buffer; // DX11 index buffer (Phase 3)
  DWORD m_dwBufferSize;
  D3DFORMAT m_d3dFmt;
  int m_iidxCount;
};
