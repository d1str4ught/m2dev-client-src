#include "StdAfx.h"
#include "NetStream.h"
#include <iomanip>
#include <sstream>

#if defined(_DEBUG) && !defined(_PACKETDUMP)
#define _PACKETDUMP
#endif

#ifdef _PACKETDUMP
#include <unordered_map>
#include "../UserInterface/Packet.h"
#endif

bool CNetworkStream::IsSecurityMode() { return m_secureCipher.IsActivated(); }

void CNetworkStream::DecryptPendingRecvData() {
  size_t remaining = m_recvBuf.ReadableBytes();
  if (remaining > 0 && m_secureCipher.IsActivated())
    m_secureCipher.DecryptInPlace(m_recvBuf.DataAt(m_recvBuf.ReadPos()),
                                  remaining);
}

void CNetworkStream::SetRecvBufferSize(int recvBufSize) {
  m_recvBuf.Reserve(static_cast<size_t>(recvBufSize));
}

void CNetworkStream::SetSendBufferSize(int sendBufSize) {
  m_sendBuf.Reserve(static_cast<size_t>(sendBufSize));
}

bool CNetworkStream::__RecvInternalBuffer() {
  m_recvBuf.EnsureWritable(4096);

  int restSize = static_cast<int>(m_recvBuf.WritableBytes());
  if (restSize > 0) {
    int recvSize = recv(m_sock, reinterpret_cast<char *>(m_recvBuf.WritePtr()),
                        restSize, 0);

    if (recvSize < 0) {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK)
        return false;
    } else if (recvSize == 0) {
      return false;
    } else {
      if (m_secureCipher.IsActivated())
        m_secureCipher.DecryptInPlace(m_recvBuf.WritePtr(), recvSize);

      m_recvBuf.CommitWrite(recvSize);
    }
  }

  return true;
}

bool CNetworkStream::__SendInternalBuffer() {
  int dataSize = __GetSendBufferSize();
  if (dataSize <= 0)
    return true;

  int sendSize = send(
      m_sock, reinterpret_cast<const char *>(m_sendBuf.ReadPtr()), dataSize, 0);
  if (sendSize < 0) {
    int err = WSAGetLastError();
    TraceError(
        "__SendInternalBuffer: send() failed, sock=%llu, dataSize=%d, error=%d",
        (unsigned long long)m_sock, dataSize, err);
    return false;
  }

  m_sendBuf.Discard(sendSize);

  return true;
}

#pragma warning(push)
#pragma warning(disable : 4127)
void CNetworkStream::Process() {
  if (m_sock == INVALID_SOCKET)
    return;

  fd_set fdsRecv;
  fd_set fdsSend;

  FD_ZERO(&fdsRecv);
  FD_ZERO(&fdsSend);

  FD_SET(m_sock, &fdsRecv);
  FD_SET(m_sock, &fdsSend);

  TIMEVAL delay;

  delay.tv_sec = 0;
  delay.tv_usec = 0;

  if (select(0, &fdsRecv, &fdsSend, NULL, &delay) == SOCKET_ERROR)
    return;

  if (!m_isOnline) {
    if (FD_ISSET(m_sock, &fdsSend)) {
      m_isOnline = true;
      OnConnectSuccess();
    } else if (time(NULL) > m_connectLimitTime) {
      Clear();
      OnConnectFailure();
    }

    return;
  }

  if (FD_ISSET(m_sock, &fdsSend) && (m_sendBuf.ReadableBytes() > 0)) {
    if (!__SendInternalBuffer()) {
      int error = WSAGetLastError();

      if (error != WSAEWOULDBLOCK) {
        OnRemoteDisconnect();
        Clear();
        return;
      }
    }
  }

  if (FD_ISSET(m_sock, &fdsRecv)) {
    if (!__RecvInternalBuffer()) {
      OnRemoteDisconnect();
      Clear();
      return;
    }
  }

  if (!OnProcess()) {
    OnRemoteDisconnect();
    Clear();
  }
}
#pragma warning(pop)

void CNetworkStream::Disconnect() {
  if (m_sock == INVALID_SOCKET)
    return;

  Clear();
}

