#include "StdAfx.h"
#include "GrpVertexShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"

#include <utf8.h>

CVertexShader::CVertexShader() { Initialize(); }

CVertexShader::~CVertexShader() { Destroy(); }

void CVertexShader::Initialize() { m_handle = 0; }

void CVertexShader::Destroy() {
  if (m_handle) {
    m_handle->Release();
    m_handle = nullptr;
  }
}

bool CVertexShader::CreateFromDiskFile(const char *c_szFileName,
                                       const DWORD *c_pdwVertexDecl) {
  Destroy();

  if (!c_szFileName || !*c_szFileName)
    return false;

  // UTF-8 → UTF-16 for D3DX
  std::wstring wFileName = Utf8ToWide(c_szFileName);

  LPD3DXBUFFER lpd3dxShaderBuffer = nullptr;
  LPD3DXBUFFER lpd3dxErrorBuffer = nullptr;

  HRESULT hr =
      D3DXAssembleShaderFromFileW(wFileName.c_str(), nullptr, nullptr, 0,
                                  &lpd3dxShaderBuffer, &lpd3dxErrorBuffer);

  if (FAILED(hr)) {
    if (lpd3dxErrorBuffer) {
      const char *err = (const char *)lpd3dxErrorBuffer->GetBufferPointer();
      TraceError("Vertex shader compile error: %s", err);
    }
    return false;
  }

  if (FAILED(ms_lpd3dDevice->CreateVertexShader(
          (const DWORD *)lpd3dxShaderBuffer->GetBufferPointer(), &m_handle)))
    return false;

  return true;
}

void CVertexShader::Set() { STATEMANAGER.SetVertexShader(m_handle); }
