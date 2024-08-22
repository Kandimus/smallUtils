
#include "net/tcp_server.h"

#include <algorithm>
#include <WS2tcpip.h>
#include <fcntl.h>

namespace su
{
namespace Net
{

TcpServer::TcpServer(const std::string& ip, uint16_t port, uint32_t maxClients, Log *plog)
{
    m_maxClients = maxClients ? maxClients : 0xffffffff;
    m_hostIp = ip;
    m_hostPort = port;
    m_log = plog;
}

TcpServer::~TcpServer()
{
    destroy();
}

void TcpServer::setIp(const std::string& ip, uint16_t port)
{
    if (isStarted())
    {
        return;
    }

    m_hostIp = ip.size() ? ip : m_hostIp;
    m_hostPort = port ? port : m_hostPort;
}

Result TcpServer::start()
{
    return start("", 0);
}

Result TcpServer::start(const std::string& ip, uint16_t port)
{
    Result result = OK;

    if (isStarted())
    {
        return result;
    }

    setIp(ip, port);

    std::lock_guard<std::mutex> lock(m_mutex);

    m_node.configureAddress(m_hostIp, m_hostPort);

    if ((result = m_node.createTcpServer()) != OK)
    {
        return result;
    }

    if ((result = m_node.configureParameters()) != OK)
    {
        return result;
    }

    if ((result = m_node.openTcpServer()) != OK)
    {
        return result;
    }

    m_isStarted = true;

    return OK;
}

void TcpServer::close()
{
    if (!isStarted())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    destroy();
}

void TcpServer::destroy()
{
    for (auto item: m_clients)
    {
        delete item;
    }
    m_clients.clear();

    m_node.disconnect();
    m_isStarted = false;
}

void TcpServer::doFinished()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    destroy();
}

void TcpServer::doWork()
{
    if (!isStarted())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_node.socket() == SOCKET_ERROR)
    {
        destroy();
        finish();
        return;
    }

    SOCKET sockAccept;
    sockaddr_in sinAccept;
    int sinSize = sizeof(sinAccept);
    SOCKET maxFd;
    fd_set readfds;
    fd_set exfds;
    timeval tv;

    FD_ZERO(&readfds);
    FD_ZERO(&exfds);
    FD_SET(m_node.socket(), &exfds);
    tv.tv_sec = m_selectSec;
    tv.tv_usec = m_selectUSec;
    maxFd = m_node.socket();

    for (auto client: m_clients)
    {
        FD_SET(client->socket(), &readfds);
        FD_SET(client->socket(), &exfds);

        maxFd = m_node.socket() > client->socket() ? m_node.socket() : client->socket();
    }

    if (select((int)maxFd + 1, &readfds, NULL, &exfds, &tv) != SOCKET_ERROR)
    {
        if ((sockAccept = accept(m_node.socket(), (sockaddr *)&sinAccept, &sinSize)) != SOCKET_ERROR)
        {
            uint8_t* ip = (uint8_t*)&sinAccept.sin_addr.s_addr;
            LOGSPN(m_log, "Accepting the client from %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

            if (!checkWhiteIp(sinAccept.sin_addr.s_addr))
            {
                LOGSPN(m_log, "Accepting client not present in the `white list`. The %02i.%02i.%02i.%02i client has been disconted",
                       ip[0], ip[1], ip[2], ip[3]);
                ::shutdown(sockAccept, SD_BOTH);
                ::closesocket(sockAccept);
            }
            else
            {
                Result result = OK;
                auto acceptedClient = newClient(sockAccept, sinAccept);
                LOGSPN(m_log, "The client %s has been created", acceptedClient->fullId().c_str());

                if (acceptedClient && (result = acceptedClient->configureParameters()) != OK)
                {
                    LOGSPN(m_log, "Failed to configure the accepting client parameters. The %s client has been disconted",
                           acceptedClient->fullId().c_str());
                    delete acceptedClient;
                    acceptedClient = nullptr;
                }

                if (acceptedClient && (result = acceptedClient->configureKeepAlive()) != OK)
                {
                    LOGSPN(m_log, "Failed to configure the accepting client keep alive property. The %s client has been disconted",
                           acceptedClient->fullId().c_str());
                    delete acceptedClient;
                    acceptedClient = nullptr;
                }

                if (acceptedClient)
                {
                    m_clients.push_back(acceptedClient);
                    onClientJoin(acceptedClient);
                    LOGSPN(m_log, "The client %s has been accepted", acceptedClient->fullId().c_str());

                    if (m_clients.size() > m_maxClients)
                    {
                        onClientDisconnected(m_clients[0]);
                        LOGSPW(m_log, "Maximum number of connected clients reached. The first client %s will be disconnected",
                               m_clients[0]->fullId().c_str());

                        delete m_clients[0];
                        m_clients.erase(m_clients.begin());
                    }
                }
            }
        }
    }

    if (FD_ISSET(m_node.socket(), &exfds))
    {
        //TRACEW(LogMask, "Ошибка обработки клиентов на '%s:%i/%i'.", m_hostIP.c_str(), m_hostPort, ServAddr.sin_port);
        //break; // ERROR
    }

    for (unsigned int ii = 0; ii < m_clients.size(); ++ii)
    {
        auto client = m_clients[ii];

        if (FD_ISSET(client->socket(), &exfds))
        {
            onClientDisconnected(client);

            LOGSPN(m_log, "Disconnecting the client %s", client->fullId().c_str());
            delete client;
            m_clients.erase(m_clients.begin() + ii);
            --ii;
        }
        else if (FD_ISSET(client->socket(), &readfds))
        {
            auto result = client->readFromSocket();

            if (result == Fault)
            {
                onClientDisconnected(client);

                LOGSPN(m_log, "The client %s has been disconnected", client->fullId().c_str());
                delete client;
                m_clients.erase(m_clients.begin() + ii);
                --ii;
            }
            else if (result == Complited && !onRecvFromNode(client))
            {
                onClientDisconnected(client);

                LOGSPN(m_log, "The client %s was disconnect", client->fullId().c_str());
                delete client;
                m_clients.erase(m_clients.begin() + ii);
                --ii;
            }
        }
    }
}

Node* TcpServer::newClient(SOCKET socket, const sockaddr_in& addr)
{
    return new Node(socket, addr);
}

bool TcpServer::send(Node* target, const void* packet, size_t size)
{
    bool result = true;

    if (!size)
    {
        return false;
    }

    for (auto client : m_clients)
    {
        if (!target || target == client)
        {
            result &= client->send(packet, size) == size;
        }
    }

    return result;
}

bool TcpServer::checkWhiteIp(uint32_t ip)
{
    if (m_whiteList.empty())
    {
        return true;
    }

    for (auto item: m_whiteList)
    {
        if (item == ip)
        {
            return true;
        }
    }

    return false;
}

bool TcpServer::addWhiteIp(uint32_t ip)
{
    for (auto item: m_whiteList)
    {
        if (item == ip)
        {
            return false;
        }
    }

    m_whiteList.push_back(ip);

    return true;
}

bool TcpServer::addWhiteIp(uint8_t* ip)
{
    return addWhiteIp(*(uint32_t*)ip);
}

bool TcpServer::addWhiteIp(const std::string& ip)
{
    uint32_t value = 0;
    // Parsing string "XXX.XXX.XXX.XXX"
    if (!inet_pton(AF_INET, ip.c_str(), &value))
    {
        return false;
    }

    return addWhiteIp(value);
}

int32_t TcpServer::getNextClientId()
{
    auto out = m_clientNum;
    ++m_clientNum;
    return out;
}

} // namespace Net
} // namespace su
