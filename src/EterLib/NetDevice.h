#pragma once

class CNetworkDevice {
public:
  CNetworkDevice();
  virtual ~CNetworkDevice();

  void Destroy();
  bool Create();

protected:
  void Initialize();

protected:
  bool m_isWSA;
};
