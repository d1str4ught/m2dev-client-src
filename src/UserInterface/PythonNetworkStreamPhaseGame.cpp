#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "GuildMarkDownloader.h"

#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonChat.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonQuest.h"
#include "PythonEventManager.h"
#include "PythonMessenger.h"
#include "PythonApplication.h"

#include "GameLib/ItemManager.h"

#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"

#include "ProcessCRC.h"

BOOL gs_bEmpireLanuageEnable = TRUE;

void CPythonNetworkStream::__RefreshAlignmentWindow() {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment",
                        Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(DWORD dwVID) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char *c_szName) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "RefreshTargetBoardByName",
                        Py_BuildValue("(s)", c_szName));
}

void CPythonNetworkStream::__RefreshTargetBoard() {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoard",
                        Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage() {
  m_isRefreshGuildWndGradePage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage() {
  m_isRefreshGuildWndSkillPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox() {
  m_isRefreshGuildWndMemberPageGradeComboBox = true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage() {
  m_isRefreshGuildWndMemberPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage() {
  m_isRefreshGuildWndBoardPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage() {
  m_isRefreshGuildWndInfoPage = true;
}

void CPythonNetworkStream::__RefreshMessengerWindow() {
  m_isRefreshMessengerWnd = true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow() {
  m_isRefreshSafeboxWnd = true;
}

void CPythonNetworkStream::__RefreshMallWindow() { m_isRefreshMallWnd = true; }

void CPythonNetworkStream::__RefreshSkillWindow() {
  m_isRefreshSkillWnd = true;
}

void CPythonNetworkStream::__RefreshExchangeWindow() {
  m_isRefreshExchangeWnd = true;
}

void CPythonNetworkStream::__RefreshStatus() { m_isRefreshStatus = true; }

void CPythonNetworkStream::__RefreshCharacterWindow() {
  m_isRefreshCharacterWnd = true;
}

void CPythonNetworkStream::__RefreshInventoryWindow() {
  m_isRefreshInventoryWnd = true;
}

void CPythonNetworkStream::__RefreshEquipmentWindow() {
  m_isRefreshEquipmentWnd = true;
}

void CPythonNetworkStream::__SetGuildID(DWORD id) {
  if (m_dwGuildID != id) {
    m_dwGuildID = id;
    IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();

    for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i)
      if (!strncmp(m_akSimplePlayerInfo[i].szName, rkPlayer.GetName(),
                   CHARACTER_NAME_MAX_LEN)) {
        m_adwGuildID[i] = id;

        std::string guildName;
        if (CPythonGuild::Instance().GetGuildName(id, &guildName)) {
          m_astrGuildName[i] = guildName;
        } else {
          m_astrGuildName[i] = "";
        }
      }
  }
}

struct PERF_PacketInfo {
  DWORD dwCount;
  DWORD dwTime;

  PERF_PacketInfo() {
    dwCount = 0;
    dwTime = 0;
  }
};

#ifdef __PERFORMANCE_CHECK__

class PERF_PacketTimeAnalyzer {
public:
  ~PERF_PacketTimeAnalyzer() {
    FILE *fp = fopen("perf_dispatch_packet_result.txt", "w");

    for (std::map<DWORD, PERF_PacketInfo>::iterator i =
             m_kMap_kPacketInfo.begin();
         i != m_kMap_kPacketInfo.end(); ++i) {
      if (i->second.dwTime > 0)
        fprintf(fp, "header %d: count %d, time %d, tpc %d\n", i->first,
                i->second.dwCount, i->second.dwTime,
                i->second.dwTime / i->second.dwCount);
    }
    fclose(fp);
  }

public:
  std::map<DWORD, PERF_PacketInfo> m_kMap_kPacketInfo;
};

PERF_PacketTimeAnalyzer gs_kPacketTimeAnalyzer;

#endif

// Game Phase
// ---------------------------------------------------------------------------
void CPythonNetworkStream::GamePhase() {
  if (!m_kQue_stHack.empty()) {
    __SendHack(m_kQue_stHack.front().c_str());
    m_kQue_stHack.pop_front();
  }

#ifdef __PERFORMANCE_CHECK__
  DWORD timeBeginDispatch = timeGetTime();
#endif

  const DWORD MAX_RECV_COUNT = 32;
  const DWORD SAFE_RECV_BUFSIZE = 8192;
  DWORD dwRecvCount = 0;

  while (true) {
    if (dwRecvCount++ >= MAX_RECV_COUNT - 1 &&
        GetRecvBufferSize() < SAFE_RECV_BUFSIZE && m_strPhase == "Game")
      break;

    if (!DispatchPacket(m_gameHandlers))
      break;
  }

#ifdef __PERFORMANCE_CHECK__
  DWORD timeEndDispatch = timeGetTime();

  if (timeEndDispatch - timeBeginDispatch > 2) {
    static FILE *fp = fopen("perf_dispatch_packet.txt", "w");
    fprintf(fp, "delay %d\n", timeEndDispatch - timeBeginDispatch);
    fputs("=====================================================\n", fp);
    fflush(fp);
  }
#endif

  static DWORD s_nextRefreshTime = ELTimer_GetMSec();

  DWORD curTime = ELTimer_GetMSec();
  if (s_nextRefreshTime > curTime)
    return;

  if (m_isRefreshCharacterWnd) {
    m_isRefreshCharacterWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshEquipmentWnd) {
    m_isRefreshEquipmentWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipment",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshInventoryWnd) {
    m_isRefreshInventoryWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshExchangeWnd) {
    m_isRefreshExchangeWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchange",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshSkillWnd) {
    m_isRefreshSkillWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshSafeboxWnd) {
    m_isRefreshSafeboxWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebox",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshMallWnd) {
    m_isRefreshMallWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshStatus) {
    m_isRefreshStatus = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshMessengerWnd) {
    m_isRefreshMessengerWnd = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMessenger",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshGuildWndInfoPage) {
    m_isRefreshGuildWndInfoPage = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshGuildInfoPage", Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshGuildWndBoardPage) {
    m_isRefreshGuildWndBoardPage = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshGuildBoardPage", Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshGuildWndMemberPage) {
    m_isRefreshGuildWndMemberPage = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshGuildMemberPage", Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshGuildWndMemberPageGradeComboBox) {
    m_isRefreshGuildWndMemberPageGradeComboBox = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshGuildMemberPageGradeComboBox",
                          Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshGuildWndSkillPage) {
    m_isRefreshGuildWndSkillPage = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshGuildSkillPage", Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }

  if (m_isRefreshGuildWndGradePage) {
    m_isRefreshGuildWndGradePage = false;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshGuildGradePage", Py_BuildValue("()"));
    s_nextRefreshTime = curTime + 300;
  }
}

void CPythonNetworkStream::__InitializeGamePhase() {
  __ServerTimeSync_Initialize();

  m_isRefreshStatus = false;
  m_isRefreshCharacterWnd = false;
  m_isRefreshEquipmentWnd = false;
  m_isRefreshInventoryWnd = false;
  m_isRefreshExchangeWnd = false;
  m_isRefreshSkillWnd = false;
  m_isRefreshSafeboxWnd = false;
  m_isRefreshMallWnd = false;
  m_isRefreshMessengerWnd = false;
  m_isRefreshGuildWndInfoPage = false;
  m_isRefreshGuildWndBoardPage = false;
  m_isRefreshGuildWndMemberPage = false;
  m_isRefreshGuildWndMemberPageGradeComboBox = false;
  m_isRefreshGuildWndSkillPage = false;
  m_isRefreshGuildWndGradePage = false;

  m_EmoticonStringVector.clear();

  m_pInstTarget = NULL;
}

void CPythonNetworkStream::Warp(LONG lGlobalX, LONG lGlobalY) {
  CPythonBackground &rkBgMgr = CPythonBackground::Instance();
  rkBgMgr.Destroy();
  rkBgMgr.Create();
  rkBgMgr.Warp(lGlobalX, lGlobalY);
  // rkBgMgr.SetShadowLevel(CPythonBackground::SHADOW_ALL);
  rkBgMgr.RefreshShadowLevel();

  // NOTE : Warp 했을때 CenterPosition의 Height가 0이기 때문에 카메라가 땅바닥에
  // 박혀있게 됨
  //        움직일때마다 Height가 갱신 되기 때문이므로 맵을 이동하면 Position을
  //        강제로 한번 셋팅해준다 - [levites]
  int32_t lLocalX = lGlobalX;
  int32_t lLocalY = lGlobalY;
  __GlobalPositionToLocalPosition(lLocalX, lLocalY);
  float fHeight =
      CPythonBackground::Instance().GetHeight(float(lLocalX), float(lLocalY));

  IAbstractApplication &rkApp = IAbstractApplication::GetSingleton();
  rkApp.SetCenterPosition(float(lLocalX), float(lLocalY), fHeight);

  __ShowMapName(lLocalX, lLocalY);
}

void CPythonNetworkStream::__ShowMapName(LONG lLocalX, LONG lLocalY) {
  const std::string &c_rstrMapFileName =
      CPythonBackground::Instance().GetWarpMapName();
  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName",
      Py_BuildValue("(sii)", c_rstrMapFileName.c_str(), lLocalX, lLocalY));
}

void CPythonNetworkStream::__LeaveGamePhase() {
  CInstanceBase::ClearPVPKeySystem();

  __ClearNetworkActorManager();

  m_bComboSkillFlag = FALSE;

  IAbstractCharacterManager &rkChrMgr =
      IAbstractCharacterManager::GetSingleton();
  rkChrMgr.Destroy();

  CPythonItem &rkItemMgr = CPythonItem::Instance();
  rkItemMgr.Destroy();
}

void CPythonNetworkStream::SetGamePhase() {
  if ("Game" != m_strPhase)
    m_phaseLeaveFunc.Run();

  m_strPhase = "Game";

  m_dwChangingPhaseTime = ELTimer_GetMSec();
  m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
  m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.SetMainCharacterIndex(GetMainActorVID());

  __RefreshStatus();
}

bool CPythonNetworkStream::RecvObserverAddPacket() {
  TPacketGCObserverAdd kObserverAddPacket;
  if (!Recv(sizeof(kObserverAddPacket), &kObserverAddPacket))
    return false;

  CPythonMiniMap::Instance().AddObserver(kObserverAddPacket.vid,
                                         kObserverAddPacket.x * 100.0f,
                                         kObserverAddPacket.y * 100.0f);

  return true;
}

bool CPythonNetworkStream::RecvObserverRemovePacket() {
  TPacketGCObserverRemove kObserverRemovePacket;
  if (!Recv(sizeof(kObserverRemovePacket), &kObserverRemovePacket))
    return false;

  CPythonMiniMap::Instance().RemoveObserver(kObserverRemovePacket.vid);

  return true;
}

bool CPythonNetworkStream::RecvObserverMovePacket() {
  TPacketGCObserverMove kObserverMovePacket;
  if (!Recv(sizeof(kObserverMovePacket), &kObserverMovePacket))
    return false;

  CPythonMiniMap::Instance().MoveObserver(kObserverMovePacket.vid,
                                          kObserverMovePacket.x * 100.0f,
                                          kObserverMovePacket.y * 100.0f);

  return true;
}

bool CPythonNetworkStream::RecvWarpPacket() {
  TPacketGCWarp kWarpPacket;

  if (!Recv(sizeof(kWarpPacket), &kWarpPacket))
    return false;

  __DirectEnterMode_Set(m_dwSelectedCharacterIndex);

  CNetworkStream::Connect((DWORD)kWarpPacket.lAddr, kWarpPacket.wPort);

  return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket() {
  TPacketGCDuelStart kDuelStartPacket;
  if (!Recv(sizeof(kDuelStartPacket), &kDuelStartPacket))
    return false;

  DWORD count =
      (kDuelStartPacket.length - sizeof(kDuelStartPacket)) / sizeof(DWORD);

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

  CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
  if (!pkInstMain) {
    TraceError(
        "CPythonNetworkStream::RecvDuelStartPacket - MainCharacter is NULL");
    return false;
  }
  DWORD dwVIDSrc = pkInstMain->GetVirtualID();
  DWORD dwVIDDest;

  for (DWORD i = 0; i < count; i++) {
    Recv(sizeof(dwVIDDest), &dwVIDDest);
    CInstanceBase::InsertDUELKey(dwVIDSrc, dwVIDDest);
  }

  if (count == 0)
    pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
  else
    pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard",
                        Py_BuildValue("()"));

  rkChrMgr.RefreshAllPCTextTail();

  return true;
}

bool CPythonNetworkStream::RecvPVPPacket() {
  TPacketGCPVP kPVPPacket;
  if (!Recv(sizeof(kPVPPacket), &kPVPPacket))
    return false;

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  CPythonPlayer &rkPlayer = CPythonPlayer::Instance();

  switch (kPVPPacket.bMode) {
  case PVP_MODE_AGREE:
    rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

    // 상대가 나(Dst)에게 동의를 구했을때
    if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDDst))
      rkPlayer.RememberChallengeInstance(kPVPPacket.dwVIDSrc);

    // 상대에게 동의를 구한 동안에는 대결 불능
    if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDSrc))
      rkPlayer.RememberCantFightInstance(kPVPPacket.dwVIDDst);
    break;
  case PVP_MODE_REVENGE: {
    rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

    DWORD dwKiller = kPVPPacket.dwVIDSrc;
    DWORD dwVictim = kPVPPacket.dwVIDDst;

    // 내(victim)가 상대에게 복수할 수 있을때
    if (rkPlayer.IsMainCharacterIndex(dwVictim))
      rkPlayer.RememberRevengeInstance(dwKiller);

    // 상대(victim)가 나에게 복수하는 동안에는 대결 불능
    if (rkPlayer.IsMainCharacterIndex(dwKiller))
      rkPlayer.RememberCantFightInstance(dwVictim);
    break;
  }

  case PVP_MODE_FIGHT:
    rkChrMgr.InsertPVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
    rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
    rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
    break;
  case PVP_MODE_NONE:
    rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
    rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
    rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
    break;
  }

  // NOTE : PVP 토글시 TargetBoard 를 업데이트 합니다.
  __RefreshTargetBoardByVID(kPVPPacket.dwVIDSrc);
  __RefreshTargetBoardByVID(kPVPPacket.dwVIDDst);

  return true;
}

// DELETEME
/*
void CPythonNetworkStream::__SendWarpPacket()
{
        TPacketCGWarp kWarpPacket;
        kWarpPacket.header=GC::WARP;
        kWarpPacket.length = sizeof(kWarpPacket);
        if (!Send(sizeof(kWarpPacket), &kWarpPacket))
        {
                return;
        }
}
*/
void CPythonNetworkStream::NotifyHack(const char *c_szMsg) {
  if (!m_kQue_stHack.empty())
    if (c_szMsg == m_kQue_stHack.back())
      return;

  m_kQue_stHack.push_back(c_szMsg);
}

bool CPythonNetworkStream::__SendHack(const char *c_szMsg) {
  Tracen(c_szMsg);

  TPacketCGHack kPacketHack;
  kPacketHack.header = CG::HACK;
  kPacketHack.length = sizeof(kPacketHack);
  strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf) - 1);

  if (!Send(sizeof(kPacketHack), &kPacketHack))
    return false;

  return true;
}

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(DWORD vid) {
  TPacketCGMessenger packet;
  packet.header = CG::MESSENGER;
  packet.length = sizeof(packet) + sizeof(vid);
  packet.subheader = MessengerSub::CG::ADD_BY_VID;
  if (!Send(sizeof(packet), &packet))
    return false;
  if (!Send(sizeof(vid), &vid))
    return false;
  return true;
}

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char *c_szName) {
  TPacketCGMessenger packet;
  packet.header = CG::MESSENGER;
  packet.length = sizeof(packet) + sizeof(char[CHARACTER_NAME_MAX_LEN]);
  packet.subheader = MessengerSub::CG::ADD_BY_NAME;
  if (!Send(sizeof(packet), &packet))
    return false;
  char szName[CHARACTER_NAME_MAX_LEN];
  strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN - 1);
  szName[CHARACTER_NAME_MAX_LEN - 1] =
      '\0'; // #720: 메신저 이름 관련 버퍼 오버플로우 버그 수정

  if (!Send(sizeof(szName), &szName))
    return false;
  Tracef(" SendMessengerAddByNamePacket : %s\n", c_szName);
  return true;
}

