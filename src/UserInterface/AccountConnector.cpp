#include "StdAfx.h"
#include "AccountConnector.h"
#include "Packet.h"
#include "PythonNetworkStream.h"

void CAccountConnector::SetHandler(PyObject *poHandler) {
  m_poHandler = poHandler;
}

void CAccountConnector::SetLoginInfo(const char *c_szName,
                                     const char *c_szPwd) {
  m_strID = c_szName;
  m_strPassword = c_szPwd;
}

void CAccountConnector::ClearLoginInfo(void) { m_strPassword = ""; }

bool CAccountConnector::Connect(const char *c_szAddr, int iPort,
                                const char *c_szAccountAddr, int iAccountPort) {
  m_strAddr = c_szAddr;
  m_iPort = iPort;
  __OfflineState_Set();

  return CNetworkStream::Connect(c_szAccountAddr, iAccountPort);
}

void CAccountConnector::Disconnect() {
  CNetworkStream::Disconnect();
  __OfflineState_Set();
}

void CAccountConnector::Process() {
  CNetworkStream::Process();

  if (!__StateProcess()) {
    __OfflineState_Set();
    Disconnect();
  }
}

bool CAccountConnector::__StateProcess() {
  switch (m_eState) {
  case STATE_HANDSHAKE:
    return __HandshakeState_Process();
    break;
  case STATE_AUTH:
    return __AuthState_Process();
    break;
  }

  return true;
}

bool CAccountConnector::__HandshakeState_Process() {
  if (!__AnalyzePacket(GC::PHASE, sizeof(TPacketGCPhase),
                       &CAccountConnector::__AuthState_RecvPhase))
    return false;

  if (!__AnalyzePacket(GC::PING, sizeof(TPacketGCPing),
                       &CAccountConnector::__AuthState_RecvPingBase))
    return false;

  if (!__AnalyzePacket(GC::KEY_CHALLENGE, sizeof(TPacketGCKeyChallenge),
                       &CAccountConnector::__AuthState_RecvKeyChallengeBase))
    return false;

  if (!__AnalyzePacket(GC::KEY_COMPLETE, sizeof(TPacketGCKeyComplete),
                       &CAccountConnector::__AuthState_RecvKeyCompleteBase))
    return false;

  return true;
}

bool CAccountConnector::__AuthState_Process() {
  if (!__AnalyzePacket(0, sizeof(BYTE),
                       &CAccountConnector::__AuthState_RecvEmpty))
    return true;

  if (!__AnalyzePacket(GC::PHASE, sizeof(TPacketGCPhase),
                       &CAccountConnector::__AuthState_RecvPhase))
    return false;

  if (!__AnalyzePacket(GC::PING, sizeof(TPacketGCPing),
                       &CAccountConnector::__AuthState_RecvPingBase))
    return false;

  if (!__AnalyzePacket(GC::AUTH_SUCCESS, sizeof(TPacketGCAuthSuccess),
                       &CAccountConnector::__AuthState_RecvAuthSuccess))
    return true;

  if (!__AnalyzePacket(GC::LOGIN_FAILURE, sizeof(TPacketGCAuthSuccess),
                       &CAccountConnector::__AuthState_RecvAuthFailure))
    return true;

  if (!__AnalyzePacket(GC::KEY_CHALLENGE, sizeof(TPacketGCKeyChallenge),
                       &CAccountConnector::__AuthState_RecvKeyChallengeBase))
    return false;

  if (!__AnalyzePacket(GC::KEY_COMPLETE, sizeof(TPacketGCKeyComplete),
                       &CAccountConnector::__AuthState_RecvKeyCompleteBase))
    return false;

  return true;
}

bool CAccountConnector::__AuthState_RecvEmpty() {
  BYTE byEmpty;
  Recv(sizeof(BYTE), &byEmpty);
  return true;
}

