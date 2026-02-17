#pragma once
#include <unordered_set>

namespace UI {
class CWindow;

class CWindowManager : public CSingleton<CWindowManager> {
public:
  typedef std::map<std::string, CWindow *> TLayerContainer;
  typedef std::list<CWindow *> TWindowContainer;
  typedef std::map<int, CWindow *> TKeyCaptureWindowMap;

public:
  CWindowManager();
  virtual ~CWindowManager();

  void Destroy();

  float GetAspect();
  void SetScreenSize(long lWidth, long lHeight);
  void SetResolution(int hres, int vres);

  void GetResolution(long &rx, long &ry) {
    rx = m_iHres;
    ry = m_iVres;
  }

  void SetMouseHandler(PyObject *poMouseHandler);
  long GetScreenWidth() { return m_lWidth; }
  long GetScreenHeight() { return m_lHeight; }
  void GetMousePosition(long &rx, long &ry);
  BOOL IsDragging();

  CWindow *GetLockWindow() { return m_pLockWindow; }
  CWindow *GetPointWindow() { return m_pPointWindow; }
  bool IsFocus() { return (m_pActiveWindow || m_pLockWindow); }
  bool IsFocusWindow(CWindow *pWindow) { return pWindow == m_pActiveWindow; }

  void SetParent(CWindow *pWindow, CWindow *pParentWindow);
  void SetPickAlways(CWindow *pWindow);

  enum {
    WT_NORMAL,
    WT_SLOT,
    WT_GRIDSLOT,
    WT_TEXTLINE,
    WT_MARKBOX,
    WT_IMAGEBOX,
    WT_EXP_IMAGEBOX,
    WT_ANI_IMAGEBOX,
    WT_BUTTON,
    WT_RATIOBUTTON,
    WT_TOGGLEBUTTON,
    WT_DRAGBUTTON,
    WT_BOX,
    WT_BAR,
    WT_LINE,
    WT_BAR3D,
    WT_NUMLINE,
  };

  CWindow *RegisterWindow(PyObject *po, const char *c_szLayer);
  CWindow *RegisterTypeWindow(PyObject *po, DWORD dwWndType,
                              const char *c_szLayer);

  CWindow *RegisterSlotWindow(PyObject *po, const char *c_szLayer);
  CWindow *RegisterGridSlotWindow(PyObject *po, const char *c_szLayer);
  CWindow *RegisterTextLine(PyObject *po, const char *c_szLayer);
  CWindow *RegisterMarkBox(PyObject *po, const char *c_szLayer);
  CWindow *RegisterImageBox(PyObject *po, const char *c_szLayer);
  CWindow *RegisterExpandedImageBox(PyObject *po, const char *c_szLayer);
  CWindow *RegisterAniImageBox(PyObject *po, const char *c_szLayer);
  CWindow *RegisterButton(PyObject *po, const char *c_szLayer);
  CWindow *RegisterRadioButton(PyObject *po, const char *c_szLayer);
  CWindow *RegisterToggleButton(PyObject *po, const char *c_szLayer);
  CWindow *RegisterDragButton(PyObject *po, const char *c_szLayer);
  CWindow *RegisterBox(PyObject *po, const char *c_szLayer);
  CWindow *RegisterBar(PyObject *po, const char *c_szLayer);
  CWindow *RegisterLine(PyObject *po, const char *c_szLayer);
  CWindow *RegisterBar3D(PyObject *po, const char *c_szLayer);
  CWindow *RegisterNumberLine(PyObject *po, const char *c_szLayer);

  void DestroyWindow(CWindow *pWin);
  void NotifyDestroyWindow(CWindow *pWindow);

  // Attaching Icon
  BOOL IsAttaching();
  DWORD GetAttachingType();
  DWORD GetAttachingIndex();
  DWORD GetAttachingSlotNumber();
  void GetAttachingIconSize(BYTE *pbyWidth, BYTE *pbyHeight);
  void AttachIcon(DWORD dwType, DWORD dwIndex, DWORD dwSlotNumber, BYTE byWidth,
                  BYTE byHeight);
  void DeattachIcon();
  void SetAttachingFlag(BOOL bFlag);
  // Attaching Icon

  void OnceIgnoreMouseLeftButtonUpEvent();
  void LockWindow(CWindow *pWin);
  void UnlockWindow();

  void ActivateWindow(CWindow *pWin);
  void DeactivateWindow();
  CWindow *GetActivateWindow();
  void SetTop(CWindow *pWin);
  void SetTopUIWindow();
  void ResetCapture();
  void ClearStoredSlotCoolTimeInAllSlotWindows(DWORD dwKey, DWORD dwSlotIndex);

  void Update();
  void Render();

  void RunMouseMove(long x, long y);
  void RunMouseLeftButtonDown(long x, long y);
  void RunMouseLeftButtonUp(long x, long y);
  void RunMouseLeftButtonDoubleClick(long x, long y);
  void RunMouseRightButtonDown(long x, long y);
  void RunMouseRightButtonUp(long x, long y);
  void RunMouseRightButtonDoubleClick(long x, long y);
  void RunMouseMiddleButtonDown(long x, long y);
  void RunMouseMiddleButtonUp(long x, long y);

  void RunIMEUpdate();
  void RunIMETabEvent();
  void RunIMEReturnEvent();
  void RunIMEKeyDown(int vkey);
  void RunOpenCandidate();
  void RunCloseCandidate();
  void RunOpenReading();
  void RunCloseReading();

  void RunKeyDown(int vkey);
  void RunKeyUp(int vkey);
  void RunPressEscapeKey();
  void RunPressExitKey();

private:
  void SetMousePosition(long x, long y);
  CWindow *__PickWindow(long x, long y);

  CWindow *__NewWindow(PyObject *po, DWORD dwWndType);
  void __ClearReserveDeleteWindowList();

private:
  long m_lWidth;
  long m_lHeight;

  int m_iVres;
  int m_iHres;

  long m_lMouseX, m_lMouseY;
  long m_lDragX, m_lDragY;
  long m_lPickedX, m_lPickedY;

  BOOL m_bOnceIgnoreMouseLeftButtonUpEventFlag;
  int m_iIgnoreEndTime;

  // Attaching Icon
  PyObject *m_poMouseHandler;
  BOOL m_bAttachingFlag;
  DWORD m_dwAttachingType;
  DWORD m_dwAttachingIndex;
  DWORD m_dwAttachingSlotNumber;
  BYTE m_byAttachingIconWidth;
  BYTE m_byAttachingIconHeight;
  // Attaching Icon

  CWindow *m_pActiveWindow;
  TWindowContainer m_ActiveWindowList;
  CWindow *m_pLockWindow;
  TWindowContainer m_LockWindowList;
  CWindow *m_pPointWindow;
  CWindow *m_pLeftCaptureWindow;
  CWindow *m_pRightCaptureWindow;
  CWindow *m_pMiddleCaptureWindow;
  TKeyCaptureWindowMap m_KeyCaptureWindowMap;
  std::unordered_set<CWindow *> m_ReserveDeleteWindowList;
  TWindowContainer m_PickAlwaysWindowList;

  CWindow *m_pRootWindow;
  TWindowContainer m_LayerWindowList;
  TLayerContainer m_LayerWindowMap;
};

PyObject *BuildEmptyTuple();
}; // namespace UI
