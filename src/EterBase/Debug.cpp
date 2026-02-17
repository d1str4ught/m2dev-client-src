#include "StdAfx.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "Debug.h"
#include "Singleton.h"
#include "Timer.h"
#include <filesystem>
#include <utf8.h>

const DWORD DEBUG_STRING_MAX_LEN = 1024;

static int isLogFile = false;
HWND g_PopupHwnd = NULL;

// ============================================================================
// OPTIMIZED LOGGING INFRASTRUCTURE
// ============================================================================

// Cached timestamp to avoid repeated time()/localtime() syscalls
// Refreshes every ~100ms (good enough for logging, avoids syscall overhead)
struct TCachedTimestamp {
  DWORD lastUpdateMs = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;

  void Update() {
    DWORD now = ELTimer_GetMSec();
    // Refresh timestamp every 100ms (not per-call)
    if (now - lastUpdateMs > 100) {
      time_t ct = time(0);
      struct tm ctm = *localtime(&ct);
      month = ctm.tm_mon + 1;
      day = ctm.tm_mday;
      hour = ctm.tm_hour;
      minute = ctm.tm_min;
      lastUpdateMs = now;
    }
  }

  void Format(char *buf, size_t bufSize) const {
    DWORD msec = ELTimer_GetMSec() % 60000;
    _snprintf_s(buf, bufSize, _TRUNCATE, "%02d%02d %02d:%02d:%05d :: ", month,
                day, hour, minute, (int)msec);
  }
};

static TCachedTimestamp g_cachedTimestamp;

// Optimized debug output: Fast path for ASCII strings (avoids Utf8ToWide
// allocation)
#ifdef _DEBUG
#define DBG_OUT_W_UTF8(psz)                                                    \
  do {                                                                         \
    const char *__s = (psz) ? (psz) : "";                                      \
    size_t __len = strlen(__s);                                                \
    if (Utf8Fast::IsAsciiOnly(__s, __len)) {                                   \
      /* ASCII fast path: direct conversion, no allocation */                  \
      wchar_t __wbuf[512];                                                     \
      size_t __wlen = (__len < 511) ? __len : 511;                             \
      for (size_t __i = 0; __i < __wlen; ++__i)                                \
        __wbuf[__i] = (wchar_t)(unsigned char)__s[__i];                        \
      __wbuf[__wlen] = L'\0';                                                  \
      OutputDebugStringW(__wbuf);                                              \
    } else {                                                                   \
      /* Non-ASCII: use full conversion */                                     \
      std::wstring __w = Utf8ToWide(__s);                                      \
      OutputDebugStringW(__w.c_str());                                         \
    }                                                                          \
  } while (0)
#else
#define DBG_OUT_W_UTF8(psz)                                                    \
  do {                                                                         \
    (void)(psz);                                                               \
  } while (0)
#endif

// MR-11: Colored console output for syserr and packet dumps
#ifdef _DEBUG
static WORD g_consoleDefaultAttrs = 0;
static bool g_consoleAttrsInit = false;

static void InitConsoleDefaultAttrs() {
  if (g_consoleAttrsInit)
    return;

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!hConsole || hConsole == INVALID_HANDLE_VALUE)
    return;

  CONSOLE_SCREEN_BUFFER_INFO info;
  if (GetConsoleScreenBufferInfo(hConsole, &info)) {
    g_consoleDefaultAttrs = info.wAttributes;
    g_consoleAttrsInit = true;
  }
}

static void WriteConsoleColored(const char *text, WORD attrs) {
  if (!text)
    return;

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!hConsole || hConsole == INVALID_HANDLE_VALUE) {
    fputs(text, stdout);
    return;
  }

  InitConsoleDefaultAttrs();
  if (g_consoleAttrsInit)
    SetConsoleTextAttribute(hConsole, attrs);

  fputs(text, stdout);

  if (g_consoleAttrsInit)
    SetConsoleTextAttribute(hConsole, g_consoleDefaultAttrs);
}

