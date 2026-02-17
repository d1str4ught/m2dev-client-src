#include "StdAfx.h"

#include "PythonNetworkStream.h"
#include "Packet.h"
#include "NetworkActorManager.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "MarkManager.h"

#include "ProcessCRC.h"

// MARK_BUG_FIX
static DWORD gs_nextDownloadMarkTime = 0;
// END_OF_MARK_BUG_FIX

void CPythonNetworkStream::ExitApplication() {
  if (__IsNotPing()) {
    AbsoluteExitApplication();
  } else {
    SendChatPacket("/quit");
  }
}

void CPythonNetworkStream::ExitGame() {
  if (__IsNotPing()) {
    LogOutGame();
  } else {
    SendChatPacket("/phase_select");
  }
}

void CPythonNetworkStream::LogOutGame() {
  if (__IsNotPing()) {
    AbsoluteExitGame();
  } else {
    SendChatPacket("/logout");
  }
}

void CPythonNetworkStream::AbsoluteExitGame() {
  if (!IsOnline())
    return;

  OnRemoteDisconnect();
  Disconnect();
}

void CPythonNetworkStream::AbsoluteExitApplication() { PostQuitMessage(0); }

bool CPythonNetworkStream::__IsNotPing() {
  // 원래는 핑이 안올때 체크이나 서버랑 정확히 맞추어야 한다.
  return false;
}

DWORD CPythonNetworkStream::GetGuildID() { return m_dwGuildID; }

UINT CPythonNetworkStream::UploadMark(const char *c_szImageFileName) {
  // MARK_BUG_FIX
  // 길드를 만든 직후는 길드 아이디가 0이다.
  if (0 == m_dwGuildID)
    return ERROR_MARK_UPLOAD_NEED_RECONNECT;

  gs_nextDownloadMarkTime = 0;
  // END_OF_MARK_BUG_FIX

  UINT uError = ERROR_UNKNOWN;
  CGuildMarkUploader &rkGuildMarkUploader = CGuildMarkUploader::Instance();
  if (!rkGuildMarkUploader.Connect(
          m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle,
          m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError)) {
    switch (uError) {
    case CGuildMarkUploader::ERROR_CONNECT:
      return ERROR_CONNECT_MARK_SERVER;
      break;
    case CGuildMarkUploader::ERROR_LOAD:
      return ERROR_LOAD_MARK;
      break;
    case CGuildMarkUploader::ERROR_WIDTH:
      return ERROR_MARK_WIDTH;
      break;
    case CGuildMarkUploader::ERROR_HEIGHT:
      return ERROR_MARK_HEIGHT;
      break;
    default:
      return ERROR_UNKNOWN;
    }
  }

  // MARK_BUG_FIX
  __DownloadMark();
  // END_OF_MARK_BUG_FIX

  if (CGuildMarkManager::INVALID_MARK_ID ==
      CGuildMarkManager::Instance().GetMarkID(m_dwGuildID))
    return ERROR_MARK_CHECK_NEED_RECONNECT;

  return ERROR_NONE;
}

UINT CPythonNetworkStream::UploadSymbol(const char *c_szImageFileName) {
  UINT uError = ERROR_UNKNOWN;
  CGuildMarkUploader &rkGuildMarkUploader = CGuildMarkUploader::Instance();
  if (!rkGuildMarkUploader.ConnectToSendSymbol(
          m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle,
          m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError)) {
    switch (uError) {
    case CGuildMarkUploader::ERROR_CONNECT:
      return ERROR_CONNECT_MARK_SERVER;
      break;
    case CGuildMarkUploader::ERROR_LOAD:
      return ERROR_LOAD_MARK;
      break;
    case CGuildMarkUploader::ERROR_WIDTH:
      return ERROR_MARK_WIDTH;
      break;
    case CGuildMarkUploader::ERROR_HEIGHT:
      return ERROR_MARK_HEIGHT;
      break;
    default:
      return ERROR_UNKNOWN;
    }
  }

  return ERROR_NONE;
}

void CPythonNetworkStream::__DownloadMark() {
  // 3분 안에는 다시 접속하지 않는다.
  DWORD curTime = ELTimer_GetMSec();

  if (curTime < gs_nextDownloadMarkTime)
    return;

  gs_nextDownloadMarkTime = curTime + 60000 * 3; // 3분

  CGuildMarkDownloader &rkGuildMarkDownloader =
      CGuildMarkDownloader::Instance();
  rkGuildMarkDownloader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle,
                                m_kMarkAuth.m_dwRandomKey);
}

void CPythonNetworkStream::__DownloadSymbol(
    const std::vector<DWORD> &c_rkVec_dwGuildID) {
  CGuildMarkDownloader &rkGuildMarkDownloader =
      CGuildMarkDownloader::Instance();
  rkGuildMarkDownloader.ConnectToRecvSymbol(
      m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey,
      c_rkVec_dwGuildID);
}

void CPythonNetworkStream::SetPhaseWindow(UINT ePhaseWnd,
                                          PyObject *poPhaseWnd) {
  if (ePhaseWnd >= PHASE_WINDOW_NUM)
    return;

  m_apoPhaseWnd[ePhaseWnd] = poPhaseWnd;
}

void CPythonNetworkStream::ClearPhaseWindow(UINT ePhaseWnd,
                                            PyObject *poPhaseWnd) {
  if (ePhaseWnd >= PHASE_WINDOW_NUM)
    return;

  if (poPhaseWnd != m_apoPhaseWnd[ePhaseWnd])
    return;

  m_apoPhaseWnd[ePhaseWnd] = 0;
}

void CPythonNetworkStream::SetServerCommandParserWindow(PyObject *poWnd) {
  m_poSerCommandParserWnd = poWnd;
}

