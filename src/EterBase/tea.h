#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Symmetric encryption API using libsodium (XChaCha20)
// Key is 16 bytes (128-bit), internally expanded to 32 bytes

#define TEA_KEY_LENGTH 16

int tea_encrypt(unsigned long *dest, const unsigned long *src,
                const unsigned long *key, int size);
int tea_decrypt(unsigned long *dest, const unsigned long *src,
                const unsigned long *key, int size);

#ifdef __cplusplus
};
#endif
