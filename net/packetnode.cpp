
#include "net/packetnode.h"

namespace su
{
namespace Net
{

PacketNode::PacketNode(uint32_t magic, SOCKET socket, const sockaddr_in& addr, int32_t id, Log* plog)
    : Node(socket, addr, id, plog)
{
    m_magic = magic;
    clear();
}

std::vector<uint8_t> PacketNode::extractPacket()
{
    if (m_packets.empty())
    {
        return {};
    }

    auto out = std::move(m_packets.back());
    m_packets.pop_back();

    return out;
}

RecvStatus PacketNode::recv(uint8_t* data, size_t size)
{
    if (m_data.empty())
    {
        //m_data.reserve(m_maxRecvBuff < size ? size : m_maxRecvBuff);
        m_data.resize(size);

        memcpy(m_data.data(), data, size);
    }
    else
    {
        auto pos = m_data.size();

        m_data.resize(m_data.size() + size);

        memcpy(m_data.data() + pos, data, size);
    }

    RecvStatus check = RecvStatus::Fault;
    while ((check = checkData()) == RecvStatus::Complited)
    {
    }

    // error
    if (check == RecvStatus::Fault)
    {
        clear();
        m_data.clear();
        m_packets.clear();
        disconnect();
        return check;
    }

    return m_packets.size() ? RecvStatus::Complited : RecvStatus::NoComplited;
}

size_t PacketNode::send(const void* data, size_t size)
{
    if (size >= 0xffffffff)
    {
        return 0;
    }

    size_t fullSize = size + sizeof(PacketHeader);
    std::vector<uint8_t> packet(fullSize);
    PacketHeader* header = (PacketHeader*)packet.data();

    header->m_magic = m_magic;
    header->m_reserved = 0;
    header->m_flags = 0;
    header->m_version = m_version;
    header->m_size = static_cast<uint32_t>(size);
    header->m_dataHash = m_crc.get(data, size);
    header->m_hash = m_crc.get(header, sizeof(PacketHeader) - sizeof(header->m_hash));

    memcpy(packet.data() + sizeof(PacketHeader), data, size);

    auto out = Node::send(packet.data(), fullSize);

    return out;
}


RecvStatus PacketNode::checkData()
{
    if (!m_header.m_size)
    {
        if (m_data.size() < sizeof(PacketHeader))
        {
            return RecvStatus::NoComplited;
        }

        m_header = *(PacketHeader*)m_data.data();

        if (m_header.m_magic != m_magic || m_header.m_version != m_version)
        {
            return RecvStatus::Fault;
        }

        if (m_header.m_hash != m_crc.get(&m_header, sizeof(m_header) - sizeof(m_header.m_hash)))
        {
            return RecvStatus::Fault;
        }

        m_data.erase(m_data.begin(), m_data.begin() + sizeof(PacketHeader));
    }

    if (m_data.size() >= m_header.m_size)
    {
        std::vector<uint8_t> packet;
        packet.insert(packet.begin(), m_data.begin(), m_data.begin() + m_header.m_size);
        m_data.erase(m_data.begin(), m_data.begin() + m_header.m_size);
        
        if (m_header.m_dataHash != m_crc.get(packet.data(), packet.size()))
        {
            return RecvStatus::Fault;
        }
        
        m_packets.emplace_back(packet);

        clear();
        return RecvStatus::Complited;
    }

    return RecvStatus::NoComplited;
}

void PacketNode::clear()
{
    m_header.m_magic = 0;
    m_header.m_size = 0;
}

} // namespace Net
} // namespace su