bool CPythonNetworkStream::SendMessengerRemovePacket(const char *c_szKey,
                                                     const char *c_szName) {
  TPacketCGMessenger packet;
  packet.header = CG::MESSENGER;
  packet.length = sizeof(packet) + sizeof(char[CHARACTER_NAME_MAX_LEN]);
  packet.subheader = MessengerSub::CG::REMOVE;
  if (!Send(sizeof(packet), &packet))
    return false;
  char szKey[CHARACTER_NAME_MAX_LEN];
  strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN - 1);
  if (!Send(sizeof(szKey), &szKey))
    return false;
  __RefreshTargetBoardByName(c_szName);
  return true;
}

bool CPythonNetworkStream::SendCharacterStatePacket(
    const TPixelPosition &c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg) {
  NANOBEGIN
  if (!__CanActMainInstance())
    return true;

  if (fDstRot < 0.0f)
    fDstRot = 360 + fDstRot;
  else if (fDstRot > 360.0f)
    fDstRot = fmodf(fDstRot, 360.0f);

  // TODO: 나중에 패킷이름을 바꾸자
  TPacketCGMove kStatePacket;
  kStatePacket.header = CG::MOVE;
  kStatePacket.length = sizeof(kStatePacket);
  kStatePacket.bFunc = eFunc;
  kStatePacket.bArg = uArg;
  kStatePacket.bRot = fDstRot / 5.0f;
  kStatePacket.lX = long(c_rkPPosDst.x);
  kStatePacket.lY = long(c_rkPPosDst.y);
  kStatePacket.dwTime = ELTimer_GetServerMSec();

  assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

  __LocalPositionToGlobalPosition(kStatePacket.lX, kStatePacket.lY);

  if (!Send(sizeof(kStatePacket), &kStatePacket)) {
    Tracenf(
        "CPythonNetworkStream::SendCharacterStatePacket(dwCmdTime=%u, "
        "fDstPos=(%f, %f), fDstRot=%f, eFunc=%d uArg=%d) - PACKET SEND ERROR",
        kStatePacket.dwTime, float(kStatePacket.lX), float(kStatePacket.lY),
        fDstRot, kStatePacket.bFunc, kStatePacket.bArg);
    return false;
  }
  NANOEND
  return true;
}

// NOTE : SlotIndex는 임시
bool CPythonNetworkStream::SendUseSkillPacket(DWORD dwSkillIndex,
                                              DWORD dwTargetVID) {
  TPacketCGUseSkill UseSkillPacket;

  UseSkillPacket.header = CG::USE_SKILL;
  UseSkillPacket.length = sizeof(UseSkillPacket);
  UseSkillPacket.dwVnum = dwSkillIndex;
  UseSkillPacket.dwTargetVID = dwTargetVID;

  // tw1x1 fix wrong fly targeting for viewing clients
  if (dwTargetVID) {
    CPythonCharacterManager &rpcm = CPythonCharacterManager::Instance();
    CInstanceBase *pTarget = rpcm.GetInstancePtr(dwTargetVID);

    if (pTarget) {
      TPixelPosition kPos;

      pTarget->NEW_GetPixelPosition(&kPos);
      SendFlyTargetingPacket(dwTargetVID, kPos);
    } else {
      TPixelPosition kPos;

      kPos.x = 0;
      kPos.y = 0;
      kPos.z = 0;

      SendFlyTargetingPacket(0, kPos);
    }
  }
  // END OF tw1x1 fix wrong fly targeting for viewing clients

  if (!Send(sizeof(TPacketCGUseSkill), &UseSkillPacket)) {
    Tracen("CPythonNetworkStream::SendUseSkillPacket - SEND PACKET ERROR");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendChatPacket(const char *c_szChat, BYTE byType) {
  if (strlen(c_szChat) == 0)
    return true;

  if (strlen(c_szChat) >= 512)
    return true;

  if (c_szChat[0] == '/') {
    if (1 == strlen(c_szChat)) {
      if (!m_strLastCommand.empty())
        c_szChat = m_strLastCommand.c_str();
    } else {
      m_strLastCommand = c_szChat;
    }
  }

  if (ClientCommand(c_szChat))
    return true;

  int iTextLen = strlen(c_szChat) + 1;
  TPacketCGChat ChatPacket;
  ChatPacket.header = CG::CHAT;
  ChatPacket.length = sizeof(ChatPacket) + iTextLen;
  ChatPacket.type = byType;

  if (!Send(sizeof(ChatPacket), &ChatPacket))
    return false;

  if (!Send(iTextLen, c_szChat))
    return false;

  return true;
}

//////////////////////////////////////////////////////////////////////////
// Emoticon
void CPythonNetworkStream::RegisterEmoticonString(
    const char *pcEmoticonString) {
  if (m_EmoticonStringVector.size() >= CInstanceBase::EMOTICON_NUM) {
    TraceError("Can't register emoticon string... vector is full (size:%d)",
               m_EmoticonStringVector.size());
    return;
  }
  m_EmoticonStringVector.push_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char *pChatMsg,
                                         DWORD *pdwEmoticon) {
  for (DWORD dwEmoticonIndex = 0;
       dwEmoticonIndex < m_EmoticonStringVector.size(); ++dwEmoticonIndex) {
    if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
      continue;

    const char *pcFind =
        strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

    if (pcFind != pChatMsg)
      continue;

    *pdwEmoticon = dwEmoticonIndex;

    return true;
  }

  return false;
}
// Emoticon
//////////////////////////////////////////////////////////////////////////

void CPythonNetworkStream::__ConvertEmpireText(DWORD dwEmpireID, char *szText) {
  if (dwEmpireID < 1 || dwEmpireID > 3)
    return;

  UINT uHanPos;

  STextConvertTable &rkTextConvTable = m_aTextConvTable[dwEmpireID - 1];

  BYTE *pbText = (BYTE *)szText;
  while (*pbText) {
    if (*pbText & 0x80) {
      if (pbText[0] >= 0xb0 && pbText[0] <= 0xc8 && pbText[1] >= 0xa1 &&
          pbText[1] <= 0xfe) {
        uHanPos = (pbText[0] - 0xb0) * (0xfe - 0xa1 + 1) + (pbText[1] - 0xa1);
        pbText[0] = rkTextConvTable.aacHan[uHanPos][0];
        pbText[1] = rkTextConvTable.aacHan[uHanPos][1];
      }
      pbText += 2;
    } else {
      if (*pbText >= 'a' && *pbText <= 'z') {
        *pbText = rkTextConvTable.acLower[*pbText - 'a'];
      } else if (*pbText >= 'A' && *pbText <= 'Z') {
        *pbText = rkTextConvTable.acUpper[*pbText - 'A'];
      }
      pbText++;
    }
  }
}

void CPythonNetworkStream::__LocalizeItemLinks(char *buf, size_t bufSize) {
  // Replace item names in hyperlinks with client-side localized names
  // Format: |Hitem:VNUM:flags:socket0:socket1:socket2...|h[ItemName]|h

  // Early exit: skip processing if no item links exist
  if (!strstr(buf, "|Hitem:"))
    return;

  char *pSearch = buf;
  char result[2048];
  char *pResult = result;
  char *pResultEnd = result + sizeof(result) - 1;

  while (*pSearch && pResult < pResultEnd) {
    // Look for |Hitem:
    if (strncmp(pSearch, "|Hitem:", 7) == 0) {
      char *pLinkStart = pSearch;
      pSearch += 7;

      // Extract vnum (first hex value after "item:")
      DWORD dwVnum = 0;
      while (*pSearch && *pSearch != ':' && *pSearch != '|') {
        if (*pSearch >= '0' && *pSearch <= '9')
          dwVnum = dwVnum * 16 + (*pSearch - '0');
        else if (*pSearch >= 'a' && *pSearch <= 'f')
          dwVnum = dwVnum * 16 + (*pSearch - 'a' + 10);
        else if (*pSearch >= 'A' && *pSearch <= 'F')
          dwVnum = dwVnum * 16 + (*pSearch - 'A' + 10);
        pSearch++;
      }

      // Find |h[ which marks the start of the item name
      char *pNameStart = strstr(pSearch, "|h[");
      if (pNameStart) {
        pNameStart += 3; // Skip "|h["

        // Find ]|h which marks the end of the item name
        char *pNameEnd = strstr(pNameStart, "]|h");
        if (pNameEnd) {
          // Get the client-side item name
          CItemData *pItemData;
          const char *szLocalName = NULL;
          if (CItemManager::Instance().GetItemDataPointer(dwVnum, &pItemData))
            szLocalName = pItemData->GetName();

          // Copy everything from link start to |h[
          size_t copyLen = pNameStart - pLinkStart;
          if (pResult + copyLen < pResultEnd) {
            memcpy(pResult, pLinkStart, copyLen);
            pResult += copyLen;
          }

          // Insert the localized name (or original if not found)
          const char *szName = szLocalName ? szLocalName : pNameStart;
          size_t nameLen =
              szLocalName ? strlen(szLocalName) : (pNameEnd - pNameStart);
          if (pResult + nameLen < pResultEnd) {
            if (szLocalName) {
              memcpy(pResult, szLocalName, nameLen);
            } else {
              memcpy(pResult, pNameStart, nameLen);
            }
            pResult += nameLen;
          }

          // Copy ]|h
          if (pResult + 3 < pResultEnd) {
            memcpy(pResult, "]|h", 3);
            pResult += 3;
          }

          pSearch = pNameEnd + 3; // Skip past ]|h
          continue;
        }
      }

      // If we couldn't parse properly, just copy the character and continue
      if (pResult < pResultEnd)
        *pResult++ = *pLinkStart;
      pSearch = pLinkStart + 1;
    } else {
      if (pResult < pResultEnd)
        *pResult++ = *pSearch;
      pSearch++;
    }
  }
  *pResult = '\0';

  strncpy(buf, result, bufSize - 1);
  buf[bufSize - 1] = '\0';
}

bool CPythonNetworkStream::RecvChatPacket() {
  TPacketGCChat kChat;
  char buf[1024 + 1];
  char line[1024 + 1];

  if (!Recv(sizeof(kChat), &kChat))
    return false;

  UINT uChatSize = kChat.length - sizeof(kChat);

  if (!Recv(uChatSize, buf))
    return false;

  buf[uChatSize] = '\0';

  // Localize item names in hyperlinks for multi-language support
  __LocalizeItemLinks(buf, sizeof(buf));

  if (kChat.type >= CHAT_TYPE_MAX_NUM)
    return true;

  if (CHAT_TYPE_COMMAND == kChat.type) {
    ServerCommand(buf);
    return true;
  }

  if (kChat.dwVID != 0) {
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase *pkInstChatter = rkChrMgr.GetInstancePtr(kChat.dwVID);
    if (NULL == pkInstChatter)
      return true;

    switch (kChat.type) {
    case CHAT_TYPE_TALKING: /* 그냥 채팅 */
    case CHAT_TYPE_PARTY:   /* 파티말 */
    case CHAT_TYPE_GUILD:   /* 길드말 */
    case CHAT_TYPE_SHOUT:   /* 외치기 */
    case CHAT_TYPE_WHISPER: // 서버와는 연동되지 않는 Only Client Enum
    {
      char *p = strchr(buf, ':');

      if (p)
        p += 2;
      else
        p = buf;

      DWORD dwEmoticon;

      if (ParseEmoticon(p, &dwEmoticon)) {
        pkInstChatter->SetEmoticon(dwEmoticon);
        return true;
      } else {
        if (gs_bEmpireLanuageEnable) {
          CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
          if (pkInstMain)
            if (!pkInstMain->IsSameEmpire(*pkInstChatter))
              __ConvertEmpireText(pkInstChatter->GetEmpireID(), p);
        }

        if (m_isEnableChatInsultFilter) {
          if (false == pkInstChatter->IsNPC() &&
              false == pkInstChatter->IsEnemy()) {
            __FilterInsult(p, strlen(p));
          }
        }

        _snprintf(line, sizeof(line), "%s", p);
      }
    } break;
    case CHAT_TYPE_COMMAND: /* 명령 */
    case CHAT_TYPE_INFO:    /* 정보 (아이템을 집었다, 경험치를 얻었다. 등) */
    case CHAT_TYPE_NOTICE:  /* 공지사항 */
    case CHAT_TYPE_BIG_NOTICE:
    case CHAT_TYPE_MAX_NUM:
    default:
      _snprintf(line, sizeof(line), "%s", buf);
      break;
    }

    if (CHAT_TYPE_SHOUT != kChat.type) {
      CPythonTextTail::Instance().RegisterChatTail(kChat.dwVID, line);
    }

    if (pkInstChatter->IsPC())
      CPythonChat::Instance().AppendChat(kChat.type, buf);
  } else {
    if (CHAT_TYPE_NOTICE == kChat.type) {
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
    } else if (CHAT_TYPE_BIG_NOTICE == kChat.type) {
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
    } else if (CHAT_TYPE_SHOUT == kChat.type) {
      char *p = strchr(buf, ':');

      if (p) {
        if (m_isEnableChatInsultFilter)
          __FilterInsult(p, strlen(p));
      }
    }

    CPythonChat::Instance().AppendChat(kChat.type, buf);
  }
  return true;
}

bool CPythonNetworkStream::RecvWhisperPacket() {
  TPacketGCWhisper whisperPacket;
  char buf[512 + 1];

  if (!Recv(sizeof(whisperPacket), &whisperPacket))
    return false;

  assert(whisperPacket.length - sizeof(whisperPacket) < 512);

  if (!Recv(whisperPacket.length - sizeof(whisperPacket), &buf))
    return false;

  buf[whisperPacket.length - sizeof(whisperPacket)] = '\0';

  static char line[256];
  if (CPythonChat::WHISPER_TYPE_CHAT == whisperPacket.bType ||
      CPythonChat::WHISPER_TYPE_GM == whisperPacket.bType) {
    _snprintf(line, sizeof(line), "%s : %s", whisperPacket.szNameFrom, buf);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper",
                          Py_BuildValue("(iss)", (int)whisperPacket.bType,
                                        whisperPacket.szNameFrom, line));
  } else if (CPythonChat::WHISPER_TYPE_SYSTEM == whisperPacket.bType ||
             CPythonChat::WHISPER_TYPE_ERROR == whisperPacket.bType) {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "OnRecvWhisperSystemMessage",
                          Py_BuildValue("(iss)", (int)whisperPacket.bType,
                                        whisperPacket.szNameFrom, buf));
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "OnRecvWhisperError",
                          Py_BuildValue("(iss)", (int)whisperPacket.bType,
                                        whisperPacket.szNameFrom, buf));
  }

  return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char *name,
                                             const char *c_szChat) {
  if (strlen(c_szChat) >= 255)
    return true;

  int iTextLen = strlen(c_szChat) + 1;
  TPacketCGWhisper WhisperPacket;
  WhisperPacket.header = CG::WHISPER;
  WhisperPacket.length = sizeof(WhisperPacket) + iTextLen;

  strncpy(WhisperPacket.szNameTo, name, sizeof(WhisperPacket.szNameTo) - 1);

  if (!Send(sizeof(WhisperPacket), &WhisperPacket))
    return false;

  if (!Send(iTextLen, c_szChat))
    return false;

  return true;
}

bool CPythonNetworkStream::RecvPointChange() {
  TPacketGCPointChange PointChange;

  if (!Recv(sizeof(TPacketGCPointChange), &PointChange)) {
    Tracen("Recv Point Change Packet Error");
    return false;
  }

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  rkChrMgr.ShowPointEffect(PointChange.Type, PointChange.dwVID);

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetMainInstancePtr();

  // 자신의 Point가 변경되었을 경우..
  if (pInstance)
    if (PointChange.dwVID == pInstance->GetVirtualID()) {
      CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
      rkPlayer.SetStatus(PointChange.Type, PointChange.value);

      switch (PointChange.Type) {
      case POINT_STAT_RESET_COUNT:
        __RefreshStatus();
        break;
      case POINT_PLAYTIME:
        m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].dwPlayMinutes =
            PointChange.value;
        break;
      case POINT_LEVEL:
        m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].byLevel =
            PointChange.value;
        __RefreshStatus();
        __RefreshSkillWindow();
        break;
      case POINT_ST:
      case POINT_DX:
      case POINT_HT:
      case POINT_IQ:
        __RefreshStatus();
        __RefreshSkillWindow();
        break;
      case POINT_SKILL:
      case POINT_SUB_SKILL:
      case POINT_HORSE_SKILL:
        __RefreshSkillWindow();
        break;
      case POINT_ENERGY:
        if (PointChange.value == 0) {
          rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
        }
        __RefreshStatus();
        break;
      default:
        __RefreshStatus();
        break;
      }

      if (POINT_GOLD == PointChange.Type) {
        if (PointChange.amount > 0) {
          PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney",
                                Py_BuildValue("(i)", PointChange.amount));
        }
      }
    } else {
      if (PointChange.Type == POINT_LEVEL) {
        pInstance = CPythonCharacterManager::Instance().GetInstancePtr(
            PointChange.dwVID);
        if (pInstance) {
          pInstance->SetLevel(PointChange.value);
          pInstance->UpdateTextTailLevel(PointChange.value);
        }
      }
    }

  return true;
}

