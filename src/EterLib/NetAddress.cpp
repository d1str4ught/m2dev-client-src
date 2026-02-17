#include "StdAfx.h"
#include "NetAddress.h"

#ifndef VC_EXTRALEAN

bool CNetworkAddress::GetHostName(char *szName, int size) {
  if (gethostname(szName, size) == SOCKET_ERROR)
    return false;
  return true;
}

CNetworkAddress::CNetworkAddress() { Clear(); }

CNetworkAddress::~CNetworkAddress() {}

CNetworkAddress::operator const SOCKADDR_IN &() const { return m_sockAddrIn; }

void CNetworkAddress::Clear() {
  memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
  m_sockAddrIn.sin_family = AF_INET;
}

bool CNetworkAddress::IsIP(const char *c_szAddr) {
  if (c_szAddr[0] < '0' || c_szAddr[0] > '9')
    return false;
  return true;
}

bool CNetworkAddress::Set(const char *c_szAddr, int port) {
  if (IsIP(c_szAddr)) {
    SetIP(c_szAddr);
  } else {
    if (!SetDNS(c_szAddr))
      return false;
  }

  SetPort(port);
  return true;
}

void CNetworkAddress::SetLocalIP() { SetIP(INADDR_ANY); }

void CNetworkAddress::SetIP(DWORD ip) {
  m_sockAddrIn.sin_addr.s_addr = htonl(ip);
}

void CNetworkAddress::SetIP(const char *c_szIP) {
  m_sockAddrIn.sin_addr.s_addr = inet_addr(c_szIP);
}

bool CNetworkAddress::SetDNS(const char *c_szDNS) {
  HOSTENT *pHostent = gethostbyname(c_szDNS);
  if (!pHostent)
    return false;
  memcpy(&m_sockAddrIn.sin_addr, pHostent->h_addr,
         sizeof(m_sockAddrIn.sin_addr));
  return true;
}

void CNetworkAddress::SetPort(int port) { m_sockAddrIn.sin_port = htons(port); }

int CNetworkAddress::GetSize() { return sizeof(m_sockAddrIn); }

DWORD CNetworkAddress::GetIP() { return ntohl(m_sockAddrIn.sin_addr.s_addr); }

void CNetworkAddress::GetIP(char *szIP, int len) {
  BYTE IPs[4];
  *((DWORD *)IPs) = m_sockAddrIn.sin_addr.s_addr;

  _snprintf(szIP, len, "%d.%d.%d.%d", IPs[0], IPs[1], IPs[2], IPs[3]);
}

int CNetworkAddress::GetPort() { return ntohs(m_sockAddrIn.sin_port); }

#endif