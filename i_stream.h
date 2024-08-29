#ifndef _SMALLUTILL_FILESYSTEM_I_STREAM_H_
#define _SMALLUTILL_FILESYSTEM_I_STREAM_H_
#pragma once

#include <stdint.h>

namespace su
{
namespace fs
{

enum class Seek
{
    Beg = 0,
    Cur,
    End,
};

class IStreamIn
{
public:
    IStreamIn() = default;
    virtual ~IStreamIn() = default;

    virtual size_t read(void* data, size_t size) = 0;
    virtual size_t tell() = 0;
    virtual void seek(int64_t pos, Seek way) = 0;
};

class IStreamOut
{
public:
    IStreamOut() = default;
    virtual ~IStreamOut() = default;

    virtual size_t write(const void* data, size_t size) = 0;
    virtual size_t tell() = 0;
    virtual void seek(int64_t pos, Seek way) = 0;
};

} // namespace fs
} // namespace su
#endif
