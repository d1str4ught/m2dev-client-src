#include "StdAfx.h"
#include "GrpVertexBufferStatic.h"

bool CStaticVertexBuffer::Create(int vtxCount, DWORD fvf, bool /*isManaged*/) {
  // DirectX 9Ex: Use DEFAULT pool instead of MANAGED
  return CGraphicVertexBuffer::Create(vtxCount, fvf, D3DUSAGE_WRITEONLY,
                                      D3DPOOL_DEFAULT);
}

CStaticVertexBuffer::CStaticVertexBuffer() {}

CStaticVertexBuffer::~CStaticVertexBuffer() {}
