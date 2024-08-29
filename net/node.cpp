
#include <thread>

#include "net/node.h"
#include <vector>
#include <ws2tcpip.h>
#include <ws2def.h>

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

    m_ip = addrToString(m_addr);
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

    // The interval (seconds) between the last data packet sent (simple ACKs are not considered data) and
    // the first keepalive probe; after the connection is marked to need keepalive, this counter is not
    // used any further
    int keepIdle = 1;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&keepIdle, sizeof(keepIdle)) != 0)
    {
        disconnect();
        return CantSetKeepAlive; // Can't set socket keepidle
    }

    // The interval (seconds) between subsequential keepalive probes, regardless of what the connection has exchanged
    // in the meantime
    int interval = 1;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&interval, sizeof(interval)) != 0)
    {
        disconnect();
        return Result::CantSetKeepAlive; // Can't set socket keepintvl
    }

    // The number of unacknowledged probes to send before considering the connection dead and notifying
    // the application layer
    int count = 3;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&count, sizeof(count)) != 0)
    {
        disconnect();
        return Result::CantSetKeepAlive; // Can't set socket keepcnt
    }

#ifdef WIN32
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (::setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
    {
        disconnect();
        return Result::CantSetTimeout;
    }

    if (::setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
    {
        disconnect();
        return Result::CantSetTimeout;
    }
#else
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_USER_TIMEOUT, (const char*)&count, sizeof(count)) != 0)
    {
        disconnect();
        return Result::CantSetTimeout; // Can't set socket keepcnt
    }
#endif
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

Result Node::configureImmediatelyClose()
{
    linger l;
    l.l_onoff = 1;
    l.l_linger = 0;

    if (setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l)) == SOCKET_ERROR)
    {
        disconnect();
        return CantSetLinger;
    }

    m_immediatelyClose = true;
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

    m_countOfRecvErrrors = 0;
    m_countOfSendErrrors = 0;
    return OK;
}

RecvStatus Node::recv(uint8_t*, size_t, const sockaddr_in&)
{
    return Complited;
}

size_t Node::send(const void *packet, size_t size)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    size_t pos = m_sendBuffer.size();
    m_sendBuffer.resize(m_sendBuffer.size() + size);

    memcpy(m_sendBuffer.data() + pos, packet, size);

    return m_sendBuffer.size();
}

bool Node::disconnect()
{
    if (m_socket == SOCKET_ERROR)
    {
        return false;
    }

    if (!m_immediatelyClose)
    {
        shutdown(m_socket, SD_SEND);
    }

    closesocket(m_socket);
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

bool Node::tooManySendErrors() const
{
    return m_maxOfSendErrrors ? m_countOfSendErrrors >= m_maxOfSendErrrors : false;
}

bool Node::tooManyRecvErrors() const
{
    return m_maxOfRecvErrrors ? m_countOfRecvErrrors >= m_maxOfRecvErrrors : false;
}

int32_t Node::lastRecvBytes() const
{
    return m_recvBytes;
}

int32_t Node::lastSendBytes() const
{
    return m_sendBytes;
}

RecvStatus Node::readFromSocket()
{
    std::vector<uint8_t> buff(m_maxRecvBuff * 2);
    RecvStatus result = RecvStatus::Fault;
    sockaddr_in addr;
    int addrSize = sizeof(addr);

    m_recvBytes = ::recvfrom(m_socket, (char*)buff.data(), (int)buff.size(), 0, (sockaddr*)&addr, &addrSize);

    if (::WSAGetLastError())
    {
        return Fault;
    }

    if (m_recvBytes > 0)
    {
        result = recv(buff.data(), m_recvBytes, addr);
    }

    if (m_recvBytes <= 0 && errno == ETIMEDOUT)
    {
        LOGSPD(m_log, "Socket %s, result %i, errno %i (ETIMEDOUT)", m_fullId.c_str(), m_recvBytes, errno);
    }

    if (m_recvBytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        LOGSPD(m_log, "Socket %s, result %i, errno %i", m_fullId.c_str(), m_recvBytes, errno);
        return NoComplited;
    }

    // -1 - error
    // 0 - may be it is keep-alive and remote socket was close
    if (m_recvBytes <= 0 || result == Fault)
    {
        ++m_countOfRecvErrrors;
        return NoComplited;
        //return Fault;
    }

    m_countOfRecvErrrors = 0;
    return result;
}

bool Node::sendToSocket()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    m_sendBytes = 0;

    if (m_sendBuffer.empty())
    {
        return true;
    }

    m_sendBytes = ::send(m_socket, (char*)m_sendBuffer.data(), (int)m_sendBuffer.size(), 0);
    if (m_sendBytes < 0)
    {
        ++m_countOfSendErrrors;
        return true;
    }

    m_countOfSendErrrors = 0;
    m_sendBuffer.erase(m_sendBuffer.begin(), m_sendBuffer.begin() + m_sendBytes);
    return true;
}

} // namespace Net
} // namespace su
