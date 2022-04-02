#ifndef TYPES
#define TYPES

#include <array>
#include <vector>
#include <iostream>


#define ZKEY 0

#define BLOCK 30 // 128 caused seg_fault
#define ID_SIZE 16 // it was 16 before
#define FID_SIZE 6
#define COM (BLOCK/FID_SIZE)
// 1kb, 10kb, and 100kb filesize 
#define FILEBLOCK 64

#define LAST -1
#define KS "a"
#define KB "b"
#define FS "c"
#define FB "d" 

// The main type for passing around raw file data
using byte_t = uint8_t;
using block = std::vector<byte_t>;

template <size_t N>
using bytes = std::array<byte_t, N>;
typedef std::vector<uint8_t> fblock;

// A bucket contains a number of Blocks
constexpr int Z = 4;

enum Op {
    READ,
    WRITE
};


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
