#include "GrpDevice.h"
#include "DX11PostProcess.h"
#include "DX11ShadowMap.h"
#include "EterBase/Debug.h"
#include "EterBase/Stl.h"
#include "StdAfx.h"

#include <intrin.h>
#include <utf8.h>

bool CPU_HAS_SSE2 = false;
bool GRAPHICS_CAPS_CAN_NOT_DRAW_LINE = false;
bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = false;
bool GRAPHICS_CAPS_HALF_SIZE_IMAGE = false;
bool GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = false;
bool GRAPHICS_CAPS_SOFTWARE_TILING = false;

D3DPRESENT_PARAMETERS g_kD3DPP;
bool g_isBrowserMode = false;
RECT g_rcBrowser;

CGraphicDevice::CGraphicDevice() : m_uBackBufferCount(0) { __Initialize(); }

CGraphicDevice::~CGraphicDevice() { Destroy(); }

void CGraphicDevice::__Initialize() {
  ms_lpd3d = NULL;
  ms_lpd3dDevice = NULL;
  ms_lpd3dMatStack = NULL;

  ms_dwWavingEndTime = 0;
  ms_dwFlashingEndTime = 0;

  m_pStateManager = NULL;

  __InitializeDefaultIndexBufferList();
  __InitializePDTVertexBufferList();
}

void CGraphicDevice::RegisterWarningString(UINT uiMsg, const char *c_szString) {
  m_kMap_strWarningMessage[uiMsg] = c_szString;
}

void CGraphicDevice::__WarningMessage(HWND hWnd, UINT uiMsg) {
  auto it = m_kMap_strWarningMessage.find(uiMsg);
  if (it == m_kMap_strWarningMessage.end())
    return;

  const std::string &msgUtf8 = it->second;

  std::wstring wMsg = Utf8ToWide(msgUtf8);
  std::wstring wCaption = L"Warning"; // static wide literal is fine

  MessageBoxW(hWnd, wMsg.c_str(), wCaption.c_str(), MB_OK | MB_TOPMOST);
}

void CGraphicDevice::MoveWebBrowserRect(const RECT &c_rcWebPage) {
  g_rcBrowser = c_rcWebPage;
}

void CGraphicDevice::EnableWebBrowserMode(const RECT &c_rcWebPage) {
  if (!ms_lpd3dDevice)
    return;

  D3DPRESENT_PARAMETERS &rkD3DPP = ms_d3dPresentParameter;

  g_isBrowserMode = true;

  if (D3DSWAPEFFECT_COPY == rkD3DPP.SwapEffect)
    return;

  g_kD3DPP = rkD3DPP;
  g_rcBrowser = c_rcWebPage;

  // rkD3DPP.Windowed=TRUE;
  rkD3DPP.SwapEffect = D3DSWAPEFFECT_COPY;
  rkD3DPP.BackBufferCount = 1;
  rkD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

  IDirect3DDevice9Ex &rkD3DDev = *ms_lpd3dDevice;
  HRESULT hr = rkD3DDev.Reset(&rkD3DPP);
  if (FAILED(hr))
    return;

  STATEMANAGER.SetDefaultState();
}

void CGraphicDevice::DisableWebBrowserMode() {
  if (!ms_lpd3dDevice)
    return;

  D3DPRESENT_PARAMETERS &rkD3DPP = ms_d3dPresentParameter;

  g_isBrowserMode = false;

  rkD3DPP = g_kD3DPP;

  IDirect3DDevice9Ex &rkD3DDev = *ms_lpd3dDevice;
  HRESULT hr = rkD3DDev.Reset(&rkD3DPP);
  if (FAILED(hr))
    return;

  STATEMANAGER.SetDefaultState();
}

