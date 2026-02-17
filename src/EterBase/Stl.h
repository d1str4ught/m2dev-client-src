#ifndef __INC_ETERBASE_STL_H__
#define __INC_ETERBASE_STL_H__

#pragma warning(disable : 4786) // identifier was truncated to '255' characters
                                // in the browser information
#pragma warning(disable : 4018) // signed <-> unsigned mismatch
#pragma warning(disable                                                        \
                : 4503) // decorated name length exceeded, name was truncated
#pragma warning(disable : 4018) // '<' : signed/unsigned mismatch

#include <assert.h>

#pragma warning(push, 3)

#include <algorithm>
#include <string>
#include <vector>
#include <stack>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <functional>
#include <SSTREAM>

#pragma warning(pop)

extern char ascii_tolower(const char c);
extern std::string &stl_static_string(const char *c_sz);
extern void stl_lowers(std::string &rstRet);
extern int split_string(const std::string &input, const std::string &delimiter,
                        std::vector<std::string> &results, bool includeEmpties);

struct stl_sz_less {
  bool operator()(char *const &left, char *const &right) const {
    return (strcmp(left, right) < 0);
  }
};

template <typename TContainer> inline void stl_wipe(TContainer &container) {
  for (auto i = container.begin(); i != container.end(); ++i) {
    delete *i;
    *i = NULL;
  }

  container.clear();
}

template <typename TString> inline int hex2dec(TString szhex) {
  int hex0 = toupper(szhex[0]);
  int hex1 = toupper(szhex[1]);

  return (hex1 >= 'A' ? hex1 - 'A' + 10 : hex1 - '0') +
         (hex0 >= 'A' ? hex0 - 'A' + 10 : hex0 - '0') * 16;
}

template <typename TString>
inline unsigned long htmlColorStringToARGB(TString str) {
  unsigned long alp = hex2dec(str);
  unsigned long red = hex2dec(str + 2);
  unsigned long green = hex2dec(str + 4);
  unsigned long blue = hex2dec(str + 6);
  return (alp << 24 | red << 16 | green << 8 | blue);
}

template <typename TContainer>
inline void stl_wipe_second(TContainer &container) {
  for (auto i = container.begin(); i != container.end(); ++i) {
    delete i->second;
  }

  container.clear();
}

template <typename T> inline void safe_release(T &rpObject) {
  if (!rpObject)
    return;

  rpObject->Release();
  rpObject = NULL;
}

template <typename T>
void DeleteVectorItem(std::vector<T> *pVector, unsigned long dwIndex) {
  if (dwIndex >= pVector->size()) {
    assert(!"Wrong index to delete!");
    return;
  }
  if (1 == pVector->size()) {
    pVector->clear();
    return;
  }

  auto itor = pVector->begin();
  for (unsigned long i = 0; i < dwIndex; ++i)
    ++itor;

  pVector->erase(itor);
}

template <typename T>
void DeleteVectorItem(T *pVector, unsigned long dwStartIndex,
                      unsigned long dwEndIndex) {
  if (dwStartIndex >= pVector->size()) {
    assert(!"Wrong start index to delete!");
    return;
  }
  if (dwEndIndex >= pVector->size()) {
    assert(!"Wrong end index to delete!");
    return;
  }

  auto itorStart = pVector->begin();
  for (unsigned long i = 0; i < dwStartIndex; ++i)
    ++itorStart;
  auto itorEnd = pVector->begin();
  for (unsigned long j = 0; j < dwEndIndex; ++j)
    ++itorEnd;

  pVector->erase(itorStart, itorEnd);
}

template <typename T> void DeleteVectorItem(std::vector<T> *pVector, T pItem) {
  auto itor = pVector->begin();
  for (; itor != pVector->end(); ++itor) {
    if (pItem == *itor) {
      if (1 == pVector->size()) {
        pVector->clear();
      } else {
        pVector->erase(itor);
      }
      break;
    }
  }
}

template <typename T> void DeleteListItem(std::list<T> *pList, T pItem) {
  auto itor = pList->begin();
  for (; itor != pList->end(); ++itor) {
    if (pItem == *itor) {
      if (1 == pList->size()) {
        pList->clear();
      } else {
        pList->erase(itor);
      }
      break;
    }
  }
}

template <typename T, typename F>
void stl_vector_qsort(std::vector<T> &rdataVector, F comp) {
  if (rdataVector.empty())
    return;
  qsort(&rdataVector[0], rdataVector.size(), sizeof(T), comp);
}

