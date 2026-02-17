#pragma once

#include "GrpVertexBuffer.h"

class CStaticVertexBuffer : public CGraphicVertexBuffer {
public:
  CStaticVertexBuffer();
  virtual ~CStaticVertexBuffer();

  bool Create(int vtxCount, DWORD fvf, bool isManaged = true);
};
