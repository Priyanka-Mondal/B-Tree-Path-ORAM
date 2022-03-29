#pragma once
#include "Types.hpp"
#include <array>
#include <cstdint>

constexpr int IV = 16;
constexpr int Key = 128;

class AES {
	static int EncryptBytes(bytes<Key> key, bytes<IV> iv, byte_t *plaintext, int plen, byte_t *ciphertext);
	static int DecryptBytes(bytes<Key> key, bytes<IV> iv, byte_t *ciphertext, int clen, byte_t *plaintext);
	static fblock EncryptBlock(bytes<Key> key, bytes<IV> iv, fblock plaintext,size_t clen_size,size_t plaintext_size);
	static fblock DecryptBlock(bytes<Key> key, bytes<IV> iv, fblock ciphertext,size_t clen_size);


public:
	static void Setup();
	static void Cleanup();
	static fblock Encrypt(bytes<Key> key, fblock b,size_t clen_size,size_t plaintext_size);
	static fblock Decrypt(bytes<Key> key, fblock b,size_t clen_size);
	static int GetCiphertextLength(int plen);
	static bytes<IV> GenerateIV();
};
