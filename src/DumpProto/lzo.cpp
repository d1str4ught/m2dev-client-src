// #include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lzo.h"
#include "tea.h"

// Global instance of CLZO singleton to initialize the singleton pattern
static CLZO g_lzoInstance;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                         \
  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |                            \
   ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

DWORD CLZObject::ms_dwFourCC = MAKEFOURCC('M', 'C', 'O', 'Z');

CLZObject::CLZObject() { Initialize(); }

void CLZObject::Initialize() {
  m_pHeader = NULL;
  m_pbBuffer = NULL;
  m_dwBufferSize = 0;
  m_pbIn = NULL;
  m_bCompressed = false;
}

void CLZObject::Clear() {
  if (m_pbBuffer)
    delete[] m_pbBuffer;

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
  // ��ȣȭ�� ���� fourCC 4����Ʈ
  // ����� �� ������� �� �ִ� �ִ� �뷮 +
  // ��ȣȭ�� ���� 8 ����Ʈ
  m_dwBufferSize =
      sizeof(THeader) + sizeof(DWORD) + (uiInLen + uiInLen / 64 + 16 + 3) + 8;

  m_pbBuffer = new BYTE[m_dwBufferSize];
  memset(m_pbBuffer, 0, m_dwBufferSize);

  m_pHeader = (THeader *)m_pbBuffer;
  m_pHeader->dwFourCC = ms_dwFourCC;
  m_pHeader->dwEncryptSize = m_pHeader->dwCompressedSize =
      m_pHeader->dwRealSize = 0;
  m_pHeader->dwRealSize = uiInLen;
}

bool CLZObject::Compress() {
  BYTE *pbBuffer = m_pbBuffer + sizeof(THeader);
  *(DWORD *)pbBuffer = 0x5A4F434D; // 'MOCZ'
  pbBuffer += sizeof(DWORD);

  // Calculate available space for compressed data
  lzo_uint available_space =
      (lzo_uint)(m_dwBufferSize - sizeof(THeader) - sizeof(DWORD));
  lzo_uint compressed_size = available_space;

  int r = lzo1x_1_compress((BYTE *)m_pbIn, (lzo_uint)m_pHeader->dwRealSize,
                           pbBuffer, &compressed_size,
                           CLZO::instance().GetWorkMemory());

  if (LZO_E_OK != r) {
    return false;
  }

  m_pHeader->dwCompressedSize = (DWORD)compressed_size;
  m_bCompressed = true;

  return true;
}

bool CLZObject::BeginDecompress(const void *pvIn) {
  THeader *pHeader = (THeader *)pvIn;

  if (pHeader->dwFourCC != ms_dwFourCC) {
    fprintf(stderr, "LZObject: not a valid data");
    return false;
  }

  m_pHeader = pHeader;
  m_pbIn = (const BYTE *)pvIn + (sizeof(THeader) + sizeof(DWORD));

  m_pbBuffer = new BYTE[pHeader->dwRealSize];
  memset(m_pbBuffer, 0, pHeader->dwRealSize);
  return true;
}

bool CLZObject::Decompress(DWORD *pdwKey) {
  UINT uiSize;
  int r;

  if (m_pHeader->dwEncryptSize) {
    BYTE *pbDecryptedBuffer = Decrypt(pdwKey);

    if (*(DWORD *)pbDecryptedBuffer != ms_dwFourCC) {
      fprintf(stderr, "LZObject: key incorrect");
      return false;
    }

    if (LZO_E_OK !=
        (r = lzo1x_decompress(pbDecryptedBuffer + sizeof(DWORD),
                              m_pHeader->dwCompressedSize, m_pbBuffer,
                              (lzo_uint *)&uiSize, NULL))) {
      fprintf(stderr, "LZObject: Decompress failed(decrypt) ret %d\n", r);
      return false;
    }

    delete[] pbDecryptedBuffer;
  } else {
    uiSize = m_pHeader->dwRealSize;

    if (LZO_E_OK !=
        (r = lzo1x_decompress_safe(m_pbIn, m_pHeader->dwCompressedSize,
                                   m_pbBuffer, (lzo_uint *)&uiSize, NULL))) {
      fprintf(stderr,
              "LZObject: Decompress failed : ret %d, CompressedSize %d\n", r,
              m_pHeader->dwCompressedSize);
      return false;
    }
  }

  if (uiSize != m_pHeader->dwRealSize) {
    fprintf(stderr, "LZObject: Size differs");
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
  // TEA pads to 8-byte boundaries; encrypt the FourCC + compressed data
  DWORD dwSizeToEncrypt = sizeof(DWORD) + m_pHeader->dwCompressedSize;
  fprintf(stderr, "Encrypt: dwCompressedSize=%u, dwSizeToEncrypt=%u\n",
          m_pHeader->dwCompressedSize, dwSizeToEncrypt);
  m_pHeader->dwEncryptSize = tea_encrypt(
      (DWORD *)pbBuffer, (const DWORD *)pbBuffer, pdwKey, dwSizeToEncrypt);
  fprintf(stderr, "Encrypt: dwEncryptSize=%u (return from tea_encrypt)\n",
          m_pHeader->dwEncryptSize);
  if (m_pHeader->dwEncryptSize == 0) {
    fprintf(stderr, "Encrypt: tea_encrypt failed (returned 0)\n");
    return false;
  }
  return true;
}

BYTE *CLZObject::Decrypt(DWORD *pdwKey) {
  assert(m_pbBuffer);

  BYTE *pbDecryptBuffer = new BYTE[m_pHeader->dwEncryptSize];

  tea_encrypt((DWORD *)pbDecryptBuffer, (const DWORD *)(m_pbIn - sizeof(DWORD)),
              pdwKey, m_pHeader->dwEncryptSize);
  return pbDecryptBuffer;
}

CLZO::CLZO() : m_pWorkMem(NULL) {
  if (lzo_init() != LZO_E_OK) {
    fprintf(stderr, "LZO: cannot initialize\n");
    return;
  }

  m_pWorkMem = (BYTE *)malloc(LZO1X_MEM_COMPRESS);

  if (NULL == m_pWorkMem) {
    fprintf(stderr, "LZO: cannot alloc memory\n");
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

  if (!rObj.Compress()) {
    return false;
  }

  return rObj.Encrypt(pdwKey);
}

bool CLZO::Decompress(CLZObject &rObj, const BYTE *pbBuf, DWORD *pdwKey) {
  if (!rObj.BeginDecompress(pbBuf))
    return false;

  if (!rObj.Decompress(pdwKey))
    return false;

  return true;
}

BYTE *CLZO::GetWorkMemory() { return m_pWorkMem; }
