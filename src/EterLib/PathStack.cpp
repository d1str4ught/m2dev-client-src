#include "StdAfx.h"
#include "PathStack.h"

CPathStack::CPathStack() { SetBase(); }

CPathStack::~CPathStack() { MoveBase(); }

void CPathStack::GetCurrentPathName(std::string *pstCurPathName) {
  assert(pstCurPathName != NULL);

  char szPathName[MAX_PATH + 1];
  _getcwd(szPathName, MAX_PATH);

  *pstCurPathName = szPathName;
}

void CPathStack::Push() {
  char szPathName[MAX_PATH + 1];
  _getcwd(szPathName, MAX_PATH);

  m_stPathNameDeque.push_front(szPathName);
}

bool CPathStack::Pop() {
  if (m_stPathNameDeque.empty()) {
    assert(!"CPathStack::Pop Empty Stack");
    return false;
  }

  _chdir(m_stPathNameDeque.front().c_str());
  m_stPathNameDeque.pop_front();
  return true;
}

void CPathStack::MoveBase() { _chdir(m_stBasePathName.c_str()); }

void CPathStack::SetBase() { GetCurrentPathName(&m_stBasePathName); }

void CPathStack::Move(const char *c_szPathName) { _chdir(c_szPathName); }
