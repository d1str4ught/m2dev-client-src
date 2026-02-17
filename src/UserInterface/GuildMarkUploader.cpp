#include "StdAfx.h"
#include "GuildMarkUploader.h"
#include "Packet.h"

#include "stb_image.h"
#include "stb_image_write.h"

#ifdef __VTUNE__
#else

CGuildMarkUploader::CGuildMarkUploader() {
  SetRecvBufferSize(1024);
  SetSendBufferSize(1024);
  __Inialize();
}

CGuildMarkUploader::~CGuildMarkUploader() { __OfflineState_Set(); }

void CGuildMarkUploader::Disconnect() { __OfflineState_Set(); }

bool CGuildMarkUploader::IsCompleteUploading() {
  return STATE_OFFLINE == m_eState;
}

bool CGuildMarkUploader::__Save(const char *c_szFileName) {
  /*
  int width = CGuildMarkImage::WIDTH;
  int height = CGuildMarkImage::HEIGHT;
  std::vector<unsigned char> rgba(width * height * 4);

  for (int i = 0; i < width * height; ++i) {
          rgba[i * 4 + 0] = m_kMark.m_apxBuf[i * 4 + 2]; // R
          rgba[i * 4 + 1] = m_kMark.m_apxBuf[i * 4 + 1]; // G
          rgba[i * 4 + 2] = m_kMark.m_apxBuf[i * 4 + 0]; // B
          rgba[i * 4 + 3] = m_kMark.m_apxBuf[i * 4 + 3]; // A
  }

  // Save as PNG
  if (!stbi_write_png(c_szFileName, width, height, 4, rgba.data(), width * 4)) {
          return false;
  }

  return true;
  */
  return true;
}

bool CGuildMarkUploader::__Load(const char *c_szFileName, UINT *peError) {
  int width, height, channels;
  unsigned char *data =
      stbi_load(c_szFileName, &width, &height, &channels, 4); // force RGBA

  if (!data) {
    *peError = ERROR_LOAD;
    return false;
  }

  if (width != SGuildMark::WIDTH) {
    stbi_image_free(data);
    *peError = ERROR_WIDTH;
    return false;
  }

  if (height != SGuildMark::HEIGHT) {
    stbi_image_free(data);
    *peError = ERROR_HEIGHT;
    return false;
  }

  // Copy into our mark buffer (BGRA expected)
  for (uint32_t i = 0; i < width * height; ++i) {
    const uint8_t R = data[i * 4 + 0];
    const uint8_t G = data[i * 4 + 1];
    const uint8_t B = data[i * 4 + 2];
    const uint8_t A = data[i * 4 + 3];

    m_kMark.m_apxBuf[i] = (uint32_t(A) << 24) | (uint32_t(R) << 16) |
                          (uint32_t(G) << 8) | uint32_t(B);
  }

  stbi_image_free(data);
  return true;
}

