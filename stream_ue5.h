#ifndef _SMALLUTILL_FILESYSTEM_STREAM_STD_H_
#define _SMALLUTILL_FILESYSTEM_STREAM_STD_H_
#pragma once

#include "Containers/Array.h"
#include "i_stream.h"

namespace su
{
namespace FileSystem
{

class UE5StreamIn : public IStreamIn
{
public:
    UE5StreamIn(const TArray<uint8>& v) : m_data(v) {}
    virtual ~UE5StreamIn() = default;

    virtual size_t read(void* data, size_t size) override
    {
        if (size + m_pos > m_data.size() || !size)
        {
            return 0;
        }

        uint8_t* buf = reinterpret_cast<uint8_t*>(data);
        //TOD Using FastCopy function!
        for (int ii = 0; ii < size; ++ii, ++m_pos, ++buf)
        {
            buf[ii] = m_data[m_pos];
        }
        return size;
    }

    virtual size_t tell() override
    {
        return m_pos;
    }

    virtual void seek(int64_t pos, Seek way) override
    {
        int64_t newpos = pos;
        if (way == Seek::Cur)
        {
            newpos = m_pos + pos;
        }
        else if (way == Seek::End)
        {
            newpos = m_data.size() - pos;
        }
        m_pos = newpos < 0 ? 0 : ((size_t)newpos < m_data.size() ? pos : m_data.size());
    }

protected:
    const TArray<uint8>& m_data;
    size_t m_pos = 0;
};

class UE5StreamOut : public IStreamOut
{
public:
    UE5StreamOut(TArray<uint8>& v) : m_data(v) {}
    virtual ~UE5StreamOut() = default;

    virtual size_t write(const void* data, size_t size) override
    {
        uint8_t* buf = reinterpret_cast<uint8_t*>(data);
        for (size_t ii = 0; ii < size; ++ii, ++m_pos, ++buf)
        {
            if (m_pos >= m_data.Num())
            {
                m_data.Add(*buf);
            }
            else
            {
                m_data[m_pos] = *buf;
            }
        }

        return size;
    }
    virtual size_t tell() override
        { return m_pos; }

    virtual void seek(int64_t pos, Seek way) override
    {
        int64_t newpos = pos;
        if (way == Seek::Cur)
        {
            newpos = m_pos + pos;
        }
        else if (way == Seek::End)
        {
            newpos = m_data.size() - pos;
        }
        m_pos = newpos < 0 ? 0 : ((size_t)newpos < m_data.size() ? pos : m_data.size());
    }

protected:
    TArray<uint8>& m_data;
    size_t m_pos = 0;
};

} // namespace FileSystem
} // namespace su
#endif
