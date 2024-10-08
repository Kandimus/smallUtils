#ifndef _SMALLUTILL_FILESYSTEM_H_
#define _SMALLUTILL_FILESYSTEM_H_
#pragma once

//
// Store and load unsigned integer values without packing
//
//#define SU_FILESYSTEM_NOT_USING_PACKING

#include <string>
#include <vector>
#include <fstream>

#include "i_stream.h"

using StringArray = std::vector<std::string>;
using ByteArray = std::vector<uint8_t>;
using CharArray = std::vector<int8_t>;

namespace su
{
namespace fs
{

class BaseHeader;

class BaseBuffer
{
public:
    BaseBuffer(const std::string& name, const BaseHeader* parent = nullptr) : m_name(name), m_parent(parent) {}
    virtual ~BaseBuffer() = default;

    virtual const std::string& getName() const
        { return m_name; }

    virtual const std::string& getMarker() = 0;
    virtual bool read(IStreamIn& ifs, uint64_t size) = 0;
    virtual bool write(IStreamOut& ofs) const = 0;

    virtual void setParent(BaseHeader* hdr)
        { m_parent = hdr; }

    virtual const BaseHeader* getParent() const
        { return m_parent; }

protected:
    std::string m_name = "";
    const BaseHeader* m_parent = nullptr;
};

class DataBuffer : public BaseBuffer
{
public:
    DataBuffer(const std::string& name, const BaseHeader* parent = nullptr) : BaseBuffer(name, parent) {}
    virtual ~DataBuffer() = default;

    // BaseBuffer
    virtual const std::string& getMarker() override { return marker(); }
    virtual bool read(IStreamIn& ifs, uint64_t size) override;
    virtual bool write(IStreamOut& ofs) const override;

    // DataBuffer
    DataBuffer& operator << (bool val) { uint8_t v = !!val; return add(&v, sizeof(v)); }
    DataBuffer& operator << (char val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int8_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int16_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int32_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (int64_t val) { return add(&val, sizeof(val)); }

#ifndef SU_FILESYSTEM_NOT_USING_PACKING
    DataBuffer& operator << (uint8_t val) { return addPackUnsigned(val); }
    DataBuffer& operator << (uint16_t val) { return addPackUnsigned(val); }
    DataBuffer& operator << (uint32_t val) { return addPackUnsigned(val); }
    DataBuffer& operator << (uint64_t val) { return addPackUnsigned(val); }
#else
    DataBuffer& operator << (uint8_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint16_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint32_t val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (uint64_t val) { return add(&val, sizeof(val)); }
#endif
    
    DataBuffer& operator << (float val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (double val) { return add(&val, sizeof(val)); }
    DataBuffer& operator << (const std::string& val)
    {
        *this << val.size();
        add(val.c_str(), val.size());
        return *this;
    }

    DataBuffer& operator >> (bool& val) { uint8_t v = 0; get(&v, sizeof(v)); val = v != 0; return *this; }
    DataBuffer& operator >> (char& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int8_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int16_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int32_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (int64_t& val) { return get(&val, sizeof(val)); }

#ifndef SU_FILESYSTEM_NOT_USING_PACKING
    DataBuffer& operator >> (uint8_t& val) { uint64_t v; getPackUnsigned(v); val = static_cast<uint8_t>(v); return *this; }
    DataBuffer& operator >> (uint16_t& val) { uint64_t v; getPackUnsigned(v); val = static_cast<uint16_t>(v); return *this; }
    DataBuffer& operator >> (uint32_t& val) { uint64_t v; getPackUnsigned(v); val = static_cast<uint32_t>(v); return *this; }
    DataBuffer& operator >> (uint64_t& val) { return getPackUnsigned(val); }
#else
    DataBuffer& operator >> (uint8_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint16_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint32_t& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (uint64_t& val) { return get(&val, sizeof(val)); }
#endif

    DataBuffer& operator >> (float& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (double& val) { return get(&val, sizeof(val)); }
    DataBuffer& operator >> (std::string& val)
    {
        size_t size = 0;
        *this >> size;
        val.resize(size);
        get(val.data(), val.size());
        return *this;
    }

    DataBuffer& add(const void* buf, size_t size);
    DataBuffer& addCString(const char* c_str);
    DataBuffer& addPackUnsigned(uint64_t v);
    DataBuffer& get(void* buf, size_t size);
    DataBuffer& getCString(std::string& str);
    DataBuffer& getPackUnsigned(uint64_t& v);

    virtual const ByteArray& array() const
        { return m_data; }

    virtual bool isEof() const
        { return m_pos >= m_data.size(); }

    virtual uint64_t tell() const
        { return m_pos; }

    virtual void seek(int64_t pos, Seek way);

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
    StringBuffer(const std::string& name, const BaseHeader* parent = nullptr) : BaseBuffer(name, parent) {}
    virtual ~StringBuffer() = default;

    // BaseBuffer
    virtual const std::string& getMarker() override { return marker(); }
    virtual bool read(IStreamIn& ifs, uint64_t size) override;
    virtual bool write(IStreamOut& ofs) const override;

    //
    uint32_t add(const std::string& str);
    std::vector<uint32_t> add(const StringArray& v);

    std::string get(uint32_t idx) const
        { return idx < m_data.size() ? m_data[idx] : ""; }

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
    virtual ~BaseHeader();

    void add(BaseBuffer* buf)
        { m_buffer.push_back(buf); }

    virtual BaseBuffer* bufferFabric(const std::string& name, const std::string& marker) const;

    static uint8_t getVersionMajor() { return 1; }
    static uint8_t getVersionMinor() { return 0; }

    bool read(IStreamIn& ifs);
    bool write(IStreamOut& ofs) const;

    BaseBuffer* getBuffer(const std::string& name);

protected:
    std::string m_type;
    std::vector<BaseBuffer*> m_buffer;
};

template<class T>
inline DataBuffer& operator << (DataBuffer& db, const std::vector<T>& v)
{
    db << v.size();
    for (auto item : v)
    {
        db << item;
    }
    return db;
}

template<class T>
inline DataBuffer& operator >> (DataBuffer& db, std::vector<T>& v)
{
    size_t count;

    db >> count;
    v.clear();
    v.resize(count);
    for (size_t ii = 0; ii < count; ++ii)
    {
        db >> v[ii];
    }
    return db;
}

} // namespace fs
} // namespace su


#endif