static const WORD kConsoleSyserrRed = FOREGROUND_RED | FOREGROUND_INTENSITY;
static const WORD kConsolePacketDumpDim = FOREGROUND_INTENSITY;
static const WORD kConsoleTempTraceBg = BACKGROUND_BLUE | BACKGROUND_INTENSITY |
                                        FOREGROUND_RED | FOREGROUND_GREEN |
                                        FOREGROUND_BLUE;
#endif
// MR-11: -- END OF -- Colored console output for syserr and packet dumps

// Buffered log file writer
// OPTIMIZATION: Buffered writes with periodic flush instead of per-write
// fflush()
// - Collects writes in memory buffer
// - Flushes when buffer is full OR every 500ms OR on shutdown
// - Reduces disk I/O from 1000s of syncs to ~2 per second
class CLogFile : public CSingleton<CLogFile> {
public:
  CLogFile() : m_fp(NULL), m_bufferPos(0), m_lastFlushMs(0) {}

  virtual ~CLogFile() {
    Flush(); // Ensure all buffered data is written
    if (m_fp)
      fclose(m_fp);
    m_fp = NULL;
  }

  void Initialize() {
    m_fp = fopen("log/log.txt", "w");
    m_bufferPos = 0;
    m_lastFlushMs = ELTimer_GetMSec();
  }

  void Write(const char *c_pszMsg) {
    if (!m_fp)
      return;

    // Use cached timestamp (updated every ~100ms)
    g_cachedTimestamp.Update();
    char timestamp[32];
    g_cachedTimestamp.Format(timestamp, sizeof(timestamp));

    // Calculate total length needed
    size_t timestampLen = strlen(timestamp);
    size_t msgLen = c_pszMsg ? strlen(c_pszMsg) : 0;
    size_t totalLen = timestampLen + msgLen;

    // If this write would overflow the buffer, flush first
    if (m_bufferPos + totalLen >= BUFFER_SIZE - 1)
      Flush();

    // If message is larger than buffer, write directly (rare case)
    if (totalLen >= BUFFER_SIZE - 1) {
      fputs(timestamp, m_fp);
      if (c_pszMsg)
        fputs(c_pszMsg, m_fp);
      fflush(m_fp);
      return;
    }

    // Append to buffer
    memcpy(m_buffer + m_bufferPos, timestamp, timestampLen);
    m_bufferPos += timestampLen;
    if (msgLen > 0) {
      memcpy(m_buffer + m_bufferPos, c_pszMsg, msgLen);
      m_bufferPos += msgLen;
    }

    // Periodic flush: every 500ms or when buffer is >75% full
    DWORD now = ELTimer_GetMSec();
    if (now - m_lastFlushMs > 500 || m_bufferPos > BUFFER_SIZE * 3 / 4)
      Flush();
  }

  void Flush() {
    if (!m_fp || m_bufferPos == 0)
      return;

    m_buffer[m_bufferPos] = '\0';
    fputs(m_buffer, m_fp);
    fflush(m_fp);
    m_bufferPos = 0;
    m_lastFlushMs = ELTimer_GetMSec();
  }

protected:
  static const size_t BUFFER_SIZE = 8192; // 8KB buffer
  FILE *m_fp;
  char m_buffer[BUFFER_SIZE];
  size_t m_bufferPos;
  DWORD m_lastFlushMs;
};

static CLogFile gs_logfile;

// MR-11: Separate packet dump log from the main log file
class CExtraLogFile {
public:
  CExtraLogFile() : m_fp(NULL), m_bufferPos(0), m_lastFlushMs(0) {}

  ~CExtraLogFile() {
    Flush();
    if (m_fp)
      fclose(m_fp);
    m_fp = NULL;
  }

  bool Initialize(const char *path) {
    m_fp = fopen(path, "w");
    m_bufferPos = 0;
    m_lastFlushMs = ELTimer_GetMSec();
    return m_fp != NULL;
  }

  bool IsOpen() const { return m_fp != NULL; }

