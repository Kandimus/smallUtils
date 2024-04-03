#include "filesystem.h"

namespace su
{
namespace FileSystem
{

const std::string DataBuffer::m_marker = "db";
const std::string StringBuffer::m_marker = "sb";

// Limits of packed bytes
// 1 byte  0x7f
// 2 bytes 0x3f ff
// 3 bytes 0x1f ff ff
// 4 bytes 0x0f ff ff ff
// 5 bytes 0x07 ff ff ff ff
// 6 bytes 0x03 ff ff ff ff ff
// 7 bytes 0x01 ff ff ff ff ff ff
// 8 bytes 0xff ff ff ff ff ff ff
// 9 bytes 0x7f ff ff ff ff ff ff ff


size_t packUnsigned(uint64_t val, uint8_t* buff)
{
    uint64_t maxVal = 0x7f;
    int count = 1;
    bool pushHiBit = false;

    if (val > 0x7fffffffffffffff)
    {
        pushHiBit = true;
        val = val & 0x7fffffffffffffff;
    }

    while (count <= 10)
    {
        if (val <= maxVal)
        {
            for (int ii = 0; ii < count - 1; ++ii)
            {
                *buff++ = (val & 0x7f) | 0x80;
                val >>= 7;
            }

            *buff++ = (val & 0x7f) | (pushHiBit ? 0x80 : 0x00);

            if (pushHiBit)
            {
                *buff++ = 0x01;
            }
            return count + pushHiBit;
        }
        else
        {
            ++count;
            maxVal = (maxVal << 7) | 0x7f;
        }
    }

    SU_ASSERT(true);
    return 0;
}

size_t unpackUnsigned(uint64_t& v, uint8_t* buff)
{
    v = 0;
    uint8_t ii = 0;
    for (ii = 0; ii < 10; ++ii)
    {
        uint64_t t = 0;

        t = *buff++;

        bool exit = !(t & 0x80);

        t &= 0x7F;
        t <<= (7 * ii);
        v |= t;

        if (exit)
        {
            return ii + 1;
        }
    }

    SU_ASSERT(true);
    return 0;
}

std::string readCString(std::ifstream& ifs)
{
    char ch = -1;
    std::string out = "";
    while (ch)
    {
        ifs.read(&ch, 1);
        if (ch)
        {
            out += ch;
        }
    }
    return out;
}

DataBuffer& DataBuffer::add(const void* buf, size_t size)
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

DataBuffer& DataBuffer::addCString(const char* c_str)
{
    while (c_str)
    {
        *this << *c_str;
        ++c_str;
    }
    *this << (char)0;

    return* this;
}

DataBuffer& DataBuffer::addPackUnsigned(uint64_t val)
{
    uint8_t buff[12];

    add(buff, packUnsigned(val, buff));

    return *this;
}

DataBuffer& DataBuffer::get(void* buf, size_t size)
{
    uint8_t* v = reinterpret_cast<uint8_t*>(buf);

    if (m_pos + size > m_data.size())
    {
        m_pos = m_data.size() + 1;
        return *this;
    }

    for (int ii = 0; ii < size; ++ii, ++m_pos)
    {
        v[ii] = m_data[m_pos];
    }

    return *this;
}

DataBuffer& DataBuffer::getCString(std::string& str)
{
    char ch = -1;
    
    str = "";
    while (ch)
    {
        *this >> ch;
        if (ch)
        {
            str += ch;
        }
    }

    return *this;
}

DataBuffer& DataBuffer::getPackUnsigned(uint64_t& v)
{
    if (isEof())
    {
        return *this;
    }
    auto count = unpackUnsigned(v, &m_data[m_pos]);

    m_pos += count;
    if (m_pos > m_data.size())
    {
        m_pos = m_data.size() + 1;
        v = 0;
    }
    return *this;
}

void DataBuffer::seek(int64_t pos, Seek way)
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
    m_pos = newpos < 0 ? 0 : ((size_t)newpos < m_data.size() ? pos : m_data.size());
}

bool DataBuffer::read(std::ifstream& ifs, uint64_t size)
{
    m_data.clear();
    m_data.resize(size);
    return ifs.read(reinterpret_cast<char*>(m_data.data()), size) ? true : false;
}

bool DataBuffer::write(std::ofstream& ofs) const
{
    ofs.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());

    return true;
}

// StringBuffer

uint32_t StringBuffer::add(const std::string& str)
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

std::vector<uint32_t> StringBuffer::add(const StringArray& v)
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

bool StringBuffer::read(std::ifstream& ifs, uint64_t size)
{
    DataBuffer db("");

    if (!db.read(ifs, size))
    {
        return false;
    }

    size_t count = 0;

    db >> count;
    for (size_t ii = 0; ii < count; ++ii)
    {
        std::string str;

        db >> str;
        m_data.push_back(str);
    }

    return true;
}