bool CGraphicDevice::ResizeBackBuffer(UINT uWidth, UINT uHeight) {
  if (!ms_lpd3dDevice)
    return false;

  D3DPRESENT_PARAMETERS &rkD3DPP = ms_d3dPresentParameter;
  if (rkD3DPP.Windowed) {
    if (rkD3DPP.BackBufferWidth != uWidth ||
        rkD3DPP.BackBufferHeight != uHeight) {
      rkD3DPP.BackBufferWidth = uWidth;
      rkD3DPP.BackBufferHeight = uHeight;

      IDirect3DDevice9Ex &rkD3DDev = *ms_lpd3dDevice;

      HRESULT hr = rkD3DDev.Reset(&rkD3DPP);
      if (FAILED(hr)) {
        return false;
      }

      STATEMANAGER.SetDefaultState();
    }
  }

  return true;
}

LPDIRECT3DVERTEXDECLARATION9 CGraphicDevice::CreatePNTStreamVertexShader() {
  assert(ms_lpd3dDevice != NULL);

  LPDIRECT3DVERTEXDECLARATION9 dwShader = NULL;

  D3DVERTEXELEMENT9 pShaderDecl[] = {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,
       0},
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,
       0},
      {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,
       0},
      D3DDECL_END()};

  if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) !=
      D3D_OK) {
    wchar_t szError[1024];
    swprintf(szError, L"Failed to create CreatePNTStreamVertexShader");
    MessageBoxW(NULL, szError, L"Vertex Shader Error", MB_ICONSTOP);
  }

  return dwShader;
}

LPDIRECT3DVERTEXDECLARATION9 CGraphicDevice::CreatePNT2StreamVertexShader() {
  assert(ms_lpd3dDevice != NULL);

  LPDIRECT3DVERTEXDECLARATION9 dwShader = NULL;

  D3DVERTEXELEMENT9 pShaderDecl[] = {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,
       0},
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,
       0},
      {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,
       0},
      {0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,
       1},
      D3DDECL_END()};

  if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) !=
      D3D_OK) {
    wchar_t szError[1024];
    swprintf(szError, L"Failed to create CreatePNT2StreamVertexShader");
    MessageBoxW(NULL, szError, L"Vertex Shader Error", MB_ICONSTOP);
  }

  return dwShader;
}

LPDIRECT3DVERTEXDECLARATION9 CGraphicDevice::CreatePTStreamVertexShader() {
  assert(ms_lpd3dDevice != NULL);

  LPDIRECT3DVERTEXDECLARATION9 dwShader = NULL;

  D3DVERTEXELEMENT9 pShaderDecl[] = {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,
       0},
      {1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,
       0},
      D3DDECL_END()};

  if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) !=
      D3D_OK) {
    wchar_t szError[1024];
    swprintf(szError, L"Failed to create CreatePTStreamVertexShader");
    MessageBoxW(NULL, szError, L"Vertex Shader Error", MB_ICONSTOP);
  }

  return dwShader;
}

LPDIRECT3DVERTEXDECLARATION9
CGraphicDevice::CreateDoublePNTStreamVertexShader() {
  assert(ms_lpd3dDevice != NULL);

  LPDIRECT3DVERTEXDECLARATION9 dwShader = NULL;

  D3DVERTEXELEMENT9 pShaderDecl[] = {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,
       0},
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,
       0},
      {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,
       0},

      {1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,
       1},
      {1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,
       1},
      {1, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,
       1},
      D3DDECL_END()};

  if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) !=
      D3D_OK) {
    wchar_t szError[1024];
    swprintf(szError, L"Failed to create CreateDoublePNTStreamVertexShader");
    MessageBoxW(NULL, szError, L"Vertex Shader Error", MB_ICONSTOP);
  }

  return dwShader;
}

CGraphicDevice::EDeviceState CGraphicDevice::GetDeviceState() {
  if (!ms_lpd3dDevice)
    return DEVICESTATE_NULL;

  HRESULT hr;

  if (FAILED(hr = ms_lpd3dDevice->TestCooperativeLevel())) {
    if (D3DERR_DEVICELOST == hr)
      return DEVICESTATE_BROKEN;

    if (D3DERR_DEVICENOTRESET == hr)
      return DEVICESTATE_NEEDS_RESET;

    return DEVICESTATE_BROKEN;
  }

  return DEVICESTATE_OK;
}

