#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include "Packet.h"

void CPythonNetworkStream::OffLinePhase() { DispatchPacket(m_offlineHandlers); }