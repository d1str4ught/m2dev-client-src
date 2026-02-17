#ifndef __INC_ETERLIB_THREAD_H__
#define __INC_ETERLIB_THREAD_H__

class CThread {
public:
  CThread();
  int Create(void *arg);

protected:
  static UINT CALLBACK EntryPoint(void *pThis);

  virtual UINT Setup() = 0;            // Execute이 불려지기 전에 불려진다.
  virtual UINT Execute(void *arg) = 0; // 실제 쓰레드가 하는 일이 들어가는 곳

  UINT Run(void *arg);

  void *Arg() const { return m_pArg; }
  void Arg(void *arg) { m_pArg = arg; }

  HANDLE m_hThread;

private:
  void *m_pArg;
  unsigned m_uThreadID;
};

#endif
