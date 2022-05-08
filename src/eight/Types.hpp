#ifndef TYPES
#define TYPES

#include <array>
#include <vector>
#include <iostream>
#include <set>
#include <string>

// The main type for passing around raw file data
#define ID_SIZE 16
#define BLOCK 256 // 128 caused seg_fault
using byte_t = uint8_t;
using block = std::vector<byte_t>;

template <size_t N>
using bytes = std::array<byte_t, N>;

constexpr int Z = 4;// A bucket contains a number of Blocks
constexpr int T = 8;
constexpr int D = 2*T;// Each node of Btree has 3 keys

enum Op {
    READ,
    WRITE
};

template< typename T >
std::array< byte_t, sizeof (T) > to_bytes(const T& object) 
{
    std::array< byte_t, sizeof (T) > bytes;
    const byte_t* begin = reinterpret_cast<const byte_t*> (std::addressof(object));
    const byte_t* end = begin + sizeof (T);
    std::copy(begin, end, std::begin(bytes));
    return bytes;
}

template< typename T >
T& from_bytes(const std::array< byte_t, sizeof (T) >& bytes, T& object) 
{
    byte_t* begin_object = reinterpret_cast<byte_t*> (std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);
    return object;
}

#endif
