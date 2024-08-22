#pragma once
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <winsock2.h>

#include "log.h"
#include "net/net.h"

namespace su
{
namespace Net
{

class Node
{
public:
    Node(int32_t id = -1, Log* plog = nullptr) { m_id = id; m_log = plog; }
    Node(SOCKET socket, const sockaddr_in& addr, int32_t id = -1, Log* plog = nullptr);
    virtual ~Node();

    SOCKET socket() const { return m_socket; }

    void   setIp(const std::string& ip) { m_ip = ip; }
    Result configureAddress(const std::string& ip, uint16_t port);
    Result createTcpServer();
    Result createUdpServer();
    Result createTcpClient();
    Result configureParameters();
    Result configureNoBlock();
    Result configureKeepAlive();
    Result configureReuse();
    Result openTcpServer();
    Result openUdpServer(bool isMulticast);
    Result connectToTcpServer();

    virtual RecvStatus recv(uint8_t* read_buff, size_t read_size);
    virtual size_t send(const void *packet, size_t size);

    bool disconnect();
    bool isConnected() const;
    int  getLastError() const;

    RecvStatus readFromSocket();

    const std::string& address() const { return m_ip; }
    const std::string& fullId() const { return m_fullId; }

protected:
    SOCKET m_socket = SOCKET_ERROR;
    sockaddr_in m_addr;
    int m_lastError = 0;

    uint32_t m_maxRecvBuff = 16 * 1024;
    uint32_t m_maxSendBuff = 16 * 1024;

private:
    int32_t m_id = -1;
    Log* m_log = nullptr;

    std::string m_ip = "";
    std::string m_fullId = "";
};

} // namespace Net
} // namespace su
