
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

void TcpClient::connect(const std::string& ip, uint16_t port)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    destroy();

    m_ip = ip;
    m_port = port;
    m_doConnect = true;
}

bool TcpClient::doConnect()
{
    if (!m_doConnect)
    {
        return true;
    }

    m_lastError.store(OK);

    destroy();

    m_node.configureAddress(m_ip, m_port);

    if ((m_lastError = m_node.createTcpClient()) != OK)
    {
        return false;
    }

    if ((m_lastError = m_node.connectToTcpServer()) != OK)
    {
        return false;
    }

    if ((m_lastError = m_node.configureParameters()) != OK)
    {
        return false;
    }

    if ((m_lastError = m_node.configureKeepAlive()) != OK)
    {
        return false;
    }

    if (m_settingKeepAlive)
    {
        m_timerKeepAlive.start(m_settingKeepAlive);
    }

    m_isConnected.store(true);
    m_doConnect.store(false);

    if (!onConnect())
    {
        destroy();
        m_isConnected.store(false);
        m_lastError.store(Breaking);
        return false;
    }

    return true;
}

size_t TcpClient::send(void* packet, size_t size)
{
    return m_node.send(packet, size);
}

void TcpClient::restartKeepAliveTimer()
{
    if (m_timerKeepAlive.isStarted())
    {
        m_timerKeepAlive.restart();
    }
}

void TcpClient::doWork()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    doConnect();

    if (!isConnected())
    {
        return;
    }

    if (!m_node.isConnected())
    {
        m_node.disconnect();
        m_isConnected.store(false);
    }

    int     maxFd;
    fd_set  readfds;
//    fd_set  writefds;
    fd_set  exfds;
    timeval tv;

    FD_ZERO(&readfds);
//    FD_ZERO(&writefds);
    FD_ZERO(&exfds);
    FD_SET(m_node.socket(), &readfds);
//    FD_SET(m_node.socket(), &writefds);
    FD_SET(m_node.socket(), &exfds);
    tv.tv_sec = m_selectSec;
    tv.tv_usec = m_selectUSec;
    maxFd = (int)m_node.socket();

    auto result = select(maxFd + 1, &readfds, /*&writefds*/nullptr, &exfds, &tv);
    if (result == -1)
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

//    if (!FD_ISSET(m_node.socket(), &writefds))
//    {
//        LOGSPW(m_log, "Send data error. Disconnecting");
//        destroy();
//        return;
//    }
    
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

    if (!m_node.sendToSocket())
    {
        LOGSPW(m_log, "Can not send data to server. Disconnecting");
        destroy();
        return;
    }

    // Timeout
    if ((m_node.lastRecvBytes() > 0 || m_node.lastSendBytes() > 0) && m_timerKeepAlive.isStarted())
    {
        m_timerKeepAlive.restart();
    }
    if (m_timerKeepAlive.isFinished())
    {
        LOGSPW(m_log, "Timeout of recv/send function. Disconnecting");
        destroy();
        return;
    }

    // Errors
    if (m_node.tooManySendErrors())
    {
        LOGSPW(m_log, "Too many sending errors. Disconnecting");
        destroy();
        return;
    }

    if (m_node.tooManyRecvErrors())
    {
        LOGSPW(m_log, "Too many receiving errors. Disconnecting");
        destroy();
        return;
    }
}

void TcpClient::doFinished()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    LOGSPW(m_log, "TcpClient %s has been finished", m_node.fullId().c_str());
    destroy();
}

void TcpClient::destroy()
{
    m_isConnected.store(false);
    m_node.disconnect();
}

} // namespace Net
} // namespace su
