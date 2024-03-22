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

enum Seek
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

    virtual ByteArray getData() const = 0;

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

    DataBuffer& add(const void* buf, size_t size)
    {
        const int8_t* v = reinterpret_cast<const int8_t*>(buf);

        for (int ii = 0; ii < size; ++ii)
        {
            if (m_pos < m_data.size())
            {
                m_data[m_pos] = v[ii];
            }
            else
            {
                m_data.push_back(v[ii]);
            }
            ++m_pos;
        }
        return *this;
    }

    virtual ByteArray getData() const override
        { return m_data; }
    virtual const ByteArray& getDataConst() const
        { return m_data; }

    virtual uint64_t tell() const
        { return m_pos; }

    virtual void seek(int64_t pos, Seek way)
    {
        int64_t newpos = pos;
        if (way == Seek::Cur)
        {
            newpos = m_pos + pos;
        }
        else if (way == Seek::End)
        {
            newpos = m_data.size() -pos;
        }
        m_pos = newpos < 0 ? 0 : (newpos < m_data.size() ? pos : m_data.size());
    }

protected:
    uint64_t m_pos = 0;
    ByteArray m_data;
};

class StringBuffer : public BaseBuffer
{
public:
    StringBuffer(const std::string& name) : BaseBuffer(name) {}
    virtual ~StringBuffer() = default;

    uint32_t add(const std::string& str)
    {
        for (uint32_t ii = 0; ii < m_data.size(); ++ii)
        {
            if (m_data[ii] == str)
            {
                return ii;
            }
        }
        m_data.push_back(str);
        return static_cast<uint32_t>(m_data.size() - 1);
    }

    std::vector<uint32_t> add(const StringArray& v)
    {
        std::vector<uint32_t> out;

        for (const auto& item : v)
        {
            auto id = add(item);
            bool found = false;

            for (uint32_t ii = 0; ii < out.size(); ++ii)
            {
                if (out[ii] == id)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                out.push_back(id);
            }
        }

        return out;
    }

    virtual ByteArray getData() const override
    {
        DataBuffer db("");

        db << m_data.size();
        for (const auto& str: m_data)
        {
            db << str;
        }

        return db.getData();
    }

protected:
    StringArray m_data;
};

class Header
{
public:
    Header(const std::string& type) : m_type(type) {}
    virtual ~Header() = default;

    void add(BaseBuffer* buf)
        { m_buffer.push_back(buf); }

    static uint8_t getVersionMajor() { return 1; }
    static uint8_t getVersionMinor() { return 0; }

    bool write(std::ofstream& ofs) const
    {
        struct BufferOffset
        {
            uint64_t pos;
            uint64_t offset;
            uint32_t size;
        };
        std::vector<BufferOffset> offset;
        DataBuffer out("");

        offset.resize(m_buffer.size());

        out.add(m_type.c_str(), m_type.size() + 1);
        out << getVersionMajor() << getVersionMinor();
        out << static_cast<uint8_t>(m_buffer.size());

        for (int ii = 0; ii < m_buffer.size(); ++ii)
        {
            out << m_buffer[ii]->getName();

            offset[ii].pos = out.tell();
            offset[ii].offset = 0;
            offset[ii].size = 0;
            out << offset[ii].offset << offset[ii].size;
        }

        for (int ii = 0; ii < m_buffer.size(); ++ii)
        {
            ByteArray v = m_buffer[ii]->getData();
            
            offset[ii].offset = out.tell();
            out.add(v.data(), v.size());
            offset[ii].size = static_cast<uint32_t>(out.tell() - offset[ii].offset);
        }

        for (const BufferOffset& item : offset)
        {
            out.seek(item.pos, Seek::Beg);
            out << item.offset << item.size;
        }

        ofs.write(reinterpret_cast<const char*>(out.getDataConst().data()), out.getDataConst().size());

        return true;
    }

protected:
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
