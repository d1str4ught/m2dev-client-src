#ifndef __ETER_FILE_DIR__
#define __ETER_FILE_DIR__

#include <windows.h>

class CDir {
public:
  CDir();
  virtual ~CDir();

  void Destroy();
  bool Create(const char *c_szFilter, const char *c_szPath = "",
              BOOL bCheckedExtension = FALSE);

protected:
  virtual bool OnFolder(const char *c_szFilter, const char *c_szPath,
                        const char *c_szName) = 0;
  virtual bool OnFile(const char *c_szPath, const char *c_szName) = 0;

protected:
  bool IsFolder();

  void Initialize();

protected:
  WIN32_FIND_DATA m_wfd;
  HANDLE m_hFind;
};

#endif