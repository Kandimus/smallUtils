
#pragma once

#include <string>
#include <mutex>

#include "log.h"
#include "net/node.h"
#include "thread_class.h"
#include "tickcount.h"

namespace su
{
namespace Net
{

class TcpClient : public ThreadClass
{
public:
    TcpClient() = delete;
    TcpClient(Node& client, Log* plog = nullptr);
    virtual ~TcpClient() = default;

    bool isConnected() const { return m_isConnected.load(); }
    bool isConnecting() const { return m_doConnect.load(); }
    void disconnect();
    Node* getNode() { return &m_node; }
    void connect(const std::string& ip, uint16_t port);
    size_t send(void* packet, size_t size);
    Result getLastError() const { return m_lastError; }
    void restartKeepAliveTimer();

    std::string destination() const { return m_ip + ":" + std::to_string(m_port); }

protected:
    // ThreadClass
    virtual void doWork() override;
    virtual void doFinished() override;

    // TcpClient
    virtual bool onConnect() { return true; }
    virtual bool onRecvFromNode() { return true; }

    Log* getLog() { return m_log; }

private:
    bool doConnect();
    void destroy();

protected:
   std::string m_ip = "127.0.0.1";
   uint16_t m_port = 1024;
   uint32_t m_selectSec = 0;
   uint32_t m_selectUSec = 100;
   size_t m_settingKeepAlive = 15000;

private:
   Log* m_log = nullptr;
   Node& m_node;
   std::mutex m_mutex;
   std::atomic_bool m_isConnected = false;
   std::atomic_bool m_doConnect = false;
   std::atomic<Result> m_lastError = OK;
   TickCount m_timerKeepAlive;
};

} // namespace Net
} // namespace su
