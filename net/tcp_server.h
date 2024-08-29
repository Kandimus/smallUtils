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

class TcpServer : public ThreadClass
{
public:
    TcpServer(const std::string& ip, uint16_t port, uint32_t maxclient, Log *plog);
    virtual ~TcpServer();

    Result start();
    Result start(const std::string& ip, uint16_t port);

    const Node* node() const { return &m_node; }

    void setIp(const std::string& ip, uint16_t port);

    bool addWhiteIp(uint32_t ip);
    bool addWhiteIp(uint8_t* ip);
    bool addWhiteIp(const std::string& ip);

    bool isStarted() const { return m_isStarted.load(); }
    uint32_t clientsCount() const { return m_clientsCount.load(); }

    bool send(Node* target, const void* packet, size_t size);

protected:
    // ThreadClass
    virtual void doWork() override;
    virtual void doFinished() override;

    // TcpServer
    virtual bool checkWhiteIp(uint32_t ip);
    virtual void onClientJoin(Node*) {}
    virtual void onClientDisconnected(Node*) {}
    virtual bool onRecvFromNode(Node* node) { return true; }
    virtual Node* newClient(SOCKET socket, const sockaddr_in& addr);

    std::mutex& getMutex() { return m_mutex; }
    Log* getLog() { return m_log; }
    int32_t getNextClientId();

private:
    void destroy();

protected:
    std::vector<Node*> m_clients;
    std::vector<uint32_t> m_whiteList;
    std::vector<uint32_t> m_hosts;
    std::string m_hostIp = "127.0.0.1";
    uint16_t m_hostPort = 1024;
    uint32_t m_selectSec = 0;
    uint32_t m_selectUSec = 100;
    uint32_t m_maxClients = 0xffffffff;
    bool m_immediatelyCloseClients = false;

private:
    Log* m_log = nullptr;
    Node m_node;
    std::mutex m_mutex;
    std::atomic<bool> m_isStarted = false;
    int32_t m_clientNum = 0;
    std::atomic<uint32_t> m_clientsCount = 0;
};

} // namespace Net
} // namespace su