bool CPythonNetworkStream::RecvStunPacket() {
  TPacketGCStun StunPacket;

  if (!Recv(sizeof(StunPacket), &StunPacket)) {
    Tracen("CPythonNetworkStream::RecvStunPacket Error");
    return false;
  }

  // Tracef("RecvStunPacket %d\n", StunPacket.vid);

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  CInstanceBase *pkInstSel = rkChrMgr.GetInstancePtr(StunPacket.vid);

  if (pkInstSel) {
    if (CPythonCharacterManager::Instance().GetMainInstancePtr() == pkInstSel)
      pkInstSel->Die();
    else
      pkInstSel->Stun();
  }

  return true;
}

bool CPythonNetworkStream::RecvDeadPacket() {
  TPacketGCDead DeadPacket;
  if (!Recv(sizeof(DeadPacket), &DeadPacket)) {
    Tracen("CPythonNetworkStream::RecvDeadPacket Error");
    return false;
  }

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  CInstanceBase *pkChrInstSel = rkChrMgr.GetInstancePtr(DeadPacket.vid);
  if (pkChrInstSel) {
    CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (pkInstMain == pkChrInstSel) {
      Tracenf("주인공 사망");
      if (false == pkInstMain->GetDuelMode()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver",
                              Py_BuildValue("()"));
      }
      CPythonPlayer::Instance().NotifyDeadMainCharacter();
    }

    pkChrInstSel->Die();
  }

  return true;
}

