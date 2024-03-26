#include "filesystem.h"

namespace su
{
namespace FileSystem
{

const std::string DataBuffer::m_marker = "db";
const std::string StringBuffer::m_marker = "sb";

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

DataBuffer& DataBuffer::get(void* buf, size_t size)
{
    int8_t* v = reinterpret_cast<int8_t*>(buf);

    if (m_pos + size >= m_data.size())
    {
        m_pos = m_data.size();
        return *this;
    }

    for (int ii = 0; ii < size; ++ii, ++m_pos)
    {
        v[ii] = m_data[m_pos];
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

ByteArray StringBuffer::getData() const
{
    DataBuffer db("");

    db << m_data.size();
    for (const auto& str: m_data)
    {
        db << str;
    }

    return db.getData();
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

bool BaseHeader::write(std::ofstream& ofs) const
{
    BufferOffset header;
    std::vector<BufferOffset> offset;
    DataBuffer out("");

    offset.resize(m_buffer.size());

    out.add(m_type.c_str(), m_type.size() + 1);
    out << getVersionMajor() << getVersionMinor();

    header.pos = out.tell();
    header.size = 0;
    out << header.size;
    header.offset = out.tell();
    out << static_cast<uint8_t>(m_buffer.size());

    for (int ii = 0; ii < m_buffer.size(); ++ii)
    {
        out << m_buffer[ii]->getMarker();
        out << m_buffer[ii]->getName();

        offset[ii].pos = out.tell();
        offset[ii].offset = 0;
        offset[ii].size = 0;
        out << offset[ii].offset << offset[ii].size;
    }

    header.size = static_cast<uint32_t>(out.tell() - header.offset);

    for (int ii = 0; ii < m_buffer.size(); ++ii)
    {
        ByteArray v = m_buffer[ii]->getData();
        
        offset[ii].offset = out.tell();
        out.add(v.data(), v.size());
        offset[ii].size = static_cast<uint32_t>(out.tell() - offset[ii].offset);
    }

    out.seek(header.pos, Seek::Beg);
    out << header.size;

    for (const BufferOffset& item : offset)
    {
        out.seek(item.pos, Seek::Beg);
        out << item.offset << item.size;
    }

    ofs.write(reinterpret_cast<const char*>(out.getDataConst().data()), out.getDataConst().size());

    return true;
}

bool BaseHeader::read(std::ifstream& ifs)
{
    char ch = -1;
    m_type = "";
    while (1)
    {
        ifs.read(&ch, 1);
        if (ch)
        {
            m_type += ch;
        }
        else
        {
            break;
        }
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

    DataBuffer buff("");
    buff.read(ifs, header.size);

    uint8_t count = 0;
    buff >> count;

    for (int ii = 0; ii < count; ++ii)
    {
        std::string name;
        std::string marker;
        BufferOffset offset;

        buff >> marker;
        buff >> name;
        buff >> offset.offset;
        buff >> offset.size;

        BaseBuffer* bb = bufferFabric(name, marker);

        if (!bb)
        {
            // TODO need to add an error
            continue;
        }

        ifs.seekg(offset.offset, std::ios_base::beg);
        bb->read(ifs, offset.size);

        m_buffer.push_back(bb);
    }

    return true;
}

BaseBuffer* BaseHeader::bufferFabric(const std::string& name, const std::string& marker) const
{
    if (marker == StringBuffer::marker())
    {
        return new StringBuffer(name);
    }

    if (marker == DataBuffer::marker())
    {
        return new DataBuffer(name);
    }

    return nullptr;
}


} // namespace FileSystem
} // namespace su

