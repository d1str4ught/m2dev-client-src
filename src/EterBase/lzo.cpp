#include "StdAfx.h"
#include <stdlib.h>

#include "lzo.h"
#include "tea.h"
#include "debug.h"

#define dbg_printf

static class LZOFreeMemoryMgr {
public:
  enum {
    REUSING_CAPACITY = 64 * 1024,
  };

public:
  ~LZOFreeMemoryMgr() {
    std::vector<BYTE *>::iterator i;
    for (i = m_freeVector.begin(); i != m_freeVector.end(); ++i)
      delete *i;

    m_freeVector.clear();
  }
  BYTE *Alloc(unsigned capacity) {
    assert(capacity > 0);
    if (capacity < REUSING_CAPACITY) {
      if (!m_freeVector.empty()) {
        BYTE *freeMem = m_freeVector.back();
        m_freeVector.pop_back();

        dbg_printf("lzo.reuse_alloc\t%p(%d) free\n", freeMem, capacity);
        return freeMem;
      }
      BYTE *newMem = new BYTE[REUSING_CAPACITY];
      dbg_printf("lzo.reuse_alloc\t%p(%d) real\n", newMem, capacity);
      return newMem;
    }
    BYTE *newMem = new BYTE[capacity];
    dbg_printf("lzo.real_alloc\t%p(%d)\n", newMem, capacity);
    return newMem;
  }
  void Free(BYTE *ptr, unsigned capacity) {
    assert(ptr != NULL);
    assert(capacity > 0);
    if (capacity < REUSING_CAPACITY) {
      dbg_printf("lzo.reuse_free\t%p(%d)\n", ptr, capacity);
      m_freeVector.push_back(ptr);
      return;
    }

    dbg_printf("lzo.real_free\t%p(%d)\n", ptr, capacity);
    delete[] ptr;
  }

private:
  std::vector<BYTE *> m_freeVector;
} gs_freeMemMgr;

DWORD CLZObject::ms_dwFourCC = MAKEFOURCC('M', 'C', 'O', 'Z');

CLZObject::CLZObject() { Initialize(); }

void CLZObject::Initialize() {
  m_bInBuffer = false;
  m_pbBuffer = NULL;
  m_dwBufferSize = 0;

  m_pHeader = NULL;
  m_pbIn = NULL;
  m_bCompressed = false;
}

void CLZObject::Clear() {
  if (m_pbBuffer && !m_bInBuffer)
    gs_freeMemMgr.Free(m_pbBuffer, m_dwBufferSize);

  if (m_dwBufferSize > 0) {
    dbg_printf("lzo.free %d\n", m_dwBufferSize);
  }

  Initialize();
}

CLZObject::~CLZObject() { Clear(); }

DWORD CLZObject::GetSize() {
  assert(m_pHeader);

  if (m_bCompressed) {
    if (m_pHeader->dwEncryptSize)
      return sizeof(THeader) + sizeof(DWORD) + m_pHeader->dwEncryptSize;
    else
      return sizeof(THeader) + sizeof(DWORD) + m_pHeader->dwCompressedSize;
  } else
    return m_pHeader->dwRealSize;
}

void CLZObject::BeginCompress(const void *pvIn, UINT uiInLen) {
  m_pbIn = (const BYTE *)pvIn;

  // sizeof(SHeader) +
  // 암호화를 위한 fourCC 4바이트
  // 압축된 후 만들어질 수 있는 최대 용량 +
  // 암호화를 위한 8 바이트
  m_dwBufferSize =
      sizeof(THeader) + sizeof(DWORD) + (uiInLen + uiInLen / 64 + 16 + 3) + 8;

  m_pbBuffer = gs_freeMemMgr.Alloc(m_dwBufferSize);
  memset(m_pbBuffer, 0, m_dwBufferSize);

  m_pHeader = (THeader *)m_pbBuffer;
  m_pHeader->dwFourCC = ms_dwFourCC;
  m_pHeader->dwEncryptSize = m_pHeader->dwCompressedSize =
      m_pHeader->dwRealSize = 0;
  m_pHeader->dwRealSize = uiInLen;
}

