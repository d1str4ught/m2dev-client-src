/*
 * Symmetric encryption module using libsodium (XChaCha20)
 * API-compatible replacement for legacy TEA encryption
 *
 * Key expansion: 16-byte input key is hashed to 32 bytes using BLAKE2b
 * Nonce: Derived deterministically from the key to ensure encrypt/decrypt
 * consistency
 */
#include "tea.h"
#include <sodium.h>
#include <cstring>

// Fixed context string for key derivation
static const char *KEY_CONTEXT = "M2DevPackEncrypt";

// Derive a 32-byte key and 24-byte nonce from the 16-byte input key
static void DeriveKeyAndNonce(const unsigned long *key, uint8_t *derived_key,
                              uint8_t *nonce) {
  // Use BLAKE2b to derive a 32-byte key
  crypto_generichash(derived_key, crypto_stream_xchacha20_KEYBYTES,
                     (const uint8_t *)key, TEA_KEY_LENGTH,
                     (const uint8_t *)KEY_CONTEXT, strlen(KEY_CONTEXT));

  // Derive nonce from key (using different context)
  uint8_t nonce_seed[crypto_stream_xchacha20_NONCEBYTES + 8];
  crypto_generichash(nonce_seed, sizeof(nonce_seed), (const uint8_t *)key,
                     TEA_KEY_LENGTH, (const uint8_t *)"M2DevNonce", 10);
  memcpy(nonce, nonce_seed, crypto_stream_xchacha20_NONCEBYTES);
}

int tea_encrypt(unsigned long *dest, const unsigned long *src,
                const unsigned long *key, int size) {
  int resize;

  // Align to 8 bytes for compatibility with legacy format
  if (size % 8 != 0) {
    resize = size + 8 - (size % 8);
    // Zero-pad the source (caller must ensure buffer is large enough)
    memset((char *)src + size, 0, resize - size);
  } else {
    resize = size;
  }

  // Derive 32-byte key and nonce from 16-byte input
  uint8_t derived_key[crypto_stream_xchacha20_KEYBYTES];
  uint8_t nonce[crypto_stream_xchacha20_NONCEBYTES];
  DeriveKeyAndNonce(key, derived_key, nonce);

  // XOR encrypt using XChaCha20 stream
  crypto_stream_xchacha20_xor((uint8_t *)dest, (const uint8_t *)src, resize,
                              nonce, derived_key);

  // Securely clear derived key
  sodium_memzero(derived_key, sizeof(derived_key));

  return resize;
}

int tea_decrypt(unsigned long *dest, const unsigned long *src,
                const unsigned long *key, int size) {
  int resize;

  // Align to 8 bytes for compatibility with legacy format
  if (size % 8 != 0) {
    resize = size + 8 - (size % 8);
  } else {
    resize = size;
  }

  // Derive 32-byte key and nonce from 16-byte input
  uint8_t derived_key[crypto_stream_xchacha20_KEYBYTES];
  uint8_t nonce[crypto_stream_xchacha20_NONCEBYTES];
  DeriveKeyAndNonce(key, derived_key, nonce);

  // XOR decrypt using XChaCha20 stream (same as encrypt for stream cipher)
  crypto_stream_xchacha20_xor((uint8_t *)dest, (const uint8_t *)src, resize,
                              nonce, derived_key);

  // Securely clear derived key
  sodium_memzero(derived_key, sizeof(derived_key));

  return resize;
}
