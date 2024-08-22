
#include "net/node.h"
#include <vector>
#include <ws2tcpip.h>

namespace su
{
namespace Net
{

Node::Node(SOCKET socket, const sockaddr_in& addr, int32_t id, Log* plog)
{
    m_socket = socket;
    m_addr = addr;
    m_log = plog;
    m_id = id;

    m_ip = std::to_string(m_addr.sin_addr.S_un.S_un_b.s_b1) + "." +
           std::to_string(m_addr.sin_addr.S_un.S_un_b.s_b2) + "." +
           std::to_string(m_addr.sin_addr.S_un.S_un_b.s_b3) + "." +
           std::to_string(m_addr.sin_addr.S_un.S_un_b.s_b4);
    m_fullId = (m_id >= 0) ? std::to_string(m_id) + "/" + m_ip : m_ip;
}

Node::~Node()
{
    disconnect();
}

Result Node::configureAddress(const std::string& ip, uint16_t port)
{
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr.s_addr);

    return OK;
}

Result Node::createTcpServer()
{
    m_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    return m_socket == SOCKET_ERROR ? CantCreateSocket : OK;
}

Result Node::createUdpServer()
{
    m_socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    return m_socket == SOCKET_ERROR ? CantCreateSocket : OK;
}

Result Node::createTcpClient()
{
    m_socket = ::socket(AF_INET, SOCK_STREAM, 0);

    return m_socket == SOCKET_ERROR ? CantCreateSocket : OK;
}

Result Node::configureParameters()
{
    if (::setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&m_maxSendBuff, sizeof(m_maxSendBuff)) == SOCKET_ERROR)
    {
        disconnect();
        return CantSetRecvBuff;
    }

    if (::setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&m_maxRecvBuff, sizeof(m_maxRecvBuff)) == SOCKET_ERROR)
    {
        disconnect();
        return CantSetSendBuff;
    }

    // disabling of Nagle's algorithm
    int flag = 1;
    if (::setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) == SOCKET_ERROR)
    {
        disconnect();
        return CantDisableNagle;
    }

    auto result = configureNoBlock();
    if (result != Result::OK)
    {
        disconnect();
        return result;
    }

    return Result::OK;
}

Result Node::configureNoBlock()
{
#ifdef WIN32
    u_long uflag = 1;
    return ::ioctlsocket(m_socket, FIONBIO, &uflag) == SOCKET_ERROR ? CantSetNoBlock : OK;
#else
    ::fcntl(m_socket, F_SETFL, O_NONBLOCK);
#endif
}

Result Node::configureKeepAlive()
{
    int enableKeepAlive = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enableKeepAlive, sizeof(enableKeepAlive)) != 0)
    {
        disconnect();
        return CantSetKeepAlive;
    }

    // The interval between the last data packet sent (simple ACKs are not considered data) and the first
    // keepalive probe; after the connection is marked to need keepalive, this counter is not used any further
    int keepIdle = 2;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&keepIdle, sizeof(keepIdle)) != 0)
    {
        disconnect();
        return CantSetKeepAlive; // Can't set socket keepidle
    }

    // The number of unacknowledged probes to send before considering the connection dead and notifying the application layer
    int count = 3;//10;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&count, sizeof(count)) != 0)
    {
        disconnect();
        return Result::CantSetKeepAlive; // Can't set socket keepcnt
    }

    // The interval between subsequential keepalive probes, regardless of what the connection has exchanged in the meantime
    int interval = 1;//12;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&interval, sizeof(interval)) != 0)
    {
        disconnect();
        return Result::CantSetKeepAlive; // Can't set socket keepintvl
    }

    return OK;
}

Result Node::configureReuse()
{
    int flag_on = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag_on, sizeof(flag_on)) == SOCKET_ERROR)
    {
        disconnect();
        return CantSetReuse;
    }

    return OK;
}

Result Node::openTcpServer()
{
    if (bind(m_socket, (sockaddr*)&m_addr, sizeof(m_addr)) < 0)
    {
        disconnect();
        return Result::CantBind;
    }

    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        disconnect();
        return Result::CantListen;
    }

    return Result::OK;
}

Result Node::openUdpServer(bool isMulticast)
{
    if (bind(m_socket, (sockaddr*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
    {
        disconnect();
        return CantBind;
    }

    if (isMulticast)
    {
        struct ip_mreq op;
        op.imr_interface.s_addr = INADDR_ANY;
        inet_pton(AF_INET, m_ip.c_str(), &op.imr_multiaddr.s_addr);

        if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&op, sizeof(op)) == SOCKET_ERROR)
        {
            disconnect();
            return CantEnableMulticast;
        }
    }

    return OK;
}

Result Node::connectToTcpServer()
{
    if (::connect(m_socket, (struct sockaddr*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
    {
        disconnect();
        return CantConnect;
    }

    return OK;
}

RecvStatus Node::recv(uint8_t* read_buff, size_t read_size)
{
    return Complited;
}

size_t Node::send(const void *packet, size_t size)
{
    size_t sendbytes = 0;
    size_t result = 0;
    uint8_t* buff = (uint8_t*)packet;

    while (sendbytes < size)
    {
        result = ::send(m_socket, (const char*)(buff + sendbytes), int(size - sendbytes), 0);

        if (result <= 0)
        {
            return result;
        }
        
        sendbytes += result;
    }

    return sendbytes;
}

bool Node::disconnect()
{
    if (m_socket == SOCKET_ERROR)
    {
        return false;
    }

    ::shutdown(m_socket, SD_BOTH);
    ::closesocket(m_socket); // Posix "close"
    m_socket = SOCKET_ERROR;

    return true;
}

bool Node::isConnected() const
{
    return m_socket != SOCKET_ERROR;
}

int Node::getLastError() const
{
    return ::WSAGetLastError();
}

RecvStatus Node::readFromSocket()
{
    std::vector<uint8_t> buff(m_maxRecvBuff * 2);
    RecvStatus result = RecvStatus::Fault;
    auto readbytes = ::recv(m_socket, (char*)buff.data(), (int)buff.size(), 0);

    if (::WSAGetLastError())
    {
        disconnect();
        return Fault;
    }

    if (readbytes > 0)
    {
        result = recv(buff.data(), readbytes);
    }

    if (readbytes == -1 && (errno != EAGAIN || errno != EWOULDBLOCK))
    {
        return NoComplited; // Все хорошо, даже если мы ничего не считали, то сокет висит на приеме данных
    }

    if (readbytes < 0 || result == Fault)
    {
        disconnect();
        return Fault;
    }

    return result;
}

} // namespace Net
} // namespace su
