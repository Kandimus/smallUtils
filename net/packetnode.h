
#pragma once

#include <vector>
#include "net/node.h"
#include "crc32.h"

namespace su
{

namespace Net
{

class PacketNode : public Node
{
    struct PacketHeader
    {
        uint32_t m_magic = 0;
        uint16_t m_version = 0;
        uint16_t m_flags = 0;
        uint32_t m_reserved = 0;
        uint32_t m_size = 0;
        uint32_t m_dataHash =0;
        uint32_t m_hash = 0;
    };

public:
    PacketNode() = delete;
    PacketNode(uint32_t magic, SOCKET socket, const sockaddr_in& addr, int32_t id, Log* plog);
    virtual ~PacketNode() = default;

    size_t countOfPackets() const { return m_packets.size(); }
    std::vector<uint8_t> extractPacket();
    void clearPackets() { m_packets.clear(); }

    // su::Net::Node
    virtual RecvStatus recv(uint8_t* data, size_t size) override;
    virtual size_t send(const void* data, size_t size) override;

protected:
    RecvStatus checkData();
    void clear();

protected:
    uint32_t m_magic;
    const uint16_t m_version = 0x0100;
    std::vector<uint8_t> m_data;
    std::vector<std::vector<uint8_t>> m_packets;
    PacketHeader m_header;
    Crc32 m_crc;
};

} // namespace Net
} // namespace su