bool CGraphicDevice::Reset() {
  HRESULT hr;

  if (FAILED(hr = ms_lpd3dDevice->Reset(&ms_d3dPresentParameter)))
    return false;

  return true;
}

static LPDIRECT3DSURFACE9 s_lpStencil;
static DWORD s_MaxTextureWidth, s_MaxTextureHeight;

BOOL EL3D_ConfirmDevice(D3DCAPS9 &rkD3DCaps, UINT uBehavior) {
  // PUREDEVICE´Â GetTransform / GetViewport µîÀÌ µÇÁö ¾Ê´Â´Ù.
  if (uBehavior & D3DCREATE_PUREDEVICE)
    return FALSE;

  if (uBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) {
    // DirectionalLight
    if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS))
      return FALSE;

    // PositionalLight
    if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS))
      return FALSE;

    // Shadow/Terrain
    if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN))
      return FALSE;
  }

  s_MaxTextureWidth = rkD3DCaps.MaxTextureWidth;
  s_MaxTextureHeight = rkD3DCaps.MaxTextureHeight;

  return TRUE;
}

DWORD GetMaxTextureWidth() { return s_MaxTextureWidth; }

DWORD GetMaxTextureHeight() { return s_MaxTextureHeight; }

int CGraphicDevice::Create(HWND hWnd, int iHres, int iVres, bool Windowed,
                           int /*iBit*/, int iReflashRate) {
  int iRet = CREATE_OK;

  Destroy();

  ms_iWidth = iHres;
  ms_iHeight = iVres;

  ms_hWnd = hWnd;
  ms_hDC = GetDC(hWnd);

  // =========================================================================
  // PHASE 1: Create DX11 device FIRST — it owns the real window/swap chain
  // =========================================================================
  if (!__CreateDX11Device(hWnd, iHres, iVres, Windowed)) {
    Tracenf("FATAL: DX11 device creation failed");
    return CREATE_NO_DIRECTX;
  }

  // =========================================================================
  // Create DX9 device with a HIDDEN dummy window (internal computation only)
  // DX9 is used temporarily for subsystems not yet converted to DX11
  // =========================================================================
  Direct3DCreate9Ex(D3D_SDK_VERSION, &ms_lpd3d);

  if (!ms_lpd3d)
    return CREATE_NO_DIRECTX;

  D3DADAPTER_IDENTIFIER9 d3dAdapterId;
  ms_lpd3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &d3dAdapterId);

  D3DDISPLAYMODE d3dDisplayMode;
  ms_lpd3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3dDisplayMode);

  if (Windowed && strnicmp(d3dAdapterId.Driver, "3dfx", 4) == 0 &&
      D3DFMT_X8R8G8B8 == d3dDisplayMode.Format) {
    return CREATE_FORMAT;
  }

  D3DCAPS9 d3dCaps;
  ms_lpd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);

  BOOL isFormatConfirmed = FALSE;
  if (d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
    if (d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
      ms_dwD3DBehavior =
          D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
      isFormatConfirmed = EL3D_ConfirmDevice(d3dCaps, ms_dwD3DBehavior);
    }

    if (!isFormatConfirmed) {
      ms_dwD3DBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
      isFormatConfirmed = EL3D_ConfirmDevice(d3dCaps, ms_dwD3DBehavior);
    }

    if (!isFormatConfirmed) {
      ms_dwD3DBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;
      isFormatConfirmed = EL3D_ConfirmDevice(d3dCaps, ms_dwD3DBehavior);
    }
  }

  if (!isFormatConfirmed) {
    ms_dwD3DBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    isFormatConfirmed = EL3D_ConfirmDevice(d3dCaps, ms_dwD3DBehavior);
    iRet |= CREATE_NO_TNL;
  }

  int ErrorCorrection = 0;
  bool disableMSAA = false;

  // Create a hidden 1x1 dummy window for DX9 (it must NOT touch the real
  // window)
  HWND hDummyWnd =
      CreateWindowExA(0, "STATIC", "DX9Dummy", WS_POPUP, 0, 0, 1, 1, nullptr,
                      nullptr, GetModuleHandle(nullptr), nullptr);