void CLZObject::BeginCompressInBuffer(const void *pvIn, UINT uiInLen,
                                      void * /*pvOut*/) {
  m_pbIn = (const BYTE *)pvIn;

  // sizeof(SHeader) +
  // 암호화를 위한 fourCC 4바이트
  // 압축된 후 만들어질 수 있는 최대 용량 +
  // 암호화를 위한 8 바이트
  m_dwBufferSize =
      sizeof(THeader) + sizeof(DWORD) + (uiInLen + uiInLen / 64 + 16 + 3) + 8;

  m_pbBuffer = gs_freeMemMgr.Alloc(m_dwBufferSize);
  memset(m_pbBuffer, 0, m_dwBufferSize);

  m_pHeader = (THeader *)m_pbBuffer;
  m_pHeader->dwFourCC = ms_dwFourCC;
  m_pHeader->dwEncryptSize = m_pHeader->dwCompressedSize =
      m_pHeader->dwRealSize = 0;
  m_pHeader->dwRealSize = uiInLen;
  m_bInBuffer = true;
}

bool CLZObject::Compress() {
  UINT iOutLen;
  BYTE *pbBuffer;

  pbBuffer = m_pbBuffer + sizeof(THeader);
  *(DWORD *)pbBuffer = ms_dwFourCC;
  pbBuffer += sizeof(DWORD);

#if defined(LZO1X_999_MEM_COMPRESS)
  int r = lzo1x_999_compress((BYTE *)m_pbIn, m_pHeader->dwRealSize, pbBuffer,
                             (lzo_uint *)&iOutLen,
                             CLZO::Instance().GetWorkMemory());
#else
  int r =
      lzo1x_1_compress((BYTE *)m_pbIn, m_pHeader->dwRealSize, pbBuffer,
                       (lzo_uint *)&iOutLen, CLZO::Instance().GetWorkMemory());
#endif

  if (LZO_E_OK != r) {
    TraceError("LZO: lzo1x_999_compress failed");
    return false;
  }

  m_pHeader->dwCompressedSize = iOutLen;
  m_bCompressed = true;
  return true;
}

bool CLZObject::BeginDecompress(const void *pvIn) {
  THeader *pHeader = (THeader *)pvIn;

  if (pHeader->dwFourCC != ms_dwFourCC) {
    TraceError("LZObject: not a valid data");
    return false;
  }

  m_pHeader = pHeader;
  m_pbIn = (const BYTE *)pvIn + (sizeof(THeader) + sizeof(DWORD));

  /*
  static unsigned sum = 0;
  static unsigned count = 0;
  sum += pHeader->dwRealSize;
  count++;
  printf("decompress cur: %d, ave: %d\n", pHeader->dwRealSize, sum/count);
  */
  m_dwBufferSize = pHeader->dwRealSize;
  m_pbBuffer = gs_freeMemMgr.Alloc(m_dwBufferSize);
  memset(m_pbBuffer, 0, pHeader->dwRealSize);
  return true;
}

class DecryptBuffer {
public:
  enum {
    LOCAL_BUF_SIZE = 8 * 1024,
  };

public:
  DecryptBuffer(unsigned size) {
    if (size >= LOCAL_BUF_SIZE) {
      m_buf = new char[size];
      dbg_printf("DecryptBuffer - AllocHeap %d\n", size);
    } else {
      dbg_printf("DecryptBuffer - AllocStack %d\n", size);
      m_buf = m_local_buf;
    }
  }
  ~DecryptBuffer() {
    if (m_local_buf != m_buf) {
      dbg_printf("DecryptBuffer - FreeHeap\n");
      delete[] m_buf;
    } else {
      dbg_printf("DecryptBuffer - FreeStack\n");
    }
  }
  void *GetBufferPtr() { return m_buf; }

private:
  char *m_buf;
  char m_local_buf[LOCAL_BUF_SIZE];
};

