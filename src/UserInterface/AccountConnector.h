#pragma once

#include "EterLib/NetStream.h"
#include "EterLib/FuncObject.h"

class CAccountConnector : public CNetworkStream,
                          public CSingleton<CAccountConnector> {
public:
  enum {
    STATE_OFFLINE,
    STATE_HANDSHAKE,
    STATE_AUTH,
  };

public:
  CAccountConnector();
  virtual ~CAccountConnector();

  void SetHandler(PyObject *poHandler);
  void SetLoginInfo(const char *c_szName, const char *c_szPwd);
  void ClearLoginInfo(void);

  bool Connect(const char *c_szAddr, int iPort, const char *c_szAccountAddr,
               int iAccountPort);
  void Disconnect();
  void Process();

protected:
  void OnConnectFailure();
  void OnConnectSuccess();
  void OnRemoteDisconnect();
  void OnDisconnect();

protected:
  void __Inialize();
  bool __StateProcess();

  void __OfflineState_Set();
  void __HandshakeState_Set();
  void __AuthState_Set();

  bool __HandshakeState_Process();
  bool __AuthState_Process();

  bool __AuthState_RecvEmpty();
  bool __AuthState_RecvPhase();
  bool __AuthState_RecvAuthSuccess();
  bool __AuthState_RecvAuthFailure();

  // Thin wrappers for __AnalyzePacket dispatch (delegates to CNetworkStream
  // base)
  bool __AuthState_RecvPingBase() { return RecvPingPacket(); }
  bool __AuthState_RecvKeyChallengeBase() { return RecvKeyChallenge(); }
  bool __AuthState_RecvKeyCompleteBase() { return RecvKeyComplete(); }

  bool __AnalyzePacket(UINT uHeader, UINT uPacketSize,
                       bool (CAccountConnector::*pfnDispatchPacket)());

protected:
  UINT m_eState;
  std::string m_strID;
  std::string m_strPassword;

  std::string m_strAddr;
  int m_iPort;
  BOOL m_isWaitKey;

  PyObject *m_poHandler;
};
