
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
    void disconnect();
    Node* getNode() { return &m_node; } //TODO can this function be constant?
    Result connect(const std::string& ip, uint16_t port);
    size_t send(void* packet, size_t size);

protected:
    // ThreadClass
    virtual void doWork() override;
    virtual void doFinished() override;

    // TcpClient
    virtual bool onConnect() { return true; }
    virtual bool onRecvFromNode() { return true; }

    Log* getLog() { return m_log; }

private:
   void destroy();

protected:
   std::string m_ip = "127.0.0.1";
   uint16_t m_port = 1024;
   uint32_t m_selectSec = 0;
   uint32_t m_selectUSec = 100;

private:
   Log* m_log = nullptr;
   Node& m_node;
   std::mutex m_mutex;
   std::atomic_bool m_isConnected = false;
   TickCount m_timerKeepAlive;
};

} // namespace Net
} // namespace su
