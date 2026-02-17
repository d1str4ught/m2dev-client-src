#include "StdAfx.h"
#include "MsWindow.h"

#include <windowsx.h>
#include <utf8.h>

CMSWindow::TWindowClassSet CMSWindow::ms_stWCSet;
HINSTANCE CMSWindow::ms_hInstance = NULL;

LRESULT CALLBACK MSWindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam,
                                   LPARAM lParam) {
  CMSWindow *pWnd = (CMSWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

  if (pWnd)
    return pWnd->WindowProcedure(hWnd, uiMsg, wParam, lParam);

  return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

LRESULT CMSWindow::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam,
                                   LPARAM lParam) {
  switch (uiMsg) {
  case WM_SIZE:
    OnSize(wParam, lParam);
    break;

  case WM_ACTIVATEAPP:
    m_isActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
    break;
  }

  return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

void CMSWindow::OnSize(WPARAM wParam, LPARAM /*lParam*/) {
  if (wParam == SIZE_MINIMIZED) {
    InvalidateRect(m_hWnd, NULL, true);
    m_isActive = false;
    m_isVisible = false;
  } else {
    m_isActive = true;
    m_isVisible = true;
  }
}

void CMSWindow::Destroy() {
  if (!m_hWnd)
    return;

  if (IsWindow(m_hWnd))
    DestroyWindow(m_hWnd);

  m_hWnd = NULL;
  m_isVisible = false;
}

bool CMSWindow::Create(const char *c_szName, int brush, DWORD cs, DWORD ws,
                       HICON hIcon, int iCursorResource) {
  Destroy();

  const wchar_t *wClassName =
      RegisterWindowClass(cs, brush, MSWindowProcedure, hIcon, iCursorResource);

  if (!wClassName)
    return false;

  // Window title is UTF-8 → convert
  std::wstring wWindowName = Utf8ToWide(c_szName ? c_szName : "");

  m_hWnd =
      CreateWindowW(wClassName,          // already wide
                    wWindowName.c_str(), // wide
                    ws, 0, 0, 0, 0, nullptr, nullptr, ms_hInstance, nullptr);

  if (!m_hWnd)
    return false;

  SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
  return true;
}

void CMSWindow::SetVisibleMode(bool isVisible) {
  m_isVisible = isVisible;

  if (m_isVisible) {
    ShowWindow(m_hWnd, SW_SHOW);
  } else {
    ShowWindow(m_hWnd, SW_HIDE);
  }
}

void CMSWindow::Show() {
  m_isVisible = true;
  ShowWindow(m_hWnd, SW_SHOW);
}

void CMSWindow::Hide() {
  m_isVisible = false;
  ShowWindow(m_hWnd, SW_HIDE);
}

bool CMSWindow::IsVisible() { return m_isVisible; }

bool CMSWindow::IsActive() { return m_isActive; }

HINSTANCE CMSWindow::GetInstance() { return ms_hInstance; }

HWND CMSWindow::GetWindowHandle() { return m_hWnd; }

int CMSWindow::GetScreenWidth() { return GetSystemMetrics(SM_CXSCREEN); }

int CMSWindow::GetScreenHeight() { return GetSystemMetrics(SM_CYSCREEN); }

void CMSWindow::GetWindowRect(RECT *prc) { ::GetWindowRect(m_hWnd, prc); }

void CMSWindow::GetClientRect(RECT *prc) { ::GetClientRect(m_hWnd, prc); }

void CMSWindow::GetMousePosition(POINT *ppt) {
  GetCursorPos(ppt);
  ScreenToClient(m_hWnd, ppt);
}

void CMSWindow::SetPosition(int x, int y) {
  SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void CMSWindow::SetCenterPosition() {
  RECT rc;

  GetClientRect(&rc);

  int windowWidth = rc.right - rc.left;
  int windowHeight = rc.bottom - rc.top;

  SetPosition((GetScreenWidth() - windowWidth) / 2,
              (GetScreenHeight() - windowHeight) / 2);
}

void CMSWindow::AdjustSize(int width, int height) {
  SetRect(&m_rect, 0, 0, width, height);

  AdjustWindowRectEx(&m_rect, GetWindowStyle(m_hWnd), GetMenu(m_hWnd) != NULL,
                     GetWindowExStyle(m_hWnd));

  MoveWindow(m_hWnd, 0, 0, m_rect.right - m_rect.left,
             m_rect.bottom - m_rect.top, FALSE);
}

void CMSWindow::SetText(const char *c_szText) {
  if (!m_hWnd)
    return;

  std::wstring wText = Utf8ToWide(c_szText ? c_szText : "");
  SetWindowTextW(m_hWnd, wText.c_str());
}

void CMSWindow::SetSize(int width, int height) {
  SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
}

const wchar_t *CMSWindow::RegisterWindowClass(DWORD style, int brush,
                                              WNDPROC pfnWndProc, HICON hIcon,
                                              int iCursorResource) {
  wchar_t szClassName[1024];
  swprintf_s(szClassName, L"eter - s%x:b%x:p:%p", style, brush, pfnWndProc);

  // Use a set of std::wstring (NOT char*)
  TWindowClassSet::iterator it = ms_stWCSet.find(szClassName);
  if (it != ms_stWCSet.end())
    return it->c_str();

  // Persist the string
  std::wstring staticClassName = szClassName;
  ms_stWCSet.insert(staticClassName);

  WNDCLASSW wc{};
  wc.style = style;
  wc.lpfnWndProc = pfnWndProc;
  wc.hCursor = LoadCursor(ms_hInstance, MAKEINTRESOURCE(iCursorResource));
  wc.hIcon = hIcon ? hIcon : LoadIcon(ms_hInstance, IDI_APPLICATION);
  wc.hbrBackground = (HBRUSH)GetStockObject(brush);
  wc.hInstance = ms_hInstance;
  wc.lpszClassName = staticClassName.c_str();
  wc.lpszMenuName = nullptr;

  if (!RegisterClassW(&wc))
    return nullptr;

  // Return pointer stable inside the set
  return ms_stWCSet.find(staticClassName)->c_str();
}

CMSWindow::CMSWindow() {
  m_hWnd = NULL;
  m_isVisible = false;
}

CMSWindow::~CMSWindow() {}