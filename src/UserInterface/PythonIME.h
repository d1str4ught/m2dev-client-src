#pragma once

#include "EterBase/Singleton.h"
#include "EterLib/IME.h"

class CPythonIME : public IIMEEventSink,
                   public CIME,
                   public CSingleton<CPythonIME> {
public:
  CPythonIME();
  virtual ~CPythonIME();

  void MoveLeft();
  void MoveRight();
  void MoveHome();
  void MoveEnd();
  void SetCursorPosition(int iPosition);
  void Delete();

  void SelectAll();
  void DeleteSelection();
  void CopySelectionToClipboard();
  void CutSelection();
  void PasteTextFromClipBoard();

  void Create(HWND hWnd);

protected:
  virtual void OnTab();
  virtual void OnReturn();
  virtual void OnEscape();

  virtual bool OnWM_CHAR(WPARAM wParam, LPARAM lParam);
  virtual void OnUpdate();
  virtual void OnOpenCandidateList();
  virtual void OnCloseCandidateList();
  virtual void OnOpenReadingWnd();
  virtual void OnCloseReadingWnd();
};
