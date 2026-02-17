#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "AccountConnector.h"

// Login
// ---------------------------------------------------------------------------
void CPythonNetworkStream::LoginPhase() { DispatchPacket(m_loginHandlers); }

void CPythonNetworkStream::SetLoginPhase() {
  if ("Login" != m_strPhase)
    m_phaseLeaveFunc.Run();

  Tracef("[PHASE] Entering phase: Login\n");
  Tracen("");
  Tracen("## Network - Login Phase ##");
  Tracen("");

  m_strPhase = "Login";

  m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoginPhase);
  m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoginPhase);

  m_dwChangingPhaseTime = ELTimer_GetMSec();

  if (0 == m_dwLoginKey) {
    ClearLoginInfo();
    return;
  }

  if (__DirectEnterMode_IsSet()) {
    SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());

    ClearLoginInfo();
    CAccountConnector &rkAccountConnector = CAccountConnector::Instance();
    rkAccountConnector.ClearLoginInfo();
  } else {
    SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());

    ClearLoginInfo();
    CAccountConnector &rkAccountConnector = CAccountConnector::Instance();
    rkAccountConnector.ClearLoginInfo();

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginStart",
                          Py_BuildValue("()"));

    __ClearSelectCharacterData();
  }
}

bool CPythonNetworkStream::__RecvEmpirePacket() {
  TPacketGCEmpire kPacketEmpire;
  if (!Recv(sizeof(kPacketEmpire), &kPacketEmpire))
    return false;

  m_dwEmpireID = kPacketEmpire.bEmpire;
  return true;
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket3() {
  TPacketGCLoginSuccess3 kPacketLoginSuccess;

  if (!Recv(sizeof(kPacketLoginSuccess), &kPacketLoginSuccess))
    return false;

  for (int i = 0; i < PLAYER_PER_ACCOUNT3; ++i) {
    m_akSimplePlayerInfo[i] = kPacketLoginSuccess.akSimplePlayerInformation[i];
    m_adwGuildID[i] = kPacketLoginSuccess.guild_id[i];
    m_astrGuildName[i] = kPacketLoginSuccess.guild_name[i];
  }

  m_kMarkAuth.m_dwHandle = kPacketLoginSuccess.handle;
  m_kMarkAuth.m_dwRandomKey = kPacketLoginSuccess.random_key;

  if (__DirectEnterMode_IsSet()) {
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "Refresh",
                          Py_BuildValue("()"));
  }

  return true;
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket4() {
  TPacketGCLoginSuccess4 kPacketLoginSuccess;

  if (!Recv(sizeof(kPacketLoginSuccess), &kPacketLoginSuccess))
    return false;

  for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i) {
    m_akSimplePlayerInfo[i] = kPacketLoginSuccess.akSimplePlayerInformation[i];
    m_adwGuildID[i] = kPacketLoginSuccess.guild_id[i];
    m_astrGuildName[i] = kPacketLoginSuccess.guild_name[i];
  }

  m_kMarkAuth.m_dwHandle = kPacketLoginSuccess.handle;
  m_kMarkAuth.m_dwRandomKey = kPacketLoginSuccess.random_key;

  if (!__DirectEnterMode_IsSet()) {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "Refresh",
                          Py_BuildValue("()"));
  }

  return true;
}

void CPythonNetworkStream::OnConnectFailure() {
  if (__DirectEnterMode_IsSet()) {
    ClosePhase();
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnConnectFailure",
                          Py_BuildValue("()"));
  }
}

bool CPythonNetworkStream::__RecvLoginFailurePacket() {
  TPacketGCLoginFailure packet_failure;
  if (!Recv(sizeof(TPacketGCLoginFailure), &packet_failure))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginFailure",
                        Py_BuildValue("(s)", packet_failure.szStatus));
#ifdef _DEBUG
  Tracef(" RecvLoginFailurePacket : [%s]\n", packet_failure.szStatus);
#endif
  return true;
}

bool CPythonNetworkStream::SendLoginPacketNew(const char *c_szName,
                                              const char *c_szPassword) {
  TPacketCGLogin2 LoginPacket;
  LoginPacket.header = CG::LOGIN2;
  LoginPacket.length = sizeof(LoginPacket);
  LoginPacket.login_key = m_dwLoginKey;

  strncpy(LoginPacket.name, c_szName, sizeof(LoginPacket.name) - 1);
  LoginPacket.name[ID_MAX_NUM] = '\0';

  if (!Send(sizeof(LoginPacket), &LoginPacket)) {
    Tracen("SendLogin Error");
    return false;
  }

  __SendInternalBuffer();

  return true;
}

bool CPythonNetworkStream::__RecvLoginKeyPacket() {
  TPacketGCLoginKey kLoginKeyPacket;
  if (!Recv(sizeof(TPacketGCLoginKey), &kLoginKeyPacket))
    return false;

  m_dwLoginKey = kLoginKeyPacket.dwLoginKey;
  m_isWaitLoginKey = FALSE;

  return true;
}