bool CPythonNetworkStream::SendCharacterPositionPacket(BYTE iPosition) {
  TPacketCGPosition PositionPacket;

  PositionPacket.header = CG::CHARACTER_POSITION;
  PositionPacket.length = sizeof(PositionPacket);
  PositionPacket.position = iPosition;

  if (!Send(sizeof(TPacketCGPosition), &PositionPacket)) {
    Tracen("Send Character Position Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendOnClickPacket(DWORD vid) {
  TPacketCGOnClick OnClickPacket;
  OnClickPacket.header = CG::ON_CLICK;
  OnClickPacket.length = sizeof(OnClickPacket);
  OnClickPacket.vid = vid;

  if (!Send(sizeof(OnClickPacket), &OnClickPacket)) {
    Tracen("Send On_Click Packet Error");
    return false;
  }

  Tracef("SendOnClickPacket\n");
  return true;
}

bool CPythonNetworkStream::RecvCharacterPositionPacket() {
  TPacketGCPosition PositionPacket;

  if (!Recv(sizeof(TPacketGCPosition), &PositionPacket))
    return false;

  CInstanceBase *pChrInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(PositionPacket.vid);

  if (!pChrInstance)
    return true;

  // pChrInstance->UpdatePosition(PositionPacket.position);

  return true;
}

bool CPythonNetworkStream::RecvMotionPacket() {
  TPacketGCMotion MotionPacket;

  if (!Recv(sizeof(TPacketGCMotion), &MotionPacket))
    return false;

  CInstanceBase *pMainInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.vid);
  CInstanceBase *pVictimInstance = NULL;

  if (0 != MotionPacket.victim_vid)
    pVictimInstance = CPythonCharacterManager::Instance().GetInstancePtr(
        MotionPacket.victim_vid);

  if (!pMainInstance)
    return false;

  return true;
}

bool CPythonNetworkStream::RecvShopPacket() {
  TPacketGCShop packet_shop;
  if (!Recv(sizeof(packet_shop), &packet_shop))
    return false;

  std::vector<char> buf;
  int iSize = packet_shop.length - sizeof(packet_shop);
  if (iSize > 0) {
    buf.resize(iSize);
    if (!Recv(iSize, &buf[0]))
      return false;
  }

  static const std::unordered_map<uint8_t, bool (CPythonNetworkStream::*)(
                                               const std::vector<char> &)>
      handlers = {
          {ShopSub::GC::START, &CPythonNetworkStream::RecvShopSub_Start},
          {ShopSub::GC::START_EX, &CPythonNetworkStream::RecvShopSub_StartEx},
          {ShopSub::GC::END, &CPythonNetworkStream::RecvShopSub_End},
          {ShopSub::GC::UPDATE_ITEM,
           &CPythonNetworkStream::RecvShopSub_UpdateItem},
          {ShopSub::GC::UPDATE_PRICE,
           &CPythonNetworkStream::RecvShopSub_UpdatePrice},
          {ShopSub::GC::NOT_ENOUGH_MONEY,
           &CPythonNetworkStream::RecvShopSub_NotEnoughMoney},
          {ShopSub::GC::NOT_ENOUGH_MONEY_EX,
           &CPythonNetworkStream::RecvShopSub_NotEnoughMoneyEx},
          {ShopSub::GC::SOLDOUT, &CPythonNetworkStream::RecvShopSub_Soldout},
          {ShopSub::GC::INVENTORY_FULL,
           &CPythonNetworkStream::RecvShopSub_InventoryFull},
          {ShopSub::GC::INVALID_POS,
           &CPythonNetworkStream::RecvShopSub_InvalidPos},
      };

  auto it = handlers.find(packet_shop.subheader);
  if (it == handlers.end()) {
    TraceError("RecvShopPacket: unknown subheader %d", packet_shop.subheader);
    return true;
  }
  return (this->*(it->second))(buf);
}

bool CPythonNetworkStream::RecvShopSub_Start(const std::vector<char> &buf) {
  CPythonShop::Instance().Clear();

  DWORD dwVID = *(DWORD *)&buf[0];

  TPacketGCShopStart *pShopStartPacket = (TPacketGCShopStart *)&buf[4];
  for (BYTE iItemIndex = 0; iItemIndex < SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex) {
    CPythonShop::Instance().SetItemData(iItemIndex,
                                        pShopStartPacket->items[iItemIndex]);
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop",
                        Py_BuildValue("(i)", dwVID));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_StartEx(const std::vector<char> &buf) {
  CPythonShop::Instance().Clear();

  TPacketGCShopStartEx *pShopStartPacket = (TPacketGCShopStartEx *)&buf[0];
  size_t read_point = sizeof(TPacketGCShopStartEx);

  DWORD dwVID = pShopStartPacket->owner_vid;
  BYTE shop_tab_count = pShopStartPacket->shop_tab_count;

  CPythonShop::instance().SetTabCount(shop_tab_count);

  for (unsigned char i = 0; i < shop_tab_count; i++) {
    TPacketGCShopStartEx::TSubPacketShopTab *pPackTab =
        (TPacketGCShopStartEx::TSubPacketShopTab *)&buf[read_point];
    read_point += sizeof(TPacketGCShopStartEx::TSubPacketShopTab);

    CPythonShop::instance().SetTabCoinType(i, pPackTab->coin_type);
    CPythonShop::instance().SetTabName(i, pPackTab->name);

    struct packet_shop_item *item = &pPackTab->items[0];

    for (BYTE j = 0; j < SHOP_HOST_ITEM_MAX_NUM; j++) {
      TShopItemData *itemData = (item + j);
      CPythonShop::Instance().SetItemData(i, j, *itemData);
    }
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop",
                        Py_BuildValue("(i)", dwVID));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_End(const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_UpdateItem(
    const std::vector<char> &buf) {
  TPacketGCShopUpdateItem *pShopUpdateItemPacket =
      (TPacketGCShopUpdateItem *)&buf[0];
  CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos,
                                      pShopUpdateItemPacket->item);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_UpdatePrice(
    const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice",
                        Py_BuildValue("(i)", *(int *)&buf[0]));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_NotEnoughMoney(
    const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError",
                        Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_NotEnoughMoneyEx(
    const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError",
                        Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_Soldout(const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError",
                        Py_BuildValue("(s)", "SOLDOUT"));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_InventoryFull(
    const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError",
                        Py_BuildValue("(s)", "INVENTORY_FULL"));
  return true;
}

bool CPythonNetworkStream::RecvShopSub_InvalidPos(
    const std::vector<char> &buf) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError",
                        Py_BuildValue("(s)", "INVALID_POS"));
  return true;
}

bool CPythonNetworkStream::RecvExchangePacket() {
  TPacketGCExchange exchange_packet;

  if (!Recv(sizeof(exchange_packet), &exchange_packet))
    return false;

  static const std::unordered_map<uint8_t, bool (CPythonNetworkStream::*)(
                                               const TPacketGCExchange &)>
      handlers = {
          {ExchangeSub::GC::START,
           &CPythonNetworkStream::RecvExchangeSub_Start},
          {ExchangeSub::GC::ITEM_ADD,
           &CPythonNetworkStream::RecvExchangeSub_ItemAdd},
          {ExchangeSub::GC::ITEM_DEL,
           &CPythonNetworkStream::RecvExchangeSub_ItemDel},
          {ExchangeSub::GC::ELK_ADD,
           &CPythonNetworkStream::RecvExchangeSub_ElkAdd},
          {ExchangeSub::GC::ACCEPT,
           &CPythonNetworkStream::RecvExchangeSub_Accept},
          {ExchangeSub::GC::END, &CPythonNetworkStream::RecvExchangeSub_End},
          {ExchangeSub::GC::ALREADY,
           &CPythonNetworkStream::RecvExchangeSub_Already},
          {ExchangeSub::GC::LESS_ELK,
           &CPythonNetworkStream::RecvExchangeSub_LessElk},
      };

  auto it = handlers.find(exchange_packet.subheader);
  if (it == handlers.end()) {
    TraceError("RecvExchangePacket: unknown subheader %d",
               exchange_packet.subheader);
    return true;
  }
  return (this->*(it->second))(exchange_packet);
}

bool CPythonNetworkStream::RecvExchangeSub_Start(
    const TPacketGCExchange &pack) {
  CPythonExchange::Instance().Clear();
  CPythonExchange::Instance().Start();
  CPythonExchange::Instance().SetSelfName(CPythonPlayer::Instance().GetName());

  {
    CInstanceBase *pCharacterInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(pack.arg1);

    if (pCharacterInstance)
      CPythonExchange::Instance().SetTargetName(
          pCharacterInstance->GetNameString());
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExchange",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_ItemAdd(
    const TPacketGCExchange &pack) {
  if (pack.is_me) {
    int iSlotIndex = pack.arg2.cell;
    CPythonExchange::Instance().SetItemToSelf(iSlotIndex, pack.arg1,
                                              (BYTE)pack.arg3);
    for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
      CPythonExchange::Instance().SetItemMetinSocketToSelf(iSlotIndex, i,
                                                           pack.alValues[i]);
    for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
      CPythonExchange::Instance().SetItemAttributeToSelf(
          iSlotIndex, j, pack.aAttr[j].bType, pack.aAttr[j].sValue);
  } else {
    int iSlotIndex = pack.arg2.cell;
    CPythonExchange::Instance().SetItemToTarget(iSlotIndex, pack.arg1,
                                                (BYTE)pack.arg3);
    for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
      CPythonExchange::Instance().SetItemMetinSocketToTarget(iSlotIndex, i,
                                                             pack.alValues[i]);
    for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
      CPythonExchange::Instance().SetItemAttributeToTarget(
          iSlotIndex, j, pack.aAttr[j].bType, pack.aAttr[j].sValue);
  }

  __RefreshExchangeWindow();
  __RefreshInventoryWindow();
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_ItemDel(
    const TPacketGCExchange &pack) {
  if (pack.is_me) {
    CPythonExchange::Instance().DelItemOfSelf((BYTE)pack.arg1);
  } else {
    CPythonExchange::Instance().DelItemOfTarget((BYTE)pack.arg1);
  }
  __RefreshExchangeWindow();
  __RefreshInventoryWindow();
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_ElkAdd(
    const TPacketGCExchange &pack) {
  if (pack.is_me)
    CPythonExchange::Instance().SetElkToSelf(pack.arg1);
  else
    CPythonExchange::Instance().SetElkToTarget(pack.arg1);

  __RefreshExchangeWindow();
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_Accept(
    const TPacketGCExchange &pack) {
  if (pack.is_me) {
    CPythonExchange::Instance().SetAcceptToSelf((BYTE)pack.arg1);
  } else {
    CPythonExchange::Instance().SetAcceptToTarget((BYTE)pack.arg1);
  }
  __RefreshExchangeWindow();
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_End(const TPacketGCExchange &pack) {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchange",
                        Py_BuildValue("()"));
  __RefreshInventoryWindow();
  CPythonExchange::Instance().End();
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_Already(
    const TPacketGCExchange &pack) {
  Tracef("trade_already");
  return true;
}

bool CPythonNetworkStream::RecvExchangeSub_LessElk(
    const TPacketGCExchange &pack) {
  Tracef("trade_less_elk");
  return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket() {
  TPacketGCQuestInfo QuestInfo;

  if (!Peek(sizeof(TPacketGCQuestInfo), &QuestInfo)) {
    Tracen("Recv Quest Info Packet Error #1");
    return false;
  }

  if (!Peek(QuestInfo.length)) {
    Tracen("Recv Quest Info Packet Error #2");
    return false;
  }

  Recv(sizeof(TPacketGCQuestInfo));

  const BYTE &c_rFlag = QuestInfo.flag;

  enum {
    QUEST_PACKET_TYPE_NONE,
    QUEST_PACKET_TYPE_BEGIN,
    QUEST_PACKET_TYPE_UPDATE,
    QUEST_PACKET_TYPE_END,
  };

  BYTE byQuestPacketType = QUEST_PACKET_TYPE_NONE;

  if (0 != (c_rFlag & QUEST_SEND_IS_BEGIN)) {
    BYTE isBegin;
    if (!Recv(sizeof(isBegin), &isBegin))
      return false;

    if (isBegin)
      byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
    else
      byQuestPacketType = QUEST_PACKET_TYPE_END;
  } else {
    byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;
  }

  // Recv Data Start
  char szTitle[30 + 1] = "";
  char szClockName[16 + 1] = "";
  int iClockValue = 0;
  char szCounterName[16 + 1] = "";
  int iCounterValue = 0;
  char szIconFileName[24 + 1] = "";

  if (0 != (c_rFlag & QUEST_SEND_TITLE)) {
    if (!Recv(sizeof(szTitle), &szTitle))
      return false;

    szTitle[30] = '\0';
  }
  if (0 != (c_rFlag & QUEST_SEND_CLOCK_NAME)) {
    if (!Recv(sizeof(szClockName), &szClockName))
      return false;

    szClockName[16] = '\0';
  }
  if (0 != (c_rFlag & QUEST_SEND_CLOCK_VALUE)) {
    if (!Recv(sizeof(iClockValue), &iClockValue))
      return false;
  }
  if (0 != (c_rFlag & QUEST_SEND_COUNTER_NAME)) {
    if (!Recv(sizeof(szCounterName), &szCounterName))
      return false;

    szCounterName[16] = '\0';
  }
  if (0 != (c_rFlag & QUEST_SEND_COUNTER_VALUE)) {
    if (!Recv(sizeof(iCounterValue), &iCounterValue))
      return false;
  }
  if (0 != (c_rFlag & QUEST_SEND_ICON_FILE)) {
    if (!Recv(sizeof(szIconFileName), &szIconFileName))
      return false;

    szIconFileName[24] = '\0';
  }
  // Recv Data End

  CPythonQuest &rkQuest = CPythonQuest::Instance();

  // Process Start
  if (QUEST_PACKET_TYPE_END == byQuestPacketType) {
    rkQuest.DeleteQuestInstance(QuestInfo.index);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ClearQuest",
                          Py_BuildValue("(i)", QuestInfo.index));
  } else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType) {
    if (!rkQuest.IsQuest(QuestInfo.index)) {
      rkQuest.MakeQuest(QuestInfo.index);
    }

    if (strlen(szTitle) > 0)
      rkQuest.SetQuestTitle(QuestInfo.index, szTitle);
    if (strlen(szClockName) > 0)
      rkQuest.SetQuestClockName(QuestInfo.index, szClockName);
    if (strlen(szCounterName) > 0)
      rkQuest.SetQuestCounterName(QuestInfo.index, szCounterName);
    if (strlen(szIconFileName) > 0)
      rkQuest.SetQuestIconFileName(QuestInfo.index, szIconFileName);

    if (c_rFlag & QUEST_SEND_CLOCK_VALUE)
      rkQuest.SetQuestClockValue(QuestInfo.index, iClockValue);
    if (c_rFlag & QUEST_SEND_COUNTER_VALUE)
      rkQuest.SetQuestCounterValue(QuestInfo.index, iCounterValue);
  } else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType) {
    CPythonQuest::SQuestInstance QuestInstance;
    QuestInstance.dwIndex = QuestInfo.index;
    QuestInstance.strTitle = szTitle;
    QuestInstance.strClockName = szClockName;
    QuestInstance.iClockValue = iClockValue;
    QuestInstance.strCounterName = szCounterName;
    QuestInstance.iCounterValue = iCounterValue;
    QuestInstance.strIconFileName = szIconFileName;
    CPythonQuest::Instance().RegisterQuestInstance(QuestInstance);
  }
  // Process Start End

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::RecvQuestConfirmPacket() {
  TPacketGCQuestConfirm kQuestConfirmPacket;
  if (!Recv(sizeof(kQuestConfirmPacket), &kQuestConfirmPacket)) {
    Tracen("RecvQuestConfirmPacket Error");
    return false;
  }

  PyObject *poArg = Py_BuildValue("(sii)", kQuestConfirmPacket.msg,
                                  kQuestConfirmPacket.timeout,
                                  kQuestConfirmPacket.requestPID);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "BINARY_OnQuestConfirm", poArg);
  return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild() {
  TPacketGCBlank blank;
  if (!Recv(sizeof(blank), &blank)) {
    Tracen("RecvRequestMakeGuild Packet Error");
    return false;
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName",
                        Py_BuildValue("()"));

  return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo() {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo",
                        Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendExchangeStartPacket(DWORD vid) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGExchange packet;

  packet.header = CG::EXCHANGE;
  packet.length = sizeof(packet);
  packet.subheader = ExchangeSub::CG::START;
  packet.arg1 = vid;

  if (!Send(sizeof(packet), &packet)) {
    Tracef("send_trade_start_packet Error\n");
    return false;
  }

  Tracef("send_trade_start_packet   vid %d \n", vid);
  return true;
}

bool CPythonNetworkStream::SendExchangeElkAddPacket(DWORD elk) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGExchange packet;

  packet.header = CG::EXCHANGE;
  packet.length = sizeof(packet);
  packet.subheader = ExchangeSub::CG::ELK_ADD;
  packet.arg1 = elk;

  if (!Send(sizeof(packet), &packet)) {
    Tracef("send_trade_elk_add_packet Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos,
                                                     BYTE byDisplayPos) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGExchange packet;

  packet.header = CG::EXCHANGE;
  packet.length = sizeof(packet);
  packet.subheader = ExchangeSub::CG::ITEM_ADD;
  packet.Pos = ItemPos;
  packet.arg2 = byDisplayPos;

  if (!Send(sizeof(packet), &packet)) {
    Tracef("send_trade_item_add_packet Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(BYTE pos) {
  assert(!"Can't be called function - "
          "CPythonNetworkStream::SendExchangeItemDelPacket");
  return true;

  if (!__CanActMainInstance())
    return true;

  TPacketCGExchange packet;

  packet.header = CG::EXCHANGE;
  packet.length = sizeof(packet);
  packet.subheader = ExchangeSub::CG::ITEM_DEL;
  packet.arg1 = pos;

  if (!Send(sizeof(packet), &packet)) {
    Tracef("send_trade_item_del_packet Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendExchangeAcceptPacket() {
  if (!__CanActMainInstance())
    return true;

  TPacketCGExchange packet;

  packet.header = CG::EXCHANGE;
  packet.length = sizeof(packet);
  packet.subheader = ExchangeSub::CG::ACCEPT;

  if (!Send(sizeof(packet), &packet)) {
    Tracef("send_trade_accept_packet Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendExchangeExitPacket() {
  if (!__CanActMainInstance())
    return true;

  TPacketCGExchange packet;

  packet.header = CG::EXCHANGE;
  packet.length = sizeof(packet);
  packet.subheader = ExchangeSub::CG::CANCEL;

  if (!Send(sizeof(packet), &packet)) {
    Tracef("send_trade_exit_packet Error\n");
    return false;
  }

  return true;
}

// PointReset 개임시
bool CPythonNetworkStream::SendPointResetPacket() {
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset",
                        Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking() {
  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
  if (!pkInstMain)
    return false;

  if (!pkInstMain->IsAttacking())
    return false;

  return true;
}

bool CPythonNetworkStream::RecvScriptPacket() {
  TPacketGCScript ScriptPacket;

  if (!Recv(sizeof(TPacketGCScript), &ScriptPacket)) {
    TraceError("RecvScriptPacket_RecvError");
    return false;
  }

  if (ScriptPacket.length < sizeof(TPacketGCScript)) {
    TraceError("RecvScriptPacket_SizeError");
    return false;
  }

  ScriptPacket.length -= sizeof(TPacketGCScript);

  static std::string str;
  str = "";
  str.resize(ScriptPacket.length + 1);

  if (!Recv(ScriptPacket.length, &str[0]))
    return false;

  str[str.size() - 1] = '\0';

  int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(str);

  if (-1 != iIndex) {
    CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);
    CPythonNetworkStream::Instance().OnScriptEventStart(ScriptPacket.skin,
                                                        iIndex);
  }

  return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer) {
  TPacketCGScriptAnswer ScriptAnswer;

  ScriptAnswer.header = CG::SCRIPT_ANSWER;
  ScriptAnswer.length = sizeof(ScriptAnswer);
  ScriptAnswer.answer = (BYTE)iAnswer;
  if (!Send(sizeof(TPacketCGScriptAnswer), &ScriptAnswer)) {
    Tracen("Send Script Answer Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex) {
  TPacketCGScriptButton ScriptButton;

  ScriptButton.header = CG::SCRIPT_BUTTON;
  ScriptButton.length = sizeof(ScriptButton);
  ScriptButton.idx = iIndex;
  if (!Send(sizeof(TPacketCGScriptButton), &ScriptButton)) {
    Tracen("Send Script Button Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char *c_szName) {
  TPacketCGAnswerMakeGuild Packet;

  Packet.header = CG::ANSWER_MAKE_GUILD;
  Packet.length = sizeof(Packet);
  strncpy(Packet.guild_name, c_szName, GUILD_NAME_MAX_LEN);
  Packet.guild_name[GUILD_NAME_MAX_LEN] = '\0';

  if (!Send(sizeof(Packet), &Packet)) {
    Tracen("SendAnswerMakeGuild Packet Error");
    return false;
  }

  // 	Tracef(" SendAnswerMakeGuildPacket : %s", c_szName);
  return true;
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char *c_szString) {
  TPacketCGQuestInputString Packet;
  Packet.header = CG::QUEST_INPUT_STRING;
  Packet.length = sizeof(Packet);
  strncpy(Packet.szString, c_szString, QUEST_INPUT_STRING_MAX_NUM);

  if (!Send(sizeof(Packet), &Packet)) {
    Tracen("SendQuestInputStringPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID) {
  TPacketCGQuestConfirm kPacket;
  kPacket.header = CG::QUEST_CONFIRM;
  kPacket.length = sizeof(kPacket);
  kPacket.answer = byAnswer;
  kPacket.requestPID = dwPID;

  if (!Send(sizeof(kPacket), &kPacket)) {
    Tracen("SendQuestConfirmPacket Error");
    return false;
  }

  Tracenf(" SendQuestConfirmPacket : %d, %d", byAnswer, dwPID);
  return true;
}

bool CPythonNetworkStream::SendQuestCancelPacket() {
  TPacketCGQuestCancel Packet;
  Packet.header = CG::QUEST_CANCEL;
  Packet.length = sizeof(Packet);

  if (!Send(sizeof(Packet), &Packet)) {
    Tracen("SendQuestCancelPacket Error");
    return false;
  }

  Tracenf(" SendQuestCancelPacket");
  return true;
}

bool CPythonNetworkStream::RecvSkillCoolTimeEnd() {
  TPacketGCSkillCoolTimeEnd kPacketSkillCoolTimeEnd;
  if (!Recv(sizeof(kPacketSkillCoolTimeEnd), &kPacketSkillCoolTimeEnd)) {
    Tracen("CPythonNetworkStream::RecvSkillCoolTimeEnd - RecvError");
    return false;
  }

  CPythonPlayer::Instance().EndSkillCoolTime(kPacketSkillCoolTimeEnd.bSkill);

  return true;
}

bool CPythonNetworkStream::RecvSkillLevel() {
  assert(!"CPythonNetworkStream::RecvSkillLevel - 사용하지 않는 함수");

  TPacketGCSkillLevel packet;

  if (!Recv(sizeof(TPacketGCSkillLevel), &packet)) {
    Tracen("CPythonNetworkStream::RecvSkillLevel - RecvError");

    return false;
  }

  DWORD dwSlotIndex;
  CPythonPlayer &rkPlayer = CPythonPlayer::Instance();

  for (int i = 0; i < SKILL_MAX_NUM; ++i) {
    if (rkPlayer.GetSkillSlotIndex(i, &dwSlotIndex))
      rkPlayer.SetSkillLevel(dwSlotIndex, packet.abSkillLevels[i]);
  }

  __RefreshSkillWindow();
  __RefreshStatus();

  Tracef(" >> RecvSkillLevel\n");

  return true;
}

bool CPythonNetworkStream::RecvSkillLevelNew() {
  TPacketGCSkillLevelNew packet;

  if (!Recv(sizeof(TPacketGCSkillLevelNew), &packet)) {
    Tracen("CPythonNetworkStream::RecvSkillLevelNew - RecvError");
    return false;
  }

  CPythonPlayer &rkPlayer = CPythonPlayer::Instance();

  rkPlayer.SetSkill(7, 0);
  rkPlayer.SetSkill(8, 0);

  for (int i = 0; i < SKILL_MAX_NUM; ++i) {
    TPlayerSkill &rPlayerSkill = packet.skills[i];

    if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
      rkPlayer.SetSkill(7, i);

    if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
      rkPlayer.SetSkill(8, i);

    rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
  }

  __RefreshSkillWindow();
  __RefreshStatus();
  // Tracef(" >> RecvSkillLevelNew\n");
  return true;
}

bool CPythonNetworkStream::RecvDamageInfoPacket() {
  TPacketGCDamageInfo DamageInfoPacket;

  if (!Recv(sizeof(TPacketGCDamageInfo), &DamageInfoPacket)) {
    Tracen("Recv Target Packet Error");
    return false;
  }

  CInstanceBase *pInstTarget =
      CPythonCharacterManager::Instance().GetInstancePtr(
          DamageInfoPacket.dwVID);
  bool bSelf =
      (pInstTarget == CPythonCharacterManager::Instance().GetMainInstancePtr());
  bool bTarget = (pInstTarget == m_pInstTarget);
  if (pInstTarget) {
    if (DamageInfoPacket.damage >= 0)
      pInstTarget->AddDamageEffect(DamageInfoPacket.damage,
                                   DamageInfoPacket.flag, bSelf, bTarget);
  }

  return true;
}

bool CPythonNetworkStream::RecvTargetPacket() {
  TPacketGCTarget TargetPacket;

  if (!Recv(sizeof(TPacketGCTarget), &TargetPacket)) {
    Tracen("Recv Target Packet Error");
    return false;
  }

  CInstanceBase *pInstPlayer =
      CPythonCharacterManager::Instance().GetMainInstancePtr();
  CInstanceBase *pInstTarget =
      CPythonCharacterManager::Instance().GetInstancePtr(TargetPacket.dwVID);
  if (pInstPlayer && pInstTarget) {
    if (!pInstTarget->IsDead()) {
      if (pInstTarget->IsPC() || pInstTarget->IsBuilding())
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "CloseTargetBoardIfDifferent",
                              Py_BuildValue("(i)", TargetPacket.dwVID));
      else if (pInstPlayer->CanViewTargetHP(*pInstTarget))
        PyCallClassMemberFunc(
            m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard",
            Py_BuildValue("(ii)", TargetPacket.dwVID, TargetPacket.bHPPercent));
      else
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "CloseTargetBoard", Py_BuildValue("()"));

      m_pInstTarget = pInstTarget;
    }
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard",
                          Py_BuildValue("()"));
  }

  return true;
}

bool CPythonNetworkStream::RecvMountPacket() {
  TPacketGCMount MountPacket;

  if (!Recv(sizeof(TPacketGCMount), &MountPacket)) {
    Tracen("Recv Mount Packet Error");
    return false;
  }

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(MountPacket.vid);

  if (pInstance) {
    // Mount
    if (0 != MountPacket.mount_vid) {
      //			pInstance->Ride(MountPacket.pos,
      // MountPacket.mount_vid);
    }
    // Unmount
    else {
      //			pInstance->Unride(MountPacket.pos,
      // MountPacket.x, MountPacket.y);
    }
  }

  if (CPythonPlayer::Instance().IsMainCharacterIndex(MountPacket.vid)) {
    //		CPythonPlayer::Instance().SetRidingVehicleIndex(MountPacket.mount_vid);
  }

  return true;
}

bool CPythonNetworkStream::RecvChangeSpeedPacket() {
  TPacketGCChangeSpeed SpeedPacket;

  if (!Recv(sizeof(TPacketGCChangeSpeed), &SpeedPacket)) {
    Tracen("Recv Speed Packet Error");
    return false;
  }

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(SpeedPacket.vid);

  if (!pInstance)
    return true;

  //	pInstance->SetWalkSpeed(SpeedPacket.walking_speed);
  //	pInstance->SetRunSpeed(SpeedPacket.running_speed);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Recv

bool CPythonNetworkStream::SendAttackPacket(UINT uMotAttack,
                                            DWORD dwVIDVictim) {
  if (!__CanActMainInstance())
    return true;

#ifdef ATTACK_TIME_LOG
  static DWORD prevTime = timeGetTime();
  DWORD curTime = timeGetTime();
  TraceError("TIME: %.4f(%.4f) ATTACK_PACKET: %d TARGET: %d", curTime / 1000.0f,
             (curTime - prevTime) / 1000.0f, uMotAttack, dwVIDVictim);
  prevTime = curTime;
#endif

  TPacketCGAttack kPacketAtk;

  kPacketAtk.header = CG::ATTACK;
  kPacketAtk.length = sizeof(kPacketAtk);
  kPacketAtk.bType = uMotAttack;
  kPacketAtk.dwVictimVID = dwVIDVictim;

  if (!SendSpecial(sizeof(kPacketAtk), &kPacketAtk)) {
    Tracen("Send Battle Attack Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendSpecial(int nLen, void *pvBuf) {
  uint16_t wHeader = *(uint16_t *)pvBuf;

  switch (wHeader) {
  case CG::ATTACK: {
    TPacketCGAttack *pkPacketAtk = (TPacketCGAttack *)pvBuf;
    pkPacketAtk->bCRCMagicCubeProcPiece = GetProcessCRCMagicCubePiece();
    pkPacketAtk->bCRCMagicCubeFilePiece = GetProcessCRCMagicCubePiece();
    return Send(nLen, pvBuf);
  } break;
  }

  return Send(nLen, pvBuf);
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket() {
  TPacketGCFlyTargeting kPacket;
  if (!Recv(sizeof(kPacket), &kPacket))
    return false;

  __GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

  Tracef("VID [%d]가 타겟을 추가 설정\n", kPacket.dwShooterVID);

  CPythonCharacterManager &rpcm = CPythonCharacterManager::Instance();

  CInstanceBase *pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

  if (!pShooter) {
#ifndef _DEBUG
    TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - "
               "dwShooterVID[%d] NOT EXIST",
               kPacket.dwShooterVID);
#endif
    return true;
  }

  CInstanceBase *pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

  if (kPacket.dwTargetVID && pTarget) {
    pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(
        pTarget->GetGraphicThingInstancePtr());
  } else {
    float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) +
              60.0f; // TEMPORARY HEIGHT
    pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(
        D3DXVECTOR3(kPacket.lX, kPacket.lY, h));
    // pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
  }

  return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket() {
  TPacketGCFlyTargeting kPacket;
  if (!Recv(sizeof(kPacket), &kPacket))
    return false;

  __GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

  // Tracef("CPythonNetworkStream::RecvFlyTargetingPacket - VID
  // [%d]\n",kPacket.dwShooterVID);

  CPythonCharacterManager &rpcm = CPythonCharacterManager::Instance();

  CInstanceBase *pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

  if (!pShooter) {
#ifdef _DEBUG
    TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - "
               "dwShooterVID[%d] NOT EXIST",
               kPacket.dwShooterVID);
#endif
    return true;
  }

  CInstanceBase *pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

  if (kPacket.dwTargetVID && pTarget) {
    pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(
        pTarget->GetGraphicThingInstancePtr());
  } else {
    float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) +
              60.0f; // TEMPORARY HEIGHT
    pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(
        D3DXVECTOR3(kPacket.lX, kPacket.lY, h));
    // pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
  }

  return true;
}

bool CPythonNetworkStream::SendShootPacket(UINT uSkill) {
  TPacketCGShoot kPacketShoot;
  kPacketShoot.header = CG::SHOOT;
  kPacketShoot.length = sizeof(kPacketShoot);
  kPacketShoot.bType = uSkill;

  if (!Send(sizeof(kPacketShoot), &kPacketShoot)) {
    Tracen("SendShootPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendAddFlyTargetingPacket(
    DWORD dwTargetVID, const TPixelPosition &kPPosTarget) {
  TPacketCGFlyTargeting packet;

  // CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

  packet.header = CG::ADD_FLY_TARGETING;
  packet.length = sizeof(packet);
  packet.dwTargetVID = dwTargetVID;
  packet.lX = kPPosTarget.x;
  packet.lY = kPPosTarget.y;

  __LocalPositionToGlobalPosition(packet.lX, packet.lY);

  if (!Send(sizeof(packet), &packet)) {
    Tracen("Send FlyTargeting Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendFlyTargetingPacket(
    DWORD dwTargetVID, const TPixelPosition &kPPosTarget) {
  TPacketCGFlyTargeting packet;

  // CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

  packet.header = CG::FLY_TARGETING;
  packet.length = sizeof(packet);
  packet.dwTargetVID = dwTargetVID;
  packet.lX = kPPosTarget.x;
  packet.lY = kPPosTarget.y;

  __LocalPositionToGlobalPosition(packet.lX, packet.lY);

  if (!Send(sizeof(packet), &packet)) {
    Tracen("Send FlyTargeting Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::RecvCreateFlyPacket() {
  TPacketGCCreateFly kPacket;
  if (!Recv(sizeof(TPacketGCCreateFly), &kPacket))
    return false;

  CFlyingManager &rkFlyMgr = CFlyingManager::Instance();
  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

  CInstanceBase *pkStartInst = rkChrMgr.GetInstancePtr(kPacket.dwStartVID);
  CInstanceBase *pkEndInst = rkChrMgr.GetInstancePtr(kPacket.dwEndVID);
  if (!pkStartInst || !pkEndInst)
    return true;

  rkFlyMgr.CreateIndexedFly(kPacket.bType,
                            pkStartInst->GetGraphicThingInstancePtr(),
                            pkEndInst->GetGraphicThingInstancePtr());

  return true;
}

bool CPythonNetworkStream::SendTargetPacket(DWORD dwVID) {
  TPacketCGTarget packet;
  packet.header = CG::TARGET;
  packet.length = sizeof(packet);
  packet.dwVID = dwVID;

  if (!Send(sizeof(packet), &packet)) {
    Tracen("Send Target Packet Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendSyncPositionElementPacket(DWORD dwVictimVID,
                                                         DWORD dwVictimX,
                                                         DWORD dwVictimY) {
  TPacketCGSyncPositionElement kSyncPos;
  kSyncPos.dwVID = dwVictimVID;
  kSyncPos.lX = dwVictimX;
  kSyncPos.lY = dwVictimY;

  __LocalPositionToGlobalPosition(kSyncPos.lX, kSyncPos.lY);

  if (!Send(sizeof(kSyncPos), &kSyncPos)) {
    Tracen("CPythonNetworkStream::SendSyncPositionElementPacket - ERROR");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::RecvMessenger() {
  TPacketGCMessenger p;
  if (!Recv(sizeof(p), &p))
    return false;

  int iSize = p.length - sizeof(p);
  char char_name[24 + 1];

  switch (p.subheader) {
  case MessengerSub::GC::LIST: {
    TPacketGCMessengerListOnline on;
    while (iSize) {
      if (!Recv(sizeof(TPacketGCMessengerListOffline), &on))
        return false;

      if (!Recv(on.length, char_name))
        return false;

      char_name[on.length] = 0;

      if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
        CPythonMessenger::Instance().OnFriendLogin(char_name);
      else
        CPythonMessenger::Instance().OnFriendLogout(char_name);

      iSize -= sizeof(TPacketGCMessengerListOffline);
      iSize -= on.length;
    }
    break;
  }

  case MessengerSub::GC::LOGIN: {
    TPacketGCMessengerLogin p;
    if (!Recv(sizeof(p), &p))
      return false;
    if (!Recv(p.length, char_name))
      return false;
    char_name[p.length] = 0;
    CPythonMessenger::Instance().OnFriendLogin(char_name);
    __RefreshTargetBoardByName(char_name);
    break;
  }

  case MessengerSub::GC::LOGOUT: {
    TPacketGCMessengerLogout logout;
    if (!Recv(sizeof(logout), &logout))
      return false;
    if (!Recv(logout.length, char_name))
      return false;
    char_name[logout.length] = 0;
    CPythonMessenger::Instance().OnFriendLogout(char_name);
    break;
  }

  case MessengerSub::GC::REMOVE_FRIEND: {
    BYTE bLength;

    if (!Recv(sizeof(bLength), &bLength))
      return false;

    if (!Recv(bLength, char_name))
      return false;

    char_name[bLength] = 0;

    CPythonMessenger::Instance().RemoveFriend(char_name);
    __RefreshTargetBoardByName(char_name);

    break;
  }

  default:
    TraceError("RecvMessenger: unknown subheader %d", p.subheader);
    break;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Party

bool CPythonNetworkStream::SendPartyInvitePacket(DWORD dwVID) {
  TPacketCGPartyInvite kPartyInvitePacket;
  kPartyInvitePacket.header = CG::PARTY_INVITE;
  kPartyInvitePacket.length = sizeof(kPartyInvitePacket);
  kPartyInvitePacket.vid = dwVID;

  if (!Send(sizeof(kPartyInvitePacket), &kPartyInvitePacket)) {
    Tracenf(
        "CPythonNetworkStream::SendPartyInvitePacket [%ud] - PACKET SEND ERROR",
        dwVID);
    return false;
  }

  Tracef(" << SendPartyInvitePacket : %d\n", dwVID);
  return true;
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(DWORD dwLeaderVID,
                                                       BYTE byAnswer) {
  TPacketCGPartyInviteAnswer kPartyInviteAnswerPacket;
  kPartyInviteAnswerPacket.header = CG::PARTY_INVITE_ANSWER;
  kPartyInviteAnswerPacket.length = sizeof(kPartyInviteAnswerPacket);
  kPartyInviteAnswerPacket.leader_pid = dwLeaderVID;
  kPartyInviteAnswerPacket.accept = byAnswer;

  if (!Send(sizeof(kPartyInviteAnswerPacket), &kPartyInviteAnswerPacket)) {
    Tracenf("CPythonNetworkStream::SendPartyInviteAnswerPacket [%ud %ud] - "
            "PACKET SEND ERROR",
            dwLeaderVID, byAnswer);
    return false;
  }

  Tracef(" << SendPartyInviteAnswerPacket : %d, %d\n", dwLeaderVID, byAnswer);
  return true;
}

bool CPythonNetworkStream::SendPartyRemovePacket(DWORD dwPID) {
  TPacketCGPartyRemove kPartyInviteRemove;
  kPartyInviteRemove.header = CG::PARTY_REMOVE;
  kPartyInviteRemove.length = sizeof(kPartyInviteRemove);
  kPartyInviteRemove.pid = dwPID;

  if (!Send(sizeof(kPartyInviteRemove), &kPartyInviteRemove)) {
    Tracenf(
        "CPythonNetworkStream::SendPartyRemovePacket [%ud] - PACKET SEND ERROR",
        dwPID);
    return false;
  }

  Tracef(" << SendPartyRemovePacket : %d\n", dwPID);
  return true;
}

bool CPythonNetworkStream::SendPartySetStatePacket(DWORD dwVID, BYTE byState,
                                                   BYTE byFlag) {
  TPacketCGPartySetState kPartySetState;
  kPartySetState.header = CG::PARTY_SET_STATE;
  kPartySetState.length = sizeof(kPartySetState);
  kPartySetState.dwVID = dwVID;
  kPartySetState.byState = byState;
  kPartySetState.byFlag = byFlag;

  if (!Send(sizeof(kPartySetState), &kPartySetState)) {
    Tracenf("CPythonNetworkStream::SendPartySetStatePacket(%ud, %ud) - PACKET "
            "SEND ERROR",
            dwVID, byState);
    return false;
  }

  Tracef(" << SendPartySetStatePacket : %d, %d, %d\n", dwVID, byState, byFlag);
  return true;
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(BYTE bySkillIndex,
                                                   DWORD dwVID) {
  TPacketCGPartyUseSkill kPartyUseSkill;
  kPartyUseSkill.header = CG::PARTY_USE_SKILL;
  kPartyUseSkill.length = sizeof(kPartyUseSkill);
  kPartyUseSkill.bySkillIndex = bySkillIndex;
  kPartyUseSkill.dwTargetVID = dwVID;

  if (!Send(sizeof(kPartyUseSkill), &kPartyUseSkill)) {
    Tracenf("CPythonNetworkStream::SendPartyUseSkillPacket(%ud, %ud) - PACKET "
            "SEND ERROR",
            bySkillIndex, dwVID);
    return false;
  }

  Tracef(" << SendPartyUseSkillPacket : %d, %d\n", bySkillIndex, dwVID);
  return true;
}

bool CPythonNetworkStream::SendPartyParameterPacket(BYTE byDistributeMode) {
  TPacketCGPartyParameter kPartyParameter;
  kPartyParameter.header = CG::PARTY_PARAMETER;
  kPartyParameter.length = sizeof(kPartyParameter);
  kPartyParameter.bDistributeMode = byDistributeMode;

  if (!Send(sizeof(kPartyParameter), &kPartyParameter)) {
    Tracenf("CPythonNetworkStream::SendPartyParameterPacket(%d) - PACKET SEND "
            "ERROR",
            byDistributeMode);
    return false;
  }

  Tracef(" << SendPartyParameterPacket : %d\n", byDistributeMode);
  return true;
}

bool CPythonNetworkStream::RecvPartyInvite() {
  TPacketGCPartyInvite kPartyInvitePacket;
  if (!Recv(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
    return false;

  CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(
      kPartyInvitePacket.leader_pid);
  if (!pInstance) {
    TraceError(" CPythonNetworkStream::RecvPartyInvite - Failed to find leader "
               "instance [%d]\n",
               kPartyInvitePacket.leader_pid);
    return true;
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "RecvPartyInviteQuestion",
                        Py_BuildValue("(is)", kPartyInvitePacket.leader_pid,
                                      pInstance->GetNameString()));
  Tracef(" >> RecvPartyInvite : %d, %s\n", kPartyInvitePacket.leader_pid,
         pInstance->GetNameString());

  return true;
}

bool CPythonNetworkStream::RecvPartyAdd() {
  TPacketGCPartyAdd kPartyAddPacket;
  if (!Recv(sizeof(kPartyAddPacket), &kPartyAddPacket))
    return false;

  CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid,
                                              kPartyAddPacket.name);
  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember",
      Py_BuildValue("(is)", kPartyAddPacket.pid, kPartyAddPacket.name));
  Tracef(" >> RecvPartyAdd : %d, %s\n", kPartyAddPacket.pid,
         kPartyAddPacket.name);

  return true;
}

bool CPythonNetworkStream::RecvPartyUpdate() {
  TPacketGCPartyUpdate kPartyUpdatePacket;
  if (!Recv(sizeof(kPartyUpdatePacket), &kPartyUpdatePacket))
    return false;

  CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
  if (!CPythonPlayer::Instance().GetPartyMemberPtr(kPartyUpdatePacket.pid,
                                                   &pPartyMemberInfo))
    return true;

  BYTE byOldState = pPartyMemberInfo->byState;

  CPythonPlayer::Instance().UpdatePartyMemberInfo(
      kPartyUpdatePacket.pid, kPartyUpdatePacket.state,
      kPartyUpdatePacket.percent_hp);
  for (int i = 0; i < PARTY_AFFECT_SLOT_MAX_NUM; ++i) {
    CPythonPlayer::Instance().UpdatePartyMemberAffect(
        kPartyUpdatePacket.pid, i, kPartyUpdatePacket.affects[i]);
  }

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "UpdatePartyMemberInfo",
                        Py_BuildValue("(i)", kPartyUpdatePacket.pid));

  // 만약 리더가 바뀌었다면, TargetBoard 의 버튼을 업데이트 한다.
  DWORD dwVID;
  if (CPythonPlayer::Instance().PartyMemberPIDToVID(kPartyUpdatePacket.pid,
                                                    &dwVID))
    if (byOldState != kPartyUpdatePacket.state) {
      __RefreshTargetBoardByVID(dwVID);
    }

  // 	Tracef(" >> RecvPartyUpdate : %d, %d, %d\n", kPartyUpdatePacket.pid,
  // kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);

  return true;
}

bool CPythonNetworkStream::RecvPartyRemove() {
  TPacketGCPartyRemove kPartyRemovePacket;
  if (!Recv(sizeof(kPartyRemovePacket), &kPartyRemovePacket))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember",
                        Py_BuildValue("(i)", kPartyRemovePacket.pid));
  Tracef(" >> RecvPartyRemove : %d\n", kPartyRemovePacket.pid);

  return true;
}

bool CPythonNetworkStream::RecvPartyLink() {
  TPacketGCPartyLink kPartyLinkPacket;
  if (!Recv(sizeof(kPartyLinkPacket), &kPartyLinkPacket))
    return false;

  CPythonPlayer::Instance().LinkPartyMember(kPartyLinkPacket.pid,
                                            kPartyLinkPacket.vid);
  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember",
      Py_BuildValue("(ii)", kPartyLinkPacket.pid, kPartyLinkPacket.vid));
  Tracef(" >> RecvPartyLink : %d, %d\n", kPartyLinkPacket.pid,
         kPartyLinkPacket.vid);

  return true;
}

bool CPythonNetworkStream::RecvPartyUnlink() {
  TPacketGCPartyUnlink kPartyUnlinkPacket;
  if (!Recv(sizeof(kPartyUnlinkPacket), &kPartyUnlinkPacket))
    return false;

  CPythonPlayer::Instance().UnlinkPartyMember(kPartyUnlinkPacket.pid);

  if (CPythonPlayer::Instance().IsMainCharacterIndex(kPartyUnlinkPacket.vid)) {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "UnlinkAllPartyMember", Py_BuildValue("()"));
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkPartyMember",
                          Py_BuildValue("(i)", kPartyUnlinkPacket.pid));
  }

  Tracef(" >> RecvPartyUnlink : %d, %d\n", kPartyUnlinkPacket.pid,
         kPartyUnlinkPacket.vid);

  return true;
}

bool CPythonNetworkStream::RecvPartyParameter() {
  TPacketGCPartyParameter kPartyParameterPacket;
  if (!Recv(sizeof(kPartyParameterPacket), &kPartyParameterPacket))
    return false;

  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "ChangePartyParameter",
      Py_BuildValue("(i)", kPartyParameterPacket.bDistributeMode));
  Tracef(" >> RecvPartyParameter : %d\n",
         kPartyParameterPacket.bDistributeMode);

  return true;
}

// Party
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Guild

bool CPythonNetworkStream::SendGuildAddMemberPacket(DWORD dwVID) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwVID);
  GuildPacket.bySubHeader = GuildSub::CG::ADD_MEMBER;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(dwVID), &dwVID))
    return false;

  Tracef(" SendGuildAddMemberPacket\n", dwVID);
  return true;
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(DWORD dwPID) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwPID);
  GuildPacket.bySubHeader = GuildSub::CG::REMOVE_MEMBER;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(dwPID), &dwPID))
    return false;

  Tracef(" SendGuildRemoveMemberPacket %d\n", dwPID);
  return true;
}

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(
    BYTE byGradeNumber, const char *c_szName) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length =
      sizeof(GuildPacket) + sizeof(BYTE) + (GUILD_GRADE_NAME_MAX_LEN + 1);
  GuildPacket.bySubHeader = GuildSub::CG::CHANGE_GRADE_NAME;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(byGradeNumber), &byGradeNumber))
    return false;

  char szName[GUILD_GRADE_NAME_MAX_LEN + 1];
  strncpy(szName, c_szName, GUILD_GRADE_NAME_MAX_LEN);
  szName[GUILD_GRADE_NAME_MAX_LEN] = '\0';

  if (!Send(sizeof(szName), &szName))
    return false;

  Tracef(" SendGuildChangeGradeNamePacket %d, %s\n", byGradeNumber, c_szName);
  return true;
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(
    BYTE byGradeNumber, BYTE byAuthority) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(BYTE) + sizeof(BYTE);
  GuildPacket.bySubHeader = GuildSub::CG::CHANGE_GRADE_AUTHORITY;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(byGradeNumber), &byGradeNumber))
    return false;
  if (!Send(sizeof(byAuthority), &byAuthority))
    return false;

  Tracef(" SendGuildChangeGradeAuthorityPacket %d, %d\n", byGradeNumber,
         byAuthority);
  return true;
}

bool CPythonNetworkStream::SendGuildOfferPacket(DWORD dwExperience) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwExperience);
  GuildPacket.bySubHeader = GuildSub::CG::OFFER;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(dwExperience), &dwExperience))
    return false;

  Tracef(" SendGuildOfferPacket %d\n", dwExperience);
  return true;
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char *c_szMessage) {
  TPacketCGGuild GuildPacket;
  BYTE bySize = BYTE(strlen(c_szMessage)) + 1;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(BYTE) + bySize;
  GuildPacket.bySubHeader = GuildSub::CG::POST_COMMENT;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(bySize), &bySize))
    return false;
  if (!Send(bySize, c_szMessage))
    return false;

  Tracef(" SendGuildPostCommentPacket %d, %s\n", bySize, c_szMessage);
  return true;
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(DWORD dwIndex) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwIndex);
  GuildPacket.bySubHeader = GuildSub::CG::DELETE_COMMENT;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(dwIndex), &dwIndex))
    return false;

  Tracef(" SendGuildDeleteCommentPacket %d\n", dwIndex);
  return true;
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(
    DWORD dwHighestIndex) {
  static DWORD s_LastTime = timeGetTime() - 1001;

  if (timeGetTime() - s_LastTime < 1000)
    return true;
  s_LastTime = timeGetTime();

  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket);
  GuildPacket.bySubHeader = GuildSub::CG::REFRESH_COMMENT;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  Tracef(" SendGuildRefreshCommentPacket %d\n", dwHighestIndex);
  return true;
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(DWORD dwPID,
                                                            BYTE byGrade) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwPID) + sizeof(byGrade);
  GuildPacket.bySubHeader = GuildSub::CG::CHANGE_MEMBER_GRADE;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(dwPID), &dwPID))
    return false;
  if (!Send(sizeof(byGrade), &byGrade))
    return false;

  Tracef(" SendGuildChangeMemberGradePacket %d, %d\n", dwPID, byGrade);
  return true;
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(DWORD dwSkillID,
                                                   DWORD dwTargetVID) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket);
  GuildPacket.bySubHeader = GuildSub::CG::USE_SKILL;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(dwSkillID), &dwSkillID))
    return false;
  if (!Send(sizeof(dwTargetVID), &dwTargetVID))
    return false;

  Tracef(" SendGuildUseSkillPacket %d, %d\n", dwSkillID, dwTargetVID);
  return true;
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(DWORD dwPID,
                                                              BYTE byFlag) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwPID) + sizeof(byFlag);
  GuildPacket.bySubHeader = GuildSub::CG::CHANGE_MEMBER_GENERAL;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(dwPID), &dwPID))
    return false;
  if (!Send(sizeof(byFlag), &byFlag))
    return false;

  Tracef(" SendGuildChangeMemberGeneralFlagPacket %d, %d\n", dwPID, byFlag);
  return true;
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(DWORD dwGuildID,
                                                       BYTE byAnswer) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length =
      sizeof(GuildPacket) + sizeof(dwGuildID) + sizeof(byAnswer);
  GuildPacket.bySubHeader = GuildSub::CG::GUILD_INVITE_ANSWER;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(dwGuildID), &dwGuildID))
    return false;
  if (!Send(sizeof(byAnswer), &byAnswer))
    return false;

  Tracef(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
  return true;
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(DWORD dwMoney) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwMoney);
  GuildPacket.bySubHeader = GuildSub::CG::CHARGE_GSP;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;

  if (!Send(sizeof(dwMoney), &dwMoney))
    return false;

  Tracef(" SendGuildChargeGSPPacket %d\n", dwMoney);
  return true;
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(DWORD dwMoney) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwMoney);
  GuildPacket.bySubHeader = GuildSub::CG::DEPOSIT_MONEY;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(dwMoney), &dwMoney))
    return false;

  Tracef(" SendGuildDepositMoneyPacket %d\n", dwMoney);
  return true;
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(DWORD dwMoney) {
  TPacketCGGuild GuildPacket;
  GuildPacket.header = CG::GUILD;
  GuildPacket.length = sizeof(GuildPacket) + sizeof(dwMoney);
  GuildPacket.bySubHeader = GuildSub::CG::WITHDRAW_MONEY;
  if (!Send(sizeof(GuildPacket), &GuildPacket))
    return false;
  if (!Send(sizeof(dwMoney), &dwMoney))
    return false;

  Tracef(" SendGuildWithdrawMoneyPacket %d\n", dwMoney);
  return true;
}

