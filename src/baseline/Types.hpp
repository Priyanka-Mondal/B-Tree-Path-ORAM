#ifndef TYPES
#define TYPES

#include <array>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include "utils/Utilities.h"
#include <cryptopp/rng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>

#define BLOCK 64 // 128 caused seg_fault

using byte_t = uint8_t;
using block = std::vector<byte_t>;

template <size_t N>
using bytes = std::array<byte_t, N>;
typedef std::vector<uint8_t> fblock;

#endif
#ifndef AES_KEY_SIZE
#define AES_KEY_SIZE CryptoPP::AES::DEFAULT_KEYLENGTH
typedef std::array<uint8_t, AES_KEY_SIZE> prf_type;
#define BLOCK_SIZE 480
typedef std::array<uint8_t, BLOCK_SIZE> block_type;
template< typename T >
std::array< byte_t, sizeof (T) > to_bytes(const T& object) {
    std::array< byte_t, sizeof (T) > bytes;

    const byte_t* begin = reinterpret_cast<const byte_t*> (std::addressof(object));
    const byte_t* end = begin + sizeof (T);
    std::copy(begin, end, std::begin(bytes));

    return bytes;
}

template< typename T >
T& from_bytes(const std::array< byte_t, sizeof (T) >& bytes, T& object) {
    byte_t* begin_object = reinterpret_cast<byte_t*> (std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);

    return object;
}

#endif
