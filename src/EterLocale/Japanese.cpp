#include "StdAfx.h"
#include "Japanese.h"

//
// CP932: Shift-JIS (Japan Industry Standard)
//

#define MACRO_IsLeadByte(c)                                                    \
  ((0x81 <= (c) && (c) <= 0x9f) || ((0xe0 <= (c)) && (c) <= 0xfc))
#define MACRO_IsTailByte(c)                                                    \
  ((0x40 <= (c) && (c) <= 0x7e) || (0x80 <= (c) && (c) <= 0xfc))

BOOL ShiftJIS_IsLeadByte(const char chByte) {
  const unsigned char byByte = (const unsigned char)chByte;

  return MACRO_IsLeadByte(byByte);
}

BOOL ShiftJIS_IsTrailByte(const char chByte) {
  const unsigned char byByte = (const unsigned char)chByte;

  return MACRO_IsTailByte(byByte);
}

int ShiftJIS_StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight,
                             size_t sizeLength) {
  const unsigned char *uszString1 = (const unsigned char *)szStringLeft;
  const unsigned char *uszString2 = (const unsigned char *)szStringRight;
  unsigned short c1, c2;

  if (sizeLength == 0)
    return 0;
  while (sizeLength--) {
    c1 = *uszString1++;
    if (MACRO_IsLeadByte(c1)) {
      if (sizeLength == 0) {
        c1 = 0;
        c2 = MACRO_IsLeadByte(*uszString2) ? 0 : *uszString2;
        goto test;
      }
      if (*uszString1 == '\0')
        c1 = 0;
      else {
        c1 = ((c1 << 8) | *uszString1++);
      }
    } else
      c1 = tolower(c1);

    c2 = *uszString2++;
    if (MACRO_IsLeadByte(c2)) {
      if (sizeLength == 0) {
        c2 = 0;
        goto test;
      }
      sizeLength--;
      if (*uszString2 == '\0')
        c2 = 0;
      else {
        c2 = ((c2 << 8) | *uszString2++);
      }
    } else
      c2 = tolower(c2);
  test:
    if (c1 != c2)
      return ((c1 > c2) ? 1 : -1);

    if (c1 == 0)
      return (0);
  }
  return 0;
}
