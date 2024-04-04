#ifndef _SMALLUTILL_FILESYSTEM_STREAM_STD_H_
#define _SMALLUTILL_FILESYSTEM_STREAM_STD_H_
#pragma once

#include <fstream>
#include "i_stream.h"

namespace su
{
namespace FileSystem
{

class StdStreamIn : public IStreamIn
{
public:
    StdStreamIn(std::ifstream& ifs) : m_ifs(ifs) {}
    virtual ~StdStreamIn() = default;

    virtual size_t read(void* data, size_t size) override
        { return m_ifs.read(reinterpret_cast<char*>(data), size) ? size : 0; }
    virtual size_t tell() override
        { return m_ifs.tellg(); }
    virtual void seek(int64_t pos, Seek way) override
        { m_ifs.seekg(pos, way == Seek::Beg ? std::ios::beg : (way == Seek::Cur ? std::ios::cur : std::ios::end)); }

protected:
    std::ifstream& m_ifs;
};

class StdStreamOut : public IStreamOut
{
public:
    StdStreamOut(std::ofstream& ofs) : m_ofs(ofs) {}
    virtual ~StdStreamOut() = default;

    virtual size_t write(const void* data, size_t size) override
        { return m_ofs.write(reinterpret_cast<const char*>(data), size) ? size : 0; }
    virtual size_t tell() override
        { return m_ofs.tellp(); }
    virtual void seek(int64_t pos, Seek way) override
        { m_ofs.seekp(pos, way == Seek::Beg ? std::ios::beg : (way == Seek::Cur ? std::ios::cur : std::ios::end)); }

protected:
    std::ofstream& m_ofs;
};

} // namespace FileSystem
} // namespace su
#endif
