#include "StdAfx.h"
#include "Packet.h"
#include "ServerStateChecker.h"

#include "EterLib/NetAddress.h"

#pragma pack(push)
#pragma pack(1)

typedef unsigned char ServerStateChecker_Header;
typedef unsigned long ServerStateChecker_Key;
typedef unsigned long ServerStateChecker_Index;
typedef unsigned char ServerStateChecker_State;

#pragma pack(pop)

CServerStateChecker::CServerStateChecker() { Initialize(); }

CServerStateChecker::~CServerStateChecker() {
  Initialize();
  m_poWnd = NULL;
}

void CServerStateChecker::Create(PyObject *poWnd) { m_poWnd = poWnd; }

void CServerStateChecker::AddChannel(UINT uServerIndex, const char *c_szAddr,
                                     UINT uPort) {
  TChannel c;
  c.uServerIndex = uServerIndex;
  c.c_szAddr = c_szAddr;
  c.uPort = uPort;

  m_lstChannel.push_back(c);
}

void CServerStateChecker::Request() {
  if (m_lstChannel.empty()) {
    return;
  }

  if (!m_kStream.Connect(m_lstChannel.begin()->c_szAddr,
                         m_lstChannel.begin()->uPort)) {
    for (std::list<TChannel>::const_iterator it = m_lstChannel.begin();
         it != m_lstChannel.end(); ++it) {
      PyCallClassMemberFunc(m_poWnd, "NotifyChannelState",
                            Py_BuildValue("(ii)", it->uServerIndex, 0));
    }
    return;
  }
  m_kStream.ClearRecvBuffer();
  m_kStream.SetSendBufferSize(1024);
  m_kStream.SetRecvBufferSize(1024);

  TPacketGCBlank pack;
  pack.header = CG::STATE_CHECKER;
  pack.length = sizeof(pack);
  if (!m_kStream.Send(sizeof(pack), &pack)) {
    for (std::list<TChannel>::const_iterator it = m_lstChannel.begin();
         it != m_lstChannel.end(); ++it) {
      PyCallClassMemberFunc(m_poWnd, "NotifyChannelState",
                            Py_BuildValue("(ii)", it->uServerIndex, 0));
    }
    Initialize();
    return;
  }
}

void CServerStateChecker::Update() {
  m_kStream.Process();

  // Skip packets until we find GC::RESPOND_CHANNELSTATUS.
  // The server may send other packets first, which we need to discard.
  while (true) {
    TDynamicSizePacketHeader packHeader;
    if (!m_kStream.Peek(sizeof(packHeader), &packHeader))
      return;

    if (packHeader.header == GC::RESPOND_CHANNELSTATUS) {
      // Consume the header we peeked
      m_kStream.Recv(sizeof(packHeader));
      break;
    }

    // Not our packet — skip it using its length field
    if (packHeader.length < sizeof(packHeader))
      return;

    if (!m_kStream.Peek(packHeader.length))
      return; // Not enough data yet

    m_kStream.Recv(packHeader.length);
  }

  int nSize;
  if (!m_kStream.Recv(sizeof(nSize), &nSize)) {
    return;
  }
  for (int i = 0; i < nSize; i++) {
    TChannelStatus channelStatus;
    if (!m_kStream.Recv(sizeof(channelStatus), &channelStatus)) {
      return;
    }
    for (std::list<TChannel>::const_iterator it = m_lstChannel.begin();
         it != m_lstChannel.end(); ++it) {
      if (channelStatus.nPort == it->uPort) {
        PyCallClassMemberFunc(
            m_poWnd, "NotifyChannelState",
            Py_BuildValue("(ii)", it->uServerIndex, channelStatus.bStatus));
        break;
      }
    }
  }
  Initialize();
}

void CServerStateChecker::Initialize() {
  m_lstChannel.clear();
  m_kStream.Disconnect();
}