#include "StdAfx.h"
#include "Input.h"

LPDIRECTINPUT8 CInputDevice::ms_lpDI = NULL;
LPDIRECTINPUTDEVICE8 CInputKeyboard::ms_lpKeyboard = NULL;
bool CInputKeyboard::ms_bPressedKey[256];
char CInputKeyboard::ms_diks[256];

CInputDevice::CInputDevice() {}

CInputDevice::~CInputDevice() { SAFE_RELEASE(ms_lpDI); }

HRESULT CInputDevice::CreateDevice(HWND /*hWnd*/) {
  if (ms_lpDI) {
    ms_lpDI->AddRef();
    return S_OK;
  }

  HRESULT hr;

  // Create a DInput object
  if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (VOID **)&ms_lpDI,
                                     NULL)))
    return hr;

  return S_OK;
}

CInputKeyboard::CInputKeyboard() { ResetKeyboard(); }

CInputKeyboard::~CInputKeyboard() {
  if (ms_lpKeyboard)
    ms_lpKeyboard->Unacquire();

  SAFE_RELEASE(ms_lpKeyboard);
}

void CInputKeyboard::ResetKeyboard() {
  memset(ms_diks, 0, sizeof(ms_diks));
  memset(ms_bPressedKey, 0, sizeof(ms_bPressedKey));
}

bool CInputKeyboard::InitializeKeyboard(HWND hWnd) {
  NANOBEGIN

  if (ms_lpKeyboard)
    return true;

  if (FAILED(CreateDevice(hWnd)))
    return false;

  HRESULT hr;

  // Obtain an interface to the system keyboard device.
  if (FAILED(hr =
                 ms_lpDI->CreateDevice(GUID_SysKeyboard, &ms_lpKeyboard, NULL)))
    return false;

  if (FAILED(hr = ms_lpKeyboard->SetDataFormat(&c_dfDIKeyboard)))
    return false;

  // Alt + F4를 위해 비독점 모드로 - [levites]
  //	DWORD dwCoopFlags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;
  //	DWORD dwCoopFlags = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
  DWORD dwCoopFlags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

  if (FAILED(hr = ms_lpKeyboard->SetCooperativeLevel(hWnd, dwCoopFlags)))
    return false;

  ms_lpKeyboard->Acquire();

  NANOEND
  return true;
}

void CInputKeyboard::UpdateKeyboard() {
  if (!ms_lpKeyboard)
    return;

  HRESULT hr;

  hr = ms_lpKeyboard->GetDeviceState(sizeof(ms_diks), &ms_diks);

  if (FAILED(hr)) {
    hr = ms_lpKeyboard->Acquire();

    // 현재 어플리케이션이 비활성화 되어 있어 입력을 받을 수 없다.
    // if (hr == DIERR_OTHERAPPHASPRIO || hr == DIERR_NOTACQUIRED);
    return;
  }

  for (int i = 0; i < 256; ++i) {
    if (ms_diks[i] & 0x80) {
      if (!IsPressed(i))
        KeyDown(i);
    } else if (IsPressed(i))
      KeyUp(i);
  }
}

void CInputKeyboard::KeyDown(int iIndex) {
  ms_bPressedKey[iIndex] = true;
  OnKeyDown(iIndex);
}

void CInputKeyboard::KeyUp(int iIndex) {
  ms_bPressedKey[iIndex] = false;
  OnKeyUp(iIndex);
}

bool CInputKeyboard::IsPressed(int iIndex) { return ms_bPressedKey[iIndex]; }