RETRY:
  ZeroMemory(&ms_d3dPresentParameter, sizeof(ms_d3dPresentParameter));

  ms_d3dPresentParameter.Windowed = TRUE; // Always windowed for dummy
  ms_d3dPresentParameter.BackBufferWidth = 1;
  ms_d3dPresentParameter.BackBufferHeight = 1;
  ms_d3dPresentParameter.hDeviceWindow = hDummyWnd;
  ms_d3dPresentParameter.BackBufferFormat = D3DFMT_UNKNOWN;
  ms_d3dPresentParameter.BackBufferCount = 1;
  ms_d3dPresentParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;
  ms_d3dPresentParameter.MultiSampleType = D3DMULTISAMPLE_NONE;
  ms_d3dPresentParameter.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
  ms_d3dPresentParameter.FullScreen_RefreshRateInHz = 0;
  ms_d3dPresentParameter.EnableAutoDepthStencil = TRUE;
  ms_d3dPresentParameter.AutoDepthStencilFormat = D3DFMT_D24S8;

  // Store real dimensions for use by other systems
  // (ms_d3dPresentParameter dimensions are 1x1 for dummy window)

  if (FAILED(ms_hLastResult = ms_lpd3d->CreateDeviceEx(
                 D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hDummyWnd,
                 ms_dwD3DBehavior, &ms_d3dPresentParameter, nullptr,
                 &ms_lpd3dDevice))) {
    switch (ms_hLastResult) {
    case D3DERR_INVALIDCALL:
      Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_INVALIDCALL\nThe "
             "method call is invalid. For example, a method's parameter may "
             "have an invalid value.");
      break;
    case D3DERR_NOTAVAILABLE:
      Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_NOTAVAILABLE\nThis "
             "device does not support the queried technique. ");
      break;
    case D3DERR_OUTOFVIDEOMEMORY:
      Tracen("IDirect3DDevice.CreateDevice - ERROR "
             "D3DERR_OUTOFVIDEOMEMORY\nDirect3D does not have enough display "
             "memory to perform the operation");
      break;
    default:
      Tracenf("IDirect3DDevice.CreateDevice - ERROR %d", ms_hLastResult);
      break;
    }

    if (ErrorCorrection) {
      DestroyWindow(hDummyWnd);
      return CREATE_DEVICE;
    }

    disableMSAA = true;
    iReflashRate = 0;
    ++ErrorCorrection;
    iRet = CREATE_REFRESHRATE;
    goto RETRY;
  }

  // Restore real backbuffer dimensions for systems that read them
  ms_d3dPresentParameter.BackBufferWidth = iHres;
  ms_d3dPresentParameter.BackBufferHeight = iVres;
  ms_d3dPresentParameter.hDeviceWindow = hWnd; // Systems read this

  // Check DXT Support Info
  const D3DFORMAT baseFormatForTextureCheck = d3dDisplayMode.Format;
  if (ms_lpd3d->CheckDeviceFormat(
          D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, baseFormatForTextureCheck, 0,
          D3DRTYPE_TEXTURE, D3DFMT_DXT1) == D3DERR_NOTAVAILABLE) {
    ms_bSupportDXT = false;
  }

  if (ms_lpd3d->CheckDeviceFormat(
          D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, baseFormatForTextureCheck, 0,
          D3DRTYPE_TEXTURE, D3DFMT_DXT3) == D3DERR_NOTAVAILABLE) {
    ms_bSupportDXT = false;
  }

  if (ms_lpd3d->CheckDeviceFormat(
          D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, baseFormatForTextureCheck, 0,
          D3DRTYPE_TEXTURE, D3DFMT_DXT5) == D3DERR_NOTAVAILABLE) {
    ms_bSupportDXT = false;
  }

  if (FAILED((ms_hLastResult = ms_lpd3dDevice->GetDeviceCaps(&ms_d3dCaps)))) {
    Tracenf("IDirect3DDevice.GetDeviceCaps - ERROR %d", ms_hLastResult);
    return CREATE_GET_DEVICE_CAPS2;
  }

  if (!Windowed)
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, iHres, iVres, SWP_SHOWWINDOW);

  ms_lpd3dDevice->GetViewport(&ms_Viewport);

  m_pStateManager = new CStateManager(ms_lpd3dDevice);

  D3DXCreateMatrixStack(0, &ms_lpd3dMatStack);
  ms_lpd3dMatStack->LoadIdentity();

  ms_ptVS = CreatePTStreamVertexShader();
  ms_pntVS = CreatePNTStreamVertexShader();
  ms_pnt2VS = CreatePNT2StreamVertexShader();

  D3DXMatrixIdentity(&ms_matIdentity);
  D3DXMatrixIdentity(&ms_matView);
  D3DXMatrixIdentity(&ms_matProj);
  D3DXMatrixIdentity(&ms_matInverseView);
  D3DXMatrixIdentity(&ms_matInverseViewYAxis);
  D3DXMatrixIdentity(&ms_matScreen0);
  D3DXMatrixIdentity(&ms_matScreen1);
  D3DXMatrixIdentity(&ms_matScreen2);

  ms_matScreen0._11 = 1;
  ms_matScreen0._22 = -1;

  ms_matScreen1._41 = 1;
  ms_matScreen1._42 = 1;

  ms_matScreen2._11 = (float)iHres / 2;
  ms_matScreen2._22 = (float)iVres / 2;

  D3DXCreateSphere(ms_lpd3dDevice, 1.0f, 32, 32, &ms_lpSphereMesh, NULL);
  D3DXCreateCylinder(ms_lpd3dDevice, 1.0f, 1.0f, 1.0f, 8, 8, &ms_lpCylinderMesh,
                     NULL);

  ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0xff000000, 1.0f, 0);

  if (!__CreateDefaultIndexBufferList())
    return false;

  if (!__CreatePDTVertexBufferList())
    return false;

  DWORD dwTexMemSize = GetAvailableTextureMemory();

  if (dwTexMemSize < 64 * 1024 * 1024)
    ms_isLowTextureMemory = true;
  else
    ms_isLowTextureMemory = false;

  if (dwTexMemSize > 100 * 1024 * 1024)
    ms_isHighTextureMemory = true;
  else
    ms_isHighTextureMemory = false;

  if (ms_d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
    GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = false;
  else
    GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = true;

  if (strnicmp(d3dAdapterId.Driver, "SIS", 3) == 0) {
    GRAPHICS_CAPS_CAN_NOT_DRAW_LINE = true;
    GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = true;
    GRAPHICS_CAPS_HALF_SIZE_IMAGE = true;
    ms_isLowTextureMemory = true;
  } else if (strnicmp(d3dAdapterId.Driver, "3dfx", 4) == 0) {
    GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = true;
    GRAPHICS_CAPS_HALF_SIZE_IMAGE = true;
    ms_isLowTextureMemory = true;
  }

  // CPU check
  int cpuInfo[4] = {0};
  __cpuid(cpuInfo, 1);

  CPU_HAS_SSE2 = cpuInfo[3] & (1 << 26);

  return (iRet);
}

