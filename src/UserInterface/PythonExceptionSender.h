#pragma once

class CPythonExceptionSender : public IPythonExceptionSender {
public:
  CPythonExceptionSender();
  virtual ~CPythonExceptionSender();

  void Send();

protected:
  std::set<DWORD> m_kSet_dwSendedExceptionCRC;
};
