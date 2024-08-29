#pragma once
#define WIN32_LEAN_AND_MEAN

#include <mutex>
#include <string>
#include <vector>

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
    Log* getLog() { return m_log; }

    void   setIp(const std::string& ip) { m_ip = ip; }
    Result configureAddress(const std::string& ip, uint16_t port);
    Result createTcpServer();
    Result createUdpServer();
    Result createTcpClient();
    Result configureParameters();
    Result configureNoBlock();
    Result configureKeepAlive();
    Result configureReuse();
    Result configureImmediatelyClose();
    Result openTcpServer();
    Result openUdpServer(bool isMulticast);
    Result connectToTcpServer();

    virtual RecvStatus recv(uint8_t* read_buff, size_t read_size, const sockaddr_in& addr);
    virtual size_t send(const void *packet, size_t size);
    virtual RecvStatus readFromSocket();
    virtual bool sendToSocket();

    bool disconnect();
    bool isConnected() const;
    int  getLastError() const;
    bool tooManySendErrors() const;
    bool tooManyRecvErrors() const;
    int32_t lastRecvBytes() const;
    int32_t lastSendBytes() const;

    size_t sizeSendBuffer() const { return m_sendBuffer.size(); }

    const std::string& address() const { return m_ip; }
    const std::string& fullId() const { return m_fullId; }

private:

protected:
    SOCKET m_socket = SOCKET_ERROR;
    sockaddr_in m_addr;
    int m_lastError = 0;

    uint32_t m_maxRecvBuff = 256 * 1024;
    uint32_t m_maxSendBuff = 256 * 1024;
    size_t m_maxOfSendErrrors = 0;
    size_t m_maxOfRecvErrrors = 0;//4096;

private:
    std::mutex m_mutex;
    int32_t m_id = -1;
    Log* m_log = nullptr;

    std::string m_ip = "";
    std::string m_fullId = "";

    int32_t m_recvBytes = 0;
    int32_t m_sendBytes = 0;
    size_t m_countOfSendErrrors = 0;
    size_t m_countOfRecvErrrors = 0;
    std::vector<uint8_t> m_sendBuffer;
    bool m_immediatelyClose = false;
};

} // namespace Net
} // namespace su