// ============================================================================
// DX11 Device Lifecycle (Phase 1)
// ============================================================================

bool CGraphicDevice::__CreateDX11Device(HWND hWnd, int iWidth, int iHeight,
                                        bool bWindowed) {
  DXGI_SWAP_CHAIN_DESC swapDesc = {};
  swapDesc.BufferCount = 1;
  swapDesc.BufferDesc.Width = iWidth;
  swapDesc.BufferDesc.Height = iHeight;
  swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapDesc.BufferDesc.RefreshRate.Numerator = 60;
  swapDesc.BufferDesc.RefreshRate.Denominator = 1;
  swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapDesc.OutputWindow = hWnd;
  swapDesc.Windowed = bWindowed ? TRUE : FALSE;
  swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  // MSAA: Try 4x, fallback to 1x
  UINT msaaSampleCount = 4;
  UINT msaaQuality = 0;
  // We need a temporary device to check MSAA support
  // For now, default to 4x MSAA — validated after device creation
  swapDesc.SampleDesc.Count = 1; // Start with 1x, upgrade after device creation
  swapDesc.SampleDesc.Quality = 0;

  UINT createFlags = 0;
#ifdef _DEBUG
  createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };

  HRESULT hr = D3D11CreateDeviceAndSwapChain(
      nullptr, // default adapter
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr, // no software rasterizer
      createFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION,
      &swapDesc, &ms_pSwapChain, &ms_pD3D11Device, &ms_featureLevel,
      &ms_pD3D11Context);

  if (FAILED(hr)) {
    Tracenf("D3D11CreateDeviceAndSwapChain failed: 0x%08X", hr);
    return false;
  }

  Tracenf("DX11 device created — Feature Level: 0x%X", (UINT)ms_featureLevel);

  // Create render target view from back buffer
  ID3D11Texture2D *pBackBuffer = nullptr;
  hr = ms_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                (void **)&pBackBuffer);
  if (FAILED(hr)) {
    Tracenf("GetBuffer failed: 0x%08X", hr);
    return false;
  }

  hr = ms_pD3D11Device->CreateRenderTargetView(pBackBuffer, nullptr,
                                               &ms_pRenderTargetView);
  pBackBuffer->Release();

  if (FAILED(hr)) {
    Tracenf("CreateRenderTargetView failed: 0x%08X", hr);
    return false;
  }

  // Create depth stencil
  if (!__CreateDX11DepthStencil(iWidth, iHeight))
    return false;

  // Bind render target + depth
  ms_pD3D11Context->OMSetRenderTargets(1, &ms_pRenderTargetView,
                                       ms_pDepthStencilView);

  // Set viewport
  D3D11_VIEWPORT viewport = {};
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;
  viewport.Width = (float)iWidth;
  viewport.Height = (float)iHeight;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  ms_pD3D11Context->RSSetViewports(1, &viewport);

  // Create shared texture for DX9->DX11 frame bridge (fallback)
  {
    D3D11_TEXTURE2D_DESC sharedDesc = {};
    sharedDesc.Width = iWidth;
    sharedDesc.Height = iHeight;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // matches DX9 back buffer
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = D3D11_USAGE_DEFAULT;
    sharedDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    hr = ms_pD3D11Device->CreateTexture2D(&sharedDesc, nullptr,
                                          &ms_pSharedTexture);
    if (SUCCEEDED(hr))
      ms_pD3D11Device->CreateShaderResourceView(ms_pSharedTexture, nullptr,
                                                &ms_pSharedSRV);
  }

  // Create native DX11 scene render target (Phase 2A)
  {
    D3D11_TEXTURE2D_DESC sceneDesc = {};
    sceneDesc.Width = iWidth;
    sceneDesc.Height = iHeight;
    sceneDesc.MipLevels = 1;
    sceneDesc.ArraySize = 1;
    sceneDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sceneDesc.SampleDesc.Count = 1;
    sceneDesc.Usage = D3D11_USAGE_DEFAULT;
    sceneDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    hr = ms_pD3D11Device->CreateTexture2D(&sceneDesc, nullptr, &ms_pSceneRT);
    if (SUCCEEDED(hr)) {
      ms_pD3D11Device->CreateRenderTargetView(ms_pSceneRT, nullptr,
                                              &ms_pSceneRTV);
      ms_pD3D11Device->CreateShaderResourceView(ms_pSceneRT, nullptr,
                                                &ms_pSceneSRV);
    }
  }

  // Initialize post-processing pipeline
  ms_pPostProcess = new CDX11PostProcess();
  ms_pPostProcess->Initialize(ms_pD3D11Device, ms_pD3D11Context, ms_pSwapChain,
                              iWidth, iHeight);

  // Initialize shadow map (Phase 2B)
  ms_pShadowMap = new CDX11ShadowMap();
  ms_pShadowMap->Initialize(ms_pD3D11Device, ms_pD3D11Context);

  return true;
}

