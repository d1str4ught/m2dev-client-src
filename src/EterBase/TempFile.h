#ifndef __INC_ETERBASE_TEMPFILE_H__
#define __INC_ETERBASE_TEMPFILE_H__

#include "FileBase.h"

class CTempFile : public CFileBase {
public:
  CTempFile(const char *c_pszPrefix = NULL);
  virtual ~CTempFile();

protected:
  char m_szFileName[MAX_PATH + 1];
};

#endif
