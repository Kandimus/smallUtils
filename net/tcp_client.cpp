
#include "tcp_client.h"

namespace su
{
namespace Net
{

TcpClient::TcpClient(Node& node, Log* plog)
    : m_node(node)
{
    m_log = plog;
}

void TcpClient::disconnect()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    return destroy();
}

Result TcpClient::connect(const std::string& ip, uint16_t port)
{
    Result result = OK;

    std::lock_guard<std::mutex> guard(m_mutex);

    destroy();

    m_ip = ip;
    m_port = port;

    m_node.configureAddress(m_ip, m_port);

    if ((result = m_node.createTcpClient()) != OK)
    {
        return result;
    }

    if ((result = m_node.connectToTcpServer()) != OK)
    {
        return result;
    }

    if ((result = m_node.configureParameters()) != OK)
    {
        return result;
    }

    if ((result = m_node.configureKeepAlive()) != OK)
    {
        return result;
    }

    m_timerKeepAlive.start(1000);
    m_isConnected.store(true);

    if (!onConnect())
    {
        destroy();
        m_isConnected.store(false);
        return Breaking;
    }

    return OK;
}

size_t TcpClient::send(void* packet, size_t size)
{
    return m_node.send(packet, size);
}

void TcpClient::doWork()
{
    if (!isConnected())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_node.isConnected())
    {
        m_node.disconnect();
        m_isConnected.store(false);
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
        LOGSPW(m_log, "Disconecting from the server.");
        destroy();
        return;
    }
    
    if (FD_ISSET(m_node.socket(), &readfds))
    {
        auto result = m_node.readFromSocket();

        if (result == Fault)
        {
            LOGSPW(m_log, "Can not receive the data from the server. Error: %i", m_node.getLastError());
            destroy();
            return;
        }
        else if (result == Complited && !onRecvFromNode())
        {
            LOGSPW(m_log, "Can not process the data from the server.");
            destroy();
            return;
        }
    }

    if (m_timerKeepAlive.isFinished())
    {
        m_timerKeepAlive.restart();

        int result = ::send(m_node.socket(), nullptr, 0, 0);

        if (result == -1)
        {
            LOGSPW(m_log, "Server is shutdown...");
            destroy();
            return;
        }
    }
}

void TcpClient::doFinished()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    destroy();
}

void TcpClient::destroy()
{
    m_isConnected.store(false);
    m_node.disconnect();
}

} // namespace Net
} // namespace su
