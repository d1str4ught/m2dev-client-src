#include "StdAfx.h"
#include "PythonApplication.h"
#include "Eterlib/Camera.h"

#include <winuser.h>

static int gs_nMouseCaptureRef = 0;

void CPythonApplication::SafeSetCapture() {
  SetCapture(m_hWnd);
  gs_nMouseCaptureRef++;
}

void CPythonApplication::SafeReleaseCapture() {
  gs_nMouseCaptureRef--;
  if (gs_nMouseCaptureRef == 0)
    ReleaseCapture();
}

void CPythonApplication::__SetFullScreenWindow(HWND hWnd, DWORD dwWidth,
                                               DWORD dwHeight, DWORD dwBPP) {
  DEVMODE DevMode;
  DevMode.dmSize = sizeof(DevMode);
  DevMode.dmBitsPerPel = dwBPP;
  DevMode.dmPelsWidth = dwWidth;
  DevMode.dmPelsHeight = dwHeight;
  DevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

  LONG Error = ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
  if (Error == DISP_CHANGE_RESTART) {
    ChangeDisplaySettings(0, 0);
  }
}

void CPythonApplication::__MinimizeFullScreenWindow(HWND hWnd, DWORD dwWidth,
                                                    DWORD dwHeight) {
  ChangeDisplaySettings(0, 0);
  SetWindowPos(hWnd, 0, 0, 0, dwWidth, dwHeight, SWP_SHOWWINDOW);
  ShowWindow(hWnd, SW_MINIMIZE);
}

