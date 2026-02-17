#pragma once

#include <deque>
#include <string>

class CPathStack {
public:
  CPathStack();
  virtual ~CPathStack();

  void SetBase();

  void MoveBase();

  void Push();

  bool Pop();

  void Move(const char *c_szPathName);
  void GetCurrentPathName(std::string *pstCurPathName);

protected:
  std::string m_stBasePathName;
  std::deque<std::string> m_stPathNameDeque;
};
