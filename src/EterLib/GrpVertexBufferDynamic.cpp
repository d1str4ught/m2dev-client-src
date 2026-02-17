#include "StdAfx.h"
#include "GrpVertexBufferDynamic.h"

bool CDynamicVertexBuffer::Create(int vtxCount, int fvf) {
  if (m_lpd3dVB) {
    if (m_fvf == fvf) {
      if (m_vtxCount >= vtxCount)
        return true;
    }
  }

  m_vtxCount = vtxCount;
  m_fvf = fvf;

  return CGraphicVertexBuffer::Create(m_vtxCount, m_fvf,
                                      D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                      D3DPOOL_DEFAULT);
}

CDynamicVertexBuffer::CDynamicVertexBuffer() {
  m_vtxCount = 0;
  m_fvf = 0;
}

CDynamicVertexBuffer::~CDynamicVertexBuffer() {}