template <typename TData> class stl_stack_pool {
public:
  stl_stack_pool() { m_pos = 0; }

  stl_stack_pool(int capacity) {
    m_pos = 0;
    initialize(capacity);
  }

  virtual ~stl_stack_pool() {}

  void initialize(int capacity) {
    m_dataVector.clear();
    m_dataVector.resize(capacity);
  }

  void clear() { m_pos = 0; }

  TData *alloc() {
    assert(!m_dataVector.empty() &&
           "stl_stack_pool::alloc you MUST run stl_stack_pool::initialize");

    int max = m_dataVector.size();

    if (m_pos >= max) {
      assert(!"stl_stack_pool::alloc OUT of memory");
      m_pos = 0;
    }

    return &m_dataVector[m_pos++];
  }

  TData *base() { return &m_dataVector[0]; }

  int size() { return m_pos; }

private:
  int m_pos;

  std::vector<TData> m_dataVector;
};

template <typename TData, typename THandle = int> class stl_circle_pool {
public:
  typedef bool TFlag;

public:
  stl_circle_pool() { initialize(); }
  virtual ~stl_circle_pool() { destroy(); }
  void destroy() {
    if (m_datas) {
      delete[] m_datas;
      m_datas = NULL;
    }
    if (m_flags) {
      delete[] m_flags;
      m_flags = NULL;
    }
  }
  void create(int size) {
    destroy();

    initialize();

    m_size = size;
    m_datas = new TData[m_size];
    m_flags = new TFlag[m_size];

    for (int i = 0; i < m_size; ++i)
      m_flags[i] = false;
  }
  THandle alloc() {
    THandle max = m_size;

    THandle loop = max;
    while (loop--) {
      int cur = m_pos % max;
      ++m_pos;
      if (!m_flags[cur]) {
        m_flags[cur] = true;
        return cur;
      }
    }

    assert(!"Out of Memory");

    return 0;
  }
  void free(THandle handle) {
    assert(check(handle) && "Out of RANGE");
    m_flags[handle] = false;
  }
  inline bool check(THandle handle) {
    if (handle >= m_size)
      return false;
    return true;
  }
  inline int size() { return m_size; }
  inline TData &refer(THandle handle) {
    assert(check(handle) && "Out of RANGE");
    return m_datas[handle];
  }

protected:
  void initialize() {
    m_datas = NULL;
    m_flags = NULL;
    m_pos = 0;
    m_size = 0;
  }

protected:
  TData *m_datas;
  TFlag *m_flags;

  THandle m_size;

  THandle m_pos;
};

typedef std::vector<std::string> CTokenVector;
typedef std::map<std::string, std::string> CTokenMap;
typedef std::map<std::string, CTokenVector> CTokenVectorMap;

// class CTokenVector : public std::vector<std::string>
//{
//	public:
//		CTokenVector() : std::vector<std::string> ()
//		{
//		}
//		virtual ~CTokenVector()
//		{
//		}
// };
//
// class CTokenMap : public std::map<std::string, std::string>
//{
//	public:
//		CTokenMap() : std::map<std::string, std::string>()
//		{
//		}
//		virtual ~CTokenMap()
//		{
//		}
// };
//
// class CTokenVectorMap : public std::map<std::string, CTokenVector>
//{
//	public:
//		CTokenVectorMap() : std::map<std::string, CTokenVector>()
//		{
//		}
//		virtual ~CTokenVectorMap()
//		{
//		}
//
// };

/*
template <typename T1, typename T2>
class CMapIterator
{
public:
        typedef std::map<T1, T2> TMapType;

public:
        CMapIterator(TMapType & rMap)
        {
                m_it = rMap.begin();
                m_itEnd = rMap.end();
        }

        inline T2 operator * () { return m_it->second; }

        inline bool operator ++()
        {
                if (m_itEnd == m_it)
                        return false;

                ++m_it;
                return m_itEnd != m_it;
        }

        inline T1 GetFirst() { return m_it->first; }
        inline T2 GetSecond() { return m_it->second; }

        private:
                TMapType::iterator m_it;
                TMapType::iterator m_itEnd;
};
*/

struct stringhash {
  size_t GetHash(const std::string &str) const {
    const unsigned char *s = (const unsigned char *)str.c_str();
    const unsigned char *end = s + str.size();
    size_t h = 0;

    while (s < end) {
      h *= 16777619;
      h ^= (unsigned char)*(unsigned char *)(s++);
    }

    return h;
  }

  size_t operator()(const std::string &str) const { return GetHash(str); }
};

#endif