bool CPythonNetworkStream::IsSelectedEmpire() {
  if (m_dwEmpireID)
    return true;

  return false;
}

UINT CPythonNetworkStream::GetAccountCharacterSlotDatau(UINT iSlot,
                                                        UINT eType) {
  if (iSlot >= PLAYER_PER_ACCOUNT4)
    return 0;

  TSimplePlayerInformation &rkSimplePlayerInfo = m_akSimplePlayerInfo[iSlot];

  switch (eType) {
  case ACCOUNT_CHARACTER_SLOT_ID:
    return rkSimplePlayerInfo.dwID;
  case ACCOUNT_CHARACTER_SLOT_RACE:
    return rkSimplePlayerInfo.byJob;
  case ACCOUNT_CHARACTER_SLOT_LEVEL:
    return rkSimplePlayerInfo.byLevel;
  case ACCOUNT_CHARACTER_SLOT_STR:
    return rkSimplePlayerInfo.byST;
  case ACCOUNT_CHARACTER_SLOT_DEX:
    return rkSimplePlayerInfo.byDX;
  case ACCOUNT_CHARACTER_SLOT_HTH:
    return rkSimplePlayerInfo.byHT;
  case ACCOUNT_CHARACTER_SLOT_INT:
    return rkSimplePlayerInfo.byIQ;
  case ACCOUNT_CHARACTER_SLOT_PLAYTIME:
    return rkSimplePlayerInfo.dwPlayMinutes;
  case ACCOUNT_CHARACTER_SLOT_FORM:
    //			return rkSimplePlayerInfo.wParts[CRaceData::PART_MAIN];
    return rkSimplePlayerInfo.wMainPart;
  case ACCOUNT_CHARACTER_SLOT_PORT:
    return rkSimplePlayerInfo.wPort;
  case ACCOUNT_CHARACTER_SLOT_GUILD_ID:
    return m_adwGuildID[iSlot];
    break;
  case ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG:
    return rkSimplePlayerInfo.bChangeName;
    break;
  case ACCOUNT_CHARACTER_SLOT_HAIR:
    return rkSimplePlayerInfo.wHairPart;
    break;
  }
  return 0;
}

