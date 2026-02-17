#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

// Select Character
// ---------------------------------------------------------------------------
void CPythonNetworkStream::SetSelectPhase() {
  if ("Select" != m_strPhase)
    m_phaseLeaveFunc.Run();

  m_strPhase = "Select";

  m_dwChangingPhaseTime = ELTimer_GetMSec();
  m_phaseProcessFunc.Set(this, &CPythonNetworkStream::SelectPhase);
  m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveSelectPhase);

  if (__DirectEnterMode_IsSet()) {
    PyCallClassMemberFunc(m_poHandler, "SetLoadingPhase", Py_BuildValue("()"));
  } else {
    if (IsSelectedEmpire())
      PyCallClassMemberFunc(m_poHandler, "SetSelectCharacterPhase",
                            Py_BuildValue("()"));
    else
      PyCallClassMemberFunc(m_poHandler, "SetSelectEmpirePhase",
                            Py_BuildValue("()"));
  }
}

void CPythonNetworkStream::SelectPhase() { DispatchPacket(m_selectHandlers); }

bool CPythonNetworkStream::SendSelectEmpirePacket(DWORD dwEmpireID) {
  TPacketCGEmpire kPacketEmpire;
  kPacketEmpire.header = CG::EMPIRE;
  kPacketEmpire.length = sizeof(kPacketEmpire);
  kPacketEmpire.bEmpire = dwEmpireID;

  if (!Send(sizeof(kPacketEmpire), &kPacketEmpire)) {
    Tracen("SendSelectEmpirePacket - Error");
    return false;
  }

  SetEmpireID(dwEmpireID);
  return true;
}

bool CPythonNetworkStream::SendSelectCharacterPacket(BYTE Index) {
  TPacketCGSelectCharacter SelectCharacterPacket;

  SelectCharacterPacket.header = CG::CHARACTER_SELECT;
  SelectCharacterPacket.length = sizeof(SelectCharacterPacket);
  SelectCharacterPacket.player_index = Index;

  if (!Send(sizeof(TPacketCGSelectCharacter), &SelectCharacterPacket)) {
    Tracen("SendSelectCharacterPacket - Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendDestroyCharacterPacket(
    BYTE index, const char *szPrivateCode) {
  TPacketCGDestroyCharacter DestroyCharacterPacket;

  DestroyCharacterPacket.header = CG::CHARACTER_DELETE;
  DestroyCharacterPacket.length = sizeof(DestroyCharacterPacket);
  DestroyCharacterPacket.index = index;
  strncpy(DestroyCharacterPacket.szPrivateCode, szPrivateCode,
          PRIVATE_CODE_LENGTH - 1);

  if (!Send(sizeof(TPacketCGDestroyCharacter), &DestroyCharacterPacket)) {
    Tracen("SendDestroyCharacterPacket");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendCreateCharacterPacket(BYTE index,
                                                     const char *name, BYTE job,
                                                     BYTE shape, BYTE byCON,
                                                     BYTE byINT, BYTE bySTR,
                                                     BYTE byDEX) {
  TPacketCGCreateCharacter createCharacterPacket;

  createCharacterPacket.header = CG::CHARACTER_CREATE;
  createCharacterPacket.length = sizeof(createCharacterPacket);
  createCharacterPacket.index = index;
  strncpy(createCharacterPacket.name, name, CHARACTER_NAME_MAX_LEN);
  createCharacterPacket.job = job;
  createCharacterPacket.shape = shape;
  createCharacterPacket.CON = byCON;
  createCharacterPacket.INT = byINT;
  createCharacterPacket.STR = bySTR;
  createCharacterPacket.DEX = byDEX;

  if (!Send(sizeof(TPacketCGCreateCharacter), &createCharacterPacket)) {
    Tracen("Failed to SendCreateCharacterPacket");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendChangeNamePacket(BYTE index, const char *name) {
  TPacketCGChangeName ChangeNamePacket;
  ChangeNamePacket.header = CG::CHANGE_NAME;
  ChangeNamePacket.length = sizeof(ChangeNamePacket);
  ChangeNamePacket.index = index;
  strncpy(ChangeNamePacket.name, name, CHARACTER_NAME_MAX_LEN);

  if (!Send(sizeof(TPacketCGChangeName), &ChangeNamePacket)) {
    Tracen("Failed to SendChangeNamePacket");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::__RecvPlayerCreateSuccessPacket() {
  TPacketGCPlayerCreateSuccess kCreateSuccessPacket;

  if (!Recv(sizeof(kCreateSuccessPacket), &kCreateSuccessPacket))
    return false;

  if (kCreateSuccessPacket.bAccountCharacterSlot >= PLAYER_PER_ACCOUNT4) {
    TraceError("CPythonNetworkStream::RecvPlayerCreateSuccessPacket - OUT OF "
               "RANGE SLOT(%d) > PLATER_PER_ACCOUNT(%d)",
               kCreateSuccessPacket.bAccountCharacterSlot, PLAYER_PER_ACCOUNT4);
    return true;
  }

  m_akSimplePlayerInfo[kCreateSuccessPacket.bAccountCharacterSlot] =
      kCreateSuccessPacket.kSimplePlayerInfomation;
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_CREATE], "OnCreateSuccess",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::__RecvPlayerCreateFailurePacket() {
  TPacketGCCreateFailure packet;

  if (!Recv(sizeof(TPacketGCCreateFailure), &packet))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_CREATE], "OnCreateFailure",
                        Py_BuildValue("(i)", packet.bType));
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnCreateFailure",
                        Py_BuildValue("(i)", packet.bType));
  return true;
}

bool CPythonNetworkStream::__RecvPlayerDestroySuccessPacket() {
  TPacketGCDestroyCharacterSuccess packet;
  if (!Recv(sizeof(TPacketGCDestroyCharacterSuccess), &packet))
    return false;

  memset(&m_akSimplePlayerInfo[packet.account_index], 0,
         sizeof(m_akSimplePlayerInfo[packet.account_index]));
  m_adwGuildID[packet.account_index] = 0;
  m_astrGuildName[packet.account_index] = "";

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnDeleteSuccess",
                        Py_BuildValue("(i)", packet.account_index));
  return true;
}

bool CPythonNetworkStream::__RecvPlayerDestroyFailurePacket() {
  TPacketGCBlank packet_blank;
  if (!Recv(sizeof(TPacketGCBlank), &packet_blank))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnDeleteFailure",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::__RecvChangeName() {
  TPacketGCChangeName ChangeNamePacket;
  if (!Recv(sizeof(TPacketGCChangeName), &ChangeNamePacket))
    return false;

  for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i) {
    if (ChangeNamePacket.pid == m_akSimplePlayerInfo[i].dwID) {
      m_akSimplePlayerInfo[i].bChangeName = FALSE;
      strncpy(m_akSimplePlayerInfo[i].szName, ChangeNamePacket.name,
              CHARACTER_NAME_MAX_LEN);

      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnChangeName",
                            Py_BuildValue("(is)", i, ChangeNamePacket.name));
      return true;
    }
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnCreateFailure",
                        Py_BuildValue("(i)", 100));
  return true;
}
