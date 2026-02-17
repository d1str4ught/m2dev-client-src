#include "StdAfx.h"
#include "PythonIME.h"
#include "AbstractApplication.h"

CPythonIME::CPythonIME() : CIME() { ms_pEvent = this; }

CPythonIME::~CPythonIME() { Tracen("PythonIME Clear"); }

void CPythonIME::Create(HWND hWnd) { Initialize(hWnd); }

void CPythonIME::MoveLeft() { DecCurPos(); }

void CPythonIME::MoveRight() { IncCurPos(); }

void CPythonIME::MoveHome() { ms_curpos = 0; }

void CPythonIME::MoveEnd() { ms_curpos = ms_lastpos; }

void CPythonIME::SetCursorPosition(int iPosition) { SetCurPos(iPosition); }

void CPythonIME::Delete() { DelCurPos(); }

void CPythonIME::OnUpdate() {
  IAbstractApplication::GetSingleton().RunIMEUpdate();
}

void CPythonIME::OnTab() {
  IAbstractApplication::GetSingleton().RunIMETabEvent();
}

void CPythonIME::OnReturn() {
  IAbstractApplication::GetSingleton().RunIMEReturnEvent();
}

void CPythonIME::OnEscape() {
  //	IAbstractApplication::GetSingleton().RunIMEEscapeEvent();
}

bool CPythonIME::OnWM_CHAR(WPARAM wParam, LPARAM lParam) {
  const wchar_t wc = (wchar_t)wParam;

  const bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  // Ctrl+<key> combos only when Ctrl is pressed
  if (ctrlDown) {
    switch (wc) {
    case 0x01: // Ctrl+A
      if (ms_bCaptureInput) {
        SelectAll();
        if (ms_pEvent)
          ms_pEvent->OnUpdate();
        return true;
      }
      return false;

    case 0x1A: // Ctrl+Z
      if (ms_bCaptureInput) {
        CIME::Undo();
        return true;
      }
      return false;

    case 0x03: // Ctrl+C
      if (ms_bCaptureInput) {
        CopySelectionToClipboard();
        return true;
      }
      return false;

    case 0x16: // Ctrl+V
      if (ms_bCaptureInput) {
        PasteTextFromClipBoard();
        return true;
      }
      return false;

    case 0x18: // Ctrl+X
      if (ms_bCaptureInput) {
        CutSelection();
        if (ms_pEvent)
          ms_pEvent->OnUpdate();
        return true;
      }
      return false;

    case 0x19: // Ctrl+Y
      if (ms_bCaptureInput) {
        CIME::Redo();
        return true;
      }
      return false;
    }
  }

  // Non-Ctrl special keys (WM_CHAR gives these too)
  switch (wc) {
  case VK_RETURN:
    OnReturn();
    return true;

  case VK_TAB:
    if (!ms_bCaptureInput)
      return 0;
    OnTab();
    return true;

  case VK_ESCAPE:
    if (!ms_bCaptureInput)
      return 0;
    OnEscape();
    return true;
  }

  return false;
}

void CPythonIME::OnOpenCandidateList() {
  IAbstractApplication::GetSingleton().RunIMEOpenCandidateListEvent();
}

void CPythonIME::OnCloseCandidateList() {
  IAbstractApplication::GetSingleton().RunIMECloseCandidateListEvent();
}

void CPythonIME::OnOpenReadingWnd() {
  IAbstractApplication::GetSingleton().RunIMEOpenReadingWndEvent();
}

void CPythonIME::OnCloseReadingWnd() {
  IAbstractApplication::GetSingleton().RunIMECloseReadingWndEvent();
}

void CPythonIME::SelectAll() { CIME::SelectAll(); }

void CPythonIME::DeleteSelection() { CIME::DeleteSelection(); }

void CPythonIME::CopySelectionToClipboard() {
  CIME::CopySelectionToClipboard(ms_hWnd);
}

void CPythonIME::CutSelection() {
  CIME::CopySelectionToClipboard(ms_hWnd);
  CIME::DeleteSelection();
}

void CPythonIME::PasteTextFromClipBoard() { CIME::PasteTextFromClipBoard(); }