void CNetworkStream::Clear() {
  // Always clean cipher state (erase key material promptly)
  m_secureCipher.CleanUp();

  if (m_sock != INVALID_SOCKET) {
    closesocket(m_sock);
    m_sock = INVALID_SOCKET;
  }

  m_isOnline = false;
  m_connectLimitTime = 0;

  m_recvBuf.Clear();
  m_sendBuf.Clear();
}

bool CNetworkStream::Connect(const CNetworkAddress &c_rkNetAddr, int limitSec) {
  Clear();

  m_addr = c_rkNetAddr;

  m_sock = socket(AF_INET, SOCK_STREAM, 0);

  if (m_sock == INVALID_SOCKET) {
    Clear();
    OnConnectFailure();
    return false;
  }

  DWORD arg = 1;
  ioctlsocket(m_sock, FIONBIO, &arg); // Non-blocking mode

  // Enable TCP_NODELAY to disable Nagle's algorithm for lower latency
  int opt = 1;
  if (setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&opt,
                 sizeof(opt)) != 0) {
    TraceError("setsockopt TCP_NODELAY failed: %d", WSAGetLastError());
  }

  if (connect(m_sock, (PSOCKADDR)&m_addr, m_addr.GetSize()) == SOCKET_ERROR) {
    int error = WSAGetLastError();

    if (error != WSAEWOULDBLOCK) {
      Tracen("error != WSAEWOULDBLOCK");
      Clear();
      OnConnectFailure();
      return false;
    }
  }

  m_connectLimitTime = time(NULL) + limitSec;
  return true;
}