bool CLZObject::Decompress(DWORD *pdwKey) {
  int r;
  lzo_uint out_len =
      static_cast<lzo_uint>(m_dwBufferSize); // capacity of m_pbBuffer

  if (m_pHeader->dwEncryptSize) {
    DecryptBuffer buf(m_pHeader->dwEncryptSize);
    BYTE *pbDecryptedBuffer = static_cast<BYTE *>(buf.GetBufferPtr());

    __Decrypt(pdwKey, pbDecryptedBuffer);

    if (*reinterpret_cast<DWORD *>(pbDecryptedBuffer) != ms_dwFourCC) {
      TraceError("LZObject: key incorrect");
      return false;
    }

    r = lzo1x_decompress_safe(
        pbDecryptedBuffer + sizeof(DWORD),
        static_cast<lzo_uint>(m_pHeader->dwCompressedSize), m_pbBuffer,
        &out_len, nullptr);
  } else {
    out_len = static_cast<lzo_uint>(m_dwBufferSize); // reset capacity
    r = lzo1x_decompress_safe(
        m_pbIn, static_cast<lzo_uint>(m_pHeader->dwCompressedSize), m_pbBuffer,
        &out_len, nullptr);
  }

  if (r != LZO_E_OK) {
    TraceError("LZObject: Decompress failed: ret %d, CompressedSize %u", r,
               m_pHeader->dwCompressedSize);
    return false;
  }

  if (out_len != static_cast<lzo_uint>(m_pHeader->dwRealSize)) {
    TraceError("LZObject: Size differs");
    return false;
  }

  return true;
}

bool CLZObject::Encrypt(DWORD *pdwKey) {
  if (!m_bCompressed) {
    assert(!"not compressed yet");
    return false;
  }

  BYTE *pbBuffer = m_pbBuffer + sizeof(THeader);
  m_pHeader->dwEncryptSize =
      tea_encrypt((DWORD *)pbBuffer, (const DWORD *)pbBuffer, pdwKey,
                  m_pHeader->dwCompressedSize + 19);
  return true;
}

bool CLZObject::__Decrypt(DWORD *key, BYTE *data) {
  assert(m_pbBuffer);

  tea_decrypt((DWORD *)data, (const DWORD *)(m_pbIn - sizeof(DWORD)), key,
              m_pHeader->dwEncryptSize);
  return true;
}

void CLZObject::AllocBuffer(DWORD dwSrcSize) {
  if (m_pbBuffer && !m_bInBuffer)
    gs_freeMemMgr.Free(m_pbBuffer, m_dwBufferSize);

  m_pbBuffer = gs_freeMemMgr.Alloc(dwSrcSize);
  m_dwBufferSize = dwSrcSize;
}
/*
void CLZObject::CopyBuffer(const char* pbSrc, DWORD dwSrcSize)
{
        AllocBuffer(dwSrcSize);
        memcpy(m_pbBuffer, pbSrc, dwSrcSize);
}
*/

CLZO::CLZO() : m_pWorkMem(NULL) {
  if (lzo_init() != LZO_E_OK) {
    TraceError("LZO: cannot initialize");
    return;
  }

#if defined(LZO1X_999_MEM_COMPRESS)
  m_pWorkMem = (BYTE *)malloc(LZO1X_999_MEM_COMPRESS);
#else
  m_pWorkMem = (BYTE *)malloc(LZO1X_1_MEM_COMPRESS);
#endif

  if (NULL == m_pWorkMem) {
    TraceError("LZO: cannot alloc memory");
    return;
  }
}

CLZO::~CLZO() {
  if (m_pWorkMem) {
    free(m_pWorkMem);
    m_pWorkMem = NULL;
  }
}

bool CLZO::CompressMemory(CLZObject &rObj, const void *pIn, UINT uiInLen) {
  rObj.BeginCompress(pIn, uiInLen);
  return rObj.Compress();
}

bool CLZO::CompressEncryptedMemory(CLZObject &rObj, const void *pIn,
                                   UINT uiInLen, DWORD *pdwKey) {
  rObj.BeginCompress(pIn, uiInLen);

  if (rObj.Compress()) {
    if (rObj.Encrypt(pdwKey))
      return true;

    return false;
  }

  return false;
}

bool CLZO::Decompress(CLZObject &rObj, const BYTE *pbBuf, DWORD *pdwKey) {
  if (!rObj.BeginDecompress(pbBuf))
    return false;

  if (!rObj.Decompress(pdwKey))
    return false;

  return true;
}

BYTE *CLZO::GetWorkMemory() { return m_pWorkMem; }
