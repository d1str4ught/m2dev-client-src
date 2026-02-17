#ifndef __INC_CRC32_H__
#define __INC_CRC32_H__

#include <windows.h>

DWORD GetCRC32(const char *buffer, size_t count);
DWORD GetCaseCRC32(const char *buf, size_t len);
DWORD GetHFILECRC32(HANDLE hFile);
DWORD GetFileCRC32(const wchar_t *c_szFileName);
DWORD GetFileCRC32(const char *fileUtf8);
DWORD GetFileSize(const char *c_szFileName);

#endif
