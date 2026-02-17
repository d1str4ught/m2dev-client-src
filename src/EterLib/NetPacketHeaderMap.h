#pragma once

#include <map>

class CNetworkPacketHeaderMap {
public:
  typedef struct SPacketType {
    SPacketType(int iSize = 0) : iPacketSize(iSize) {}

    int iPacketSize; // Expected/minimum packet size (actual size from wire
                     // length field)
  } TPacketType;

public:
  CNetworkPacketHeaderMap();
  virtual ~CNetworkPacketHeaderMap();

  void Set(int header, TPacketType rPacketType);
  bool Get(int header, TPacketType *pPacketType);

protected:
  std::map<int, TPacketType> m_headerMap;
};
