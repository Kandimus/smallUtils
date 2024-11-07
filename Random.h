#ifndef SMALLUTILS_RANDOM_H
#define SMALLUTILS_RANDOM_H
#pragma once

#include "defines.h"
#include <random>

namespace su
{
/** General purpose 64 bit generator.
* See reference implementation at https://xoshiro.di.unimi.it/xoshiro256plusplus.c
*/
class RandomXoshiro256PlusPlus
{
public:
    RandomXoshiro256PlusPlus()
    {
        uint64_t seeds[4];
        std::random_device true_random;
        for (uint64_t& val : seeds)
        {
            do
            {
                val = (uint64_t(true_random()) << 32) | true_random();
            }
            while (val == 0);
        }
        SetSeed(seeds[0], seeds[1], seeds[2], seeds[3]);
    }
    /** Initializes from specific seed, useful for reproducible randomness. */
    explicit RandomXoshiro256PlusPlus(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
    {
        SetSeed(a, b, c, d);
    }

    /**
    * @params a, b, c, d - Should be non zero.
    */
    inline void SetSeed(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
    {
        SU_ASSERT(a != 0 && b != 0 && c != 0 && d != 0);
        m_state[0] = a;
        m_state[1] = b;
        m_state[2] = c;
        m_state[3] = d;
    }

    /** Generates uint32 in whole range of values. */
    uint32_t get32()
    {
        // Upper bits are more random so better to use them
        // https://github.com/rust-random/rngs/blob/a5a67ccf4a4c57b25cd04236a993f92cd47c0c99/rand_xoshiro/src/xoshiro256plusplus.rs#L99
        return get64() >> 32;
    }
    /** Generates uint32 less than upperBound. */
    uint32_t get32(const uint32_t upperBound)
    {
        if (upperBound <= 1)
        {
            return 0;
        }
        const uint32_t cap = 0xffffffff - (0xffffffff % upperBound);
        uint32_t ret;
        do
        {
            // this loop ensures that the partial wrap-around at the end of the range doesn't weigh the lower-portion of range more heavily
            // this loop will rarely execute, particularly on smaller ranges, but provides much better randomness on larger ranges.
            ret = get32();
        }
        while (ret >= cap);

        return ret % upperBound;
    }
    /** Generates int32 in inclusive range. */
    int32_t get32(const int32_t low, const int32_t high)
    {
        SU_ASSERT(high >= low);
        return low + static_cast<int32_t>(get32(high - low + 1));
    }

    /** Generates uint64_t in whole range of values. */
    uint64_t get64()
    {
        const uint64_t result = Rotl(m_state[0] + m_state[3], 23) + m_state[0];

        // Advance to next generation
        const uint64_t t = m_state[1] << 17;

        m_state[2] ^= m_state[0];
        m_state[3] ^= m_state[1];
        m_state[1] ^= m_state[2];
        m_state[0] ^= m_state[3];

        m_state[2] ^= t;

        m_state[3] = Rotl(m_state[3], 45);

        return result;
    }

    // Generates uint64_t less than upperBound
    uint64_t get64(const uint64_t upperBound)
    {
        if (upperBound <= 1)
        {
            return 0;
        }
        const uint64_t cap = 0xffffffffffffffff - (0xffffffffffffffff % upperBound);
        uint64_t ret;
        do
        {
            // this loop ensures that the partial wrap-around at the end of the range doesn't weigh the lower-portion of range more heavily
            // this loop will rarely execute, particularly on smaller ranges, but provides much better randomness on larger ranges.
            ret = get64();
        }
        while (ret >= cap);
        return ret % upperBound;
    }
    /** Generates int64 in inclusive range. */
    int64_t get64(const int64_t low, const int64_t high)
    {
        SU_ASSERT(high >= low);
        return low + static_cast<int64_t>(get64(high - low + 1));
    }

    /** Generates float in range [0..1.0] */
    float getFloat()
    {
        return static_cast<float>(get32()) / static_cast<float>(0xffffffff);
    }
    /** Generates double in range [0..1.0] */
    double getDouble()
    {
        return static_cast<double>(get64()) / static_cast<double>(0xffffffffffffffff);
    }

private:
    inline static uint64_t Rotl(const uint64_t x, const uint8_t k);

private:
    uint64_t m_state[4] = {};
};

inline uint64_t RandomXoshiro256PlusPlus::Rotl(const uint64_t x, const uint8_t k)
{
    return (x << k) | (x >> (64u - k));
}

}   // namespace su

#endif

