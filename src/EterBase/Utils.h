#ifndef __INC_ETER2_ETERBASE_UTILS_H__
#define __INC_ETER2_ETERBASE_UTILS_H__

#include <windows.h>
#include <vector>
#include <string>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)                                                         \
  {                                                                            \
    if (p) {                                                                   \
      delete (p);                                                              \
      (p) = NULL;                                                              \
    }                                                                          \
  }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)                                                   \
  {                                                                            \
    if (p) {                                                                   \
      delete[] (p);                                                            \
      (p) = NULL;                                                              \
    }                                                                          \
  }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)                                                        \
  {                                                                            \
    if (p) {                                                                   \
      (p)->Release();                                                          \
      (p) = NULL;                                                              \
    }                                                                          \
  }
#endif

#ifndef SAFE_FREE_GLOBAL
#define SAFE_FREE_GLOBAL(p)                                                    \
  {                                                                            \
    if (p) {                                                                   \
      ::GlobalFree(p);                                                         \
      (p) = NULL;                                                              \
    }                                                                          \
  }
#endif

#ifndef SAFE_FREE_LIBRARY
#define SAFE_FREE_LIBRARY(p)                                                   \
  {                                                                            \
    if (p) {                                                                   \
      ::FreeLibrary(p);                                                        \
      (p) = NULL;                                                              \
    }                                                                          \
  }
#endif

#define AssertLog(str)                                                         \
  TraceError(str);                                                             \
  assert(!str)

#ifndef IS_SET
#define IS_SET(flag, bit) ((flag) & (bit))
#endif

#ifndef SET_BIT
#define SET_BIT(var, bit) ((var) |= (bit))
#endif

#ifndef REMOVE_BIT
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(var, bit) ((var) = (var) ^ (bit))
#endif

extern const char *CreateTempFileName(const char *c_pszPrefix = NULL);
extern void GetFilePathNameExtension(const char *c_szFile, int len,
                                     std::string *pstPath, std::string *pstName,
                                     std::string *pstExt);
extern void GetFileExtension(const char *c_szFile, int len,
                             std::string *pstExt);
extern void GetFileNameParts(const char *c_szFile, int len, char *pszPath,
                             char *pszName, char *pszExt);
extern void GetOldIndexingName(char *szName, int Index);
extern void GetIndexingName(char *szName, DWORD Index);
extern void stl_lowers(std::string &rstRet);
extern void GetOnlyFileName(const char *sz_Name, std::string &strFileName);
extern void GetOnlyPathName(const char *sz_Name, std::string &OnlyPathName);
extern const char *GetOnlyPathName(const char *c_szName);
bool GetLocalFileName(const char *c_szGlobalPath,
                      const char *c_szFullPathFileName,
                      std::string *pstrLocalFileName);
extern void GetExceptionPathName(const char *sz_Name,
                                 std::string &OnlyFileName);
extern void GetWorkingFolder(std::string &strFileName);
extern void StringLowers(char *pString);
extern void StringPath(std::string &rString);
extern void StringPath(char *pString); // 모두 소문자로 만들고, \는 /로 바꾼다.
extern void StringPath(const char *c_szSrc,
                       char *szDest); // 모두 소문자로 만들고, \는 /로 바꾼다.
extern void
StringPath(const char *c_szSrc,
           std::string &rString); // 모두 소문자로 만들고, \는 /로 바꾼다.
extern void PrintAsciiData(const void *data, int bytes);
bool IsFile(const char *filename);
bool IsGlobalFileName(const char *c_szFileName);
int MIN(int a, int b);
int MAX(int a, int b);
int MINMAX(int min, int value, int max);
float fMIN(float a, float b);
float fMAX(float a, float b);
float fMINMAX(float min, float value, float max);
void MyCreateDirectory(const char *path);
void RemoveAllDirectory(const char *c_szDirectoryName);
bool SplitLine(const char *c_szLine, const char *c_szDelimeter,
               std::vector<std::string> *pkVec_strToken);