bool StringBuffer::write(std::ofstream& ofs) const
{
    DataBuffer db("");

    db << m_data.size();
    for (size_t ii = 0; ii < m_data.size(); ++ii)
    {
        db << m_data[ii];
    }

    return db.write(ofs);
}

// BaseHeader

BaseHeader::~BaseHeader()
{
    for (auto item : m_buffer)
    {
        if (item->getParent() == this)
        {
            delete item;
        }
    }
}

bool BaseHeader::write(std::ofstream& ofs) const
{
    BufferOffset header;
    std::vector<BufferOffset> offset;

    ofs.write(m_type.c_str(), m_type.size() + 1);

    uint8_t verMajor = getVersionMajor();
    uint8_t verMinor = getVersionMinor();
    ofs.write(reinterpret_cast<const char*>(&verMajor), sizeof(verMajor));
    ofs.write(reinterpret_cast<const char*>(&verMinor), sizeof(verMajor));

    header.pos = ofs.tellp();
    header.size = 0;
    ofs.write(reinterpret_cast<const char*>(&header.size), sizeof(header.size));
    header.offset = ofs.tellp();

    uint16_t count = static_cast<uint16_t>(m_buffer.size());
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));

    offset.resize(m_buffer.size());
    for (int ii = 0; ii < m_buffer.size(); ++ii)
    {
        auto marker = m_buffer[ii]->getMarker();
        auto name = m_buffer[ii]->getName();

        ofs.write(marker.c_str(), marker.size() + 1);
        ofs.write(name.c_str(), name.size() + 1);

        offset[ii].pos = ofs.tellp();
        offset[ii].offset = 0;
        offset[ii].size = 0;

        ofs.write(reinterpret_cast<const char*>(&offset[ii].offset), sizeof(offset[ii].offset));
        ofs.write(reinterpret_cast<const char*>(&offset[ii].size), sizeof(offset[ii].size));
    }

    header.size = static_cast<uint32_t>(static_cast<uint64_t>(ofs.tellp()) - header.offset);

    for (int ii = 0; ii < m_buffer.size(); ++ii)
    {
        offset[ii].offset = ofs.tellp();
        m_buffer[ii]->write(ofs);
        offset[ii].size = static_cast<uint32_t>(static_cast<uint64_t>(ofs.tellp()) - offset[ii].offset);
    }

    ofs.seekp(header.pos, std::ios_base::beg);
    ofs.write(reinterpret_cast<const char*>(&header.size), sizeof(header.size));

    for (const BufferOffset& item : offset)
    {
        ofs.seekp(item.pos, std::ios_base::beg);

        ofs.write(reinterpret_cast<const char*>(&item.offset), sizeof(item.offset));
        ofs.write(reinterpret_cast<const char*>(&item.size), sizeof(item.size));
    }

    return true;
}

bool BaseHeader::read(std::ifstream& ifs)
{
    std::string type = readCString(ifs);
    if (m_type.size() && m_type != type)
    {
        return false;
    }

    uint8_t verMajor = 0;
    uint8_t verMinor = 0;
    ifs.read(reinterpret_cast<char*>(&verMajor), 1);
    ifs.read(reinterpret_cast<char*>(&verMinor), 1);
    if (verMajor != getVersionMajor() || verMinor != getVersionMinor())
    {
        return false;
    }

    BufferOffset header;
    ifs.read(reinterpret_cast<char*>(&header.size), sizeof(header.size));

    uint16_t count = 0;
    ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (int ii = 0; ii < count; ++ii)
    {
        BufferOffset offset;
        std::string marker = readCString(ifs);
        std::string name = readCString(ifs);

        ifs.read(reinterpret_cast<char*>(&offset.offset), sizeof(offset.offset));
        ifs.read(reinterpret_cast<char*>(&offset.size), sizeof(offset.size));

        BaseBuffer* bb = bufferFabric(name, marker);
        if (!bb)
        {
            SU_ASSERT(true);
            continue;
        }

        auto pos = ifs.tellg();
        ifs.seekg(offset.offset, std::ios_base::beg);
        bb->read(ifs, offset.size);
        m_buffer.push_back(bb);

        ifs.seekg(pos, std::ios_base::beg);
    }

    return true;
}

BaseBuffer* BaseHeader::bufferFabric(const std::string& name, const std::string& marker) const
{
    if (marker == StringBuffer::marker())
    {
        return new StringBuffer(name, this);
    }

    if (marker == DataBuffer::marker())
    {
        return new DataBuffer(name, this);
    }

    return nullptr;
}

BaseBuffer* BaseHeader::getBuffer(const std::string& name)
{
    for (auto item : m_buffer)
    {
        if (item->getName() == name)
        {
            return item;
        }
    }
    return nullptr;
}


} // namespace FileSystem

} // namespace su