const char *CPythonNetworkStream::GetAccountCharacterSlotDataz(UINT iSlot,
                                                               UINT eType) {
  static const char *sc_szEmpty = "";

  if (iSlot >= PLAYER_PER_ACCOUNT4)
    return sc_szEmpty;

  TSimplePlayerInformation &rkSimplePlayerInfo = m_akSimplePlayerInfo[iSlot];

  switch (eType) {
  case ACCOUNT_CHARACTER_SLOT_ADDR: {
    BYTE ip[4];

    const int LEN = 4;
    for (int i = 0; i < LEN; i++) {
      ip[i] = BYTE(rkSimplePlayerInfo.lAddr & 0xff);
      rkSimplePlayerInfo.lAddr >>= 8;
    }

    static char s_szAddr[256];
    sprintf(s_szAddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return s_szAddr;
  } break;
  case ACCOUNT_CHARACTER_SLOT_NAME:
    return rkSimplePlayerInfo.szName;
    break;
  case ACCOUNT_CHARACTER_SLOT_GUILD_NAME:
    return m_astrGuildName[iSlot].c_str();
    break;
  }
  return sc_szEmpty;
}

void CPythonNetworkStream::ConnectLoginServer(const char *c_szAddr,
                                              UINT uPort) {
  CNetworkStream::Connect(c_szAddr, uPort);
}

void CPythonNetworkStream::SetMarkServer(const char *c_szAddr, UINT uPort) {
  m_kMarkAuth.m_kNetAddr.Set(c_szAddr, uPort);
}

void CPythonNetworkStream::ConnectGameServer(UINT iChrSlot) {
  if (iChrSlot >= PLAYER_PER_ACCOUNT4)
    return;

  m_dwSelectedCharacterIndex = iChrSlot;

  __DirectEnterMode_Set(iChrSlot);

  TSimplePlayerInformation &rkSimplePlayerInfo = m_akSimplePlayerInfo[iChrSlot];
  CNetworkStream::Connect((DWORD)rkSimplePlayerInfo.lAddr,
                          rkSimplePlayerInfo.wPort);
}

void CPythonNetworkStream::SetLoginInfo(const char *c_szID,
                                        const char *c_szPassword) {
  m_stID = c_szID;
  m_stPassword = c_szPassword;
}

void CPythonNetworkStream::ClearLoginInfo(void) { m_stPassword = ""; }

void CPythonNetworkStream::SetLoginKey(DWORD dwLoginKey) {
  m_dwLoginKey = dwLoginKey;
}

// Table-driven packet dispatch — replaces CheckPacket() + per-phase switch
// statements. Returns true if a packet was processed and more may follow.
// Returns false to exit the phase loop (no data, error, or exitPhase handler).
bool CPythonNetworkStream::DispatchPacket(const PacketHandlerMap &handlers) {
  TPacketHeader header;
  if (!Peek(sizeof(TPacketHeader), &header))
    return false;

  // Skip zero-padding (can occur from encryption alignment)
  while (0 == header) {
    if (!Recv(sizeof(TPacketHeader), &header))
      return false;
    if (!Peek(sizeof(TPacketHeader), &header))
      return false;
  }

  // Look up handler in this phase's table
  auto it = handlers.find(header);
  if (it == handlers.end()) {
    TraceError("Unknown packet header: 0x%04X (recv_seq #%u), Phase: %s",
               header, m_dwRecvPacketSeq, m_strPhase.c_str());
    DumpRecentPackets();
    ClearRecvBuffer();
    return false;
  }

  // All packets use uniform framing: [header:2][length:2][payload...]
  TDynamicSizePacketHeader packetFrame;
  if (!Peek(sizeof(TDynamicSizePacketHeader), &packetFrame))
    return false;

  constexpr uint16_t MAX_PACKET_LENGTH = 65000;
  if (packetFrame.length < PACKET_HEADER_SIZE ||
      packetFrame.length > MAX_PACKET_LENGTH) {
    TraceError("DispatchPacket: Invalid packet length: header 0x%04X length: "
               "%u (min %u, max %u), recv_seq #%u",
               header, packetFrame.length, PACKET_HEADER_SIZE,
               MAX_PACKET_LENGTH, m_dwRecvPacketSeq);
    DumpRecentPackets();
    ClearRecvBuffer();
    PostQuitMessage(0);
    return false;
  }

  // Wait for full packet to be received
  if (!Peek(packetFrame.length))
    return false;

  // Log this packet
  LogRecvPacket(header, packetFrame.length);

  // Call handler
  bool ret = (this->*(it->second.handler))();

  if (!ret || it->second.exitPhase)
    return false;

  return true;
}

bool CPythonNetworkStream::RecvErrorPacket(int header) {
  TraceError("Phase %s does not handle header 0x%04X (recv_seq #%u)",
             m_strPhase.c_str(), header, m_dwRecvPacketSeq - 1);
  DumpRecentPackets();

  ClearRecvBuffer();
  return true;
}

bool CPythonNetworkStream::RecvPhasePacket() {
  TPacketGCPhase packet_phase;

  if (!Recv(sizeof(TPacketGCPhase), &packet_phase))
    return false;

  switch (packet_phase.phase) {
  case PHASE_CLOSE:
    ClosePhase();
    break;

  case PHASE_HANDSHAKE:
    SetHandShakePhase();
    break;

  case PHASE_LOGIN:
    SetLoginPhase();
    break;

  case PHASE_SELECT:
    SetSelectPhase();
#if defined(ENABLE_DISCORD_RPC)
    Discord_Update(false);
#endif
    BuildProcessCRC();
    // MARK_BUG_FIX
    __DownloadMark();
    // END_OF_MARK_BUG_FIX
    break;

  case PHASE_LOADING:
    SetLoadingPhase();
    break;

  case PHASE_GAME:
    SetGamePhase();
#if defined(ENABLE_DISCORD_RPC)
    Discord_Update(true);
#endif
    break;

  case PHASE_DEAD:
    break;
  }

  return true;
}

bool CPythonNetworkStream::RecvPingPacket() {
  TPacketGCPing kPacketPing;

  if (!Recv(sizeof(TPacketGCPing), &kPacketPing))
    return false;

  m_dwLastGamePingTime = ELTimer_GetMSec();

  // Sync server time from ping
  ELTimer_SetServerMSec(kPacketPing.server_time);

  TPacketCGPong kPacketPong;
  kPacketPong.header = CG::PONG;
  kPacketPong.length = sizeof(kPacketPong);

  if (!Send(sizeof(TPacketCGPong), &kPacketPong))
    return false;

  return true;
}

bool CPythonNetworkStream::OnProcess() {
  if (m_isStartGame) {
    m_isStartGame = FALSE;
    PyCallClassMemberFunc(m_poHandler, "SetGamePhase", Py_BuildValue("()"));
  }

  m_rokNetActorMgr->Update();

  if (m_phaseProcessFunc.IsEmpty())
    return true;

  m_phaseProcessFunc.Run();

  return true;
}

// Set
void CPythonNetworkStream::SetOffLinePhase() {
  if ("OffLine" != m_strPhase)
    m_phaseLeaveFunc.Run();

  Tracef("[PHASE] Entering phase: OffLine\n");
  m_strPhase = "OffLine";

  Tracen("");
  Tracen("## Network - OffLine Phase ##");
  Tracen("");

#if defined(ENABLE_DISCORD_RPC)
  Discord_Update(false);
#endif

  m_dwChangingPhaseTime = ELTimer_GetMSec();
  m_phaseProcessFunc.Set(this, &CPythonNetworkStream::OffLinePhase);
  m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveOfflinePhase);

  SetGameOffline();

  m_dwSelectedCharacterIndex = 0;

  __DirectEnterMode_Initialize();
  __BettingGuildWar_Initialize();
}

void CPythonNetworkStream::ClosePhase() {
  PyCallClassMemberFunc(m_poHandler, "SetLoginPhase", Py_BuildValue("()"));
}

// Game Online
void CPythonNetworkStream::SetGameOnline() { m_isGameOnline = TRUE; }

void CPythonNetworkStream::SetGameOffline() { m_isGameOnline = FALSE; }

BOOL CPythonNetworkStream::IsGameOnline() { return m_isGameOnline; }

// Handler
void CPythonNetworkStream::SetHandler(PyObject *poHandler) {
  m_poHandler = poHandler;
}

// ETC
DWORD CPythonNetworkStream::GetMainActorVID() { return m_dwMainActorVID; }

DWORD CPythonNetworkStream::GetMainActorRace() { return m_dwMainActorRace; }

DWORD CPythonNetworkStream::GetMainActorEmpire() { return m_dwMainActorEmpire; }

DWORD CPythonNetworkStream::GetMainActorSkillGroup() {
  return m_dwMainActorSkillGroup;
}

void CPythonNetworkStream::SetEmpireID(DWORD dwEmpireID) {
  m_dwEmpireID = dwEmpireID;
}

DWORD CPythonNetworkStream::GetEmpireID() { return m_dwEmpireID; }

