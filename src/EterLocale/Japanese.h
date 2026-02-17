#pragma once

BOOL ShiftJIS_IsLeadByte(const char chByte);
BOOL ShiftJIS_IsTrailByte(const char chByte);
int ShiftJIS_StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight,
                             size_t sizeLength);