bool CGraphicDevice::__CreateDX11DepthStencil(int iWidth, int iHeight) {
  D3D11_TEXTURE2D_DESC depthDesc = {};
  depthDesc.Width = iWidth;
  depthDesc.Height = iHeight;
  depthDesc.MipLevels = 1;
  depthDesc.ArraySize = 1;
  depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthDesc.SampleDesc.Count = 1;
  depthDesc.SampleDesc.Quality = 0;
  depthDesc.Usage = D3D11_USAGE_DEFAULT;
  depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

  HRESULT hr = ms_pD3D11Device->CreateTexture2D(&depthDesc, nullptr,
                                                &ms_pDepthStencilBuffer);
  if (FAILED(hr)) {
    Tracenf("CreateTexture2D (depth) failed: 0x%08X", hr);
    return false;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

  hr = ms_pD3D11Device->CreateDepthStencilView(ms_pDepthStencilBuffer, &dsvDesc,
                                               &ms_pDepthStencilView);
  if (FAILED(hr)) {
    Tracenf("CreateDepthStencilView failed: 0x%08X", hr);
    return false;
  }

  return true;
}

void CGraphicDevice::__DestroyDX11Device() {
  // Clean up frame bridge
  if (ms_pPostProcess) {
    ms_pPostProcess->Shutdown();
    delete ms_pPostProcess;
    ms_pPostProcess = nullptr;
  }
  if (ms_pSharedSRV) {
    ms_pSharedSRV->Release();
    ms_pSharedSRV = nullptr;
  }
  if (ms_pSharedTexture) {
    ms_pSharedTexture->Release();
    ms_pSharedTexture = nullptr;
  }
  if (ms_pDepthStencilView) {
    ms_pDepthStencilView->Release();
    ms_pDepthStencilView = nullptr;
  }
  if (ms_pDepthStencilBuffer) {
    ms_pDepthStencilBuffer->Release();
    ms_pDepthStencilBuffer = nullptr;
  }
  if (ms_pRenderTargetView) {
    ms_pRenderTargetView->Release();
    ms_pRenderTargetView = nullptr;
  }
  if (ms_pSwapChain) {
    ms_pSwapChain->Release();
    ms_pSwapChain = nullptr;
  }
  if (ms_pD3D11Context) {
    ms_pD3D11Context->Release();
    ms_pD3D11Context = nullptr;
  }
  if (ms_pD3D11Device) {
    ms_pD3D11Device->Release();
    ms_pD3D11Device = nullptr;
  }
}

void CGraphicDevice::__InitializePDTVertexBufferList() {
  for (UINT i = 0; i < PDT_VERTEXBUFFER_NUM; ++i)
    ms_alpd3dPDTVB[i] = NULL;
}

void CGraphicDevice::__DestroyPDTVertexBufferList() {
  for (UINT i = 0; i < PDT_VERTEXBUFFER_NUM; ++i) {
    if (ms_alpd3dPDTVB[i]) {
      ms_alpd3dPDTVB[i]->Release();
      ms_alpd3dPDTVB[i] = NULL;
    }
  }
}

bool CGraphicDevice::__CreatePDTVertexBufferList() {
  for (UINT i = 0; i < PDT_VERTEXBUFFER_NUM; ++i) {
    if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(
            sizeof(TPDTVertex) * PDT_VERTEX_NUM,
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT,
            &ms_alpd3dPDTVB[i], nullptr)))
      return false;
  }
  return true;
}

