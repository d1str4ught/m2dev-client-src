#pragma once

class CFrameController {
public:
  CFrameController();
  virtual ~CFrameController();

  void Clear();

  void Update(float fElapsedTime);

  void SetCurrentFrame(DWORD dwFrame);
  BYTE GetCurrentFrame();

  void SetMaxFrame(DWORD dwMaxFrame);
  void SetFrameTime(float fTime);
  void SetStartFrame(DWORD dwStartFrame);
  void SetLoopFlag(BOOL bFlag);
  void SetLoopCount(int iLoopCount);

  void SetActive(BOOL bFlag);
  BOOL isActive(DWORD dwMainFrame = 0);

protected:
  // Dynamic
  BOOL m_isActive;
  DWORD m_dwcurFrame;
  float m_fLastFrameTime;

  int m_iLoopCount;

  // Static
  BOOL m_isLoop;
  DWORD m_dwMaxFrame;
  float m_fFrameTime;
  DWORD m_dwStartFrame;
};
