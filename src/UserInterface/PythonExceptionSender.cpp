#include "StdAfx.h"
#include "PythonExceptionSender.h"

void CPythonExceptionSender::Send() {
  // DWORD dwCRC32 = GetCaseCRC32(m_strExceptionString.c_str(),
  // m_strExceptionString.length()); if (m_kSet_dwSendedExceptionCRC.end() !=
  // m_kSet_dwSendedExceptionCRC.find(dwCRC32)) 	return;

  // TraceError("%s", m_strExceptionString.c_str());

  // SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // if (s==INVALID_SOCKET)
  //{
  //	Tracef(" CPythonExceptionSender::Send() - Invalid Socket");
  // }

  // DWORD arg = 1;
  // ioctlsocket(s, FIONBIO, &arg);	// Non-blocking mode

  // char szServerAddr[256];
  // int  nServerPort;
  // strncpy(szServerAddr, "211.105.222.20", sizeof(szServerAddr)-1);

  // nServerPort=LocaleService_GetPythonErrorReportPort();
  //
  // sockaddr_in sa;
  // sa.sin_family = AF_INET;
  // sa.sin_port = htons(nServerPort);
  // sa.sin_addr.s_addr = inet_addr(szServerAddr);
  // if (connect(s,(sockaddr*)&sa,sizeof(sa)))
  //	return;

  // int number_ticket=0;
  // if (recv(s,(char*)&number_ticket,4,0))
  //	return;

  // int length=m_strExceptionString.length();
  // int total=0;
  // int ret=0;

  // int count = 100;
  // while(count--)
  //{
  //	// send plain text
  //	ret = send(s,(char*)(&m_strExceptionString[0])+total,length-total,0);
  //
  //	if (ret<0)
  //		break;

  //	total+=ret;
  //	if (total>=length)
  //		break;
  //}
  // closesocket(s);

  // m_kSet_dwSendedExceptionCRC.insert(dwCRC32);
}

CPythonExceptionSender::CPythonExceptionSender() {}
CPythonExceptionSender::~CPythonExceptionSender() {}
