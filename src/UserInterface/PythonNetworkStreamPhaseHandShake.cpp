#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include "Packet.h"

// HandShake
// ---------------------------------------------------------------------------
void CPythonNetworkStream::HandShakePhase() {
  DispatchPacket(m_handshakeHandlers);
}

void CPythonNetworkStream::SetHandShakePhase() {
  if ("HandShake" != m_strPhase)
    m_phaseLeaveFunc.Run();

  Tracef("[PHASE] Entering phase: HandShake\n");
  Tracen("");
  Tracen("## Network - Hand Shake Phase ##");
  Tracen("");

  m_strPhase = "HandShake";

  m_dwChangingPhaseTime = ELTimer_GetMSec();
  m_phaseProcessFunc.Set(this, &CPythonNetworkStream::HandShakePhase);
  m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveHandshakePhase);

  SetGameOnline();

  if (__DirectEnterMode_IsSet()) {
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnHandShake",
                          Py_BuildValue("()"));
  }
}

// Override: extract time sync before delegating crypto to base class
bool CPythonNetworkStream::RecvKeyChallenge() {
  // Peek to extract server_time before base class Recv() consumes the packet
  TPacketGCKeyChallenge packet;
  if (!Peek(sizeof(packet), &packet))
    return false;

  // Time sync from server (game connection only)
  m_kServerTimeSync.m_dwChangeServerTime = packet.server_time;
  m_kServerTimeSync.m_dwChangeClientTime = ELTimer_GetMSec();
  ELTimer_SetServerMSec(packet.server_time);
  CTimer::Instance().SetBaseTime();

  Tracef(
      "[HANDSHAKE] KeyChallenge: server_time=%u, client_time=%u, offset=%d\n",
      packet.server_time, ELTimer_GetMSec(),
      (int)(packet.server_time - ELTimer_GetMSec()));

  // Base class handles cipher init, key computation, and response
  return CNetworkStream::RecvKeyChallenge();
}

// RecvKeyComplete: no game-specific additions, delegate entirely to base
bool CPythonNetworkStream::RecvKeyComplete() {
  return CNetworkStream::RecvKeyComplete();
}
