#pragma once

#include "Types.hpp"
#include <array>
#include <cstdint>

constexpr int IV = 16;
constexpr int Key = 128;

class AES {
    static int EncryptBytes(bytes<Key> key, bytes<IV> iv, byte_t *plaintext, int plen, byte_t *ciphertext);
    static int DecryptBytes(bytes<Key> key, bytes<IV> iv, byte_t *ciphertext, int clen, byte_t *plaintext);
    static block EncryptBlock(bytes<Key> key, bytes<IV> iv, block plaintext, size_t clen_size, size_t plaintext_size);
    static block DecryptBlock(bytes<Key> key, bytes<IV> iv, block ciphertext, size_t clen_size);
    static fblock EncryptfBlock(bytes<Key> key, bytes<IV> iv, fblock plaintext,size_t clen_size,size_t plaintext_size);
    static fblock DecryptfBlock(bytes<Key> key, bytes<IV> iv, fblock ciphertext,size_t clen_size);

public:
    static void Setup();
    static void Cleanup();
    static block Encrypt(bytes<Key> key, block b, size_t clen_size, size_t plaintext_size);
    static block Decrypt(bytes<Key> key, block b, size_t clen_size);
    static fblock Encryptf(bytes<Key> key, fblock b,size_t clen_size,size_t plaintext_size);
    static fblock Decryptf(bytes<Key> key, fblock b,size_t clen_size);
    static int GetCiphertextLength(int plen);
    static bytes<IV> GenerateIV();
};
