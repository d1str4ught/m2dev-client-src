#ifndef __INC_METIN_II_371GNFBQOCJ_LZO_H__
#define __INC_METIN_II_371GNFBQOCJ_LZO_H__

#include <windows.h>
#include <lzo/lzo1x.h>
#include "Singleton.h"

class CLZObject {
public:
#pragma pack(4)
  typedef struct SHeader {
    DWORD dwFourCC;
    DWORD dwEncryptSize;    // 암호화된 크기
    DWORD dwCompressedSize; // 압축된 데이터 크기
    DWORD dwRealSize;       // 실제 데이터 크기
  } THeader;
#pragma pack()

  CLZObject();
  ~CLZObject();

  void Clear();

  void BeginCompress(const void *pvIn, UINT uiInLen);
  void BeginCompressInBuffer(const void *pvIn, UINT uiInLen, void *pvOut);
  bool Compress();

  bool BeginDecompress(const void *pvIn);
  bool Decompress(DWORD *pdwKey = NULL);

  bool Encrypt(DWORD *pdwKey);
  bool __Decrypt(DWORD *key, BYTE *data);

  const THeader &GetHeader() { return *m_pHeader; }
  BYTE *GetBuffer() { return m_pbBuffer; }
  DWORD GetSize();
  void AllocBuffer(DWORD dwSize);
  DWORD GetBufferSize() { return m_dwBufferSize; }
  // void			CopyBuffer(const char* pbSrc, DWORD dwSrcSize);

private:
  void Initialize();

  BYTE *m_pbBuffer;
  DWORD m_dwBufferSize;

  THeader *m_pHeader;
  const BYTE *m_pbIn;
  bool m_bCompressed;

  bool m_bInBuffer;

public:
  static DWORD ms_dwFourCC;
};

class CLZO : public CSingleton<CLZO> {
public:
  CLZO();
  virtual ~CLZO();

  bool CompressMemory(CLZObject &rObj, const void *pIn, UINT uiInLen);
  bool CompressEncryptedMemory(CLZObject &rObj, const void *pIn, UINT uiInLen,
                               DWORD *pdwKey);
  bool Decompress(CLZObject &rObj, const BYTE *pbBuf, DWORD *pdwKey = NULL);
  BYTE *GetWorkMemory();

private:
  BYTE *m_pWorkMem;
};

#endif
