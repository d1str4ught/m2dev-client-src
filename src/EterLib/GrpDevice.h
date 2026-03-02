#pragma once

#include "GrpBase.h"
#include "StateManager.h"

#include <map>

class CGraphicDevice : public CGraphicBase {
public:
  enum EDeviceState {
    DEVICESTATE_OK,
    DEVICESTATE_BROKEN,
    DEVICESTATE_NEEDS_RESET,
    DEVICESTATE_NULL
  };

  enum ECreateReturnValues {
    CREATE_OK = (1 << 0),
    CREATE_NO_DIRECTX = (1 << 1),
    CREATE_GET_DEVICE_CAPS = (1 << 2),
    CREATE_GET_DEVICE_CAPS2 = (1 << 3),
    CREATE_DEVICE = (1 << 4),
    CREATE_REFRESHRATE = (1 << 5),
    CREATE_ENUM = (1 << 6),   // 2003. 01. 09. myevan 모드 리스트 얻기 실패
    CREATE_DETECT = (1 << 7), // 2003. 01. 09. myevan 모드 선택 실패
    CREATE_NO_TNL = (1 << 8),
    CREATE_BAD_DRIVER = (1 << 9),
    CREATE_FORMAT = (1 << 10),
  };

  CGraphicDevice();
  virtual ~CGraphicDevice();

  void InitBackBufferCount(UINT uBackBufferCount);

  void Destroy();
  int Create(HWND hWnd, int hres, int vres, bool Windowed = true, int bit = 32,
             int ReflashRate = 0);

  EDeviceState GetDeviceState();
  bool Reset();

  void EnableWebBrowserMode(const RECT &c_rcWebPage);
  void DisableWebBrowserMode();
  void MoveWebBrowserRect(const RECT &c_rcWebPage);

  bool ResizeBackBuffer(UINT uWidth, UINT uHeight);
  void RegisterWarningString(UINT uiMsg, const char *c_szString);

protected:
  void __Initialize();
  void __WarningMessage(HWND hWnd, UINT uiMsg);

  void __InitializeDefaultIndexBufferList();
  void __DestroyDefaultIndexBufferList();
  bool __CreateDefaultIndexBufferList();
  bool __CreateDefaultIndexBuffer(UINT eDefIB, UINT uIdxCount,
                                  const WORD *c_awIndices);

  void __InitializePDTVertexBufferList();
  void __DestroyPDTVertexBufferList();
  bool __CreatePDTVertexBufferList();

  LPDIRECT3DVERTEXDECLARATION9 CreatePTStreamVertexShader();
  LPDIRECT3DVERTEXDECLARATION9 CreatePNTStreamVertexShader();
  LPDIRECT3DVERTEXDECLARATION9 CreatePNT2StreamVertexShader();
  LPDIRECT3DVERTEXDECLARATION9 CreateDoublePNTStreamVertexShader();

  // DX11 device lifecycle
  bool __CreateDX11Device(HWND hWnd, int iWidth, int iHeight, bool bWindowed);
  bool __CreateDX11DepthStencil(int iWidth, int iHeight);
  void __DestroyDX11Device();

protected:
  DWORD m_uBackBufferCount;
  std::map<UINT, std::string> m_kMap_strWarningMessage;
  CStateManager *m_pStateManager;
};
