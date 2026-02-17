#include "StdAfx.h"
#include "GrpPixelShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"

#include <utf8.h>

CPixelShader::CPixelShader() { Initialize(); }

CPixelShader::~CPixelShader() { Destroy(); }

void CPixelShader::Initialize() { m_handle = 0; }

void CPixelShader::Destroy() {
  if (m_handle) {
    m_handle->Release();
    m_handle = nullptr;
  }
}

bool CPixelShader::CreateFromDiskFile(const char *c_szFileName) {
  Destroy();

  if (!c_szFileName || !*c_szFileName)
    return false;

  // UTF-8 -> UTF-16 for D3DX
  std::wstring wFileName = Utf8ToWide(c_szFileName);

  LPD3DXBUFFER lpd3dxShaderBuffer = nullptr;
  LPD3DXBUFFER lpd3dxErrorBuffer = nullptr;

  HRESULT hr =
      D3DXAssembleShaderFromFileW(wFileName.c_str(), nullptr, nullptr, 0,
                                  &lpd3dxShaderBuffer, &lpd3dxErrorBuffer);

  if (FAILED(hr)) {
    // Log compiler error text (it is ANSI/ASCII)
    if (lpd3dxErrorBuffer) {
      const char *err = (const char *)lpd3dxErrorBuffer->GetBufferPointer();
      TraceError("Shader compile error: %s", err);
    }
    return false;
  }

  CDirect3DXBuffer shaderBuffer(lpd3dxShaderBuffer);
  CDirect3DXBuffer errorBuffer(lpd3dxErrorBuffer);

  if (FAILED(ms_lpd3dDevice->CreatePixelShader(
          (DWORD *)shaderBuffer.GetPointer(), &m_handle)))
    return false;

  return true;
}

void CPixelShader::Set() { STATEMANAGER.SetPixelShader(m_handle); }
