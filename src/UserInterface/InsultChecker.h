#pragma once

class CInsultChecker {
public:
  CInsultChecker &GetSingleton();

public:
  CInsultChecker();
  virtual ~CInsultChecker();

  void Clear();

  void AppendInsult(const std::string &c_rstInsult);
  bool IsInsultIn(const char *c_szLine, UINT uLineLen);
  void FilterInsult(char *szLine, UINT uLineLen);

private:
  bool __GetInsultLength(const char *c_szWord, UINT *puInsultLen);
  bool __IsInsult(const char *c_szWord);

private:
  std::list<std::string> m_kList_stInsult;
};