  void Write(const char *c_pszMsg) {
    if (!m_fp)
      return;

    g_cachedTimestamp.Update();
    char timestamp[32];
    g_cachedTimestamp.Format(timestamp, sizeof(timestamp));

    size_t timestampLen = strlen(timestamp);
    size_t msgLen = c_pszMsg ? strlen(c_pszMsg) : 0;
    size_t totalLen = timestampLen + msgLen;

    if (m_bufferPos + totalLen >= BUFFER_SIZE - 1)
      Flush();

    if (totalLen >= BUFFER_SIZE - 1) {
      fputs(timestamp, m_fp);
      if (c_pszMsg)
        fputs(c_pszMsg, m_fp);
      fflush(m_fp);
      return;
    }

    memcpy(m_buffer + m_bufferPos, timestamp, timestampLen);
    m_bufferPos += timestampLen;
    if (msgLen > 0) {
      memcpy(m_buffer + m_bufferPos, c_pszMsg, msgLen);
      m_bufferPos += msgLen;
    }

    DWORD now = ELTimer_GetMSec();
    if (now - m_lastFlushMs > 500 || m_bufferPos > BUFFER_SIZE * 3 / 4)
      Flush();
  }

  void Flush() {
    if (!m_fp || m_bufferPos == 0)
      return;

    m_buffer[m_bufferPos] = '\0';
    fputs(m_buffer, m_fp);
    fflush(m_fp);
    m_bufferPos = 0;
    m_lastFlushMs = ELTimer_GetMSec();
  }

private:
  static const size_t BUFFER_SIZE = 8192;
  FILE *m_fp;
  char m_buffer[BUFFER_SIZE];
  size_t m_bufferPos;
  DWORD m_lastFlushMs;
};

#ifdef _PACKETDUMP
static CExtraLogFile g_packetDumpFile;
static CExtraLogFile g_pdlogFile;
static bool g_packetDumpEnabled = false;
static bool g_pdlogEnabled = false;
static bool g_pdlogRequested = false;

static void EnsurePacketDumpFiles(bool enablePdlog) {
  if (!std::filesystem::exists("log"))
    std::filesystem::create_directory("log");

  if (!g_packetDumpEnabled)
    g_packetDumpEnabled = g_packetDumpFile.Initialize("log/packetdump.txt");

  if (enablePdlog && !g_pdlogEnabled)
    g_pdlogEnabled = g_pdlogFile.Initialize("log/pdlog.txt");
}
#endif
// MR-11: -- END OF -- Separate packet dump log from the main log file

static UINT gs_uLevel = 0;

void SetLogLevel(UINT uLevel) { gs_uLevel = uLevel; }

void Log(UINT uLevel, const char *c_szMsg) {
  if (uLevel >= gs_uLevel)
    Trace(c_szMsg);
}

void Logn(UINT uLevel, const char *c_szMsg) {
  if (uLevel >= gs_uLevel)
    Tracen(c_szMsg);
}

void Logf(UINT uLevel, const char *c_szFormat, ...) {
  if (uLevel < gs_uLevel)
    return;

  char szBuf[DEBUG_STRING_MAX_LEN + 1];

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  fputs(szBuf, stdout);
#endif

  if (isLogFile)
    LogFile(szBuf);
}

void Lognf(UINT uLevel, const char *c_szFormat, ...) {
  if (uLevel < gs_uLevel)
    return;

  char szBuf[DEBUG_STRING_MAX_LEN + 2];

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

  size_t cur = strnlen(szBuf, sizeof(szBuf));
  if (cur + 1 < sizeof(szBuf)) {
    szBuf[cur] = '\n';
    szBuf[cur + 1] = '\0';
  } else {
    szBuf[sizeof(szBuf) - 2] = '\n';
    szBuf[sizeof(szBuf) - 1] = '\0';
  }

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  fputs(szBuf, stdout);
#endif

  if (isLogFile)
    LogFile(szBuf);
}

void Trace(const char *c_szMsg) {
#ifdef _DEBUG
  DBG_OUT_W_UTF8(c_szMsg);
  printf("%s", c_szMsg ? c_szMsg : "");
#endif

  if (isLogFile)
    LogFile(c_szMsg ? c_szMsg : "");
}

