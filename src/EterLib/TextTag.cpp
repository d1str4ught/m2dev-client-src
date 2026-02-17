#include "stdafx.h"
#include "TextTag.h"

int GetTextTag(const wchar_t *src, int maxLen, int &tagLen,
               std::wstring &extraInfo) {
  tagLen = 1;

  if (maxLen < 2 || *src != L'|')
    return TEXT_TAG_PLAIN;

  const wchar_t *cur = ++src;

  if (*cur == L'c') // color
  {
    if (maxLen < 10)
      return TEXT_TAG_PLAIN;

    tagLen = 10;
    extraInfo.assign(++cur, 8);
    return TEXT_TAG_COLOR;
  } else if (*cur == L'|') // ||는 |로 표시한다.
  {
    tagLen = 2;
    return TEXT_TAG_TAG;
  } else if (*cur == L'r') // restore color
  {
    tagLen = 2;
    return TEXT_TAG_RESTORE_COLOR;
  } else if (*cur == L'H') // hyperlink |Hitem:10000:0:0:0:0|h[이름]|h
  {
    tagLen = 2;
    return TEXT_TAG_HYPERLINK_START;
  } else if (*cur == L'h') // end of hyperlink
  {
    tagLen = 2;
    return TEXT_TAG_HYPERLINK_END;
  }

  return TEXT_TAG_PLAIN;
}

std::wstring GetTextTagOutputString(const wchar_t *src, int src_len) {
  int len;
  std::wstring dst;
  std::wstring extraInfo;
  int output_len = 0;
  int hyperlinkStep = 0;

  for (int i = 0; i < src_len;) {
    int tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

    if (tag == TEXT_TAG_PLAIN || tag == TEXT_TAG_TAG) {
      // Show normal text (step 0) AND hyperlink visible text (step 2)
      if (hyperlinkStep == 0 || hyperlinkStep == 2) {
        ++output_len;
        dst += src[i];
      }
    } else if (tag == TEXT_TAG_HYPERLINK_START)
      hyperlinkStep = 1; // Start metadata (hidden)
    else if (tag == TEXT_TAG_HYPERLINK_END) {
      // First |h: end metadata, start visible (1 -> 2)
      // Second |h: end visible (2 -> 0)
      if (hyperlinkStep == 1)
        hyperlinkStep = 2;
      else if (hyperlinkStep == 2)
        hyperlinkStep = 0;
    }

    i += len;
  }
  return dst;
}

int GetTextTagInternalPosFromRenderPos(const wchar_t *src, int src_len,
                                       int offset) {
  int len;
  std::wstring dst;
  std::wstring extraInfo;
  int output_len = 0;
  int hyperlinkStep = 0;
  bool color_tag = false;
  int internal_offset = 0;

  for (int i = 0; i < src_len;) {
    int tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

    if (tag == TEXT_TAG_COLOR) {
      color_tag = true;
      internal_offset = i;
    } else if (tag == TEXT_TAG_RESTORE_COLOR) {
      color_tag = false;
    } else if (tag == TEXT_TAG_PLAIN || tag == TEXT_TAG_TAG) {
      // Show normal text (step 0) AND hyperlink visible text (step 2)
      if (hyperlinkStep == 0 || hyperlinkStep == 2) {
        if (!color_tag)
          internal_offset = i;

        if (offset <= output_len)
          return internal_offset;

        ++output_len;
        dst += src[i];
      }
    } else if (tag == TEXT_TAG_HYPERLINK_START)
      hyperlinkStep = 1; // Start metadata (hidden)
    else if (tag == TEXT_TAG_HYPERLINK_END) {
      // First |h: end metadata, start visible (1 -> 2)
      // Second |h: end visible (2 -> 0)
      if (hyperlinkStep == 1)
        hyperlinkStep = 2;
      else if (hyperlinkStep == 2)
        hyperlinkStep = 0;
    }

    i += len;
  }

  return internal_offset;
}

int GetTextTagOutputLen(const wchar_t *src, int src_len) {
  int len;
  std::wstring extraInfo;
  int output_len = 0;
  int hyperlinkStep = 0;

  for (int i = 0; i < src_len;) {
    int tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

    if (tag == TEXT_TAG_PLAIN || tag == TEXT_TAG_TAG) {
      if (hyperlinkStep == 0)
        ++output_len;
    } else if (tag == TEXT_TAG_HYPERLINK_START)
      hyperlinkStep = 1;
    else if (tag == TEXT_TAG_HYPERLINK_END)
      hyperlinkStep = 0;

    i += len;
  }
  return output_len;
}

int FindColorTagStartPosition(const wchar_t *src, int src_len) {
  if (src_len < 2)
    return 0;

  const wchar_t *cur = src;

  // |r의 경우
  if (*cur == L'r' && *(cur - 1) == L'|') {
    int len = src_len;

    // ||r은 무시
    if (len >= 2 && *(cur - 2) == L'|')
      return 1;

    cur -= 2;
    len -= 2;

    // |c까지 찾아서 |위치까지 리턴한다.
    while (len > 1) // 최소 2자를 검사해야 된다.
    {
      if (*cur == L'c' && *(cur - 1) == L'|')
        return (src - cur) + 1;

      --cur;
      --len;
    }
    return (src_len); // 못찾으면 전부;;
  }
  // ||의 경우
  else if (*cur == L'|' && *(cur - 1) == L'|')
    return 1;

  return 0;
}

int FindColorTagEndPosition(const wchar_t *src, int src_len) {
  const wchar_t *cur = src;

  if (src_len >= 4 && *cur == L'|' && *(cur + 1) == L'c') {
    int left = src_len - 2;
    cur += 2;

    while (left > 1) {
      if (*cur == L'|' && *(cur + 1) == L'r')
        return (cur - src) + 1;

      --left;
      ++cur;
    }
  } else if (src_len >= 2 && *cur == L'|' && *(cur + 1) == L'|')
    return 1;

  return 0;
}
