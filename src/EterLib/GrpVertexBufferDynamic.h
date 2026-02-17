#pragma once

#include "GrpVertexBuffer.h"

class CDynamicVertexBuffer : public CGraphicVertexBuffer {
public:
  CDynamicVertexBuffer();
  virtual ~CDynamicVertexBuffer();

  bool Create(int vtxCount, int fvf);

protected:
  int m_vtxCount;
  int m_fvf;
};
