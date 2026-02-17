#include "StdAfx.h"
#include "GuildMarkDownloader.h"
#include "PythonCharacterManager.h"
#include "PythonTextTail.h"
#include "Packet.h"

// MARK_BUG_FIX
struct SMarkIndex {
  WORD guild_id;
  WORD mark_id;
};

// END_OFMARK_BUG_FIX

CGuildMarkDownloader::CGuildMarkDownloader() {
  SetRecvBufferSize(640 * 1024);
  SetSendBufferSize(1024);
  __Initialize();
}

CGuildMarkDownloader::~CGuildMarkDownloader() { __OfflineState_Set(); }

bool CGuildMarkDownloader::Connect(const CNetworkAddress &c_rkNetAddr,
                                   DWORD dwHandle, DWORD dwRandomKey) {
  __OfflineState_Set();

  m_dwHandle = dwHandle;
  m_dwRandomKey = dwRandomKey;
  m_dwTodo = TODO_RECV_MARK;
  return CNetworkStream::Connect(c_rkNetAddr);
}

bool CGuildMarkDownloader::ConnectToRecvSymbol(
    const CNetworkAddress &c_rkNetAddr, DWORD dwHandle, DWORD dwRandomKey,
    const std::vector<DWORD> &c_rkVec_dwGuildID) {
  __OfflineState_Set();

  m_dwHandle = dwHandle;
  m_dwRandomKey = dwRandomKey;
  m_dwTodo = TODO_RECV_SYMBOL;
  m_kVec_dwGuildID = c_rkVec_dwGuildID;
  return CNetworkStream::Connect(c_rkNetAddr);
}

void CGuildMarkDownloader::Process() {
  CNetworkStream::Process();

  if (!__StateProcess()) {
    __OfflineState_Set();
    Disconnect();
  }
}

void CGuildMarkDownloader::OnConnectFailure() { __OfflineState_Set(); }

void CGuildMarkDownloader::OnConnectSuccess() { __LoginState_Set(); }

void CGuildMarkDownloader::OnRemoteDisconnect() { __OfflineState_Set(); }

void CGuildMarkDownloader::OnDisconnect() { __OfflineState_Set(); }

void CGuildMarkDownloader::__Initialize() {
  m_eState = STATE_OFFLINE;
  m_pkMarkMgr = NULL;
  m_currentRequestingImageIndex = 0;
  m_dwBlockIndex = 0;
  m_dwBlockDataPos = 0;
  m_dwBlockDataSize = 0;

  m_dwHandle = 0;
  m_dwRandomKey = 0;
  m_dwTodo = TODO_RECV_NONE;
  m_kVec_dwGuildID.clear();
  m_setUpdatedImageIndices.clear();
}

bool CGuildMarkDownloader::__StateProcess() {
  switch (m_eState) {
  case STATE_LOGIN:
    return __LoginState_Process();
    break;
  case STATE_COMPLETE:
    return false;
  }

  return true;
}

void CGuildMarkDownloader::__OfflineState_Set() { __Initialize(); }

void CGuildMarkDownloader::__CompleteState_Set() {
  m_eState = STATE_COMPLETE;

  // Reload all updated mark images before refreshing guild marks
  // Must use file load mode since mark images are stored on disk, not in pack
  CPackManager::instance().SetFileLoadMode();

  for (std::set<DWORD>::iterator it = m_setUpdatedImageIndices.begin();
       it != m_setUpdatedImageIndices.end(); ++it) {
    std::string imagePath;
    if (CGuildMarkManager::Instance().GetMarkImageFilename(*it, imagePath)) {
      CResource *pResource =
          CResourceManager::Instance().GetResourcePointer(imagePath.c_str());
      if (pResource && pResource->IsType(CGraphicImage::Type())) {
        static_cast<CGraphicImage *>(pResource)->Reload();
      }
    }
  }

  CPackManager::instance().SetPackLoadMode();
  m_setUpdatedImageIndices.clear();

  // Refresh all mark instances to use the updated textures
  CPythonTextTail::Instance().RefreshAllGuildMark();
}

void CGuildMarkDownloader::__LoginState_Set() { m_eState = STATE_LOGIN; }

bool CGuildMarkDownloader::__LoginState_Process() {
  uint16_t header;

  if (!Peek(sizeof(uint16_t), &header))
    return true;

  if (IsSecurityMode()) {
    if (0 == header) {
      if (!Recv(sizeof(header), &header))
        return false;

      return true;
    }
  }

  UINT needPacketSize = __GetPacketSize(header);

  if (!needPacketSize)
    return false;

  if (!Peek(needPacketSize))
    return true;

  __DispatchPacket(header);
  return true;
}

