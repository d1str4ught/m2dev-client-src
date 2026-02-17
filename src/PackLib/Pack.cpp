#include "Pack.h"
#include "EterLib/BufferPool.h"
#include <zstd.h>

static thread_local ZSTD_DCtx *g_zstdDCtx = nullptr;

static ZSTD_DCtx *GetThreadLocalZSTDContext() {
  if (!g_zstdDCtx) {
    g_zstdDCtx = ZSTD_createDCtx();
  }
  return g_zstdDCtx;
}

void CPack::DecryptData(uint8_t *data, size_t len, const uint8_t *nonce) {
  crypto_stream_xchacha20_xor(data, data, len, nonce, PACK_KEY.data());
}

bool CPack::Load(const std::string &path) {
  std::error_code ec;
  m_file.map(path, ec);

  if (ec) {
    return false;
  }

  size_t file_size = m_file.size();
  if (file_size < sizeof(TPackFileHeader)) {
    return false;
  }

  memcpy(&m_header, m_file.data(), sizeof(TPackFileHeader));

  if (file_size <
      sizeof(TPackFileHeader) + m_header.entry_num * sizeof(TPackFileEntry)) {
    return false;
  }

  m_index.resize(m_header.entry_num);

  for (size_t i = 0; i < m_header.entry_num; i++) {
    TPackFileEntry &entry = m_index[i];
    memcpy(&entry,
           m_file.data() + sizeof(TPackFileHeader) + i * sizeof(TPackFileEntry),
           sizeof(TPackFileEntry));
    DecryptData((uint8_t *)&entry, sizeof(TPackFileEntry), m_header.nonce);

    if (file_size <
        m_header.data_begin + entry.offset + entry.compressed_size) {
      return false;
    }
  }

  return true;
}

bool CPack::GetFile(const TPackFileEntry &entry, TPackFile &result) {
  return GetFileWithPool(entry, result, nullptr);
}

bool CPack::GetFileWithPool(const TPackFileEntry &entry, TPackFile &result,
                            CBufferPool *pPool) {
  result.resize(entry.file_size);

  size_t offset = m_header.data_begin + entry.offset;
  ZSTD_DCtx *dctx = GetThreadLocalZSTDContext();

  switch (entry.encryption) {
  case 0: {
    size_t decompressed_size =
        ZSTD_decompressDCtx(dctx, result.data(), result.size(),
                            m_file.data() + offset, entry.compressed_size);
    if (decompressed_size != entry.file_size) {
      return false;
    }
  } break;

  case 1: {
    std::vector<uint8_t> compressed_data;
    if (pPool) {
      compressed_data = pPool->Acquire(entry.compressed_size);
    }
    compressed_data.resize(entry.compressed_size);

    memcpy(compressed_data.data(), m_file.data() + offset,
           entry.compressed_size);

    DecryptData(compressed_data.data(), entry.compressed_size, entry.nonce);

    size_t decompressed_size =
        ZSTD_decompressDCtx(dctx, result.data(), result.size(),
                            compressed_data.data(), compressed_data.size());

    if (pPool) {
      pPool->Release(std::move(compressed_data));
    }

    if (decompressed_size != entry.file_size) {
      return false;
    }
  } break;

  default:
    return false;
  }

  return true;
}