void Tracen(const char *c_szMsg) {
#ifdef _DEBUG
  char szBuf[DEBUG_STRING_MAX_LEN + 2];
  _snprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, "%s\n", c_szMsg ? c_szMsg : "");

  DBG_OUT_W_UTF8(szBuf);

  fputs(szBuf, stdout);

  if (isLogFile)
    LogFile(szBuf);
#else
  if (isLogFile) {
    LogFile(c_szMsg ? c_szMsg : "");
    LogFile("\n");
  }
#endif
}

void Tracenf(const char *c_szFormat, ...) {
  char szBuf[DEBUG_STRING_MAX_LEN + 2];

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

  size_t cur = strnlen(szBuf, sizeof(szBuf));
  if (cur + 1 < sizeof(szBuf)) {
    szBuf[cur] = '\n';
    szBuf[cur + 1] = '\0';
  } else {
    szBuf[sizeof(szBuf) - 2] = '\n';
    szBuf[sizeof(szBuf) - 1] = '\0';
  }

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  fputs(szBuf, stdout);
#endif

  if (isLogFile)
    LogFile(szBuf);
}

void Tracef(const char *c_szFormat, ...) {
  char szBuf[DEBUG_STRING_MAX_LEN + 1];

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  fputs(szBuf, stdout);
#endif

  if (isLogFile)
    LogFile(szBuf);
}

// Buffered stderr writer for syserr (same pattern as CLogFile)
// OPTIMIZATION: Reduces fflush(stderr) from every call to every 500ms
static struct TSyserrBuffer {
  static const size_t BUFFER_SIZE = 4096;
  char buffer[BUFFER_SIZE];
  size_t pos = 0;
  DWORD lastFlushMs = 0;

  void Write(const char *msg, size_t len) {
    if (pos + len >= BUFFER_SIZE - 1)
      Flush();

    if (len >= BUFFER_SIZE - 1) {
      // Large message: write directly
      fwrite(msg, 1, len, stderr);
      fflush(stderr);
      return;
    }

    memcpy(buffer + pos, msg, len);
    pos += len;

    // DEBUG: Force flush every write to capture crash traces
    Flush();
  }

  void Flush() {
    if (pos == 0)
      return;
    fwrite(buffer, 1, pos, stderr);
    fflush(stderr);
    pos = 0;
    lastFlushMs = ELTimer_GetMSec();
  }
} g_syserrBuffer;

// MR-11: Seperate packet dump log from the main log file
static void WriteSyserrPlain(const char *msg) {
  if (!msg)
    return;

  g_cachedTimestamp.Update();
  char timestamp[32];
  g_cachedTimestamp.Format(timestamp, sizeof(timestamp));

  g_syserrBuffer.Write(timestamp, strlen(timestamp));
  g_syserrBuffer.Write(msg, strlen(msg));
}
// MR-11: -- END OF -- Seperate packet dump log from the main log file

void TraceError(const char *c_szFormat, ...) {
  // #ifndef _DISTRIBUTE
  char szBuf[DEBUG_STRING_MAX_LEN + 2];

  strncpy_s(szBuf, sizeof(szBuf), "SYSERR: ", _TRUNCATE);
  int prefixLen = (int)strlen(szBuf);

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf + prefixLen, sizeof(szBuf) - prefixLen, _TRUNCATE,
               c_szFormat, args);
  va_end(args);

  size_t cur = strnlen(szBuf, sizeof(szBuf));
  if (cur + 1 < sizeof(szBuf)) {
    szBuf[cur] = '\n';
    szBuf[cur + 1] = '\0';
  } else {
    szBuf[sizeof(szBuf) - 2] = '\n';
    szBuf[sizeof(szBuf) - 1] = '\0';
  }

  // OPTIMIZED: Use cached timestamp instead of time()/localtime() per call
  g_cachedTimestamp.Update();
  char timestamp[32];
  g_cachedTimestamp.Format(timestamp, sizeof(timestamp));

  // OPTIMIZED: Write to buffered stderr instead of fprintf+fflush per call
  g_syserrBuffer.Write(timestamp, strlen(timestamp));
  g_syserrBuffer.Write(szBuf + 8,
                       strlen(szBuf + 8)); // Skip "SYSERR: " prefix for stderr

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  WriteConsoleColored(szBuf, kConsoleSyserrRed);
#endif

  if (isLogFile)
    LogFile(szBuf);
  // #endif
}

