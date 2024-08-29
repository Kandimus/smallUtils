
#include "udp_node.h"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

#include "log.h"

namespace su
{
namespace Net
{

UdpNode::UdpNode(int32_t id, Log* plog) :
    su::Net::Node(id, plog)
{}

UdpNode::UdpNode(SOCKET socket, const sockaddr_in& addr, int32_t id, Log* plog) :
    su::Net::Node(socket, addr, id, plog)
{}

RecvStatus UdpNode::recv(uint8_t* data, size_t size, const sockaddr_in& addr)
{
    RawData packet(addr, size);

    memcpy(packet.raw.data(), data, size);

    m_packets.push_back(std::move(packet));

    return su::Net::RecvStatus::Complited;
}

RawData UdpNode::extractPacket()
{
    if (m_packets.empty())
    {
        return RawData();
    }

    auto out = std::move(m_packets.back());
    m_packets.pop_back();

    return out;
}

} // namespace Net
} // namespace su
