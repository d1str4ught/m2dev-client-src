#pragma once

#include "EterBase/SecureCipher.h"
#include "NetAddress.h"
#include "RingBuffer.h"
#include "ControlPackets.h"

class CNetworkStream {
public:
  CNetworkStream();
  virtual ~CNetworkStream();

  void SetRecvBufferSize(int recvBufSize);
  void SetSendBufferSize(int sendBufSize);

  bool IsSecurityMode();

  int GetRecvBufferSize();

  void Clear();
  void ClearRecvBuffer();

  void Process();

  bool Connect(const CNetworkAddress &c_rkNetAddr, int limitSec = 3);
  bool Connect(const char *c_szAddr, int port, int limitSec = 3);
  bool Connect(DWORD dwAddr, int port, int limitSec = 3);
  void Disconnect();

  bool Peek(int len);
  bool Peek(int len, char *pDestBuf);
  bool Recv(int len);
  bool Recv(int len, char *pDestBuf);
  bool Send(int len, const char *pSrcBuf);

  bool Peek(int len, void *pDestBuf);
  bool Recv(int len, void *pDestBuf);

  bool Send(int len, const void *pSrcBuf);
  bool SendFlush(int len, const void *pSrcBuf);

  bool IsOnline();

protected:
  virtual void OnConnectSuccess();
  virtual void OnConnectFailure();
  virtual void OnRemoteDisconnect();
  virtual void OnDisconnect();
  virtual bool OnProcess();

  bool __SendInternalBuffer();
  bool __RecvInternalBuffer();

  int __GetSendBufferSize();

  // Secure cipher methods (libsodium)
  SecureCipher &GetSecureCipher() { return m_secureCipher; }
  bool IsSecureCipherActivated() const { return m_secureCipher.IsActivated(); }
  void ActivateSecureCipher() { m_secureCipher.SetActivated(true); }

  // Decrypt any unprocessed data already in the recv buffer
  // Must be called after activating the cipher mid-stream
  void DecryptPendingRecvData();

  // Control-plane packet handlers (shared by all connection types)
  virtual bool RecvKeyChallenge();
  virtual bool RecvKeyComplete();
  bool RecvPingPacket();
  bool SendPongPacket();

  // Packet send tracking (for debug sequence correlation)
protected:
  struct SentPacketLogEntry {
    uint32_t seq;
    uint16_t header;
    uint16_t length;
  };

  static constexpr size_t SENT_PACKET_LOG_SIZE = 32;

  SentPacketLogEntry m_aSentPacketLog[SENT_PACKET_LOG_SIZE] = {};
  uint32_t m_dwSentPacketSeq = 0;

private:
  time_t m_connectLimitTime;

  RingBuffer m_recvBuf;
  RingBuffer m_sendBuf;

  bool m_isOnline;

  // Secure cipher (libsodium/XChaCha20-Poly1305)
  SecureCipher m_secureCipher;

  SOCKET m_sock;

  CNetworkAddress m_addr;
};