void TraceErrorWithoutEnter(const char *c_szFormat, ...) {
  // #ifndef _DISTRIBUTE
  char szBuf[DEBUG_STRING_MAX_LEN];

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

  // OPTIMIZED: Use cached timestamp instead of time()/localtime() per call
  g_cachedTimestamp.Update();
  char timestamp[32];
  g_cachedTimestamp.Format(timestamp, sizeof(timestamp));

  // OPTIMIZED: Write to buffered stderr instead of fprintf+fflush per call
  g_syserrBuffer.Write(timestamp, strlen(timestamp));
  g_syserrBuffer.Write(szBuf, strlen(szBuf));

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  WriteConsoleColored(szBuf, kConsoleSyserrRed);
#endif

  if (isLogFile)
    LogFile(szBuf);
  // #endif
}

// MR-11: Temporary trace functions for debugging (not for regular logging)
void TempTrace(const char *c_szMsg, bool errType) {
  if (!c_szMsg)
    return;

#ifdef _DEBUG
  DBG_OUT_W_UTF8(c_szMsg);
  WriteConsoleColored(c_szMsg, kConsoleTempTraceBg);
#endif

  if (errType) {
    WriteSyserrPlain(c_szMsg);
    return;
  }

  if (isLogFile)
    LogFile(c_szMsg);
}

void TempTracef(const char *c_szFormat, bool errType, ...) {
  char szBuf[DEBUG_STRING_MAX_LEN + 1];

  va_list args;
  va_start(args, errType);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  WriteConsoleColored(szBuf, kConsoleTempTraceBg);
#endif

  if (errType) {
    WriteSyserrPlain(szBuf);
    return;
  }

  if (isLogFile)
    LogFile(szBuf);
}

void TempTracen(const char *c_szMsg, bool errType) {
  if (!c_szMsg)
    return;

  char szBuf[DEBUG_STRING_MAX_LEN + 2];
  _snprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, "%s\n", c_szMsg);

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  WriteConsoleColored(szBuf, kConsoleTempTraceBg);
#endif

  if (errType) {
    WriteSyserrPlain(szBuf);
    return;
  }

  if (isLogFile)
    LogFile(szBuf);
}

void TempTracenf(const char *c_szFormat, bool errType, ...) {
  char szBuf[DEBUG_STRING_MAX_LEN + 2];

  va_list args;
  va_start(args, errType);
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);
  va_end(args);

  size_t cur = strnlen(szBuf, sizeof(szBuf));
  if (cur + 1 < sizeof(szBuf)) {
    szBuf[cur] = '\n';
    szBuf[cur + 1] = '\0';
  } else {
    szBuf[sizeof(szBuf) - 2] = '\n';
    szBuf[sizeof(szBuf) - 1] = '\0';
  }

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  WriteConsoleColored(szBuf, kConsoleTempTraceBg);
#endif

  if (errType) {
    WriteSyserrPlain(szBuf);
    return;
  }

  if (isLogFile)
    LogFile(szBuf);
}
// MR-11: -- END OF -- Temporary trace functions for debugging (not for regular
// logging)

// MR-11: Seperate packet dump from the rest of the logs
void PacketDump(const char *c_szMsg) {
#ifdef _PACKETDUMP
  if (!c_szMsg)
    return;

  PacketDumpf("%s", c_szMsg);
#else
  (void)c_szMsg;
#endif
}

