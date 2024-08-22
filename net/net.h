
#pragma once

#include <stdint.h>
#include <vector>
#include <string>

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
};

enum RecvStatus
{
    Fault = -1,
    NoComplited = 0,
    Complited = 1,
};


extern bool initWinSock2();

extern std::string ipToString(uint32_t ip);
extern std::vector<uint32_t> getLocalIps();

// UDP
extern int updSend(const std::string& ip, uint16_t port, void* data, size_t size, UdpSendType type);

}
}
