
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <stdint.h>
#include <vector>
#include <string>
#include <winsock2.h>

namespace su
{
namespace Net
{

extern const std::string TcpBroadcastAddress;

enum UdpSendType
{
    None,
    Broadcast,
    Multicast,
};

enum Result
{
    OK,
    CantCreateSocket,
    CantSetRecvBuff,
    CantSetSendBuff,
    CantDisableNagle,
    CantSetKeepAlive,
    CantBind,
    CantListen,
    CantConnect,
    Breaking,
    CantEnableMulticast,
    CantSetNoBlock,
    CantSetReuse,
    CantSetTimeout,
    CantSetLinger,
};

enum RecvStatus
{
    Fault = -1,
    NoComplited = 0,
    Complited = 1,
};

struct RawData
{
    RawData() = default;
    RawData(size_t size) : raw(size) {}
    RawData(const sockaddr_in& a, size_t size) : raw(size) { addr = a; }

    sockaddr_in addr;
    std::vector<uint8_t> raw;
};


extern bool initWinSock2();

extern std::string addrToString(const sockaddr_in& addr);
extern std::string ipToString(uint32_t ip);
extern std::vector<uint32_t> getLocalIps();

// UDP
extern int updSend(const std::string& ip, uint16_t port, void* data, size_t size, UdpSendType type);

}
}