bool CNetworkStream::Connect(DWORD dwAddr, int port, int limitSec) {
  char szAddr[256];
  {
    BYTE ip[4];
    ip[0] = dwAddr & 0xff;
    dwAddr >>= 8;
    ip[1] = dwAddr & 0xff;
    dwAddr >>= 8;
    ip[2] = dwAddr & 0xff;
    dwAddr >>= 8;
    ip[3] = dwAddr & 0xff;
    dwAddr >>= 8;

    sprintf(szAddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  }

  return Connect(szAddr, port, limitSec);
}

bool CNetworkStream::Connect(const char *c_szAddr, int port, int /*limitSec*/) {
  CNetworkAddress kNetAddr;
  kNetAddr.Set(c_szAddr, port);

  return Connect(kNetAddr);
}

void CNetworkStream::ClearRecvBuffer() { m_recvBuf.Clear(); }

int CNetworkStream::GetRecvBufferSize() {
  return static_cast<int>(m_recvBuf.ReadableBytes());
}

bool CNetworkStream::Peek(int size) {
  return m_recvBuf.HasBytes(static_cast<size_t>(size));
}

bool CNetworkStream::Peek(int size, char *pDestBuf) {
  return m_recvBuf.Peek(pDestBuf, static_cast<size_t>(size));
}

#ifdef _PACKETDUMP

static const char *GetHeaderName(uint16_t header) {
  static const std::unordered_map<uint16_t, const char *> s_headerNames = {
      // Control
      {CG::PONG, "CG_PONG"},
      {GC::PING, "GC_PING"},
      {GC::PHASE, "GC_PHASE"},
      {CG::KEY_RESPONSE, "CG_KEY_RESPONSE"},
      {GC::KEY_CHALLENGE, "GC_KEY_CHALLENGE"},
      {GC::KEY_COMPLETE, "GC_KEY_COMPLETE"},
      {CG::CLIENT_VERSION, "CG_CLIENT_VERSION"},
      {CG::STATE_CHECKER, "CG_STATE_CHECKER"},
      {GC::RESPOND_CHANNELSTATUS, "GC_RESPOND_CHANNELSTATUS"},
      {CG::TEXT, "CG_TEXT"},
      // Authentication
      {CG::LOGIN2, "CG_LOGIN2"},
      {CG::LOGIN3, "CG_LOGIN3"},
      {CG::LOGIN_SECURE, "CG_LOGIN_SECURE"},
      {GC::LOGIN_SUCCESS3, "GC_LOGIN_SUCCESS3"},
      {GC::LOGIN_SUCCESS4, "GC_LOGIN_SUCCESS4"},
      {GC::LOGIN_FAILURE, "GC_LOGIN_FAILURE"},
      {GC::LOGIN_KEY, "GC_LOGIN_KEY"},
      {GC::AUTH_SUCCESS, "GC_AUTH_SUCCESS"},
      {GC::EMPIRE, "GC_EMPIRE"},
      {CG::EMPIRE, "CG_EMPIRE"},
      {CG::CHANGE_NAME, "CG_CHANGE_NAME"},
      {GC::CHANGE_NAME, "GC_CHANGE_NAME"},
      // Character
      {CG::CHARACTER_CREATE, "CG_CHARACTER_CREATE"},
      {CG::CHARACTER_DELETE, "CG_CHARACTER_DELETE"},
      {CG::CHARACTER_SELECT, "CG_CHARACTER_SELECT"},
      {CG::ENTERGAME, "CG_ENTERGAME"},
      {GC::CHARACTER_ADD, "GC_CHARACTER_ADD"},
      {GC::CHARACTER_ADD2, "GC_CHARACTER_ADD2"},
      {GC::CHAR_ADDITIONAL_INFO, "GC_CHAR_ADDITIONAL_INFO"},
      {GC::CHARACTER_DEL, "GC_CHARACTER_DEL"},
      {GC::CHARACTER_UPDATE, "GC_CHARACTER_UPDATE"},
      {GC::CHARACTER_UPDATE2, "GC_CHARACTER_UPDATE2"},
      {GC::CHARACTER_POSITION, "GC_CHARACTER_POSITION"},
      {GC::PLAYER_CREATE_SUCCESS, "GC_PLAYER_CREATE_SUCCESS"},
      {GC::PLAYER_CREATE_FAILURE, "GC_PLAYER_CREATE_FAILURE"},
      {GC::PLAYER_DELETE_SUCCESS, "GC_PLAYER_DELETE_SUCCESS"},
      {GC::PLAYER_DELETE_WRONG_SOCIAL_ID, "GC_PLAYER_DELETE_WRONG_SOCIAL_ID"},
      {GC::MAIN_CHARACTER, "GC_MAIN_CHARACTER"},
      {GC::PLAYER_POINTS, "GC_PLAYER_POINTS"},
      {GC::PLAYER_POINT_CHANGE, "GC_PLAYER_POINT_CHANGE"},
      {GC::STUN, "GC_STUN"},
      {GC::DEAD, "GC_DEAD"},
      {GC::CHANGE_SPEED, "GC_CHANGE_SPEED"},
      {GC::WALK_MODE, "GC_WALK_MODE"},
      {GC::SKILL_LEVEL, "GC_SKILL_LEVEL"},
      {GC::SKILL_LEVEL_NEW, "GC_SKILL_LEVEL_NEW"},
      {GC::SKILL_COOLTIME_END, "GC_SKILL_COOLTIME_END"},
      {GC::CHANGE_SKILL_GROUP, "GC_CHANGE_SKILL_GROUP"},
      {GC::VIEW_EQUIP, "GC_VIEW_EQUIP"},
      // Movement
      {CG::MOVE, "CG_MOVE"},
      {GC::MOVE, "GC_MOVE"},
      {CG::SYNC_POSITION, "CG_SYNC_POSITION"},
      {GC::SYNC_POSITION, "GC_SYNC_POSITION"},
      {CG::WARP, "CG_WARP"},
      {GC::WARP, "GC_WARP"},
      {GC::MOTION, "GC_MOTION"},
      {GC::DIG_MOTION, "GC_DIG_MOTION"},
      // Combat
      {CG::ATTACK, "CG_ATTACK"},
      {CG::USE_SKILL, "CG_USE_SKILL"},
      {CG::SHOOT, "CG_SHOOT"},
      {CG::FLY_TARGETING, "CG_FLY_TARGETING"},
      {CG::ADD_FLY_TARGETING, "CG_ADD_FLY_TARGETING"},
      {GC::DAMAGE_INFO, "GC_DAMAGE_INFO"},
      {GC::FLY_TARGETING, "GC_FLY_TARGETING"},
      {GC::ADD_FLY_TARGETING, "GC_ADD_FLY_TARGETING"},
      {GC::CREATE_FLY, "GC_CREATE_FLY"},
      {GC::PVP, "GC_PVP"},
      {GC::DUEL_START, "GC_DUEL_START"},
      // Items
      {CG::ITEM_USE, "CG_ITEM_USE"},
      {CG::ITEM_DROP, "CG_ITEM_DROP"},
      {CG::ITEM_DROP2, "CG_ITEM_DROP2"},
      {CG::ITEM_MOVE, "CG_ITEM_MOVE"},
      {CG::ITEM_PICKUP, "CG_ITEM_PICKUP"},
      {CG::ITEM_USE_TO_ITEM, "CG_ITEM_USE_TO_ITEM"},
      {CG::ITEM_GIVE, "CG_ITEM_GIVE"},
      {CG::EXCHANGE, "CG_EXCHANGE"},
      {CG::QUICKSLOT_ADD, "CG_QUICKSLOT_ADD"},
      {CG::QUICKSLOT_DEL, "CG_QUICKSLOT_DEL"},
      {CG::QUICKSLOT_SWAP, "CG_QUICKSLOT_SWAP"},
      {CG::REFINE, "CG_REFINE"},
      {CG::DRAGON_SOUL_REFINE, "CG_DRAGON_SOUL_REFINE"},
      {GC::ITEM_DEL, "GC_ITEM_DEL"},
      {GC::ITEM_SET, "GC_ITEM_SET"},
      {GC::ITEM_USE, "GC_ITEM_USE"},
      {GC::ITEM_DROP, "GC_ITEM_DROP"},
      {GC::ITEM_UPDATE, "GC_ITEM_UPDATE"},
      {GC::ITEM_GROUND_ADD, "GC_ITEM_GROUND_ADD"},
      {GC::ITEM_GROUND_DEL, "GC_ITEM_GROUND_DEL"},
      {GC::ITEM_OWNERSHIP, "GC_ITEM_OWNERSHIP"},
      {GC::ITEM_GET, "GC_ITEM_GET"},
      {GC::QUICKSLOT_ADD, "GC_QUICKSLOT_ADD"},
      {GC::QUICKSLOT_DEL, "GC_QUICKSLOT_DEL"},
      {GC::QUICKSLOT_SWAP, "GC_QUICKSLOT_SWAP"},
      {GC::EXCHANGE, "GC_EXCHANGE"},
      {GC::REFINE_INFORMATION, "GC_REFINE_INFORMATION"},
      {GC::DRAGON_SOUL_REFINE, "GC_DRAGON_SOUL_REFINE"},
      // Chat
      {CG::CHAT, "CG_CHAT"},
      {CG::WHISPER, "CG_WHISPER"},
      {GC::CHAT, "GC_CHAT"},
      {GC::WHISPER, "GC_WHISPER"},
      // Social
      {CG::PARTY_INVITE, "CG_PARTY_INVITE"},
      {CG::PARTY_INVITE_ANSWER, "CG_PARTY_INVITE_ANSWER"},
      {CG::PARTY_REMOVE, "CG_PARTY_REMOVE"},
      {CG::PARTY_SET_STATE, "CG_PARTY_SET_STATE"},
      {CG::PARTY_USE_SKILL, "CG_PARTY_USE_SKILL"},
      {CG::PARTY_PARAMETER, "CG_PARTY_PARAMETER"},
      {GC::PARTY_INVITE, "GC_PARTY_INVITE"},
      {GC::PARTY_ADD, "GC_PARTY_ADD"},
      {GC::PARTY_UPDATE, "GC_PARTY_UPDATE"},
      {GC::PARTY_REMOVE, "GC_PARTY_REMOVE"},
      {GC::PARTY_LINK, "GC_PARTY_LINK"},
      {GC::PARTY_UNLINK, "GC_PARTY_UNLINK"},
      {GC::PARTY_PARAMETER, "GC_PARTY_PARAMETER"},
      {CG::GUILD, "CG_GUILD"},
      {CG::ANSWER_MAKE_GUILD, "CG_ANSWER_MAKE_GUILD"},
      {CG::GUILD_SYMBOL_UPLOAD, "CG_GUILD_SYMBOL_UPLOAD"},
      {CG::SYMBOL_CRC, "CG_SYMBOL_CRC"},
      {GC::GUILD, "GC_GUILD"},
      {GC::REQUEST_MAKE_GUILD, "GC_REQUEST_MAKE_GUILD"},
      {GC::SYMBOL_DATA, "GC_SYMBOL_DATA"},
      {CG::MESSENGER, "CG_MESSENGER"},
      {GC::MESSENGER, "GC_MESSENGER"},
      {GC::LOVER_INFO, "GC_LOVER_INFO"},
      {GC::LOVE_POINT_UPDATE, "GC_LOVE_POINT_UPDATE"},
      // Shop / Trade
      {CG::SHOP, "CG_SHOP"},
      {CG::MYSHOP, "CG_MYSHOP"},
      {GC::SHOP, "GC_SHOP"},
      {GC::SHOP_SIGN, "GC_SHOP_SIGN"},
      {CG::SAFEBOX_CHECKIN, "CG_SAFEBOX_CHECKIN"},
      {CG::SAFEBOX_CHECKOUT, "CG_SAFEBOX_CHECKOUT"},
      {CG::SAFEBOX_ITEM_MOVE, "CG_SAFEBOX_ITEM_MOVE"},
      {GC::SAFEBOX_SET, "GC_SAFEBOX_SET"},
      {GC::SAFEBOX_DEL, "GC_SAFEBOX_DEL"},
      {GC::SAFEBOX_WRONG_PASSWORD, "GC_SAFEBOX_WRONG_PASSWORD"},
      {GC::SAFEBOX_SIZE, "GC_SAFEBOX_SIZE"},
      {GC::SAFEBOX_MONEY_CHANGE, "GC_SAFEBOX_MONEY_CHANGE"},
      {CG::MALL_CHECKOUT, "CG_MALL_CHECKOUT"},
      {GC::MALL_OPEN, "GC_MALL_OPEN"},
      {GC::MALL_SET, "GC_MALL_SET"},
      {GC::MALL_DEL, "GC_MALL_DEL"},
      // Quest
      {CG::SCRIPT_ANSWER, "CG_SCRIPT_ANSWER"},
      {CG::SCRIPT_BUTTON, "CG_SCRIPT_BUTTON"},
      {CG::SCRIPT_SELECT_ITEM, "CG_SCRIPT_SELECT_ITEM"},
      {CG::QUEST_INPUT_STRING, "CG_QUEST_INPUT_STRING"},
      {CG::QUEST_CONFIRM, "CG_QUEST_CONFIRM"},
      {CG::QUEST_CANCEL, "CG_QUEST_CANCEL"},
      {GC::SCRIPT, "GC_SCRIPT"},
      {GC::QUEST_CONFIRM, "GC_QUEST_CONFIRM"},
      {GC::QUEST_INFO, "GC_QUEST_INFO"},
      // UI / Effects
      {CG::TARGET, "CG_TARGET"},
      {CG::ON_CLICK, "CG_ON_CLICK"},
      {GC::TARGET, "GC_TARGET"},
      {GC::TARGET_UPDATE, "GC_TARGET_UPDATE"},
      {GC::TARGET_DELETE, "GC_TARGET_DELETE"},
      {GC::TARGET_CREATE_NEW, "GC_TARGET_CREATE_NEW"},
      {GC::AFFECT_ADD, "GC_AFFECT_ADD"},
      {GC::AFFECT_REMOVE, "GC_AFFECT_REMOVE"},
      {GC::SEPCIAL_EFFECT, "GC_SPECIAL_EFFECT"},
      {GC::SPECIFIC_EFFECT, "GC_SPECIFIC_EFFECT"},
      {GC::MOUNT, "GC_MOUNT"},
      {GC::OWNERSHIP, "GC_OWNERSHIP"},
      {GC::NPC_POSITION, "GC_NPC_POSITION"},
      {CG::CHARACTER_POSITION, "CG_CHARACTER_POSITION"},
      // World
      {CG::FISHING, "CG_FISHING"},
      {CG::DUNGEON, "CG_DUNGEON"},
      {CG::HACK, "CG_HACK"},
      {GC::FISHING, "GC_FISHING"},
      {GC::DUNGEON, "GC_DUNGEON"},
      {GC::LAND_LIST, "GC_LAND_LIST"},
      {GC::TIME, "GC_TIME"},
      {GC::CHANNEL, "GC_CHANNEL"},
      {GC::MARK_UPDATE, "GC_MARK_UPDATE"},
      // Guild Marks
      {CG::MARK_LOGIN, "CG_MARK_LOGIN"},
      {CG::MARK_CRCLIST, "CG_MARK_CRCLIST"},
      {CG::MARK_UPLOAD, "CG_MARK_UPLOAD"},
      {CG::MARK_IDXLIST, "CG_MARK_IDXLIST"},
      {GC::MARK_BLOCK, "GC_MARK_BLOCK"},
      {GC::MARK_IDXLIST, "GC_MARK_IDXLIST"},
  };

  auto it = s_headerNames.find(header);
  if (it != s_headerNames.end())
    return it->second;

  static thread_local char buf[16];
  snprintf(buf, sizeof(buf), "0x%04X", header);
  return buf;
}

#endif

bool CNetworkStream::Recv(int size) {
  if (!Peek(size))
    return false;

  m_recvBuf.Discard(static_cast<size_t>(size));
  return true;
}

static std::string dump_hex(const uint8_t *ptr, const std::size_t length) {
  if (!ptr || !length)
    return {};

  std::stringstream ss;

  std::vector<uint8_t> buffer(length);
  memcpy(&buffer[0], ptr, length);

  for (size_t i = 0; i < length; ++i)
    ss << std::hex << std::setfill('0') << std::setw(2) << (int)buffer.at(i)
       << ", ";

  const auto str = ss.str();
  return str.substr(0, str.size() - 2);
}

bool CNetworkStream::Recv(int size, char *pDestBuf) {
  if (!Peek(size, pDestBuf))
    return false;

#ifdef _PACKETDUMP
  if (size >= 2) {
    // MR-11: Separate packet dump log from the main log file
    const uint16_t kHeader = *reinterpret_cast<const uint16_t *>(pDestBuf);
    PacketDumpf("RECV< %s 0x%04X (%d bytes)", GetHeaderName(kHeader), kHeader,
                size);

    const auto contents =
        dump_hex(reinterpret_cast<const uint8_t *>(pDestBuf), size);
    PacketDumpf("%s", contents.c_str());
    // MR-11: -- END OF -- Separate packet dump log from the main log file
  }
#endif

  m_recvBuf.Discard(static_cast<size_t>(size));
  return true;
}

int CNetworkStream::__GetSendBufferSize() {
  return static_cast<int>(m_sendBuf.ReadableBytes());
}

bool CNetworkStream::Send(int size, const char *pSrcBuf) {
  // Track packet sends: detect new packet start by checking
  // [header:2][length:2] framing
  if (size >= 4) {
    const uint16_t wHeader = *reinterpret_cast<const uint16_t *>(pSrcBuf);
    const uint16_t wLength = *reinterpret_cast<const uint16_t *>(pSrcBuf + 2);

    if (wHeader != 0 && wLength >= 4) {
      auto &e = m_aSentPacketLog[m_dwSentPacketSeq % SENT_PACKET_LOG_SIZE];
      e.seq = m_dwSentPacketSeq;
      e.header = wHeader;
      e.length = wLength;
      m_dwSentPacketSeq++;
    }
  }

  m_sendBuf.EnsureWritable(static_cast<size_t>(size));

  // Copy data to send buffer
  std::memcpy(m_sendBuf.WritePtr(), pSrcBuf, static_cast<size_t>(size));

  // Encrypt in-place before committing
  if (m_secureCipher.IsActivated()) {
    m_secureCipher.EncryptInPlace(m_sendBuf.WritePtr(), size);
  }

  m_sendBuf.CommitWrite(static_cast<size_t>(size));

#ifdef _PACKETDUMP
  if (size >= 2) {
    // MR-11: Separate packet dump log from the main log file
    const uint16_t kHeader = *reinterpret_cast<const uint16_t *>(pSrcBuf);
    PacketDumpf("SEND> %s 0x%04X (%d bytes)", GetHeaderName(kHeader), kHeader,
                size);

    const auto contents =
        dump_hex(reinterpret_cast<const uint8_t *>(pSrcBuf), size);
    PacketDumpf("%s", contents.c_str());
    // MR-11: -- END OF -- Separate packet dump log from the main log file
  }
#endif

  return true;
}

bool CNetworkStream::Peek(int len, void *pDestBuf) {
  return Peek(len, (char *)pDestBuf);
}

bool CNetworkStream::Recv(int len, void *pDestBuf) {
  return Recv(len, (char *)pDestBuf);
}

bool CNetworkStream::SendFlush(int len, const void *pSrcBuf) {
  if (!Send(len, pSrcBuf))
    return false;

  return __SendInternalBuffer();
}

bool CNetworkStream::Send(int len, const void *pSrcBuf) {
  return Send(len, (const char *)pSrcBuf);
}

bool CNetworkStream::IsOnline() { return m_isOnline; }

bool CNetworkStream::OnProcess() { return true; }

void CNetworkStream::OnRemoteDisconnect() {}

void CNetworkStream::OnDisconnect() {}

void CNetworkStream::OnConnectSuccess() { Tracen("Succeed connecting."); }

void CNetworkStream::OnConnectFailure() { Tracen("Failed to connect."); }

// ---------------------------------------------------------------------------
// Control-plane packet handlers (shared by all connection types)
// ---------------------------------------------------------------------------

bool CNetworkStream::RecvKeyChallenge() {
  TPacketGCKeyChallenge packet;
  if (!Recv(sizeof(packet), &packet))
    return false;

  Tracen("KEY_CHALLENGE RECV");

  SecureCipher &cipher = GetSecureCipher();
  if (!cipher.Initialize()) {
    TraceError("SecureCipher initialization failed");
    Disconnect();
    return false;
  }

  if (!cipher.ComputeClientKeys(packet.server_pk)) {
    TraceError("Failed to compute client session keys");
    Disconnect();
    return false;
  }

  TPacketCGKeyResponse response;
  response.header = CG::KEY_RESPONSE;
  response.length = sizeof(response);
  cipher.GetPublicKey(response.client_pk);
  cipher.ComputeChallengeResponse(packet.challenge,
                                  response.challenge_response);

  if (!Send(sizeof(response), &response)) {
    TraceError("Failed to send key response");
    return false;
  }

  Tracen("KEY_RESPONSE SENT");
  return true;
}

bool CNetworkStream::RecvKeyComplete() {
  TPacketGCKeyComplete packet;
  if (!Recv(sizeof(packet), &packet))
    return false;

  SecureCipher &cipher = GetSecureCipher();

  uint8_t decrypted_token[SecureCipher::SESSION_TOKEN_SIZE];
  if (!cipher.DecryptToken(packet.encrypted_token,
                           sizeof(packet.encrypted_token), packet.nonce,
                           decrypted_token)) {
    TraceError("Failed to decrypt session token");
    Disconnect();
    return false;
  }

  cipher.SetSessionToken(decrypted_token);
  cipher.SetActivated(true);
  DecryptPendingRecvData();

  return true;
}

bool CNetworkStream::RecvPingPacket() {
  TPacketGCPing kPacketPing;
  if (!Recv(sizeof(kPacketPing), &kPacketPing))
    return false;

  return SendPongPacket();
}

bool CNetworkStream::SendPongPacket() {
  TPacketCGPong kPacketPong;
  kPacketPong.header = CG::PONG;
  kPacketPong.length = sizeof(kPacketPong);

  if (!Send(sizeof(kPacketPong), &kPacketPong))
    return false;

  return true;
}

// ---------------------------------------------------------------------------

CNetworkStream::CNetworkStream() {
  m_sock = INVALID_SOCKET;

  m_isOnline = false;
  m_connectLimitTime = 0;
}

CNetworkStream::~CNetworkStream() { Clear(); }