void PacketDumpf(const char *c_szFormat, ...) {
#ifdef _PACKETDUMP
  char szBuf[DEBUG_STRING_MAX_LEN + 2];

  strncpy_s(szBuf, sizeof(szBuf), "PACKET_DUMP: ", _TRUNCATE);
  int prefixLen = (int)strlen(szBuf);

  va_list args;
  va_start(args, c_szFormat);
  _vsnprintf_s(szBuf + prefixLen, sizeof(szBuf) - prefixLen, _TRUNCATE,
               c_szFormat, args);
  va_end(args);

  size_t cur = strnlen(szBuf, sizeof(szBuf));
  if (cur + 1 < sizeof(szBuf)) {
    szBuf[cur] = '\n';
    szBuf[cur + 1] = '\0';
  } else {
    szBuf[sizeof(szBuf) - 2] = '\n';
    szBuf[sizeof(szBuf) - 1] = '\0';
  }

#ifdef _DEBUG
  DBG_OUT_W_UTF8(szBuf);
  WriteConsoleColored(szBuf, kConsolePacketDumpDim);
#endif

  EnsurePacketDumpFiles(g_pdlogRequested);

  if (g_packetDumpEnabled)
    g_packetDumpFile.Write(szBuf);
  if (g_pdlogEnabled)
    g_pdlogFile.Write(szBuf);
#else
  (void)c_szFormat;
#endif
}
// MR-11: -- END OF -- Seperate packet dump from the rest of the logs

void LogBoxf(const char *c_szFormat, ...) {
  va_list args;
  va_start(args, c_szFormat);

  char szBuf[2048];
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szFormat, args);

  va_end(args);

  LogBox(szBuf);
}

void LogBox(const char *c_szMsg, const char *c_szCaption, HWND hWnd) {
  if (!hWnd)
    hWnd = g_PopupHwnd;

  std::wstring wMsg = Utf8ToWide(c_szMsg ? c_szMsg : "");
  std::wstring wCaption = Utf8ToWide(c_szCaption ? c_szCaption : "LOG");

  MessageBoxW(hWnd, wMsg.c_str(), wCaption.c_str(), MB_OK);

  // Logging stays UTF-8
  Tracen(c_szMsg ? c_szMsg : "");
}

void LogFile(const char *c_szMsg) {
  CLogFile::Instance().Write(c_szMsg);

// MR-11: Separate packet dump log from the main log file
#ifdef _PACKETDUMP
  if (g_pdlogEnabled)
    g_pdlogFile.Write(c_szMsg);
#endif
  // MR-11: -- END OF -- Separate packet dump log from the main log file
}

void LogFilef(const char *c_szMessage, ...) {
  va_list args;
  va_start(args, c_szMessage);

  char szBuf[DEBUG_STRING_MAX_LEN + 1];
  _vsnprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, c_szMessage, args);

  va_end(args);

  CLogFile::Instance().Write(szBuf);

// MR-11: Separate packet dump log from the main log file
#ifdef _PACKETDUMP
  if (g_pdlogEnabled)
    g_pdlogFile.Write(szBuf);
#endif
  // MR-11: -- END OF -- Separate packet dump log from the main log file
}

void OpenLogFile(bool bUseLogFIle) {
  if (!std::filesystem::exists("log")) {
    std::filesystem::create_directory("log");
  }

  // #ifndef _DISTRIBUTE
  _wfreopen(L"log/syserr.txt", L"w", stderr);

  if (bUseLogFIle) {
    isLogFile = true;
    CLogFile::Instance().Initialize();
  }

// MR-11: Separate packet dump log from the main log file
#ifdef _PACKETDUMP
  g_pdlogRequested = bUseLogFIle;
  EnsurePacketDumpFiles(g_pdlogRequested);
#endif
  // MR-11: -- END OF -- Separate packet dump log from the main log file
}

void CloseLogFile() {
  // Flush all buffered output before shutdown
  g_syserrBuffer.Flush();
  CLogFile::Instance().Flush();

// MR-11: Separate packet dump log from the main log file
#ifdef _PACKETDUMP
  if (g_packetDumpEnabled)
    g_packetDumpFile.Flush();
  if (g_pdlogEnabled)
    g_pdlogFile.Flush();
#endif
  // MR-11: -- END OF -- Separate packet dump log from the main log file
}

void OpenConsoleWindow() {
  AllocConsole();

  _wfreopen(L"CONOUT$", L"a", stdout);
  _wfreopen(L"CONIN$", L"r", stdin);
}