void CPythonNetworkStream::__ClearSelectCharacterData() {
  NANOBEGIN
  memset(&m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

  for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i) {
    m_adwGuildID[i] = 0;
    m_astrGuildName[i] = "";
  }
  NANOEND
}

void CPythonNetworkStream::__DirectEnterMode_Initialize() {
  m_kDirectEnterMode.m_isSet = false;
  m_kDirectEnterMode.m_dwChrSlotIndex = 0;
}

void CPythonNetworkStream::__DirectEnterMode_Set(UINT uChrSlotIndex) {
  m_kDirectEnterMode.m_isSet = true;
  m_kDirectEnterMode.m_dwChrSlotIndex = uChrSlotIndex;
}

bool CPythonNetworkStream::__DirectEnterMode_IsSet() {
  return m_kDirectEnterMode.m_isSet;
}

void CPythonNetworkStream::__InitializeMarkAuth() {
  m_kMarkAuth.m_dwHandle = 0;
  m_kMarkAuth.m_dwRandomKey = 0;
}

void CPythonNetworkStream::__BettingGuildWar_Initialize() {
  m_kBettingGuildWar.m_dwBettingMoney = 0;
  m_kBettingGuildWar.m_dwObserverCount = 0;
}

void CPythonNetworkStream::__BettingGuildWar_SetObserverCount(
    UINT uObserverCount) {
  m_kBettingGuildWar.m_dwObserverCount = uObserverCount;
}

void CPythonNetworkStream::__BettingGuildWar_SetBettingMoney(
    UINT uBettingMoney) {
  m_kBettingGuildWar.m_dwBettingMoney = uBettingMoney;
}

DWORD CPythonNetworkStream::EXPORT_GetBettingGuildWarValue(
    const char *c_szValueName) {
  if (stricmp(c_szValueName, "OBSERVER_COUNT") == 0)
    return m_kBettingGuildWar.m_dwObserverCount;

  if (stricmp(c_szValueName, "BETTING_MONEY") == 0)
    return m_kBettingGuildWar.m_dwBettingMoney;

  return 0;
}

void CPythonNetworkStream::__ServerTimeSync_Initialize() {
  m_kServerTimeSync.m_dwChangeClientTime = 0;
  m_kServerTimeSync.m_dwChangeServerTime = 0;
}

void CPythonNetworkStream::SetWaitFlag() { m_isWaitLoginKey = TRUE; }

void CPythonNetworkStream::SendEmoticon(UINT eEmoticon) {
  if (eEmoticon < m_EmoticonStringVector.size())
    SendChatPacket(m_EmoticonStringVector[eEmoticon].c_str());
  else
    assert(false && "SendEmoticon Error");
}

