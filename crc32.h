
#pragma once

#include <stdint.h>
#include <string.h>

namespace su
{

class Crc32
{
public:
    Crc32(uint32_t poly = 0xedb88320, uint32_t initial = 0xffffffff)
    {
        int jj;
        uint32_t c;

        memset(crc32_inv, 0, 256 * sizeof(crc32_inv[0]));

        for(int ii = 0; ii < 256; ++ii)
        {
            for(c = ii, jj = 0; jj < 8; ++jj)
            {
                //c = (c >> 1) ^ (CRC32_POLY & (-(signed long)(c & 1)));
                c = (c & 1) ? ((c >> 1) ^ m_poly) : (c >> 1);
            }
            crc32_table[ii] = c;

            crc32_inv[(unsigned char)(c >> 24) & 0xFF] = (c << 8) ^ ii;
        }
    }

    uint32_t update(uint32_t crc, uint8_t byte)
    {
        return crc32_table[(crc & 0xFF) ^ byte] ^ (crc >> 8);
    }

    uint32_t get(const void* buf, size_t size)
    {
        const uint8_t* cbuff = static_cast<const uint8_t*>(buf);
        uint32_t crc = m_initial;

        while (size--)
        {
            crc = update(crc, *cbuff++);
        }
        return ~crc;
    }

    uint32_t getReversedBytes(uint32_t reg_start, uint32_t reg_end)
    {
        reg_start ^= 0xffffffff;
        reg_end   ^= 0xffffffff;

        for (int ii = 0; ii < 4; ++ii)
        {
            for (int t_index = 0; t_index < 256; t_index++)
            {
                if ((crc32_table[t_index] & 0xff000000) == (reg_end & 0xff000000))
                {
                    reg_end  ^= crc32_table[t_index];
                    reg_end <<= 8;
                    reg_end  ^= t_index ^ (((0xff000000 >> (ii * 8)) & reg_start) >> ((3 - ii) * 8));
                    break;
                }
            }
        }

        return reg_end;
    }

private:
    uint32_t m_poly = 0xedb88320;
    uint32_t m_initial = 0xffffffff;

    uint32_t crc32_table[256];
    uint32_t crc32_inv[256];
};


class Crc16
{
public:
    Crc16()
    {
        uint16_t r;

        for(int ii = 0; ii < 256; ++ii)
        {
            //r = ((unsigned short)ii) << 8;
            r = ii;
            
            for(int jj = 0; jj < 8; ++jj)
            {
                r = (r & 1) ? ((r >> 1) ^ 0xA001) : (r >> 1);
            }

            crc16_table[ii] = r;
        }
    }

    uint16_t getModbus(unsigned char *buf, size_t Sz)
    {
        unsigned int Code = 0xFFFF;
        unsigned int Flag;
        unsigned int i;
        unsigned int j;

        for (i=0;i<Sz;i++)
        {
            Code ^= buf[i];
            for(j=0;j<8;j++)
            {
                Flag = Code&01;
                Code = (Code >> 1) & 0177777;
                if (Flag) Code ^= 0xA001;
            }
        }
        return (Code);
    }

    uint16_t update(uint16_t Code, unsigned char buf)
    {
        return crc16_table[(Code & 0xFF) ^ buf] ^ (Code >> 8);
        //return crc16_table[((Code >> 8) ^ buf) & 0xFF] ^ (Code << 8);

        for (int ii = 0; ii < 8; ++ii)
        {
            if (((Code & 0x8000) >> 8) ^ (buf & 0x80))
            {
                Code = (Code << 1) ^ 0x8005;
            }
            else
            {
                Code = (Code << 1);
            }

            buf <<= 1;
        }

        return Code;
    }

    uint16_t get(unsigned char *buf, size_t size)
    {
        unsigned short crc = 0xffff;

        while(size--)
        {
            crc = update(crc, *buf++);
        }
        return ~crc;
    }


    uint16_t crc16_table[256];
};

} // namespace su