const char *_getf(const char *c_szFormat, ...);
PCHAR *CommandLineToArgv(PCHAR CmdLine, int *_argc);

template <typename T> T EL_DegreeToRadian(T degree) {
  const T PI = T(3.141592);
  return T(PI * degree / 180.0f);
}

template <typename T>
void ELPlainCoord_GetRotatedPixelPosition(T centerX, T centerY, T distance,
                                          T rotDegree, T *pdstX, T *pdstY) {
  T rotRadian = EL_DegreeToRadian(rotDegree);
  *pdstX = centerX + distance * T(sin((double)rotRadian));
  *pdstY = centerY + distance * T(cos((double)rotRadian));
}

template <typename T> T EL_SignedDegreeToUnsignedDegree(T fSrc) {
  if (fSrc < 0.0f)
    return T(360.0 + T(fmod(fSrc, 360.0)));

  return T(fmod(fSrc, 360.0));
}

template <typename T> T ELRightCoord_ConvertToPlainCoordDegree(T srcDegree) {
  return T(fmod(450.0 - srcDegree, 360.0));
}

template <typename C>
void string_join(const std::string &sep, const C &container, std::string *ret) {
  unsigned int capacity = sep.length() * container.size() - 1;

  // calculate string sequence
  {
    for (auto i = container.begin(); i != container.end(); ++i)
      capacity += (*i).length();
  }

  std::string buf;
  buf.reserve(capacity);

  // join strings
  {
    auto cur = container.begin();
    auto end = container.end();
    --end;

    while (cur != end) {
      buf.append(*cur++);
      buf.append(sep);
    }
    buf.append(*cur);
  }

  swap(*ret, buf);
}

__forceinline int htoi(const wchar_t *s, int size) {
  const wchar_t *t = s;
  int x = 0, y = 1;
  s += size;

  while (t <= --s) {
    if (L'0' <= *s && *s <= L'9')
      x += y * (*s - L'0');
    else if (L'a' <= *s && *s <= L'f')
      x += y * (*s - L'a' + 10);
    else if (L'A' <= *s && *s <= L'F')
      x += y * (10 + *s - L'A');
    else
      return -1; /* invalid input! */
    y <<= 4;
  }

  return x;
}

__forceinline int htoi(const char *s, int size) {
  const char *t = s;
  int x = 0, y = 1;
  s += size;

  while (t <= --s) {
    if ('0' <= *s && *s <= '9')
      x += y * (*s - '0');
    else if ('a' <= *s && *s <= 'f')
      x += y * (*s - 'a' + 10);
    else if ('A' <= *s && *s <= 'F')
      x += y * (10 + *s - 'A');
    else
      return -1; /* invalid input! */
    y <<= 4;
  }

  return x;
}

__forceinline int htoi(const char *s) {
  const char *t = s;
  int x = 0, y = 1;
  s += strlen(s);

  while (t <= --s) {
    if ('0' <= *s && *s <= '9')
      x += y * (*s - '0');
    else if ('a' <= *s && *s <= 'f')
      x += y * (*s - 'a' + 10);
    else if ('A' <= *s && *s <= 'F')
      x += y * (10 + *s - 'A');
    else
      return -1; /* invalid input! */
    y <<= 4;
  }

  return x;
}

typedef std::vector<std::string> TTokenVector;

void StringExceptCharacter(std::string *pstrString, const char *c_szCharacter);

extern void GetExcutedFileName(std::string &r_str);

template <typename T>
constexpr T LinearInterpolation(const T &tMin, const T &tMax, float fRatio) {
  return T(tMin * (1.0f - fRatio) + tMax * fRatio);
}

template <typename T>
constexpr T HermiteInterpolation(const T &tMin, const T &tMax, float fRatio) {
  fRatio = MINMAX(0.0f, fRatio, 1.0f);
  fRatio = fRatio * fRatio * (3.0f - 2.0f * fRatio);
  return LinearInterpolation(tMin, tMax, fRatio);
}

#endif