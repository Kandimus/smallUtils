
#pragma once

#include "net/node.h"

namespace su
{
namespace Net
{

class UdpNode: public Node
{
public:
    UdpNode(int32_t id = -1, su::Log* plog = nullptr);
    UdpNode(SOCKET socket, const sockaddr_in& addr, int32_t id = -1, su::Log* plog = nullptr);

    size_t countOfPackets() const { return m_packets.size(); }
    std::vector<uint8_t> extractPacket();
    void clearPackets() { m_packets.clear(); }

    // su::Net::Node
    virtual su::Net::RecvStatus recv(uint8_t* read_buff, size_t read_size) override;

protected:
    std::vector<std::vector<uint8_t>> m_packets;
};

} // namespace Net
} // namespace su
