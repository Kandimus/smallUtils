#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "thread_class.h"
#include "net/node.h"
#include "log.h"

namespace su
{
namespace Net
{

class UdpServer : public ThreadClass
{
public:
    UdpServer() = delete;
    UdpServer(Node& node, const std::string& ip, uint16_t port, Log *plog);
    virtual ~UdpServer();

    Result start(bool isMulticast);
    Result start(const std::string& ip, uint16_t port, bool isMulticast);
    void close();

    Node* getNode() { return &m_node; }

    void setIp(const std::string& ip, uint16_t port);

    bool isStarted() const { return m_isStarted.load(); }

protected:
    // ThreadClass
    virtual void doWork() override;
    virtual void doFinished() override;

    // McServer
    virtual bool onRecvFromNode() { return true; }

    Log* getLog() { return m_log; }

private:
    void destroy();

protected:
    std::string m_hostIp = "127.0.0.1";
    uint16_t m_hostPort = 1024;
    uint32_t m_selectSec = 0;
    uint32_t m_selectUSec = 100;

private:
    Log* m_log = nullptr;
    Node& m_node;
    std::mutex m_mutex;
    std::atomic<bool> m_isStarted = false;
};

} // namespace Net
} // namespace su
