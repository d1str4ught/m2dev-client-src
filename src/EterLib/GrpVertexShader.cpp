#include "GrpVertexShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"
#include "StdAfx.h"


#include <d3d11.h>
#include <d3dcompiler.h>
#include <utf8.h>

#include <utf8.h>

CVertexShader::CVertexShader() { Initialize(); }

CVertexShader::~CVertexShader() { Destroy(); }

void CVertexShader::Initialize() {
  m_handle = 0;
  m_pDX11Handle = nullptr;
}

void CVertexShader::Destroy() {
  if (m_pDX11Handle) {
    m_pDX11Handle->Release();
    m_pDX11Handle = nullptr;
  }
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

  // DX11: Try compiling as HLSL (vs_main entry, vs_5_0)
  if (ms_pD3D11Device) {
    ID3D10Blob *pBlob = nullptr;
    ID3D10Blob *pErr = nullptr;
    HRESULT hr11 = D3DCompileFromFile(
        wFileName.c_str(), nullptr, nullptr, "vs_main", "vs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
        &pBlob, &pErr);
    if (SUCCEEDED(hr11) && pBlob) {
      ms_pD3D11Device->CreateVertexShader(pBlob->GetBufferPointer(),
                                          pBlob->GetBufferSize(), nullptr,
                                          &m_pDX11Handle);
      pBlob->Release();
    }
    if (pErr)
      pErr->Release();
  }

  return true;
}

void CVertexShader::Set() {
  STATEMANAGER.SetVertexShader(m_handle);

  // DX11: Bind if available
  if (m_pDX11Handle && ms_pD3D11Context)
    ms_pD3D11Context->VSSetShader(m_pDX11Handle, nullptr, 0);
}
