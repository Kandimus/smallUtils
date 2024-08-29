
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

    size_t countRecvPackets() const { return m_recvPackets.size(); }
    RawData extractRecvPacket();
    void clearRecvPackets() { m_recvPackets.clear(); }

    size_t countSendPackets() const { return m_sendPackets.size(); }
    void clearSendPackets() { m_sendPackets.clear(); }

    // su::Net::Node
    virtual RecvStatus recv(uint8_t* data, size_t size, const sockaddr_in& addr) override;
    virtual size_t send(const void* data, size_t size) override;
    virtual bool sendToSocket() override;

protected:
    RecvStatus checkData(const sockaddr_in& addr);
    void clear();

protected:
    uint32_t m_magic;
    const uint16_t m_version = 0x0100;
    std::vector<uint8_t> m_data;
    std::vector<RawData> m_recvPackets;
    std::vector<std::vector<uint8_t>> m_sendPackets;
    PacketHeader m_header;
    Crc32 m_crc;
};

} // namespace Net
} // namespace su
