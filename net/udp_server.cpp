
#include "net/udp_server.h"

#include <algorithm>
#include <WS2tcpip.h>
#include <fcntl.h>

namespace su
{
namespace Net
{

UdpServer::UdpServer(Node& node, const std::string& ip, uint16_t port, Log *plog) :
    m_node(node)
{
    m_hostIp = ip;
    m_hostPort = port;
    m_log = plog;
}

UdpServer::~UdpServer()
{
    destroy();
}

void UdpServer::setIp(const std::string& ip, uint16_t port)
{
    if (isStarted())
    {
        return;
    }

    m_hostIp = ip.size() ? ip : m_hostIp;
    m_hostPort = port ? port : m_hostPort;
}

Result UdpServer::start(bool isMulticast)
{
    return start("", 0, isMulticast);
}

Result UdpServer::start(const std::string& ip, uint16_t port, bool isMulticast)
{
    Result result = OK;

    if (isStarted())
    {
        return result;
    }

    setIp(ip, port);

    std::lock_guard<std::mutex> lock(m_mutex);

    m_node.configureAddress("", m_hostPort);
    m_node.setIp(m_hostIp);

    if ((result = m_node.createUdpServer()) != OK)
    {
        return result;
    }

    if ((result = m_node.configureReuse()) != OK)
    {
        return result;
    }

    if ((result = m_node.openUdpServer(isMulticast)) != OK)
    {
        return result;
    }

    if ((result = m_node.configureNoBlock()) != OK)
    {
        return result;
    }

    m_isStarted = true;

    return OK;
}

void UdpServer::close()
{
    if (!isStarted())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    destroy();
}

void UdpServer::destroy()
{
    m_node.disconnect();
    m_isStarted = false;
}

void UdpServer::doFinished()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    destroy();
}

void UdpServer::doWork()
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

    int     maxFd;
    fd_set  readfds;
    fd_set  exfds;
    timeval tv;

    FD_ZERO(&readfds);
    FD_ZERO(&exfds);
    FD_SET(m_node.socket(), &readfds);
    FD_SET(m_node.socket(), &exfds);
    tv.tv_sec = m_selectSec;
    tv.tv_usec = m_selectUSec;
    maxFd = (int)m_node.socket();

    if (select(maxFd + 1, &readfds, NULL, &exfds, &tv) == -1)
    {
        LOGSPE(m_log, "The select function fault. Error: %i", m_node.getLastError());
        destroy();
        return;
    }

    if (FD_ISSET(m_node.socket(), &exfds))
    {
        LOGSPW(m_log, "Disconecting from multicast.");
        destroy();
        return;
    }

    if (FD_ISSET(m_node.socket(), &readfds))
    {
        auto result = m_node.readFromSocket();

        if (result == Fault)
        {
            LOGSPW(m_log, "Can not receive the data from multicast. Error: %i", m_node.getLastError());
            destroy();
            return;
        }
        else if (result == Complited && !onRecvFromNode())
        {
            LOGSPW(m_log, "Can not process the data from multicast.");
            destroy();
            return;
        }
    }
}

} // namespace Net
} // namespace su