// MARK_BUG_FIX
UINT CGuildMarkDownloader::__GetPacketSize(UINT header) {
  switch (header) {
  case GC::PHASE:
    return sizeof(TPacketGCPhase);
  case GC::PING:
    return sizeof(TPacketGCPing);
  case GC::MARK_IDXLIST:
    return sizeof(TPacketGCMarkIDXList);
  case GC::MARK_BLOCK:
    return sizeof(TPacketGCMarkBlock);
  case GC::SYMBOL_DATA:
    return sizeof(TPacketGCGuildSymbolData);
  case GC::MARK_DIFF_DATA:
    return sizeof(BYTE);
  case GC::KEY_CHALLENGE:
    return sizeof(TPacketGCKeyChallenge);
  case GC::KEY_COMPLETE:
    return sizeof(TPacketGCKeyComplete);
  }
  return 0;
}

bool CGuildMarkDownloader::__DispatchPacket(UINT header) {
  switch (header) {
  case GC::PHASE:
    return __LoginState_RecvPhase();
  case GC::PING:
    return RecvPingPacket();
  case GC::MARK_IDXLIST:
    return __LoginState_RecvMarkIndex();
  case GC::MARK_BLOCK:
    return __LoginState_RecvMarkBlock();
  case GC::SYMBOL_DATA:
    return __LoginState_RecvSymbolData();
  case GC::MARK_DIFF_DATA:
    return true;
  case GC::KEY_CHALLENGE:
    return RecvKeyChallenge();
  case GC::KEY_COMPLETE:
    return __LoginState_RecvKeyCompleteAndLogin();
  }
  return false;
}
// END_OF_MARK_BUG_FIX

// Ping/pong now handled by CNetworkStream::RecvPingPacket()

bool CGuildMarkDownloader::__LoginState_RecvPhase() {
  TPacketGCPhase kPacketPhase;

  if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
    return false;

  if (kPacketPhase.phase == PHASE_LOGIN) {
    switch (m_dwTodo) {
    case TODO_RECV_NONE: {
      assert(
          !"CGuildMarkDownloader::__LoginState_RecvPhase - Todo type is none");
      break;
    }
    case TODO_RECV_MARK: {
      // MARK_BUG_FIX
      if (!__SendMarkIDXList())
        return false;
      // END_OF_MARK_BUG_FIX
      break;
    }
    case TODO_RECV_SYMBOL: {
      if (!__SendSymbolCRCList())
        return false;
      break;
    }
    }
  }

  return true;
}

// MARK_BUG_FIX
bool CGuildMarkDownloader::__SendMarkIDXList() {
  TPacketCGMarkIDXList kPacketMarkIDXList;
  kPacketMarkIDXList.header = CG::MARK_IDXLIST;
  kPacketMarkIDXList.length = sizeof(kPacketMarkIDXList);
  if (!Send(sizeof(kPacketMarkIDXList), &kPacketMarkIDXList))
    return false;

  return true;
}

bool CGuildMarkDownloader::__LoginState_RecvMarkIndex() {
  TPacketGCMarkIDXList kPacketMarkIndex;

  if (!Peek(sizeof(kPacketMarkIndex), &kPacketMarkIndex))
    return false;

  // DWORD bufSize = sizeof(WORD) * 2 * kPacketMarkIndex.count;

  if (!Peek(kPacketMarkIndex.bufSize))
    return false;

  Recv(sizeof(kPacketMarkIndex));

  WORD guildID, markID;

  for (DWORD i = 0; i < kPacketMarkIndex.count; ++i) {
    Recv(sizeof(WORD), &guildID);
    Recv(sizeof(WORD), &markID);

    // 길드ID -> 마크ID 인덱스 등록
    CGuildMarkManager::Instance().AddMarkIDByGuildID(guildID, markID);
  }

  // 모든 마크 이미지 파일을 로드한다. (파일이 없으면 만들어짐)
  CGuildMarkManager::Instance().LoadMarkImages();

  m_currentRequestingImageIndex = 0;
  __SendMarkCRCList();
  return true;
}

bool CGuildMarkDownloader::__SendMarkCRCList() {
  TPacketCGMarkCRCList kPacketMarkCRCList;

  if (!CGuildMarkManager::Instance().GetBlockCRCList(
          m_currentRequestingImageIndex, kPacketMarkCRCList.crclist)) {
    __CompleteState_Set();
  } else {
    kPacketMarkCRCList.header = CG::MARK_CRCLIST;
    kPacketMarkCRCList.length = sizeof(kPacketMarkCRCList);
    kPacketMarkCRCList.imgIdx = m_currentRequestingImageIndex;
    ++m_currentRequestingImageIndex;

    if (!Send(sizeof(kPacketMarkCRCList), &kPacketMarkCRCList))
      return false;
  }
  return true;
}