CPythonNetworkStream::CPythonNetworkStream() {
  m_rokNetActorMgr = new CNetworkActorManager;

  memset(m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

  // Increase network buffer sizes for better throughput
  SetRecvBufferSize(65536); // 64KB recv buffer
  SetSendBufferSize(65536); // 64KB send buffer

  m_phaseProcessFunc.Clear();

  m_dwEmpireID = 0;
  m_dwGuildID = 0;

  m_dwMainActorVID = 0;
  m_dwMainActorRace = 0;
  m_dwMainActorEmpire = 0;
  m_dwMainActorSkillGroup = 0;
  m_poHandler = NULL;

  m_dwLastGamePingTime = 0;

  m_dwLoginKey = 0;
  m_isWaitLoginKey = FALSE;
  m_isStartGame = FALSE;
  m_isEnableChatInsultFilter = FALSE;
  m_bComboSkillFlag = FALSE;
  m_strPhase = "OffLine";

  __InitializeGamePhase();
  __InitializeMarkAuth();

  __DirectEnterMode_Initialize();
  __BettingGuildWar_Initialize();

  std::fill(m_apoPhaseWnd, m_apoPhaseWnd + PHASE_WINDOW_NUM, (PyObject *)NULL);
  m_poSerCommandParserWnd = NULL;

  // Register packet handlers for all phases
  // Note: GameHandlers must be registered before LoadingHandlers (loading
  // copies game handlers as fallback)
  RegisterOfflineHandlers();
  RegisterHandshakeHandlers();
  RegisterLoginHandlers();
  RegisterSelectHandlers();
  RegisterGameHandlers();
  RegisterLoadingHandlers();

  SetOffLinePhase();
}

CPythonNetworkStream::~CPythonNetworkStream() {
  Tracen("PythonNetworkMainStream Clear");
}

// ---------------------------------------------------------------------------
// Phase 5: Packet handler registration tables
// Each phase registers {header → handler, minSize, exitPhase}
// ---------------------------------------------------------------------------

void CPythonNetworkStream::RegisterOfflineHandlers() {
  auto &h = m_offlineHandlers;
  h[GC::PHASE] = {&CPythonNetworkStream::RecvPhasePacket,
                  sizeof(TPacketGCPhase), true};
}

void CPythonNetworkStream::RegisterHandshakeHandlers() {
  auto &h = m_handshakeHandlers;
  h[GC::PHASE] = {&CPythonNetworkStream::RecvPhasePacket,
                  sizeof(TPacketGCPhase), true};
  h[GC::PING] = {&CPythonNetworkStream::RecvPingPacket, sizeof(TPacketGCPing),
                 true};
  h[GC::KEY_CHALLENGE] = {&CPythonNetworkStream::RecvKeyChallenge,
                          sizeof(TPacketGCKeyChallenge), true};
  h[GC::KEY_COMPLETE] = {&CPythonNetworkStream::RecvKeyComplete,
                         sizeof(TPacketGCKeyComplete), true};
}

void CPythonNetworkStream::RegisterLoginHandlers() {
  auto &h = m_loginHandlers;
  h[GC::PHASE] = {&CPythonNetworkStream::RecvPhasePacket,
                  sizeof(TPacketGCPhase), true};
  h[GC::LOGIN_SUCCESS3] = {&CPythonNetworkStream::__RecvLoginSuccessPacket3,
                           sizeof(TPacketGCLoginSuccess3), false};
  h[GC::LOGIN_SUCCESS4] = {&CPythonNetworkStream::__RecvLoginSuccessPacket4,
                           sizeof(TPacketGCLoginSuccess4), false};
  h[GC::LOGIN_FAILURE] = {&CPythonNetworkStream::__RecvLoginFailurePacket,
                          sizeof(TPacketGCLoginFailure), false};
  h[GC::EMPIRE] = {&CPythonNetworkStream::__RecvEmpirePacket,
                   sizeof(TPacketGCEmpire), false};
  h[GC::LOGIN_KEY] = {&CPythonNetworkStream::__RecvLoginKeyPacket,
                      sizeof(TPacketGCLoginKey), false};
  h[GC::PING] = {&CPythonNetworkStream::RecvPingPacket, sizeof(TPacketGCPing),
                 false};
  h[GC::KEY_CHALLENGE] = {&CPythonNetworkStream::RecvKeyChallenge,
                          sizeof(TPacketGCKeyChallenge), true};
  h[GC::KEY_COMPLETE] = {&CPythonNetworkStream::RecvKeyComplete,
                         sizeof(TPacketGCKeyComplete), true};
}

void CPythonNetworkStream::RegisterSelectHandlers() {
  auto &h = m_selectHandlers;
  h[GC::PHASE] = {&CPythonNetworkStream::RecvPhasePacket,
                  sizeof(TPacketGCPhase), true};
  h[GC::EMPIRE] = {&CPythonNetworkStream::__RecvEmpirePacket,
                   sizeof(TPacketGCEmpire), false};
  h[GC::LOGIN_SUCCESS3] = {&CPythonNetworkStream::__RecvLoginSuccessPacket3,
                           sizeof(TPacketGCLoginSuccess3), false};
  h[GC::LOGIN_SUCCESS4] = {&CPythonNetworkStream::__RecvLoginSuccessPacket4,
                           sizeof(TPacketGCLoginSuccess4), false};
  h[GC::PLAYER_CREATE_SUCCESS] = {
      &CPythonNetworkStream::__RecvPlayerCreateSuccessPacket,
      sizeof(TPacketGCPlayerCreateSuccess), false};
  h[GC::PLAYER_CREATE_FAILURE] = {
      &CPythonNetworkStream::__RecvPlayerCreateFailurePacket,
      sizeof(TPacketGCCreateFailure), false};
  h[GC::PLAYER_DELETE_WRONG_SOCIAL_ID] = {
      &CPythonNetworkStream::__RecvPlayerDestroyFailurePacket,
      sizeof(TPacketGCBlank), false};
  h[GC::PLAYER_DELETE_SUCCESS] = {
      &CPythonNetworkStream::__RecvPlayerDestroySuccessPacket,
      sizeof(TPacketGCDestroyCharacterSuccess), false};
  h[GC::CHANGE_NAME] = {&CPythonNetworkStream::__RecvChangeName,
                        sizeof(TPacketGCChangeName), false};
  h[GC::PLAYER_POINT_CHANGE] = {&CPythonNetworkStream::RecvPointChange,
                                sizeof(TPacketGCPointChange), false};
  h[GC::PING] = {&CPythonNetworkStream::RecvPingPacket, sizeof(TPacketGCPing),
                 false};
  h[GC::KEY_CHALLENGE] = {&CPythonNetworkStream::RecvKeyChallenge,
                          sizeof(TPacketGCKeyChallenge), true};
  h[GC::KEY_COMPLETE] = {&CPythonNetworkStream::RecvKeyComplete,
                         sizeof(TPacketGCKeyComplete), true};
}

void CPythonNetworkStream::RegisterGameHandlers() {
  auto &h = m_gameHandlers;

  // Phase / control
  h[GC::PHASE] = {&CPythonNetworkStream::RecvPhasePacket,
                  sizeof(TPacketGCPhase), true};
  h[GC::KEY_CHALLENGE] = {&CPythonNetworkStream::RecvKeyChallenge,
                          sizeof(TPacketGCKeyChallenge), true};
  h[GC::KEY_COMPLETE] = {&CPythonNetworkStream::RecvKeyComplete,
                         sizeof(TPacketGCKeyComplete), true};
  h[GC::PING] = {&CPythonNetworkStream::RecvPingPacket, sizeof(TPacketGCPing),
                 false};

  // Observer
  h[GC::OBSERVER_ADD] = {&CPythonNetworkStream::RecvObserverAddPacket,
                         sizeof(TPacketGCObserverAdd), false};
  h[GC::OBSERVER_REMOVE] = {&CPythonNetworkStream::RecvObserverRemovePacket,
                            sizeof(TPacketGCObserverRemove), false};
  h[GC::OBSERVER_MOVE] = {&CPythonNetworkStream::RecvObserverMovePacket,
                          sizeof(TPacketGCObserverMove), false};

  // Warp / PVP
  h[GC::WARP] = {&CPythonNetworkStream::RecvWarpPacket, sizeof(TPacketGCWarp),
                 false};
  h[GC::PVP] = {&CPythonNetworkStream::RecvPVPPacket, sizeof(TPacketGCPVP),
                false};
  h[GC::DUEL_START] = {&CPythonNetworkStream::RecvDuelStartPacket,
                       sizeof(TPacketGCDuelStart), false};

  // Character
  h[GC::CHARACTER_ADD] = {&CPythonNetworkStream::RecvCharacterAppendPacket,
                          sizeof(TPacketGCCharacterAdd), false};
  h[GC::CHAR_ADDITIONAL_INFO] = {
      &CPythonNetworkStream::RecvCharacterAdditionalInfo,
      sizeof(TPacketGCCharacterAdditionalInfo), false};
  h[GC::CHARACTER_ADD2] = {&CPythonNetworkStream::RecvCharacterAppendPacketNew,
                           sizeof(TPacketGCCharacterAdd2), false};
  h[GC::CHARACTER_UPDATE] = {&CPythonNetworkStream::RecvCharacterUpdatePacket,
                             sizeof(TPacketGCCharacterUpdate), false};
  h[GC::CHARACTER_UPDATE2] = {
      &CPythonNetworkStream::RecvCharacterUpdatePacketNew,
      sizeof(TPacketGCCharacterUpdate2), false};
  h[GC::CHARACTER_DEL] = {&CPythonNetworkStream::RecvCharacterDeletePacket,
                          sizeof(TPacketGCCharacterDelete), false};
  h[GC::CHAT] = {&CPythonNetworkStream::RecvChatPacket, sizeof(TPacketGCChat),
                 false};
  h[GC::SYNC_POSITION] = {&CPythonNetworkStream::RecvSyncPositionPacket,
                          sizeof(TPacketGCC2C), false};
  h[GC::OWNERSHIP] = {&CPythonNetworkStream::RecvOwnerShipPacket,
                      sizeof(TPacketGCOwnership), false};
  h[GC::WHISPER] = {&CPythonNetworkStream::RecvWhisperPacket,
                    sizeof(TPacketGCWhisper), false};
  h[GC::MOVE] = {&CPythonNetworkStream::RecvCharacterMovePacket,
                 sizeof(TPacketGCMove), false};
  h[GC::CHARACTER_POSITION] = {
      &CPythonNetworkStream::RecvCharacterPositionPacket,
      sizeof(TPacketGCPosition), false};

  // Combat
  h[GC::STUN] = {&CPythonNetworkStream::RecvStunPacket, sizeof(TPacketGCStun),
                 false};
  h[GC::DEAD] = {&CPythonNetworkStream::RecvDeadPacket, sizeof(TPacketGCDead),
                 false};
  h[GC::PLAYER_POINT_CHANGE] = {&CPythonNetworkStream::RecvPointChange,
                                sizeof(TPacketGCPointChange), false};
  h[GC::DAMAGE_INFO] = {&CPythonNetworkStream::RecvDamageInfoPacket,
                        sizeof(TPacketGCDamageInfo), false};

  // Items
  h[GC::ITEM_DEL] = {&CPythonNetworkStream::RecvItemDelPacket,
                     sizeof(TPacketGCItemDel), false};
  h[GC::ITEM_SET] = {&CPythonNetworkStream::RecvItemSetPacket,
                     sizeof(TPacketGCItemSet), false};
  h[GC::ITEM_GET] = {&CPythonNetworkStream::RecvItemGetPacket,
                     sizeof(TPacketGCItemGet), false};
  h[GC::ITEM_USE] = {&CPythonNetworkStream::RecvItemUsePacket,
                     sizeof(TPacketGCItemUse), false};
  h[GC::ITEM_UPDATE] = {&CPythonNetworkStream::RecvItemUpdatePacket,
                        sizeof(TPacketGCItemUpdate), false};
  h[GC::ITEM_GROUND_ADD] = {&CPythonNetworkStream::RecvItemGroundAddPacket,
                            sizeof(TPacketGCItemGroundAdd), false};
  h[GC::ITEM_GROUND_DEL] = {&CPythonNetworkStream::RecvItemGroundDelPacket,
                            sizeof(TPacketGCItemGroundDel), false};
  h[GC::ITEM_OWNERSHIP] = {&CPythonNetworkStream::RecvItemOwnership,
                           sizeof(TPacketGCItemOwnership), false};

  // Quickslot
  h[GC::QUICKSLOT_ADD] = {&CPythonNetworkStream::RecvQuickSlotAddPacket,
                          sizeof(TPacketGCQuickSlotAdd), false};
  h[GC::QUICKSLOT_DEL] = {&CPythonNetworkStream::RecvQuickSlotDelPacket,
                          sizeof(TPacketGCQuickSlotDel), false};
  h[GC::QUICKSLOT_SWAP] = {&CPythonNetworkStream::RecvQuickSlotMovePacket,
                           sizeof(TPacketGCQuickSlotSwap), false};

  // Motion / Movement
  h[GC::MOTION] = {&CPythonNetworkStream::RecvMotionPacket,
                   sizeof(TPacketGCMotion), false};
  h[GC::CHANGE_SPEED] = {&CPythonNetworkStream::RecvChangeSpeedPacket,
                         sizeof(TPacketGCChangeSpeed), false};

  // Shop / Exchange
  h[GC::SHOP] = {&CPythonNetworkStream::RecvShopPacket, sizeof(TPacketGCShop),
                 false};
  h[GC::SHOP_SIGN] = {&CPythonNetworkStream::RecvShopSignPacket,
                      sizeof(TPacketGCShopSign), false};
  h[GC::EXCHANGE] = {&CPythonNetworkStream::RecvExchangePacket,
                     sizeof(TPacketGCExchange), false};

  // Quest
  h[GC::QUEST_INFO] = {&CPythonNetworkStream::RecvQuestInfoPacket,
                       sizeof(TPacketGCQuestInfo), false};
  h[GC::REQUEST_MAKE_GUILD] = {&CPythonNetworkStream::RecvRequestMakeGuild,
                               sizeof(TPacketGCBlank), false};
  h[GC::SCRIPT] = {&CPythonNetworkStream::RecvScriptPacket,
                   sizeof(TPacketGCScript), false};
  h[GC::QUEST_CONFIRM] = {&CPythonNetworkStream::RecvQuestConfirmPacket,
                          sizeof(TPacketGCQuestConfirm), false};

  // Target / Mount
  h[GC::TARGET] = {&CPythonNetworkStream::RecvTargetPacket,
                   sizeof(TPacketGCTarget), false};
  h[GC::MOUNT] = {&CPythonNetworkStream::RecvMountPacket,
                  sizeof(TPacketGCMount), false};

  // Points
  h[GC::PLAYER_POINTS] = {&CPythonNetworkStream::__RecvPlayerPoints,
                          sizeof(TPacketGCPoints), false};

  // Fly
  h[GC::CREATE_FLY] = {&CPythonNetworkStream::RecvCreateFlyPacket,
                       sizeof(TPacketGCCreateFly), false};
  h[GC::FLY_TARGETING] = {&CPythonNetworkStream::RecvFlyTargetingPacket,
                          sizeof(TPacketGCFlyTargeting), false};
  h[GC::ADD_FLY_TARGETING] = {&CPythonNetworkStream::RecvAddFlyTargetingPacket,
                              sizeof(TPacketGCFlyTargeting), false};

  // Skills
  h[GC::SKILL_LEVEL] = {&CPythonNetworkStream::RecvSkillLevel,
                        sizeof(TPacketGCSkillLevel), false};
  h[GC::SKILL_LEVEL_NEW] = {&CPythonNetworkStream::RecvSkillLevelNew,
                            sizeof(TPacketGCSkillLevelNew), false};
  h[GC::SKILL_COOLTIME_END] = {&CPythonNetworkStream::RecvSkillCoolTimeEnd,
                               sizeof(TPacketGCSkillCoolTimeEnd), false};

  // Messenger / Guild
  h[GC::MESSENGER] = {&CPythonNetworkStream::RecvMessenger,
                      sizeof(TPacketGCMessenger), false};
  h[GC::GUILD] = {&CPythonNetworkStream::RecvGuild, sizeof(TPacketGCGuild),
                  false};
  h[GC::MARK_UPDATE] = {&CPythonNetworkStream::RecvMarkUpdate,
                        sizeof(TPacketGCMarkUpdate), false};

  // Party
  h[GC::PARTY_INVITE] = {&CPythonNetworkStream::RecvPartyInvite,
                         sizeof(TPacketGCPartyInvite), false};
  h[GC::PARTY_ADD] = {&CPythonNetworkStream::RecvPartyAdd,
                      sizeof(TPacketGCPartyAdd), false};
  h[GC::PARTY_UPDATE] = {&CPythonNetworkStream::RecvPartyUpdate,
                         sizeof(TPacketGCPartyUpdate), false};
  h[GC::PARTY_REMOVE] = {&CPythonNetworkStream::RecvPartyRemove,
                         sizeof(TPacketGCPartyRemove), false};
  h[GC::PARTY_LINK] = {&CPythonNetworkStream::RecvPartyLink,
                       sizeof(TPacketGCPartyLink), false};
  h[GC::PARTY_UNLINK] = {&CPythonNetworkStream::RecvPartyUnlink,
                         sizeof(TPacketGCPartyUnlink), false};
  h[GC::PARTY_PARAMETER] = {&CPythonNetworkStream::RecvPartyParameter,
                            sizeof(TPacketGCPartyParameter), false};

  // Safebox
  h[GC::SAFEBOX_SET] = {&CPythonNetworkStream::RecvSafeBoxSetPacket,
                        sizeof(TPacketGCItemSet), false};
  h[GC::SAFEBOX_DEL] = {&CPythonNetworkStream::RecvSafeBoxDelPacket,
                        sizeof(TPacketGCItemDel), false};
  h[GC::SAFEBOX_WRONG_PASSWORD] = {
      &CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket,
      sizeof(TPacketGCSafeboxWrongPassword), false};
  h[GC::SAFEBOX_SIZE] = {&CPythonNetworkStream::RecvSafeBoxSizePacket,
                         sizeof(TPacketGCSafeboxSize), false};
  h[GC::SAFEBOX_MONEY_CHANGE] = {
      &CPythonNetworkStream::RecvSafeBoxMoneyChangePacket,
      sizeof(TPacketGCSafeboxMoneyChange), false};

  // Fishing / Dungeon / Time
  h[GC::FISHING] = {&CPythonNetworkStream::RecvFishing,
                    sizeof(TPacketGCFishing), false};
  h[GC::DUNGEON] = {&CPythonNetworkStream::RecvDungeon,
                    sizeof(TPacketGCDungeon), false};
  h[GC::TIME] = {&CPythonNetworkStream::RecvTimePacket, sizeof(TPacketGCTime),
                 false};

  // Walk / Skill group / Refine
  h[GC::WALK_MODE] = {&CPythonNetworkStream::RecvWalkModePacket,
                      sizeof(TPacketGCWalkMode), false};
  h[GC::CHANGE_SKILL_GROUP] = {
      &CPythonNetworkStream::RecvChangeSkillGroupPacket,
      sizeof(TPacketGCChangeSkillGroup), false};
  h[GC::REFINE_INFORMATION] = {
      &CPythonNetworkStream::RecvRefineInformationPacket,
      sizeof(TPacketGCRefineInformation), false};
  h[GC::REFINE_INFORMATION_NEW] = {
      &CPythonNetworkStream::RecvRefineInformationPacketNew,
      sizeof(TPacketGCRefineInformationNew), false};

  // Effects
  h[GC::SEPCIAL_EFFECT] = {&CPythonNetworkStream::RecvSpecialEffect,
                           sizeof(TPacketGCSpecialEffect), false};
  h[GC::SPECIFIC_EFFECT] = {&CPythonNetworkStream::RecvSpecificEffect,
                            sizeof(TPacketGCSpecificEffect), false};

  // Map / NPC
  h[GC::NPC_POSITION] = {&CPythonNetworkStream::RecvNPCList,
                         sizeof(TPacketGCNPCPosition), false};
  h[GC::CHANNEL] = {&CPythonNetworkStream::RecvChannelPacket,
                    sizeof(TPacketGCChannel), false};
  h[GC::VIEW_EQUIP] = {&CPythonNetworkStream::RecvViewEquipPacket,
                       sizeof(TPacketGCViewEquip), false};
  h[GC::LAND_LIST] = {&CPythonNetworkStream::RecvLandPacket,
                      sizeof(TPacketGCLandList), false};

  // Target
  h[GC::TARGET_CREATE_NEW] = {&CPythonNetworkStream::RecvTargetCreatePacketNew,
                              sizeof(TPacketGCTargetCreateNew), false};
  h[GC::TARGET_UPDATE] = {&CPythonNetworkStream::RecvTargetUpdatePacket,
                          sizeof(TPacketGCTargetUpdate), false};
  h[GC::TARGET_DELETE] = {&CPythonNetworkStream::RecvTargetDeletePacket,
                          sizeof(TPacketGCTargetDelete), false};

  // Affect
  h[GC::AFFECT_ADD] = {&CPythonNetworkStream::RecvAffectAddPacket,
                       sizeof(TPacketGCAffectAdd), false};
  h[GC::AFFECT_REMOVE] = {&CPythonNetworkStream::RecvAffectRemovePacket,
                          sizeof(TPacketGCAffectRemove), false};

  // Mall
  h[GC::MALL_OPEN] = {&CPythonNetworkStream::RecvMallOpenPacket,
                      sizeof(TPacketGCMallOpen), false};
  h[GC::MALL_SET] = {&CPythonNetworkStream::RecvMallItemSetPacket,
                     sizeof(TPacketGCItemSet), false};
  h[GC::MALL_DEL] = {&CPythonNetworkStream::RecvMallItemDelPacket,
                     sizeof(TPacketGCItemDel), false};

  // Lover
  h[GC::LOVER_INFO] = {&CPythonNetworkStream::RecvLoverInfoPacket,
                       sizeof(TPacketGCLoverInfo), false};
  h[GC::LOVE_POINT_UPDATE] = {&CPythonNetworkStream::RecvLovePointUpdatePacket,
                              sizeof(TPacketGCLovePointUpdate), false};

  // Misc
  h[GC::DIG_MOTION] = {&CPythonNetworkStream::RecvDigMotionPacket,
                       sizeof(TPacketGCDigMotion), false};
  h[GC::DRAGON_SOUL_REFINE] = {&CPythonNetworkStream::RecvDragonSoulRefine,
                               sizeof(TPacketGCDragonSoulRefine), false};
}

void CPythonNetworkStream::RegisterLoadingHandlers() {
  // Loading phase: start with ALL game handlers as fallback (old code: default
  // -> GamePhase())
  m_loadingHandlers = m_gameHandlers;

  // Override/add loading-specific handlers
  m_loadingHandlers[GC::PHASE] = {&CPythonNetworkStream::RecvPhasePacket,
                                  sizeof(TPacketGCPhase), true};
  m_loadingHandlers[GC::MAIN_CHARACTER] = {
      &CPythonNetworkStream::RecvMainCharacter, sizeof(TPacketGCMainCharacter),
      false};
}

// --- Packet sequence tracking ---

void CPythonNetworkStream::LogRecvPacket(uint16_t header, uint16_t length) {
  auto &e = m_aRecentRecvPackets[m_dwRecvPacketSeq % PACKET_LOG_SIZE];
  e.seq = m_dwRecvPacketSeq;
  e.header = header;
  e.length = length;
  m_dwRecvPacketSeq++;
}

void CPythonNetworkStream::DumpRecentPackets() const {
  const uint32_t recvCount =
      std::min(m_dwRecvPacketSeq, (uint32_t)PACKET_LOG_SIZE);
  const uint32_t sentCount =
      std::min(m_dwSentPacketSeq, (uint32_t)SENT_PACKET_LOG_SIZE);

  TraceError("=== Recent RECV packets (last %u of %u total) ===", recvCount,
             m_dwRecvPacketSeq);

  for (uint32_t i = 0; i < recvCount; i++) {
    uint32_t idx = (m_dwRecvPacketSeq > PACKET_LOG_SIZE)
                       ? (m_dwRecvPacketSeq - PACKET_LOG_SIZE + i)
                       : i;
    const auto &e = m_aRecentRecvPackets[idx % PACKET_LOG_SIZE];
    TraceError("  RECV #%u: header=0x%04X len=%u", e.seq, e.header, e.length);
  }

  TraceError("=== Recent SENT packets (last %u of %u total) ===", sentCount,
             m_dwSentPacketSeq);

  for (uint32_t i = 0; i < sentCount; i++) {
    uint32_t idx = (m_dwSentPacketSeq > SENT_PACKET_LOG_SIZE)
                       ? (m_dwSentPacketSeq - SENT_PACKET_LOG_SIZE + i)
                       : i;
    const auto &e = m_aSentPacketLog[idx % SENT_PACKET_LOG_SIZE];
    TraceError("  SENT #%u: header=0x%04X len=%u", e.seq, e.header, e.length);
  }
}
