#include "StdAfx.h"
#include "parser.h"

using namespace script;

static const char *Utf8Next(const char *p, const char *end) {
  if (!p || p >= end)
    return end;
  unsigned char c = (unsigned char)*p;
  if (c < 0x80)
    return p + 1;
  if ((c >> 5) == 0x6)
    return (p + 2 <= end) ? p + 2 : end;
  if ((c >> 4) == 0xE)
    return (p + 3 <= end) ? p + 3 : end;
  if ((c >> 3) == 0x1E)
    return (p + 4 <= end) ? p + 4 : end;
  // invalid lead byte -> move 1 to avoid infinite loops
  return p + 1;
}

static const char *Utf8Prev(const char *base, const char *p) {
  if (!base || !p || p <= base)
    return base;
  const char *q = p - 1;
  // move back over continuation bytes 10xxxxxx
  while (q > base && (((unsigned char)*q & 0xC0) == 0x80))
    --q;
  return q;
}

const char *LocaleString_FindChar(const char *base, int len, char test) {
  if (!base)
    return nullptr;

  int pos = 0;
  while (pos < len) {
    const char *cur = base + pos;
    const char *next = Utf8Next(cur, base + len);
    int cur_len = int(next - cur);

    if (cur_len > 1) {
      pos += cur_len;
    } else if (cur_len == 1) {
      if (*cur == test)
        return cur;
      ++pos;
    } else {
      break;
    }
  }

  return nullptr;
}

int LocaleString_RightTrim(char *base, int len) {
  int pos = len;

  while (pos > 0) {
    char *cur = base + pos;
    char *prev = (char *)Utf8Prev(base, cur);

    int prev_len = int(cur - prev);
    if (prev_len != 1)
      break;

    if (!isspace((unsigned char)*prev) && *prev != '\n' && *prev != '\r')
      break;

    *prev = '\0';
    pos -= prev_len;
  }

  return (pos > 0) ? pos : 0;
}

void LocaleString_RightTrim(char *base) {
  LocaleString_RightTrim(base, strlen(base));
}

const char *LocaleString_Skip(const char *cur) {
  int loopCount = 0;
  const char *end = cur + strlen(cur);

  while (*cur) {
    if (++loopCount > 9216) {
      TraceError("Infinite loop in LocaleString_Skip [%s]", cur);
      break;
    }

    const char *next = Utf8Next(cur, end);
    int cur_len = int(next - cur);

    if (cur_len > 1)
      cur = next;
    else if (cur_len == 1) {
      if (!isspace((unsigned char)*cur) && *cur != '\n' && *cur != '\r')
        return cur;
      ++cur;
    } else
      break;
  }
  return cur;
}

bool Group::GetArg(const char *c_arg_base, int arg_len, TArgList &argList) {
  char szName[32 + 1];
  char szValue[64 + 1];

  int iNameLen = 0;
  int iValueLen = 0;
  int iCharLen = 0;

  int pos = 0;

  bool isValue = false;

  while (pos < arg_len) {
    const char *end = c_arg_base + arg_len;
    const char *cur = c_arg_base + pos;
    const char *next = Utf8Next(cur, end);

    iCharLen = next - cur;

    if (iCharLen > 1) {
      if (isValue) {
        if (iValueLen >= 64) {
          TraceError(
              "argument value overflow: must be shorter than 64 letters");
          return false;
        }

        memcpy(szValue + iValueLen, cur, iCharLen);
        iValueLen += iCharLen;
        szValue[iValueLen] = '\0';
      } else {
        if (iNameLen >= 32) {
          TraceError("argument name overflow: must be shorter than 32 letters");
          return false;
        }
        memcpy(szName + iNameLen, cur, iCharLen);
        iNameLen += iCharLen;
        szName[iNameLen] = '\0';
      }
    } else if (iCharLen == 1) {
      const char c = *cur;
      if (c == '|') {
        if (iNameLen == 0) {
          TraceError("no argument name");
          return false;
        }

        isValue = false;

        iNameLen = LocaleString_RightTrim(szName, iNameLen);
        iValueLen = LocaleString_RightTrim(szValue, iValueLen);
        argList.push_back(TArg(szName, szValue));

        iNameLen = 0;
        iValueLen = 0;
      } else if (c == ';') {
        isValue = true;
      } else if (!isValue && iNameLen == 0 && isspace((unsigned char)c)) {
      } else if (c == '\r' || c == '\n') {
      } else {
        if (isValue) {
          if (iValueLen >= 64) {
            TraceError(
                "argument value overflow: must be shorter than 64 letters");
            return false;
          }

          memcpy(szValue + iValueLen, cur, iCharLen);
          iValueLen += iCharLen;
          szValue[iValueLen] = '\0';
        } else {
          if (iNameLen >= 32) {
            TraceError(
                "argument name overflow: must be shorter than 32 letters");
            return false;
          }
          memcpy(szName + iNameLen, cur, iCharLen);
          iNameLen += iCharLen;
          szName[iNameLen] = '\0';
        }
      }
    } else {
      break;
    }

    pos += iCharLen;
  }

  if (iNameLen != 0 && iValueLen != 0) {
    iNameLen = LocaleString_RightTrim(szName, iNameLen);
    iValueLen = LocaleString_RightTrim(szValue, iValueLen);
    argList.push_back(TArg(szName, szValue));
  }

  return true;
}