bool CAccountConnector::__AuthState_RecvPhase() {
  TPacketGCPhase kPacketPhase;
  if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
    return false;

  if (kPacketPhase.phase == PHASE_HANDSHAKE) {
    __HandshakeState_Set();
  } else if (kPacketPhase.phase == PHASE_AUTH) {
    TPacketCGLogin3 LoginPacket;
    LoginPacket.header = CG::LOGIN3;
    LoginPacket.length = sizeof(LoginPacket);

    strncpy(LoginPacket.name, m_strID.c_str(), ID_MAX_NUM);
    strncpy(LoginPacket.pwd, m_strPassword.c_str(), PASS_MAX_NUM);
    LoginPacket.name[ID_MAX_NUM] = '\0';
    LoginPacket.pwd[PASS_MAX_NUM] = '\0';

    ClearLoginInfo();
    CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
    rkNetStream.ClearLoginInfo();

    m_strPassword = "";

    if (!Send(sizeof(LoginPacket), &LoginPacket)) {
      Tracen(" CAccountConnector::__AuthState_RecvPhase - SendLogin3 Error");
      return false;
    }

    __AuthState_Set();
  }

  return true;
}

// Key exchange (RecvKeyChallenge, RecvKeyComplete) and ping/pong
// (RecvPingPacket) are now handled by CNetworkStream base class. Thin wrappers
// in the header delegate __AnalyzePacket dispatch to those base methods.

bool CAccountConnector::__AuthState_RecvAuthSuccess() {
  TPacketGCAuthSuccess kAuthSuccessPacket;
  if (!Recv(sizeof(kAuthSuccessPacket), &kAuthSuccessPacket))
    return false;

  if (!kAuthSuccessPacket.bResult) {
    if (m_poHandler)
      PyCallClassMemberFunc(m_poHandler, "OnLoginFailure",
                            Py_BuildValue("(s)", "BESAMEKEY"));
  } else {
    CPythonNetworkStream &rkNet = CPythonNetworkStream::Instance();
    rkNet.SetLoginKey(kAuthSuccessPacket.dwLoginKey);
    rkNet.Connect(m_strAddr.c_str(), m_iPort);
  }

  Disconnect();
  __OfflineState_Set();

  return true;
}

bool CAccountConnector::__AuthState_RecvAuthFailure() {
  TPacketGCLoginFailure packet_failure;
  if (!Recv(sizeof(TPacketGCLoginFailure), &packet_failure))
    return false;

  if (m_poHandler)
    PyCallClassMemberFunc(m_poHandler, "OnLoginFailure",
                          Py_BuildValue("(s)", packet_failure.szStatus));

  return true;
}

bool CAccountConnector::__AnalyzePacket(
    UINT uHeader, UINT uPacketSize,
    bool (CAccountConnector::*pfnDispatchPacket)()) {
  uint16_t wHeader;
  if (!Peek(sizeof(wHeader), &wHeader))
    return true;

  if (wHeader != uHeader)
    return true;

  if (!Peek(uPacketSize))
    return true;

  return (this->*pfnDispatchPacket)();
}

void CAccountConnector::__OfflineState_Set() { __Inialize(); }

void CAccountConnector::__HandshakeState_Set() { m_eState = STATE_HANDSHAKE; }

void CAccountConnector::__AuthState_Set() { m_eState = STATE_AUTH; }

void CAccountConnector::OnConnectFailure() {
  if (m_poHandler)
    PyCallClassMemberFunc(m_poHandler, "OnConnectFailure", Py_BuildValue("()"));

  __OfflineState_Set();
}

void CAccountConnector::OnConnectSuccess() { m_eState = STATE_HANDSHAKE; }

void CAccountConnector::OnRemoteDisconnect() {
  if (m_isWaitKey) {
    if (m_poHandler) {
      PyCallClassMemberFunc(m_poHandler, "OnExit", Py_BuildValue("()"));
      return;
    }
  }

  __OfflineState_Set();
}

void CAccountConnector::OnDisconnect() { __OfflineState_Set(); }

void CAccountConnector::__Inialize() {
  m_eState = STATE_OFFLINE;
  m_isWaitKey = FALSE;
}

CAccountConnector::CAccountConnector() {
  m_poHandler = NULL;
  m_strAddr = "";
  m_iPort = 0;

  SetLoginInfo("", "");
  SetRecvBufferSize(1024 * 128);
  SetSendBufferSize(2048);
  __Inialize();
}

CAccountConnector::~CAccountConnector() { __OfflineState_Set(); }