bool CPythonNetworkStream::RecvGuild() {
  TPacketGCGuild GuildPacket;
  if (!Recv(sizeof(GuildPacket), &GuildPacket))
    return false;

  static const std::unordered_map<uint8_t, bool (CPythonNetworkStream::*)(
                                               const TPacketGCGuild &)>
      handlers = {
          {GuildSub::GC::LOGIN, &CPythonNetworkStream::RecvGuildSub_Login},
          {GuildSub::GC::LOGOUT, &CPythonNetworkStream::RecvGuildSub_Logout},
          {GuildSub::GC::REMOVE, &CPythonNetworkStream::RecvGuildSub_Remove},
          {GuildSub::GC::LIST, &CPythonNetworkStream::RecvGuildSub_List},
          {GuildSub::GC::GRADE, &CPythonNetworkStream::RecvGuildSub_Grade},
          {GuildSub::GC::GRADE_NAME,
           &CPythonNetworkStream::RecvGuildSub_GradeName},
          {GuildSub::GC::GRADE_AUTH,
           &CPythonNetworkStream::RecvGuildSub_GradeAuth},
          {GuildSub::GC::INFO, &CPythonNetworkStream::RecvGuildSub_Info},
          {GuildSub::GC::COMMENTS,
           &CPythonNetworkStream::RecvGuildSub_Comments},
          {GuildSub::GC::CHANGE_EXP,
           &CPythonNetworkStream::RecvGuildSub_ChangeExp},
          {GuildSub::GC::CHANGE_MEMBER_GRADE,
           &CPythonNetworkStream::RecvGuildSub_ChangeMemberGrade},
          {GuildSub::GC::SKILL_INFO,
           &CPythonNetworkStream::RecvGuildSub_SkillInfo},
          {GuildSub::GC::CHANGE_MEMBER_GENERAL,
           &CPythonNetworkStream::RecvGuildSub_ChangeMemberGeneral},
          {GuildSub::GC::GUILD_INVITE,
           &CPythonNetworkStream::RecvGuildSub_Invite},
          {GuildSub::GC::WAR, &CPythonNetworkStream::RecvGuildSub_War},
          {GuildSub::GC::GUILD_NAME, &CPythonNetworkStream::RecvGuildSub_Name},
          {GuildSub::GC::GUILD_WAR_LIST,
           &CPythonNetworkStream::RecvGuildSub_WarList},
          {GuildSub::GC::GUILD_WAR_END_LIST,
           &CPythonNetworkStream::RecvGuildSub_WarEndList},
          {GuildSub::GC::WAR_POINT,
           &CPythonNetworkStream::RecvGuildSub_WarPoint},
          {GuildSub::GC::MONEY_CHANGE,
           &CPythonNetworkStream::RecvGuildSub_MoneyChange},
      };

  auto it = handlers.find(GuildPacket.subheader);
  if (it == handlers.end()) {
    TraceError("RecvGuild: unknown subheader %d", GuildPacket.subheader);
    return true;
  }
  return (this->*(it->second))(GuildPacket);
}