bool Group::Create(const std::string &stSource) {
  m_cmdList.clear();

  if (stSource.empty())
    return false;

  const char *str_base = stSource.c_str();
  if (!str_base || !*str_base) {
    TraceError("Source file has no content");
    return false;
  }

  const int str_len = (int)stSource.size();
  int str_pos = 0;

  char box_data[1024 + 1];

  static std::string stLetter;

  while (str_pos < str_len) {
    TCmd cmd;

    const char *word = str_base + str_pos;
    const char *end = str_base + str_len;

    const char *word_next = Utf8Next(word, end);
    if (!word_next || word_next <= word) {
      // Invalid UTF-8 sequence or broken helper -> advance 1 byte to avoid
      // infinite loop
      word_next = word + 1;
    }

    const int word_len = (int)(word_next - word);

    if (word_len > 1) {
      str_pos += word_len;

      stLetter.assign(word, word_next);
      cmd.name.assign("LETTER");
      cmd.argList.push_back(TArg("value", stLetter));
      m_cmdList.push_back(cmd);
    } else if (word_len == 1) {
      const char cur = *word;

      if (cur == '[') {
        ++str_pos;

        const char *box_begin = str_base + str_pos;
        const char *box_end =
            LocaleString_FindChar(box_begin, str_len - str_pos, ']');
        if (!box_end) {
          TraceError(" !! PARSING ERROR - Syntax Error : %s\n", box_begin);
          return false;
        }

        str_pos += (int)(box_end - box_begin) + 1;

        int data_len = 0;
        {
          const char *data_begin = LocaleString_Skip(box_begin);
          const char *data_end = box_end;

          data_len = (int)(data_end - data_begin);
          if (data_len >= 1024) {
            TraceError(" !! PARSING ERROR - Buffer Overflow : %d, %s\n",
                       data_len, str_base);
            return false;
          }

          memcpy(box_data, data_begin, (size_t)data_len);
          box_data[data_len] = '\0';

          data_len = LocaleString_RightTrim(box_data, data_len);
        }

        {
          const char *space = LocaleString_FindChar(box_data, data_len, ' ');
          if (space) {
            const int name_len = (int)(space - box_data);
            cmd.name.assign(box_data, name_len);

            const char *data_end = box_data + data_len;

            const char *space_next = Utf8Next(space, data_end);
            if (!space_next || space_next <= space)
              space_next = space + 1;

            const char *arg = LocaleString_Skip(space_next);

            const int arg_len = (int)(data_len - (arg - box_data));
            if (!GetArg(arg, arg_len, cmd.argList)) {
              TraceError(" !! PARSING ERROR - Unknown Arguments : %d, %s\n",
                         arg_len, arg);
              return false;
            }
          } else {
            cmd.name.assign(box_data);
            cmd.argList.clear();
          }

          m_cmdList.push_back(cmd);
        }
      } else if (cur == '\r' || cur == '\n') {
        ++str_pos;
      } else {
        ++str_pos;

        stLetter.assign(1, cur);
        cmd.name.assign("LETTER");
        cmd.argList.push_back(TArg("value", stLetter));
        m_cmdList.push_back(cmd);
      }
    } else {
      break;
    }
  }

  return true;
}

bool Group::GetCmd(TCmd &cmd) {
  if (m_cmdList.empty())
    return false;

  cmd = m_cmdList.front();
  m_cmdList.pop_front();
  return true;
}

bool Group::ReadCmd(TCmd &cmd) {
  if (m_cmdList.empty())
    return false;

  cmd = m_cmdList.front();
  return true;
}

std::string &Group::GetError() { return m_stError; }

void Group::SetError(const char *c_pszError) { m_stError.assign(c_pszError); }

Group::Group() {}

Group::~Group() {}