void CGraphicDevice::__InitializeDefaultIndexBufferList() {
  for (UINT i = 0; i < DEFAULT_IB_NUM; ++i)
    ms_alpd3dDefIB[i] = NULL;
}

void CGraphicDevice::__DestroyDefaultIndexBufferList() {
  for (UINT i = 0; i < DEFAULT_IB_NUM; ++i)
    if (ms_alpd3dDefIB[i]) {
      ms_alpd3dDefIB[i]->Release();
      ms_alpd3dDefIB[i] = NULL;
    }
}

bool CGraphicDevice::__CreateDefaultIndexBuffer(UINT eDefIB, UINT uIdxCount,
                                                const WORD *c_awIndices) {
  assert(ms_alpd3dDefIB[eDefIB] == NULL);

  auto hr = ms_lpd3dDevice->CreateIndexBuffer(
      sizeof(WORD) * uIdxCount, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
      D3DPOOL_DEFAULT, &ms_alpd3dDefIB[eDefIB], nullptr);
  if (FAILED(hr))
    return false;

  WORD *dstIndices;
  hr = ms_alpd3dDefIB[eDefIB]->Lock(0, 0, (void **)&dstIndices, 0);
  if (FAILED(hr))
    return false;

  memcpy(dstIndices, c_awIndices, sizeof(WORD) * uIdxCount);

  ms_alpd3dDefIB[eDefIB]->Unlock();

  return true;
}

