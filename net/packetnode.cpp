
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

RawData PacketNode::extractRecvPacket()
{
    if (m_recvPackets.empty())
    {
        return {};
    }

    auto out = std::move(m_recvPackets.back());
    m_recvPackets.pop_back();

    return out;
}

RecvStatus PacketNode::recv(uint8_t* data, size_t size, const sockaddr_in& addr)
{
    if (m_data.empty())
    {
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
    while ((check = checkData(addr)) == RecvStatus::Complited)
    {
    }

    // error
    if (check == RecvStatus::Fault)
    {
        clear();
        m_data.clear();
        m_recvPackets.clear();
        m_sendPackets.clear();
        disconnect();
        return check;
    }

    return m_recvPackets.size() ? RecvStatus::Complited : RecvStatus::NoComplited;
}

size_t PacketNode::send(const void* data, size_t size)
{
    if (size >= 0xffffffff)
    {
        LOGSPE(getLog(), "Received packet is to big");
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

    m_sendPackets.push_back(std::move(packet));

    //auto out = Node::send(packet.data(), fullSize);

    return fullSize;
}

bool PacketNode::sendToSocket()
{
    if (!Node::sizeSendBuffer() && m_sendPackets.size())
    {
        Node::send(m_sendPackets[0].data(), m_sendPackets[0].size());
        m_sendPackets.erase(m_sendPackets.begin());
    }

    return Node::sendToSocket();
}

RecvStatus PacketNode::checkData(const sockaddr_in& addr)
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
            LOGSPE(getLog(), "The header of packet is unrecognizable");
            return RecvStatus::Fault;
        }

        if (m_header.m_hash != m_crc.get(&m_header, sizeof(m_header) - sizeof(m_header.m_hash)))
        {
            LOGSPE(getLog(), "The hash of packet header is broken");
            return RecvStatus::Fault;
        }

        m_data.erase(m_data.begin(), m_data.begin() + sizeof(PacketHeader));
    }

    if (m_data.size() >= m_header.m_size)
    {
        RawData packet(addr, m_header.m_size);

        memcpy(packet.raw.data(), m_data.data(), m_header.m_size);
        m_data.erase(m_data.begin(), m_data.begin() + m_header.m_size);
        
        if (m_header.m_dataHash != m_crc.get(packet.raw.data(), packet.raw.size()))
        {
            LOGSPE(getLog(), "The hash of packet is broken");
            return RecvStatus::Fault;
        }
        
        m_recvPackets.push_back(std::move(packet));

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
