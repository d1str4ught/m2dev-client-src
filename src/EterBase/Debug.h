#ifndef __INC_ETERLIB_DEBUG_H__
#define __INC_ETERLIB_DEBUG_H__

#include <windows.h>

#if defined(_DEBUG) && !defined(_PACKETDUMP)
#define _PACKETDUMP
#endif

extern void SetLogLevel(UINT uLevel);
extern void Log(UINT uLevel, const char *c_szMsg);
extern void Logn(UINT uLevel, const char *c_szMsg);
extern void Logf(UINT uLevel, const char *c_szFormat, ...);
extern void Lognf(UINT uLevel, const char *c_szFormat, ...);

extern void Trace(const char *c_szMsg);
extern void Tracen(const char *c_szMsg);
extern void Tracenf(const char *c_szFormat, ...);
extern void Tracef(const char *c_szFormat, ...);
extern void TraceError(const char *c_szFormat, ...);
extern void TraceErrorWithoutEnter(const char *c_szFormat, ...);
// MR-11: Separate packet dump log from the main log file
extern void TempTrace(const char *c_szMsg, bool errType = false);
extern void TempTracef(const char *c_szFormat, bool errType = false, ...);
extern void TempTracen(const char *c_szMsg, bool errType = false);
extern void TempTracenf(const char *c_szFormat, bool errType = false, ...);

extern void PacketDump(const char *c_szMsg);
extern void PacketDumpf(const char *c_szFormat, ...);
// MR-11: -- END OF -- Separate packet dump log from the main log file

extern void LogBox(const char *c_szMsg, const char *c_szCaption = NULL,
                   HWND hWnd = NULL);
extern void LogBoxf(const char *c_szMsg, ...);

extern void LogFile(const char *c_szMsg);
extern void LogFilef(const char *c_szMessage, ...);
extern void OpenConsoleWindow(void);
extern void CloseConsoleWindow();
extern void SetupLog(void);

extern void OpenLogFile(bool bUseLogFile = true);
extern void CloseLogFile();

extern HWND g_PopupHwnd;

#define CHECK_RETURN(flag, string)                                             \
  if (flag) {                                                                  \
    LogBox(string);                                                            \
    return;                                                                    \
  }

#endif

#if defined(_DEBUG)
#define MD_ASSERT(expr)                                                        \
  ((expr) ? true                                                               \
          : (TraceError("MD_ASSERT('%s') failed at (%s:%d)", #expr, __FILE__,  \
                        __LINE__),                                             \
             throw "ffs", false))
#else
#define MD_ASSERT(expr)                                                        \
  ((expr) ? true                                                               \
          : (TraceError("MD_ASSERT('%s') failed at (%s:%d)", #expr, __FILE__,  \
                        __LINE__),                                             \
             false))
#endif
