// A class, needed to derive a type for distances
// in the pool from the pool size#pragma once

#include <cstdint>
#include <cstddef>
#include <limits>

template <uint16_t type_size>
struct st // <8> is default
{
    using type = unsigned long long;
};

// Specializations
template <>
struct st<1>
{
    using type = uint16_t;
};

template <>
struct st<2>
{
    using type = uint16_t;
};

template <>
struct st<4>
{
    using type = uint32_t;
};

// Calculates nof bytes in pool distance type
constexpr inline uint16_t
nof_bytes(size_t val)
{
    constexpr uint16_t bits_in_byte = 8;
    uint16_t nof_bytes_in_val = 0;
    for (auto v = val; v != 0; v >>= bits_in_byte, ++nof_bytes_in_val)
        ;
    uint16_t res = 1;
    for (; nof_bytes_in_val != 1; res *= 2, nof_bytes_in_val >>= 1)
        ;
    return res;
};
