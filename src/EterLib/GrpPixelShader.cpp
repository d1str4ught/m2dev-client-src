#include "GrpPixelShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"
#include "StdAfx.h"


#include <d3d11.h>
#include <d3dcompiler.h>
#include <utf8.h>

#include <utf8.h>

CPixelShader::CPixelShader() { Initialize(); }

CPixelShader::~CPixelShader() { Destroy(); }

void CPixelShader::Initialize() {
  m_handle = 0;
  m_pDX11Handle = nullptr;
}

void CPixelShader::Destroy() {
  if (m_pDX11Handle) {
    m_pDX11Handle->Release();
    m_pDX11Handle = nullptr;
  }
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

  // DX11: Try compiling as HLSL (ps_main entry, ps_5_0)
  if (ms_pD3D11Device) {
    ID3D10Blob *pBlob = nullptr;
    ID3D10Blob *pErr = nullptr;
    HRESULT hr11 = D3DCompileFromFile(
        wFileName.c_str(), nullptr, nullptr, "ps_main", "ps_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
        &pBlob, &pErr);
    if (SUCCEEDED(hr11) && pBlob) {
      ms_pD3D11Device->CreatePixelShader(pBlob->GetBufferPointer(),
                                         pBlob->GetBufferSize(), nullptr,
                                         &m_pDX11Handle);
      pBlob->Release();
    }
    if (pErr)
      pErr->Release();
  }

  return true;
}

void CPixelShader::Set() {
  STATEMANAGER.SetPixelShader(m_handle);

  // DX11: Bind if available
  if (m_pDX11Handle && ms_pD3D11Context)
    ms_pD3D11Context->PSSetShader(m_pDX11Handle, nullptr, 0);
}
