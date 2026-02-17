#pragma once

#include "EterBase/Stl.h"

class CMSWindow {
public:
  CMSWindow();

  virtual ~CMSWindow();

  void Destroy();
  bool Create(const char *c_szName, int brush = BLACK_BRUSH, DWORD cs = 0,
              DWORD ws = WS_OVERLAPPEDWINDOW, HICON hIcon = NULL,
              int iCursorResource = 32512);

  void Show();
  void Hide();

  void SetVisibleMode(bool isVisible);

  void SetPosition(int x, int y);
  void SetCenterPosition();

  void SetText(const char *c_szText);

  void AdjustSize(int width, int height);
  void SetSize(int width, int height);

  bool IsVisible();
  bool IsActive();

  void GetMousePosition(POINT *ppt);
  void GetClientRect(RECT *prc);
  void GetWindowRect(RECT *prc);

  int GetScreenWidth();
  int GetScreenHeight();

  HWND GetWindowHandle();
  HINSTANCE GetInstance();

  virtual LRESULT WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam,
                                  LPARAM lParam);
  virtual void OnSize(WPARAM wParam, LPARAM lParam);

protected:
  const wchar_t *RegisterWindowClass(DWORD style, int brush, WNDPROC pfnWndProc,
                                     HICON hIcon = NULL,
                                     int iCursorResource = 32512);

protected:
  typedef std::set<std::wstring> TWindowClassSet;

protected:
  HWND m_hWnd;
  RECT m_rect;
  bool m_isActive;
  bool m_isVisible;

protected:
  static TWindowClassSet ms_stWCSet;
  static HINSTANCE ms_hInstance;
};
