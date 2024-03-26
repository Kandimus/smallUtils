#ifndef _SMALLUTILL_FILESYSTEM_H_
#define _SMALLUTILL_FILESYSTEM_H_
#pragma once

#include <string>
#include <vector>
#include <fstream>

using StringArray = std::vector<std::string>;
using ByteArray = std::vector<uint8_t>;

namespace su
{
namespace FileSystem
{

enum class Seek
{
    Beg = 0,
    Cur,
    End,
};


class BaseBuffer
{
public:
    BaseBuffer(const std::string& name) : m_name(name) {}
    virtual ~BaseBuffer() = default;

    virtual const std::string& getName() const
        { return m_name; }

    virtual const std::string& getMarker() = 0;

    virtual ByteArray getData() const = 0;

    virtual bool read(std::ifstream& ifs, uint64_t size) = 0;

protected:
    std::string m_name;
};

class DataBuffer : public BaseBuffer
{
public:
    DataBuffer(const std::string& name) : BaseBuffer(name) {}
    virtual ~DataBuffer() = default;

    DataBuffer& operator << (bool val) { uint8_t v = !!val; return add(&v, sizeof(v)); }
    DataBuffer& operator << (int8_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint8_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int16_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint16_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int32_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint32_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int64_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint64_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (float val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (double val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (const std::string& val)
    {
        *this << static_cast<uint32_t>(val.size());
        add(val.c_str(), val.size());
        return *this;
    }

    DataBuffer& operator >> (bool& val) { uint8_t v = 0; get(&v, sizeof(v)); val = v != 0; return *this; }
    DataBuffer& operator >> (int8_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint8_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int16_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint16_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int32_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint32_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int64_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint64_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (float& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (double& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (std::string& val)
    {
        uint32_t size = 0;
        *this >> size;
        val.resize(size);
        get(val.data(), val.size());
        return *this;
    }

    DataBuffer& add(const void* buf, size_t size);
    DataBuffer& get(void* buf, size_t size);

    virtual ByteArray getData() const override
        { return m_data; }
    virtual const ByteArray& getDataConst() const
        { return m_data; }

    virtual bool isEof() const
        { return m_pos >= m_data.size(); }

    virtual uint64_t tell() const
        { return m_pos; }

    virtual void seek(int64_t pos, Seek way);
    virtual bool read(std::ifstream& ifs, uint64_t size) override;

    virtual const std::string& getMarker() override
        { return marker(); }

    static const std::string& marker()
        { return m_marker; }

protected:
    uint64_t m_pos = 0;
    ByteArray m_data;

    static const std::string m_marker;
};

class StringBuffer : public BaseBuffer
{
public:
    StringBuffer(const std::string& name) : BaseBuffer(name) {}
    virtual ~StringBuffer() = default;

    uint32_t add(const std::string& str);
    std::vector<uint32_t> add(const StringArray& v);

    virtual ByteArray getData() const override;
    virtual bool read(std::ifstream& ifs, uint64_t size) override;

    virtual const std::string& getMarker() override
        { return marker(); }

    static const std::string& marker()
        { return m_marker; }

protected:
    StringArray m_data;

    static const std::string m_marker;
};

class BaseHeader
{
public:
    BaseHeader(const std::string& type) : m_type(type) {}
    virtual ~BaseHeader() = default;

    void add(BaseBuffer* buf)
        { m_buffer.push_back(buf); }

    virtual BaseBuffer* bufferFabric(const std::string& name, const std::string& marker) const;

    static uint8_t getVersionMajor() { return 1; }
    static uint8_t getVersionMinor() { return 0; }

    bool write(std::ofstream& ofs) const;
    bool read(std::ifstream& ifs);

protected:
    struct BufferOffset
    {
        uint64_t pos;
        uint64_t offset;
        uint32_t size;
    };

    std::string m_type;
    std::vector<BaseBuffer*> m_buffer;
};

inline DataBuffer& operator << (DataBuffer& db, const std::vector<uint32_t>& v)
{
    db << v.size();
    for (auto item : v)
    {
        db << item;
    }
    return db;
}

} // namespace FileSystem
} //


#endif