LRESULT CPythonApplication::WindowProcedure(HWND hWnd, UINT uiMsg,
                                            WPARAM wParam, LPARAM lParam) {
  const int c_DoubleClickTime = 300;
  const int c_DoubleClickBox = 5;
  static int s_xDownPosition = 0;
  static int s_yDownPosition = 0;

  switch (uiMsg) {
  case WM_ACTIVATEAPP: {
    m_isActivateWnd = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);

    if (m_isActivateWnd) {
      m_SoundEngine.RestoreVolume();

      //////////////////

      if (m_isWindowFullScreenEnable) {
        __SetFullScreenWindow(hWnd, m_dwWidth, m_dwHeight, m_pySystem.GetBPP());
      }
    } else {
      m_SoundEngine.SaveVolume(m_isMinimizedWnd);

      //////////////////

      if (m_isWindowFullScreenEnable) {
        __MinimizeFullScreenWindow(hWnd, m_dwWidth, m_dwHeight);
      }

      if (IsUserMovingMainWindow()) {
        SetUserMovingMainWindow(false);
      }
    }
  } break;

  case WM_INPUTLANGCHANGE:
    return CPythonIME::Instance().WMInputLanguage(hWnd, uiMsg, wParam, lParam);
    break;

  case WM_IME_STARTCOMPOSITION:
    return CPythonIME::Instance().WMStartComposition(hWnd, uiMsg, wParam,
                                                     lParam);
    break;

  case WM_IME_COMPOSITION:
    return CPythonIME::Instance().WMComposition(hWnd, uiMsg, wParam, lParam);
    break;

  case WM_IME_ENDCOMPOSITION:
    return CPythonIME::Instance().WMEndComposition(hWnd, uiMsg, wParam, lParam);
    break;

  case WM_IME_NOTIFY:
    return CPythonIME::Instance().WMNotify(hWnd, uiMsg, wParam, lParam);
    break;

  case WM_IME_SETCONTEXT:
    lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW);
    break;

  case WM_CHAR:
    return CPythonIME::Instance().WMChar(hWnd, uiMsg, wParam, lParam);
    break;

  case WM_KEYDOWN:
    if (wParam == VK_ESCAPE && IsUserMovingMainWindow())
      SetUserMovingMainWindow(false);
    OnIMEKeyDown(LOWORD(wParam));
    break;

  case WM_LBUTTONDOWN:
    SafeSetCapture();

    if (ELTimer_GetMSec() - m_dwLButtonDownTime < c_DoubleClickTime &&
        abs(LOWORD(lParam) - s_xDownPosition) < c_DoubleClickBox &&
        abs(HIWORD(lParam) - s_yDownPosition) < c_DoubleClickBox) {
      m_dwLButtonDownTime = 0;

      OnMouseLeftButtonDoubleClick(short(LOWORD(lParam)),
                                   short(HIWORD(lParam)));
    } else {
      m_dwLButtonDownTime = ELTimer_GetMSec();

      OnMouseLeftButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
    }

    s_xDownPosition = LOWORD(lParam);
    s_yDownPosition = HIWORD(lParam);

    if (IsUserMovingMainWindow())
      SetUserMovingMainWindow(false);
    return 0;

  case WM_LBUTTONUP:
    m_dwLButtonUpTime = ELTimer_GetMSec();

    if (hWnd == GetCapture()) {
      SafeReleaseCapture();
      OnMouseLeftButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
    }
    return 0;

  case WM_MBUTTONDOWN:
    SafeSetCapture();

    UI::CWindowManager::Instance().RunMouseMiddleButtonDown(
        short(LOWORD(lParam)), short(HIWORD(lParam)));
    //			OnMouseMiddleButtonDown(short(LOWORD(lParam)),
    // short(HIWORD(lParam)));
    break;

  case WM_MBUTTONUP:
    if (GetCapture() == hWnd) {
      SafeReleaseCapture();

      UI::CWindowManager::Instance().RunMouseMiddleButtonUp(
          short(LOWORD(lParam)), short(HIWORD(lParam)));
      //				OnMouseMiddleButtonUp(short(LOWORD(lParam)),
      // short(HIWORD(lParam)));
    }
    break;

  case WM_RBUTTONDOWN:
    SafeSetCapture();
    OnMouseRightButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
    return 0;

  case WM_RBUTTONUP:
    if (hWnd == GetCapture()) {
      SafeReleaseCapture();

      OnMouseRightButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
    }
    return 0;

  case 0x20a:
    if (CPythonApplication::Instance().IsWebPageMode()) {
      // 웹브라우저 상태일때는 휠 작동 안되도록 처리
    } else {
      OnMouseWheel(short(HIWORD(wParam)));
    }
    break;

  case WM_SIZE:
    switch (wParam) {
    case SIZE_RESTORED:
    case SIZE_MAXIMIZED: {
      RECT rcWnd;
      GetClientRect(&rcWnd);

      UINT uWidth = rcWnd.right - rcWnd.left;
      UINT uHeight = rcWnd.bottom - rcWnd.left;
      m_grpDevice.ResizeBackBuffer(uWidth, uHeight);
    } break;
    }

    if (wParam == SIZE_MINIMIZED)
      m_isMinimizedWnd = true;
    else
      m_isMinimizedWnd = false;

    OnSizeChange(short(LOWORD(lParam)), short(HIWORD(lParam)));

    break;

  case WM_EXITSIZEMOVE: {
    RECT rcWnd;
    GetClientRect(&rcWnd);

    UINT uWidth = rcWnd.right - rcWnd.left;
    UINT uHeight = rcWnd.bottom - rcWnd.left;
    m_grpDevice.ResizeBackBuffer(uWidth, uHeight);
    OnSizeChange(short(LOWORD(lParam)), short(HIWORD(lParam)));
  } break;
  case WM_NCLBUTTONDOWN: {
    switch (wParam) {
    case HTMAXBUTTON:
    case HTSYSMENU:
      return 0;
    case HTMINBUTTON:
      ShowWindow(hWnd, SW_MINIMIZE);
      return 0;
    case HTCLOSE:
      RunPressExitKey();
      return 0;
    case HTCAPTION:
      if (!IsUserMovingMainWindow())
        SetUserMovingMainWindow(true);

      return 0;
    }

    break;
  }

  case WM_NCLBUTTONUP: {
    if (IsUserMovingMainWindow())
      SetUserMovingMainWindow(false);

    break;
  }

  case WM_NCRBUTTONDOWN:
  case WM_NCRBUTTONUP:
  case WM_CONTEXTMENU:
    return 0;
  case WM_SYSCOMMAND:
    if (wParam == SC_KEYMENU)
      return 0;
    break;
  case WM_SYSKEYDOWN:
    switch (LOWORD(wParam)) {
    case VK_F10:
      break;
    }
    break;

  case WM_SYSKEYUP:
    switch (LOWORD(wParam)) {
    case 18:
      return FALSE;
      break;
    case VK_F10:
      break;
    }
    break;

  case WM_SETCURSOR:
    if (IsActive()) {
      if (m_bCursorVisible && CURSOR_MODE_HARDWARE == m_iCursorMode) {
        SetCursor((HCURSOR)m_hCurrentCursor);
        return 0;
      } else {
        SetCursor(NULL);
        return 0;
      }
    }
    break;

  case WM_CLOSE:
#ifdef _DEBUG
    PostQuitMessage(0);
#else
    RunPressExitKey();
#endif
    return 0;

  case WM_DESTROY:
    return 0;
  default:
    // Tracenf("%x msg %x", timeGetTime(), uiMsg);
    break;
  }

  return CMSApplication::WindowProcedure(hWnd, uiMsg, wParam, lParam);
}