bool CPythonNetworkStream::RecvGuildSub_Login(const TPacketGCGuild &pack) {
  uint32_t dwPID;
  if (!Recv(sizeof(uint32_t), &dwPID))
    return false;

  // Messenger
  CPythonGuild::TGuildMemberData *pGuildMemberData;
  if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
    if (0 !=
        pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
      CPythonMessenger::Instance().LoginGuildMember(
          pGuildMemberData->strName.c_str());

  // Tracef(" <Login> %d\n", dwPID);
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Logout(const TPacketGCGuild &pack) {
  uint32_t dwPID;
  if (!Recv(sizeof(uint32_t), &dwPID))
    return false;

  // Messenger
  CPythonGuild::TGuildMemberData *pGuildMemberData;
  if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
    if (0 !=
        pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
      CPythonMessenger::Instance().LogoutGuildMember(
          pGuildMemberData->strName.c_str());

  // Tracef(" <Logout> %d\n", dwPID);
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Remove(const TPacketGCGuild &pack) {
  uint32_t dwPID;
  if (!Recv(sizeof(dwPID), &dwPID))
    return false;

  // Main Player 일 경우 DeleteGuild
  if (CPythonGuild::Instance().IsMainPlayer(dwPID)) {
    CPythonGuild::Instance().Destroy();
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteGuild",
                          Py_BuildValue("()"));
    CPythonMessenger::Instance().RemoveAllGuildMember();
    __SetGuildID(0);
    __RefreshMessengerWindow();
    __RefreshTargetBoard();
    __RefreshCharacterWindow();
  } else {
    // Get Member Name
    std::string strMemberName = "";
    CPythonGuild::TGuildMemberData *pData;
    if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pData)) {
      strMemberName = pData->strName;
      CPythonMessenger::Instance().RemoveGuildMember(pData->strName.c_str());
    }

    CPythonGuild::Instance().RemoveMember(dwPID);

    // Refresh
    __RefreshTargetBoardByName(strMemberName.c_str());
    __RefreshGuildWindowMemberPage();
  }

  Tracef(" <Remove> %d\n", dwPID);
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_List(const TPacketGCGuild &pack) {
  int iPacketSize = int(pack.length) - sizeof(pack);

  for (; iPacketSize > 0;) {
    TPacketGCGuildSubMember memberPacket;
    if (!Recv(sizeof(memberPacket), &memberPacket))
      return false;

    char szName[CHARACTER_NAME_MAX_LEN + 1] = "";
    if (memberPacket.byNameFlag) {
      if (!Recv(sizeof(szName), &szName))
        return false;

      iPacketSize -= CHARACTER_NAME_MAX_LEN + 1;
    } else {
      CPythonGuild::TGuildMemberData *pMemberData;
      if (CPythonGuild::Instance().GetMemberDataPtrByPID(memberPacket.pid,
                                                         &pMemberData)) {
        strncpy(szName, pMemberData->strName.c_str(), CHARACTER_NAME_MAX_LEN);
      }
    }

    // Tracef(" <List> %d : %s, %d (%d, %d, %d)\n", memberPacket.pid, szName,
    // memberPacket.byGrade, memberPacket.byJob, memberPacket.byLevel,
    // memberPacket.dwOffer);

    CPythonGuild::SGuildMemberData GuildMemberData;
    GuildMemberData.dwPID = memberPacket.pid;
    GuildMemberData.byGrade = memberPacket.byGrade;
    GuildMemberData.strName = szName;
    GuildMemberData.byJob = memberPacket.byJob;
    GuildMemberData.byLevel = memberPacket.byLevel;
    GuildMemberData.dwOffer = memberPacket.dwOffer;
    GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
    CPythonGuild::Instance().RegisterMember(GuildMemberData);

    // Messenger
    if (strcmp(szName, CPythonPlayer::Instance().GetName()))
      CPythonMessenger::Instance().AppendGuildMember(szName);

    __RefreshTargetBoardByName(szName);

    iPacketSize -= sizeof(memberPacket);
  }

  __RefreshGuildWindowInfoPage();
  __RefreshGuildWindowMemberPage();
  __RefreshMessengerWindow();
  __RefreshCharacterWindow();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Grade(const TPacketGCGuild &pack) {
  BYTE byCount;
  if (!Recv(sizeof(byCount), &byCount))
    return false;

  for (BYTE i = 0; i < byCount; ++i) {
    BYTE byIndex;
    if (!Recv(sizeof(byCount), &byIndex))
      return false;
    TPacketGCGuildSubGrade GradePacket;
    if (!Recv(sizeof(GradePacket), &GradePacket))
      return false;

    auto data = CPythonGuild::SGuildGradeData(GradePacket.auth_flag,
                                              GradePacket.grade_name);
    CPythonGuild::Instance().SetGradeData(byIndex, data);
    // Tracef(" <Grade> [%d/%d] : %s, %d\n", byIndex, byCount,
    // GradePacket.grade_name, GradePacket.auth_flag);
  }
  __RefreshGuildWindowGradePage();
  __RefreshGuildWindowMemberPageGradeComboBox();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_GradeName(const TPacketGCGuild &pack) {
  BYTE byGradeNumber;
  if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
    return false;

  char szGradeName[GUILD_GRADE_NAME_MAX_LEN + 1] = "";
  if (!Recv(sizeof(szGradeName), &szGradeName))
    return false;

  CPythonGuild::Instance().SetGradeName(byGradeNumber, szGradeName);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade",
                        Py_BuildValue("()"));

  Tracef(" <Change Grade Name> %d, %s\n", byGradeNumber, szGradeName);
  __RefreshGuildWindowGradePage();
  __RefreshGuildWindowMemberPageGradeComboBox();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_GradeAuth(const TPacketGCGuild &pack) {
  BYTE byGradeNumber;
  if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
    return false;
  BYTE byAuthorityFlag;
  if (!Recv(sizeof(byAuthorityFlag), &byAuthorityFlag))
    return false;

  CPythonGuild::Instance().SetGradeAuthority(byGradeNumber, byAuthorityFlag);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade",
                        Py_BuildValue("()"));

  Tracef(" <Change Grade Authority> %d, %d\n", byGradeNumber, byAuthorityFlag);
  __RefreshGuildWindowGradePage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Info(const TPacketGCGuild &pack) {
  TPacketGCGuildInfo GuildInfo;
  if (!Recv(sizeof(GuildInfo), &GuildInfo))
    return false;

  CPythonGuild::Instance().EnableGuild();
  CPythonGuild::TGuildInfo &rGuildInfo =
      CPythonGuild::Instance().GetGuildInfoRef();
  strncpy(rGuildInfo.szGuildName, GuildInfo.name, GUILD_NAME_MAX_LEN);
  rGuildInfo.szGuildName[GUILD_NAME_MAX_LEN] = '\0';

  rGuildInfo.dwGuildID = GuildInfo.guild_id;
  rGuildInfo.dwMasterPID = GuildInfo.master_pid;
  rGuildInfo.dwGuildLevel = GuildInfo.level;
  rGuildInfo.dwCurrentExperience = GuildInfo.exp;
  rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
  rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
  rGuildInfo.dwGuildMoney = GuildInfo.gold;
  rGuildInfo.bHasLand = GuildInfo.hasLand;

  // Tracef(" <Info> %s, %d, %d : %d\n", GuildInfo.name, GuildInfo.master_pid,
  // GuildInfo.level, rGuildInfo.bHasLand);
  __RefreshGuildWindowInfoPage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Comments(const TPacketGCGuild &pack) {
  BYTE byCount;
  if (!Recv(sizeof(byCount), &byCount))
    return false;

  CPythonGuild::Instance().ClearComment();
  // Tracef(" >>> Comments Count : %d\n", byCount);

  for (BYTE i = 0; i < byCount; ++i) {
    DWORD dwCommentID;
    if (!Recv(sizeof(dwCommentID), &dwCommentID))
      return false;

    char szName[CHARACTER_NAME_MAX_LEN + 1] = "";
    if (!Recv(sizeof(szName), &szName))
      return false;

    char szComment[GULID_COMMENT_MAX_LEN + 1] = "";
    if (!Recv(sizeof(szComment), &szComment))
      return false;

    // Tracef(" [Comment-%d] : %s, %s\n", dwCommentID, szName, szComment);
    CPythonGuild::Instance().RegisterComment(dwCommentID, szName, szComment);
  }

  __RefreshGuildWindowBoardPage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_ChangeExp(const TPacketGCGuild &pack) {
  BYTE byLevel;
  if (!Recv(sizeof(byLevel), &byLevel))
    return false;
  uint32_t dwEXP;
  if (!Recv(sizeof(dwEXP), &dwEXP))
    return false;
  CPythonGuild::Instance().SetGuildEXP(byLevel, dwEXP);
  Tracef(" <ChangeEXP> %d, %d\n", byLevel, dwEXP);
  __RefreshGuildWindowInfoPage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_ChangeMemberGrade(
    const TPacketGCGuild &pack) {
  uint32_t dwPID;
  if (!Recv(sizeof(dwPID), &dwPID))
    return false;
  BYTE byGrade;
  if (!Recv(sizeof(byGrade), &byGrade))
    return false;
  CPythonGuild::Instance().ChangeGuildMemberGrade(dwPID, byGrade);
  Tracef(" <ChangeMemberGrade> %d, %d\n", dwPID, byGrade);
  __RefreshGuildWindowMemberPage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_SkillInfo(const TPacketGCGuild &pack) {
  CPythonGuild::TGuildSkillData &rSkillData =
      CPythonGuild::Instance().GetGuildSkillDataRef();
  if (!Recv(sizeof(rSkillData.bySkillPoint), &rSkillData.bySkillPoint))
    return false;
  if (!Recv(sizeof(rSkillData.bySkillLevel), rSkillData.bySkillLevel))
    return false;
  if (!Recv(sizeof(rSkillData.wGuildPoint), &rSkillData.wGuildPoint))
    return false;
  if (!Recv(sizeof(rSkillData.wMaxGuildPoint), &rSkillData.wMaxGuildPoint))
    return false;

  Tracef(" <SkillInfo> %d / %d, %d\n", rSkillData.bySkillPoint,
         rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
  __RefreshGuildWindowSkillPage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_ChangeMemberGeneral(
    const TPacketGCGuild &pack) {
  uint32_t dwPID;
  if (!Recv(sizeof(dwPID), &dwPID))
    return false;
  BYTE byFlag;
  if (!Recv(sizeof(byFlag), &byFlag))
    return false;

  CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(dwPID, byFlag);
  Tracef(" <ChangeMemberGeneralFlag> %d, %d\n", dwPID, byFlag);
  __RefreshGuildWindowMemberPage();
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Invite(const TPacketGCGuild &pack) {
  uint32_t dwGuildID;
  if (!Recv(sizeof(dwGuildID), &dwGuildID))
    return false;
  char szGuildName[GUILD_NAME_MAX_LEN + 1];
  if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
    return false;

  szGuildName[GUILD_NAME_MAX_LEN] = 0;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "RecvGuildInviteQuestion",
                        Py_BuildValue("(is)", dwGuildID, szGuildName));
  Tracef(" <Guild Invite> %d, %s\n", dwGuildID, szGuildName);
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_War(const TPacketGCGuild &pack) {
  TPacketGCGuildWar kGuildWar;
  if (!Recv(sizeof(kGuildWar), &kGuildWar))
    return false;

  switch (kGuildWar.bWarState) {
  case GUILD_WAR_SEND_DECLARE:
    Tracef(" >> GuildSub::GC::WAR : GUILD_WAR_SEND_DECLARE\n");
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_GuildWar_OnSendDeclare",
                          Py_BuildValue("(i)", kGuildWar.dwGuildOpp));
    break;
  case GUILD_WAR_RECV_DECLARE:
    Tracef(" >> GuildSub::GC::WAR : GUILD_WAR_RECV_DECLARE\n");
    PyCallClassMemberFunc(
        m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GuildWar_OnRecvDeclare",
        Py_BuildValue("(ii)", kGuildWar.dwGuildOpp, kGuildWar.bType));
    break;
  case GUILD_WAR_ON_WAR:
    Tracef(" >> GuildSub::GC::WAR : GUILD_WAR_ON_WAR : %d, %d\n",
           kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
    PyCallClassMemberFunc(
        m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GuildWar_OnStart",
        Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp));
    CPythonGuild::Instance().StartGuildWar(kGuildWar.dwGuildOpp);
    break;
  case GUILD_WAR_END:
    Tracef(" >> GuildSub::GC::WAR : GUILD_WAR_END\n");
    PyCallClassMemberFunc(
        m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GuildWar_OnEnd",
        Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp));
    CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
    break;
  }
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_Name(const TPacketGCGuild &pack) {
  uint32_t dwID;
  char szGuildName[GUILD_NAME_MAX_LEN + 1];

  int iPacketSize = int(pack.length) - sizeof(pack);

  int nItemSize = sizeof(dwID) + GUILD_NAME_MAX_LEN;

  assert(iPacketSize % nItemSize == 0 && "GuildSub::GC::GUILD_NAME");

  for (; iPacketSize > 0;) {
    if (!Recv(sizeof(dwID), &dwID))
      return false;

    if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
      return false;

    szGuildName[GUILD_NAME_MAX_LEN] = 0;

    // Tracef(" >> GulidName [%d : %s]\n", dwID, szGuildName);
    CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName);
    iPacketSize -= nItemSize;
  }
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_WarList(const TPacketGCGuild &pack) {
  uint32_t dwSrcGuildID;
  uint32_t dwDstGuildID;

  int iPacketSize = int(pack.length) - sizeof(pack);
  int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

  assert(iPacketSize % nItemSize == 0 && "GuildSub::GC::GUILD_WAR_LIST");

  for (; iPacketSize > 0;) {
    if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
      return false;

    if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
      return false;

    Tracef(" >> GulidWarList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
    CInstanceBase::InsertGVGKey(dwSrcGuildID, dwDstGuildID);
    CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
    iPacketSize -= nItemSize;
  }
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_WarEndList(const TPacketGCGuild &pack) {
  uint32_t dwSrcGuildID;
  uint32_t dwDstGuildID;

  int iPacketSize = int(pack.length) - sizeof(pack);
  int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

  assert(iPacketSize % nItemSize == 0 && "GuildSub::GC::GUILD_WAR_END_LIST");

  for (; iPacketSize > 0;) {

    if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
      return false;

    if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
      return false;

    Tracef(" >> GulidWarEndList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
    CInstanceBase::RemoveGVGKey(dwSrcGuildID, dwDstGuildID);
    CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
    iPacketSize -= nItemSize;
  }
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_WarPoint(const TPacketGCGuild &pack) {
  TPacketGuildWarPoint GuildWarPoint;
  if (!Recv(sizeof(GuildWarPoint), &GuildWarPoint))
    return false;

  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GuildWar_OnRecvPoint",
      Py_BuildValue("(iii)", GuildWarPoint.dwGainGuildID,
                    GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint));
  return true;
}

bool CPythonNetworkStream::RecvGuildSub_MoneyChange(
    const TPacketGCGuild &pack) {
  uint32_t dwMoney;
  if (!Recv(sizeof(dwMoney), &dwMoney))
    return false;

  CPythonGuild::Instance().SetGuildMoney(dwMoney);

  __RefreshGuildWindowInfoPage();
  Tracef(" >> Guild Money Change : %d\n", dwMoney);
  return true;
}

static DWORD gs_dwMarkUpdateRequestTime = 0;

bool CPythonNetworkStream::RecvMarkUpdate() {
  TPacketGCMarkUpdate packet;
  if (!Recv(sizeof(packet), &packet))
    return false;

  Tracef(" >> RecvMarkUpdate: guildID=%u, imgIdx=%u\n", packet.guildID,
         packet.imgIdx);

  // Rate limit mark downloads to prevent connection spam from multiple
  // simultaneous guild uploads Allow at most one download request per 1 second
  // from server-pushed updates
  DWORD dwCurrentTime = ELTimer_GetMSec();
  if (dwCurrentTime < gs_dwMarkUpdateRequestTime)
    return true;

  gs_dwMarkUpdateRequestTime = dwCurrentTime + 1000; // 1 second cooldown

  CGuildMarkDownloader &rkGuildMarkDownloader =
      CGuildMarkDownloader::Instance();
  rkGuildMarkDownloader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle,
                                m_kMarkAuth.m_dwRandomKey);

  return true;
}

// Guild
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Fishing

bool CPythonNetworkStream::SendFishingPacket(int iRotation) {
  TPacketCGFishing pack;
  pack.header = CG::FISHING;
  pack.length = sizeof(pack);
  pack.dir = iRotation / 5;

  if (!Send(sizeof(pack), &pack))
    return false;

  return true;
}

bool CPythonNetworkStream::SendGiveItemPacket(DWORD dwTargetVID,
                                              TItemPos ItemPos,
                                              int iItemCount) {
  TPacketCGGiveItem GiveItemPacket;
  GiveItemPacket.header = CG::ITEM_GIVE;
  GiveItemPacket.length = sizeof(GiveItemPacket);
  GiveItemPacket.dwTargetVID = dwTargetVID;
  GiveItemPacket.ItemPos = ItemPos;
  GiveItemPacket.byItemCount = iItemCount;

  if (!Send(sizeof(GiveItemPacket), &GiveItemPacket))
    return false;

  return true;
}

bool CPythonNetworkStream::RecvFishing() {
  TPacketGCFishing FishingPacket;
  if (!Recv(sizeof(FishingPacket), &FishingPacket))
    return false;

  CInstanceBase *pFishingInstance = NULL;
  if (FishingSub::GC::FISH != FishingPacket.subheader) {
    pFishingInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(FishingPacket.info);
    if (!pFishingInstance)
      return true;
  }

  switch (FishingPacket.subheader) {
  case FishingSub::GC::START:
    pFishingInstance->StartFishing(float(FishingPacket.dir) * 5.0f);
    break;
  case FishingSub::GC::STOP:
    if (pFishingInstance->IsFishing())
      pFishingInstance->StopFishing();
    break;
  case FishingSub::GC::REACT:
    if (pFishingInstance->IsFishing()) {
      pFishingInstance->SetFishEmoticon(); // Fish Emoticon
      pFishingInstance->ReactFishing();
    }
    break;
  case FishingSub::GC::SUCCESS:
    pFishingInstance->CatchSuccess();
    break;
  case FishingSub::GC::FAIL:
    pFishingInstance->CatchFail();
    if (pFishingInstance ==
        CPythonCharacterManager::Instance().GetMainInstancePtr()) {
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "OnFishingFailure", Py_BuildValue("()"));
    }
    break;
  case FishingSub::GC::FISH: {
    DWORD dwFishID = FishingPacket.info;

    if (0 == FishingPacket.info) {
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "OnFishingNotifyUnknown", Py_BuildValue("()"));
      return true;
    }

    CItemData *pItemData;
    if (!CItemManager::Instance().GetItemDataPointer(dwFishID, &pItemData))
      return true;

    CInstanceBase *pMainInstance =
        CPythonCharacterManager::Instance().GetMainInstancePtr();
    if (!pMainInstance)
      return true;

    if (pMainInstance->IsFishing()) {
      PyCallClassMemberFunc(
          m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotify",
          Py_BuildValue("(is)",
                        CItemData::ITEM_TYPE_FISH == pItemData->GetType(),
                        pItemData->GetName()));
    } else {
      PyCallClassMemberFunc(
          m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingSuccess",
          Py_BuildValue("(is)",
                        CItemData::ITEM_TYPE_FISH == pItemData->GetType(),
                        pItemData->GetName()));
    }
    break;
  }

  default:
    TraceError("RecvFishing: unknown subheader %d", FishingPacket.subheader);
    break;
  }

  return true;
}
// Fishing
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Dungeon
bool CPythonNetworkStream::RecvDungeon() {
  TPacketGCDungeon DungeonPacket;
  if (!Recv(sizeof(DungeonPacket), &DungeonPacket))
    return false;

  switch (DungeonPacket.subheader) {
  case DungeonSub::GC::TIME_ATTACK_START: {
    break;
  }
  case DungeonSub::GC::DESTINATION_POSITION: {
    unsigned long ulx, uly;
    if (!Recv(sizeof(ulx), &ulx))
      return false;
    if (!Recv(sizeof(uly), &uly))
      return false;

    CPythonPlayer::Instance().SetDungeonDestinationPosition(ulx, uly);
    break;
  }

  default:
    TraceError("RecvDungeon: unknown subheader %d", DungeonPacket.subheader);
    break;
  }

  return true;
}
// Dungeon
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MyShop
bool CPythonNetworkStream::SendBuildPrivateShopPacket(
    const char *c_szName,
    const std::vector<TShopItemTable> &c_rSellingItemStock) {
  TPacketCGMyShop packet;
  packet.header = CG::MYSHOP;
  packet.length =
      sizeof(packet) + sizeof(TShopItemTable) * c_rSellingItemStock.size();
  strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
  packet.bCount = static_cast<unsigned char>(c_rSellingItemStock.size());
  if (!Send(sizeof(packet), &packet))
    return false;

  for (std::vector<TShopItemTable>::const_iterator itor =
           c_rSellingItemStock.begin();
       itor < c_rSellingItemStock.end(); ++itor) {
    const TShopItemTable &c_rItem = *itor;
    if (!Send(sizeof(c_rItem), &c_rItem))
      return false;
  }

  return true;
}

bool CPythonNetworkStream::RecvShopSignPacket() {
  TPacketGCShopSign p;
  if (!Recv(sizeof(TPacketGCShopSign), &p))
    return false;

  CPythonPlayer &rkPlayer = CPythonPlayer::Instance();

  if (0 == strlen(p.szSign)) {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_PrivateShop_Disappear",
                          Py_BuildValue("(i)", p.dwVID));

    if (rkPlayer.IsMainCharacterIndex(p.dwVID))
      rkPlayer.ClosePrivateShop();
  } else {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_PrivateShop_Appear",
                          Py_BuildValue("(is)", p.dwVID, p.szSign));

    if (rkPlayer.IsMainCharacterIndex(p.dwVID))
      rkPlayer.OpenPrivateShop();
  }

  return true;
}
/////////////////////////////////////////////////////////////////////////

bool CPythonNetworkStream::RecvTimePacket() {
  TPacketGCTime TimePacket;
  if (!Recv(sizeof(TimePacket), &TimePacket))
    return false;

  IAbstractApplication &rkApp = IAbstractApplication::GetSingleton();
  rkApp.SetServerTime(TimePacket.time);

  return true;
}

bool CPythonNetworkStream::RecvWalkModePacket() {
  TPacketGCWalkMode WalkModePacket;
  if (!Recv(sizeof(WalkModePacket), &WalkModePacket))
    return false;

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(WalkModePacket.vid);
  if (pInstance) {
    if (WALKMODE_RUN == WalkModePacket.mode) {
      pInstance->SetRunMode();
    } else {
      pInstance->SetWalkMode();
    }
  }

  return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket() {
  TPacketGCChangeSkillGroup ChangeSkillGroup;
  if (!Recv(sizeof(ChangeSkillGroup), &ChangeSkillGroup))
    return false;

  m_dwMainActorSkillGroup = ChangeSkillGroup.skill_group;

  CPythonPlayer::Instance().NEW_ClearSkillData();
  __RefreshCharacterWindow();
  return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex) {
  m_dwMainActorSkillGroup = DWORD(iIndex);

  CPythonPlayer::Instance().NEW_ClearSkillData();
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter",
                        Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendRefinePacket(BYTE byPos, BYTE byType) {
  TPacketCGRefine kRefinePacket;
  kRefinePacket.header = CG::REFINE;
  kRefinePacket.length = sizeof(kRefinePacket);
  kRefinePacket.pos = byPos;
  kRefinePacket.type = byType;

  if (!Send(sizeof(kRefinePacket), &kRefinePacket))
    return false;

  return true;
}

bool CPythonNetworkStream::SendSelectItemPacket(DWORD dwItemPos) {
  TPacketCGScriptSelectItem kScriptSelectItem;
  kScriptSelectItem.header = CG::SCRIPT_SELECT_ITEM;
  kScriptSelectItem.length = sizeof(kScriptSelectItem);
  kScriptSelectItem.selection = dwItemPos;

  if (!Send(sizeof(kScriptSelectItem), &kScriptSelectItem))
    return false;

  return true;
}

bool CPythonNetworkStream::RecvRefineInformationPacket() {
  TPacketGCRefineInformation kRefineInfoPacket;
  if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
    return false;

  TRefineTable &rkRefineTable = kRefineInfoPacket.refine_table;
  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenRefineDialog",
      Py_BuildValue("(iiii)", kRefineInfoPacket.pos,
                    kRefineInfoPacket.refine_table.result_vnum,
                    rkRefineTable.cost, rkRefineTable.prob));

  for (int i = 0; i < rkRefineTable.material_count; ++i) {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "AppendMaterialToRefineDialog",
                          Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum,
                                        rkRefineTable.materials[i].count));
  }

#ifdef _DEBUG
  Tracef(" >> RecvRefineInformationPacket(pos=%d, result_vnum=%d, cost=%d, "
         "prob=%d)\n",
         kRefineInfoPacket.pos, kRefineInfoPacket.refine_table.result_vnum,
         rkRefineTable.cost, rkRefineTable.prob);
#endif

  return true;
}

bool CPythonNetworkStream::RecvRefineInformationPacketNew() {
  TPacketGCRefineInformationNew kRefineInfoPacket;
  if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
    return false;

  TRefineTable &rkRefineTable = kRefineInfoPacket.refine_table;
  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenRefineDialog",
      Py_BuildValue("(iiiii)", kRefineInfoPacket.pos,
                    kRefineInfoPacket.refine_table.result_vnum,
                    rkRefineTable.cost, rkRefineTable.prob,
                    kRefineInfoPacket.type));

  for (int i = 0; i < rkRefineTable.material_count; ++i) {
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "AppendMaterialToRefineDialog",
                          Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum,
                                        rkRefineTable.materials[i].count));
  }

#ifdef _DEBUG
  Tracef(" >> RecvRefineInformationPacketNew(pos=%d, result_vnum=%d, cost=%d, "
         "prob=%d, type=%d)\n",
         kRefineInfoPacket.pos, kRefineInfoPacket.refine_table.result_vnum,
         rkRefineTable.cost, rkRefineTable.prob, kRefineInfoPacket.type);
#endif

  return true;
}

bool CPythonNetworkStream::RecvNPCList() {
  TPacketGCNPCPosition kNPCPosition;
  if (!Recv(sizeof(kNPCPosition), &kNPCPosition))
    return false;

  assert(int(kNPCPosition.length) - sizeof(kNPCPosition) ==
             kNPCPosition.count * sizeof(TNPCPosition) &&
         "GC::NPC_POSITION");

  CPythonMiniMap::Instance().ClearAtlasMarkInfo();

  for (int i = 0; i < kNPCPosition.count; ++i) {
    TNPCPosition NPCPosition;
    if (!Recv(sizeof(TNPCPosition), &NPCPosition))
      return false;

    const char *c_szName = nullptr;
    if (CPythonNonPlayer::Instance().GetName(NPCPosition.dwVnum, &c_szName)) {
      CPythonMiniMap::Instance().RegisterAtlasMark(
          NPCPosition.bType, c_szName, NPCPosition.x, NPCPosition.y);
    } else {
      CPythonMiniMap::Instance().RegisterAtlasMark(
          NPCPosition.bType, NPCPosition.name, NPCPosition.x, NPCPosition.y);
    }
  }

  return true;
}

bool CPythonNetworkStream::__SendCRCReportPacket() { return true; }

bool CPythonNetworkStream::SendClientVersionPacket() {
  std::string filename;

  GetExcutedFileName(filename);

  filename = CFileNameHelper::NoPath(filename);
  CFileNameHelper::ChangeDosPath(filename);

  TPacketCGClientVersion kVersionPacket{};
  kVersionPacket.header = CG::CLIENT_VERSION;
  kVersionPacket.length = sizeof(kVersionPacket);

  strncpy(kVersionPacket.filename, filename.c_str(),
          sizeof(kVersionPacket.filename) - 1);
  kVersionPacket.filename[sizeof(kVersionPacket.filename) - 1] = '\0';

  snprintf(kVersionPacket.timestamp, sizeof(kVersionPacket.timestamp), "%u",
           1215955205u);

  if (!Send(sizeof(kVersionPacket), &kVersionPacket))
    Tracef("SendClientReportPacket Error");

  return true;
}

bool CPythonNetworkStream::RecvAffectAddPacket() {
  TPacketGCAffectAdd kAffectAdd;
  if (!Recv(sizeof(kAffectAdd), &kAffectAdd))
    return false;

  TPacketAffectElement &rkElement = kAffectAdd.elem;
  if (rkElement.bPointIdxApplyOn == POINT_ENERGY) {
    CPythonPlayer::instance().SetStatus(
        POINT_ENERGY_END_TIME,
        CPythonApplication::Instance().GetServerTimeStamp() +
            rkElement.lDuration);
    __RefreshStatus();
  }
  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_AddAffect",
      Py_BuildValue("(iiii)", rkElement.dwType, rkElement.bPointIdxApplyOn,
                    rkElement.lApplyValue, rkElement.lDuration));

  return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket() {
  TPacketGCAffectRemove kAffectRemove;
  if (!Recv(sizeof(kAffectRemove), &kAffectRemove))
    return false;

  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_RemoveAffect",
      Py_BuildValue("(ii)", kAffectRemove.dwType, kAffectRemove.bApplyOn));

  return true;
}

bool CPythonNetworkStream::RecvChannelPacket() {
  TPacketGCChannel kChannelPacket;
  if (!Recv(sizeof(kChannelPacket), &kChannelPacket))
    return false;

  // Tracef(" >> CPythonNetworkStream::RecvChannelPacket(channel=%d)\n",
  // kChannelPacket.channel);

  return true;
}

bool CPythonNetworkStream::RecvViewEquipPacket() {
  TPacketGCViewEquip kViewEquipPacket;
  if (!Recv(sizeof(kViewEquipPacket), &kViewEquipPacket))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenEquipmentDialog",
                        Py_BuildValue("(i)", kViewEquipPacket.dwVID));

  for (int i = 0; i < WEAR_MAX_NUM; ++i) {
    TEquipmentItemSet &rItemSet = kViewEquipPacket.equips[i];
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "SetEquipmentDialogItem",
                          Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i,
                                        rItemSet.vnum, rItemSet.count));

    for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "SetEquipmentDialogSocket",
                            Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i,
                                          j, rItemSet.alSockets[j]));

    for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
      PyCallClassMemberFunc(
          m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr",
          Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k,
                        rItemSet.aAttr[k].bType, rItemSet.aAttr[k].sValue));
  }

  return true;
}

