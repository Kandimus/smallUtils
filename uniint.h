#pragma once

#include <stdint.h>

namespace su
{
union UniInt16
{
    int16_t  i16;
    uint16_t u16;
    uint8_t  u8[2];
    int8_t   i8[2];
};

union UniInt32
{
    int32_t  i32;
    uint32_t u32;
    uint16_t u16[2];
    int16_t  i16[2];
    uint8_t  u8[4];
    int8_t   i8[4];
};

union UniInt64
{
    int64_t  i64;
    uint64_t u64;
    int32_t  i32[2];
    uint32_t u32[2];
    uint16_t u16[4];
    int16_t  i16[4];
    uint8_t  u8[8];
    int8_t   i8[8];

    UniInt64(int64_t val) { i64 = val; }
};

}