bool CGraphicDevice::__CreateDefaultIndexBufferList() {
  static const WORD c_awLineIndices[2] = {
      0,
      1,
  };
  static const WORD c_awLineTriIndices[6] = {
      0, 1, 0, 2, 1, 2,
  };
  static const WORD c_awLineRectIndices[8] = {
      0, 1, 0, 2, 1, 3, 2, 3,
  };
  static const WORD c_awLineCubeIndices[24] = {
      0, 1, 0, 2, 1, 3, 2, 3, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7,
  };
  static const WORD c_awFillTriIndices[3] = {
      0,
      1,
      2,
  };
  static const WORD c_awFillRectIndices[6] = {
      0, 2, 1, 2, 3, 1,
  };
  static const WORD c_awFillCubeIndices[36] = {
      0, 1, 2, 1, 3, 2, 2, 0, 6, 0, 4, 6, 0, 1, 4, 1, 5, 4,
      1, 3, 5, 3, 7, 5, 3, 2, 7, 2, 6, 7, 4, 5, 6, 5, 7, 6,
  };

  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE, 2, c_awLineIndices))
    return false;
  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_TRI, 6, c_awLineTriIndices))
    return false;
  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_RECT, 8, c_awLineRectIndices))
    return false;
  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_CUBE, 24,
                                  c_awLineCubeIndices))
    return false;
  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_TRI, 3, c_awFillTriIndices))
    return false;
  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_RECT, 6, c_awFillRectIndices))
    return false;
  if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE, 36,
                                  c_awFillCubeIndices))
    return false;

  return true;
}

void CGraphicDevice::InitBackBufferCount(UINT uBackBufferCount) {
  m_uBackBufferCount = uBackBufferCount;
}

void CGraphicDevice::Destroy() {
  __DestroyDX11Device();

  __DestroyPDTVertexBufferList();
  __DestroyDefaultIndexBufferList();

  if (ms_hDC) {
    ReleaseDC(ms_hWnd, ms_hDC);
    ms_hDC = NULL;
  }

  if (ms_ptVS) {
    ms_ptVS->Release();
    ms_ptVS = nullptr;
  }

  if (ms_pntVS) {
    ms_pntVS->Release();
    ms_pntVS = nullptr;
  }

  if (ms_pnt2VS) {
    ms_pnt2VS->Release();
    ms_pnt2VS = nullptr;
  }

  safe_release(ms_lpSphereMesh);
  safe_release(ms_lpCylinderMesh);

  safe_release(ms_lpd3dMatStack);
  safe_release(ms_lpd3dDevice);
  safe_release(ms_lpd3d);

  if (m_pStateManager) {
    delete m_pStateManager;
    m_pStateManager = NULL;
  }

  __Initialize();
}