bool CGuildMarkUploader::__LoadSymbol(const char *c_szFileName, UINT *peError) {
  int width, height, channels;
  unsigned char *data = stbi_load(c_szFileName, &width, &height, &channels, 4);

  if (!data) {
    *peError = ERROR_LOAD;
    return false;
  }

  if (width != 64) {
    stbi_image_free(data);
    *peError = ERROR_WIDTH;
    return false;
  }

  if (height != 128) {
    stbi_image_free(data);
    *peError = ERROR_HEIGHT;
    return false;
  }

  stbi_image_free(data);

  // Read raw file into m_symbolBuf
  FILE *file = fopen(c_szFileName, "rb");
  if (!file) {
    *peError = ERROR_LOAD;
    return false;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  m_symbolBuf.resize(static_cast<size_t>(fileSize));
  fread(m_symbolBuf.data(), m_symbolBuf.size(), 1, file);
  fclose(file);

  m_dwSymbolCRC32 = GetFileCRC32(c_szFileName);
  return true;
}

bool CGuildMarkUploader::Connect(const CNetworkAddress &c_rkNetAddr,
                                 DWORD dwHandle, DWORD dwRandomKey,
                                 DWORD dwGuildID, const char *c_szFileName,
                                 UINT *peError) {
  __OfflineState_Set();
  SetRecvBufferSize(1024);
  SetSendBufferSize(1024);

  if (!CNetworkStream::Connect(c_rkNetAddr)) {
    *peError = ERROR_CONNECT;
    return false;
  }

  m_dwSendType = SEND_TYPE_MARK;
  m_dwHandle = dwHandle;
  m_dwRandomKey = dwRandomKey;
  m_dwGuildID = dwGuildID;

  if (!__Load(c_szFileName, peError))
    return false;

  // if (!__Save(CGraphicMarkInstance::GetImageFileName().c_str()))
  //	return false;
  // CGraphicMarkInstance::ReloadImageFile();
  return true;
}

bool CGuildMarkUploader::ConnectToSendSymbol(const CNetworkAddress &c_rkNetAddr,
                                             DWORD dwHandle, DWORD dwRandomKey,
                                             DWORD dwGuildID,
                                             const char *c_szFileName,
                                             UINT *peError) {
  __OfflineState_Set();
  SetRecvBufferSize(1024);
  SetSendBufferSize(64 * 1024);

  if (!CNetworkStream::Connect(c_rkNetAddr)) {
    *peError = ERROR_CONNECT;
    return false;
  }

  m_dwSendType = SEND_TYPE_SYMBOL;
  m_dwHandle = dwHandle;
  m_dwRandomKey = dwRandomKey;
  m_dwGuildID = dwGuildID;

  if (!__LoadSymbol(c_szFileName, peError))
    return false;

  return true;
}

void CGuildMarkUploader::Process() {
  CNetworkStream::Process();

  if (!__StateProcess()) {
    __OfflineState_Set();
    Disconnect();
  }
}

void CGuildMarkUploader::OnConnectFailure() { __OfflineState_Set(); }

void CGuildMarkUploader::OnConnectSuccess() { __LoginState_Set(); }

void CGuildMarkUploader::OnRemoteDisconnect() { __OfflineState_Set(); }

void CGuildMarkUploader::OnDisconnect() { __OfflineState_Set(); }

void CGuildMarkUploader::__Inialize() {
  m_eState = STATE_OFFLINE;

  m_dwGuildID = 0;
  m_dwHandle = 0;
  m_dwRandomKey = 0;

  m_symbolBuf.clear();
}

bool CGuildMarkUploader::__StateProcess() {
  switch (m_eState) {
  case STATE_LOGIN:
    return __LoginState_Process();
    break;
  }

  return true;
}

void CGuildMarkUploader::__OfflineState_Set() { __Inialize(); }

void CGuildMarkUploader::__CompleteState_Set() {
  m_eState = STATE_COMPLETE;

  __OfflineState_Set();
}

void CGuildMarkUploader::__LoginState_Set() { m_eState = STATE_LOGIN; }

bool CGuildMarkUploader::__LoginState_Process() {
  if (!__AnalyzePacket(GC::PHASE, sizeof(TPacketGCPhase),
                       &CGuildMarkUploader::__LoginState_RecvPhase))
    return false;

  if (!__AnalyzePacket(GC::PING, sizeof(TPacketGCPing),
                       &CGuildMarkUploader::__LoginState_RecvPingBase))
    return false;

  if (!__AnalyzePacket(GC::KEY_CHALLENGE, sizeof(TPacketGCKeyChallenge),
                       &CGuildMarkUploader::__LoginState_RecvKeyChallengeBase))
    return false;

  if (!__AnalyzePacket(
          GC::KEY_COMPLETE, sizeof(TPacketGCKeyComplete),
          &CGuildMarkUploader::__LoginState_RecvKeyCompleteAndLogin))
    return false;

  return true;
}

bool CGuildMarkUploader::__SendMarkPacket() {
  TPacketCGMarkUpload kPacketMarkUpload;
  kPacketMarkUpload.header = CG::MARK_UPLOAD;
  kPacketMarkUpload.length = sizeof(kPacketMarkUpload);
  kPacketMarkUpload.gid = m_dwGuildID;

  assert(sizeof(kPacketMarkUpload.image) == sizeof(m_kMark.m_apxBuf));
  memcpy(kPacketMarkUpload.image, m_kMark.m_apxBuf,
         sizeof(kPacketMarkUpload.image));

  if (!Send(sizeof(kPacketMarkUpload), &kPacketMarkUpload))
    return false;

  return true;
}
bool CGuildMarkUploader::__SendSymbolPacket() {
  if (m_symbolBuf.empty())
    return false;

  TPacketCGSymbolUpload kPacketSymbolUpload;
  kPacketSymbolUpload.header = CG::GUILD_SYMBOL_UPLOAD;
  kPacketSymbolUpload.handle = m_dwGuildID;
  kPacketSymbolUpload.length =
      sizeof(TPacketCGSymbolUpload) + static_cast<uint16_t>(m_symbolBuf.size());

  if (!Send(sizeof(TPacketCGSymbolUpload), &kPacketSymbolUpload))
    return false;
  if (!Send(static_cast<int>(m_symbolBuf.size()), m_symbolBuf.data()))
    return false;

#ifdef _DEBUG
  printf("__SendSymbolPacket : [GuildID:%d/PacketSize:%d/BufSize:%d/CRC:%d]\n",
         m_dwGuildID, kPacketSymbolUpload.length, (int)m_symbolBuf.size(),
         m_dwSymbolCRC32);
#endif

  CNetworkStream::__SendInternalBuffer();
  __CompleteState_Set();

  return true;
}

bool CGuildMarkUploader::__LoginState_RecvPhase() {
  TPacketGCPhase kPacketPhase;
  if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
    return false;

  if (kPacketPhase.phase == PHASE_LOGIN) {
    if (SEND_TYPE_MARK == m_dwSendType) {
      if (!__SendMarkPacket())
        return false;
    } else if (SEND_TYPE_SYMBOL == m_dwSendType) {
      if (!__SendSymbolPacket())
        return false;
    }
  }

  return true;
}

// Ping/pong and key challenge now handled by CNetworkStream base class.
// Thin wrappers in the header delegate __AnalyzePacket dispatch to those base
// methods.

// RecvKeyComplete + mark-specific login authentication
bool CGuildMarkUploader::__LoginState_RecvKeyCompleteAndLogin() {
  if (!CNetworkStream::RecvKeyComplete())
    return false;

  // Send mark login (authentication) now that secure channel is established
  TPacketCGMarkLogin kPacketMarkLogin;
  kPacketMarkLogin.header = CG::MARK_LOGIN;
  kPacketMarkLogin.length = sizeof(kPacketMarkLogin);
  kPacketMarkLogin.handle = m_dwHandle;
  kPacketMarkLogin.random_key = m_dwRandomKey;

  if (!Send(sizeof(kPacketMarkLogin), &kPacketMarkLogin))
    return false;

  return true;
}

bool CGuildMarkUploader::__AnalyzePacket(
    UINT uHeader, UINT uPacketSize,
    bool (CGuildMarkUploader::*pfnDispatchPacket)()) {
  uint16_t wHeader;
  if (!Peek(sizeof(wHeader), &wHeader))
    return true;

  if (wHeader != uHeader)
    return true;

  if (!Peek(uPacketSize))
    return true;

  return (this->*pfnDispatchPacket)();
}
#endif
