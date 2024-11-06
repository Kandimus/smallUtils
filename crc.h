//=================================================================================================
//===
//=== crc.h
//===
//=== Copyright (c) 2020-2024 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaly
//===
//=================================================================================================
#ifndef _SMALLUTILS_CRC_H_
#define _SMALLUTILS_CRC_H_
#pragma once

/*
 * Define SMALLUTIL_CRC_AS_STATIC for using Crc class as static class. In this case you can
 * using only one version of Crc but extra memory for tables will not be allocated
 *
 */

#ifdef SMALLUTIL_CRC_AS_STATIC
#define SMALLUTIL_CRC_STATIC inline static
#define SMALLUTIL_CRC_VISIBLE public
#else
#define SMALLUTIL_CRC_STATIC
#define SMALLUTIL_CRC_VISIBLE private
#endif

namespace su
{

namespace Polynomial
{
    // Reversed
    constexpr uint8_t CRC8_CCITT = 0xe0;
    constexpr uint8_t CRC8_DALLAS = 0x8c;
    constexpr uint8_t CRC8_DVB_S2 = 0xab;
    constexpr uint8_t CRC8_SAE = 0xb8;
    constexpr uint16_t CRC16_CAN = 0x4cd1;
    constexpr uint16_t CRC16_IBM = 0xA001;
    constexpr uint16_t CRC16_CCITT = 0x8408;
    constexpr uint16_t CRC16_T10_DIF = 0xedd1;
    constexpr uint16_t CRC16_DNP = 0xa6bc;
    constexpr uint32_t CRC32_IEEE = 0xedb88320;
    constexpr uint32_t CRC32_C = 0x82f63b78;
    constexpr uint32_t CRC32_KOOPMAN = 0xeb31d82e;
    constexpr uint32_t CRC32_Q = 0xd5828281;
    constexpr uint64_t CRC64_ECMA = 0xC96C5795D7870F42;
    constexpr uint64_t CRC64_ISO = 0xd800000000000000;
}

template<typename T>
class Crc
{
public:
    Crc() = delete;
    explicit Crc(T poly, T initial = ~T(0), T xorout = ~T(0))
    {
        init(poly, initial, xorout);
    }
    virtual ~Crc() = default;

    SMALLUTIL_CRC_STATIC
    T update(T crc, uint8_t byte)
    {
        return m_table[(crc & 0xff) ^ byte] ^ (crc >> 8);
    }

    SMALLUTIL_CRC_STATIC
    T get(const void* buf, size_t size)
    {
        const uint8_t* cbuff = static_cast<const uint8_t*>(buf);
        T crc = m_initial;

        while (size--)
        {
            crc = update(crc, *cbuff++);
        }
        return crc ^ m_xorout;
    }

    SMALLUTIL_CRC_STATIC
    uint32_t getReversedBytes(uint32_t reg_start, uint32_t reg_end)
    {
        reg_start ^= 0xffffffff;
        reg_end   ^= 0xffffffff;

        for (int ii = 0; ii < 4; ++ii)
        {
            for (int t_index = 0; t_index < 256; t_index++)
            {
                if ((m_table[t_index] & 0xff000000) == (reg_end & 0xff000000))
                {
                    reg_end  ^= m_table[t_index];
                    reg_end <<= 8;
                    reg_end  ^= t_index ^ (((0xff000000 >> (ii * 8)) & reg_start) >> ((3 - ii) * 8));
                    break;
                }
            }
        }

        return reg_end;
    }

SMALLUTIL_CRC_VISIBLE:
    SMALLUTIL_CRC_STATIC
    void init(T poly, T initial = ~T(0), T xorout = ~T(0))
    {
        if (m_isInit)
        {
            return;
        }

        m_poly = poly;
        m_initial = initial;
        m_xorout = xorout;

        T crc = 0;
        for (T ii = 0; ii < 256; ++ii)
        {
            T jj = 0;
            for (crc = ii; jj < 8; ++jj)
            {
                crc = (crc & 1) ? ((crc >> 1) ^ m_poly) : (crc >> 1);
            }
            m_table[ii] = crc;
        }
        m_isInit = true;
    }


private:
    SMALLUTIL_CRC_STATIC bool m_isInit = false;
    SMALLUTIL_CRC_STATIC T m_poly = 0;
    SMALLUTIL_CRC_STATIC T m_initial = ~T(0);
    SMALLUTIL_CRC_STATIC T m_xorout = ~T(0);
    SMALLUTIL_CRC_STATIC T m_table[256] = { 0 };
};

using Crc64 = Crc<uint64_t>;
using Crc32 = Crc<uint32_t>;
using Crc16 = Crc<uint16_t>;
using Crc8 = Crc<uint8_t>;

} // namespace su
#endif
