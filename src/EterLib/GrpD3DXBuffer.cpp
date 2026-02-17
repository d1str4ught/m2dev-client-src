#include "StdAfx.h"
#include "GrpD3DXBuffer.h"
#include "EterBase/Stl.h"

CDirect3DXBuffer::CDirect3DXBuffer() { m_lpd3dxBuffer = NULL; }

CDirect3DXBuffer::CDirect3DXBuffer(LPD3DXBUFFER lpd3dxBuffer) {
  m_lpd3dxBuffer = lpd3dxBuffer;
}

CDirect3DXBuffer::~CDirect3DXBuffer() { Destroy(); }

void CDirect3DXBuffer::Destroy() { safe_release(m_lpd3dxBuffer); }

void CDirect3DXBuffer::Create(LPD3DXBUFFER lpd3dxBuffer) {
  Destroy();
  m_lpd3dxBuffer = lpd3dxBuffer;
}

void *CDirect3DXBuffer::GetPointer() {
  assert(m_lpd3dxBuffer != NULL);
  return m_lpd3dxBuffer->GetBufferPointer();
}

int CDirect3DXBuffer::GetSize() {
  assert(m_lpd3dxBuffer != NULL);
  return m_lpd3dxBuffer->GetBufferSize();
}