bool CGuildMarkDownloader::__LoginState_RecvMarkBlock() {
  TPacketGCMarkBlock kPacket;

  if (!Peek(sizeof(kPacket), &kPacket))
    return false;

  if (!Peek(kPacket.bufSize))
    return false;

  Recv(sizeof(kPacket));

  BYTE posBlock;
  DWORD compSize;
  char compBuf[SGuildMarkBlock::MAX_COMP_SIZE];

  for (DWORD i = 0; i < kPacket.count; ++i) {
    Recv(sizeof(BYTE), &posBlock);
    Recv(sizeof(DWORD), &compSize);

    if (compSize > SGuildMarkBlock::MAX_COMP_SIZE) {
      TraceError("RecvMarkBlock: data corrupted");
      Recv(compSize);
    } else {
      Recv(compSize, compBuf);
      // 압축된 이미지를 실제로 저장한다. CRC등 여러가지 정보가 함께 빌드된다.
      CGuildMarkManager::Instance().SaveBlockFromCompressedData(
          kPacket.imgIdx, posBlock, (const uint8_t *)compBuf, compSize);
    }
  }

  if (kPacket.count > 0) {
    // 마크 이미지 저장
    CGuildMarkManager::Instance().SaveMarkImage(kPacket.imgIdx);

    // Track updated image index for deferred reload
    m_setUpdatedImageIndices.insert(kPacket.imgIdx);
  }

  // 더 요청할 것이 있으면 요청하고 아니면 이미지를 저장하고 종료
  if (m_currentRequestingImageIndex <
      CGuildMarkManager::Instance().GetMarkImageCount())
    __SendMarkCRCList();
  else
    __CompleteState_Set();

  return true;
}
// END_OF_MARK_BUG_FIX

// RecvKeyChallenge now handled by CNetworkStream::RecvKeyChallenge()

// RecvKeyComplete + mark-specific login authentication
bool CGuildMarkDownloader::__LoginState_RecvKeyCompleteAndLogin() {
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

bool CGuildMarkDownloader::__SendSymbolCRCList() {
  for (DWORD i = 0; i < m_kVec_dwGuildID.size(); ++i) {
    TPacketCGSymbolCRC kSymbolCRCPacket;
    kSymbolCRCPacket.header = CG::SYMBOL_CRC;
    kSymbolCRCPacket.length = sizeof(kSymbolCRCPacket);
    kSymbolCRCPacket.dwGuildID = m_kVec_dwGuildID[i];

    std::string strFileName = GetGuildSymbolFileName(m_kVec_dwGuildID[i]);
    kSymbolCRCPacket.dwCRC = GetFileCRC32(strFileName.c_str());
    kSymbolCRCPacket.dwSize = GetFileSize(strFileName.c_str());
#ifdef _DEBUG
    printf("__SendSymbolCRCList [GuildID:%d / CRC:%u]\n", m_kVec_dwGuildID[i],
           kSymbolCRCPacket.dwCRC);
#endif
    if (!Send(sizeof(kSymbolCRCPacket), &kSymbolCRCPacket))
      return false;
  }

  return true;
}

bool CGuildMarkDownloader::__LoginState_RecvSymbolData() {
  TPacketGCBlankDynamic packet;
  if (!Peek(sizeof(TPacketGCBlankDynamic), &packet))
    return true;

#ifdef _DEBUG
  printf("__LoginState_RecvSymbolData [%d/%d]\n", GetRecvBufferSize(),
         packet.length);
#endif
  if (packet.length > GetRecvBufferSize())
    return true;

  //////////////////////////////////////////////////////////////

  TPacketGCGuildSymbolData kPacketSymbolData;
  if (!Recv(sizeof(kPacketSymbolData), &kPacketSymbolData))
    return false;

  WORD wDataSize = kPacketSymbolData.length - sizeof(kPacketSymbolData);
  DWORD dwGuildID = kPacketSymbolData.guild_id;
  BYTE *pbyBuf = new BYTE[wDataSize];

  if (!Recv(wDataSize, pbyBuf)) {
    delete[] pbyBuf;
    return false;
  }

  MyCreateDirectory(g_strGuildSymbolPathName.c_str());

  std::string strFileName = GetGuildSymbolFileName(dwGuildID);

  FILE *File = fopen(strFileName.c_str(), "wb");
  if (!File) {
    delete[] pbyBuf;
    return false;
  }
  fwrite(pbyBuf, wDataSize, 1, File);
  fclose(File);

#ifdef _DEBUG
  printf("__LoginState_RecvSymbolData(filename:%s, datasize:%d, guildid:%d)\n",
         strFileName.c_str(), wDataSize, dwGuildID);
#endif

  delete[] pbyBuf;
  return true;
}