bool CPythonNetworkStream::RecvLandPacket() {
  TPacketGCLandList kLandList;
  if (!Recv(sizeof(kLandList), &kLandList))
    return false;

  std::vector<DWORD> kVec_dwGuildID;

  CPythonMiniMap &rkMiniMap = CPythonMiniMap::Instance();
  CPythonBackground &rkBG = CPythonBackground::Instance();
  CInstanceBase *pMainInstance =
      CPythonPlayer::Instance().NEW_GetMainActorPtr();

  rkMiniMap.ClearGuildArea();
  rkBG.ClearGuildArea();

  int iPacketSize = (kLandList.length - sizeof(TPacketGCLandList));
  for (; iPacketSize > 0; iPacketSize -= sizeof(TLandPacketElement)) {
    TLandPacketElement kElement;
    if (!Recv(sizeof(TLandPacketElement), &kElement))
      return false;

    rkMiniMap.RegisterGuildArea(kElement.dwID, kElement.dwGuildID, kElement.x,
                                kElement.y, kElement.width, kElement.height);

    if (pMainInstance)
      if (kElement.dwGuildID == pMainInstance->GetGuildID()) {
        rkBG.RegisterGuildArea(kElement.x, kElement.y,
                               kElement.x + kElement.width,
                               kElement.y + kElement.height);
      }

    if (0 != kElement.dwGuildID)
      kVec_dwGuildID.push_back(kElement.dwGuildID);
  }

  __DownloadSymbol(kVec_dwGuildID);

  return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket() {
  TPacketGCTargetCreate kTargetCreate;
  if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
    return false;

  CPythonMiniMap &rkpyMiniMap = CPythonMiniMap::Instance();
  rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);

  // #ifdef _DEBUG
  //	char szBuf[256+1];
  //	_snprintf(szBuf, sizeof(szBuf), "타겟이 생성 되었습니다 [%d:%s]",
  // kTargetCreate.lID, kTargetCreate.szTargetName);
  //	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
  //	Tracef(" >> RecvTargetCreatePacket %d : %s\n", kTargetCreate.lID,
  // kTargetCreate.szTargetName); #endif

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacketNew() {
  TPacketGCTargetCreateNew kTargetCreate;
  if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
    return false;

  CPythonMiniMap &rkpyMiniMap = CPythonMiniMap::Instance();
  CPythonBackground &rkpyBG = CPythonBackground::Instance();
  if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.byType) {
    rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);
  } else {
    rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName,
                             kTargetCreate.dwVID);
    rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
  }

  // #ifdef _DEBUG
  //	char szBuf[256+1];
  //	_snprintf(szBuf, sizeof(szBuf), "캐릭터 타겟이 생성 되었습니다
  //[%d:%s:%d]", kTargetCreate.lID, kTargetCreate.szTargetName,
  // kTargetCreate.dwVID);
  // CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf); 	Tracef("
  // >> RecvTargetCreatePacketNew %d : %d/%d\n", kTargetCreate.lID,
  // kTargetCreate.byType, kTargetCreate.dwVID); #endif

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
  return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket() {
  TPacketGCTargetUpdate kTargetUpdate;
  if (!Recv(sizeof(kTargetUpdate), &kTargetUpdate))
    return false;

  CPythonMiniMap &rkpyMiniMap = CPythonMiniMap::Instance();
  rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX,
                           kTargetUpdate.lY);

  CPythonBackground &rkpyBG = CPythonBackground::Instance();
  rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX,
                            kTargetUpdate.lY);

  // #ifdef _DEBUG
  //	char szBuf[256+1];
  //	_snprintf(szBuf, sizeof(szBuf), "타겟의 위치가 갱신 되었습니다
  //[%d:%d/%d]", kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);
  //	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
  //	Tracef(" >> RecvTargetUpdatePacket %d : %d, %d\n", kTargetUpdate.lID,
  // kTargetUpdate.lX, kTargetUpdate.lY); #endif

  return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket() {
  TPacketGCTargetDelete kTargetDelete;
  if (!Recv(sizeof(kTargetDelete), &kTargetDelete))
    return false;

  CPythonMiniMap &rkpyMiniMap = CPythonMiniMap::Instance();
  rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

  CPythonBackground &rkpyBG = CPythonBackground::Instance();
  rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

  // #ifdef _DEBUG
  //	Tracef(" >> RecvTargetDeletePacket %d\n", kTargetDelete.lID);
  // #endif

  return true;
}

bool CPythonNetworkStream::RecvLoverInfoPacket() {
  TPacketGCLoverInfo kLoverInfo;
  if (!Recv(sizeof(kLoverInfo), &kLoverInfo))
    return false;

  PyCallClassMemberFunc(
      m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo",
      Py_BuildValue("(si)", kLoverInfo.szName, kLoverInfo.byLovePoint));
#ifdef _DEBUG
  Tracef("RECV LOVER INFO : %s, %d\n", kLoverInfo.szName,
         kLoverInfo.byLovePoint);
#endif
  return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket() {
  TPacketGCLovePointUpdate kLovePointUpdate;
  if (!Recv(sizeof(kLovePointUpdate), &kLovePointUpdate))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "BINARY_UpdateLovePoint",
                        Py_BuildValue("(i)", kLovePointUpdate.byLovePoint));
#ifdef _DEBUG
  Tracef("RECV LOVE POINT UPDATE : %d\n", kLovePointUpdate.byLovePoint);
#endif
  return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket() {
  TPacketGCDigMotion kDigMotion;
  if (!Recv(sizeof(kDigMotion), &kDigMotion))
    return false;

#ifdef _DEBUG
  Tracef(" Dig Motion [%d/%d]\n", kDigMotion.vid, kDigMotion.count);
#endif

  IAbstractCharacterManager &rkChrMgr =
      IAbstractCharacterManager::GetSingleton();
  CInstanceBase *pkInstMain = rkChrMgr.GetInstancePtr(kDigMotion.vid);
  CInstanceBase *pkInstTarget = rkChrMgr.GetInstancePtr(kDigMotion.target_vid);
  if (NULL == pkInstMain)
    return true;

  if (pkInstTarget)
    pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

  for (int i = 0; i < kDigMotion.count; ++i)
    pkInstMain->PushOnceMotion(CRaceMotionData::NAME_DIG);

  return true;
}

// 용혼석 강화
bool CPythonNetworkStream::SendDragonSoulRefinePacket(BYTE bRefineType,
                                                      TItemPos *pos) {
  TPacketCGDragonSoulRefine pk;
  pk.header = CG::DRAGON_SOUL_REFINE;
  pk.length = sizeof(pk);
  pk.bSubType = bRefineType;
  memcpy(pk.ItemGrid, pos, sizeof(TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
  if (!Send(sizeof(pk), &pk)) {
    return false;
  }
  return true;
}

#if defined(ENABLE_DISCORD_RPC)
#include "discord_rpc.h"
#include "Discord.h"
#ifdef _DEBUG
#pragma comment(lib, "discord_rpc_d.lib")
#else
#pragma comment(lib, "discord_rpc_r.lib")
#endif

static int64_t StartTime = 0;
void CPythonNetworkStream::Discord_Start() {
  StartTime = time(0);
  DiscordEventHandlers handlers;
  memset(&handlers, 0, sizeof(handlers));
  Discord_Initialize(Discord::DiscordClientID, &handlers, 1, nullptr);
  Discord_Update(false);
}

void CPythonNetworkStream::Discord_Update(const bool bInGame) {
  DiscordRichPresence discordPresence;
  memset(&discordPresence, 0, sizeof(discordPresence));
  discordPresence.startTimestamp = StartTime;

  if (!bInGame) {
    Discord_UpdatePresence(&discordPresence);
    return;
  }

  /*Name*/
  auto NameData = Discord::GetNameData();
  discordPresence.state = std::get<0>(NameData).c_str();
  discordPresence.details = std::get<1>(NameData).c_str();

  /*Race*/
  auto RaceData = Discord::GetRaceData();
  discordPresence.largeImageKey = std::get<0>(RaceData).c_str();
  discordPresence.largeImageText = std::get<1>(RaceData).c_str();

  /*Empire*/
  auto EmpireData = Discord::GetEmpireData();
  discordPresence.smallImageKey = std::get<0>(EmpireData).c_str();
  discordPresence.smallImageText = std::get<1>(EmpireData).c_str();

  Discord_UpdatePresence(&discordPresence);
}

void CPythonNetworkStream::Discord_Close() { Discord_Shutdown(); }
#endif