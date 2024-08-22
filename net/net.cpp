
#include <net/net.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

namespace su
{
namespace Net
{

const std::string TcpBroadcastAddress = "0.0.0.0";

bool initWinSock2()
{
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == NO_ERROR;
}

std::string ipToString(uint32_t ip)
{
    unsigned char* octet = (unsigned char*)&ip;
    char str[32];

    sprintf(str, "%i.%i.%i.%i", octet[0], octet[1], octet[2], octet[3]);

    return std::string(str);
}

std::vector<uint32_t> getLocalIps()
{
    char hostnamebuff[100];

    if (gethostname(hostnamebuff, 100))
    {
        return {};
    }

    struct hostent* hostinfo = ::gethostbyname(hostnamebuff);

    if (hostinfo == NULL)
    {
        return {};
    }

    char** paddrlist = hostinfo->h_addr_list;
    std::vector<uint32_t> out;

    while (*paddrlist != NULL)
    {
        char addrbuff[100];
        if (::inet_ntop(hostinfo->h_addrtype, *paddrlist, addrbuff, INET_ADDRSTRLEN))
        {
            uint32_t addr;

            inet_pton(AF_INET, addrbuff, &addr);
            out.push_back(addr);
        }

        paddrlist++;
    }

    return out;
}

// The broadcast address expample:
//  192.168.255.255
//  10.255.255.255
//  255.255.255.255
//
// Multicast address expample:
//  220..239.x.y.z
int updSend(const std::string& ip, uint16_t port, void* data, size_t size, UdpSendType type)
{
    SOCKET soc = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (soc == SOCKET_ERROR)
    {
        return 0;
    }

    struct sockaddr_in addr;

    addr.sin_addr.s_addr = 0;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);

    switch (type)
    {
        case None: break;
        case Broadcast:
        {
            // Set broadcast attributes
            int op = 1;
            ::setsockopt(soc, SOL_SOCKET, SO_BROADCAST, (const char*)&op, sizeof(op));
        }
        break;

        case Multicast:
        {
            struct in_addr imr_multiaddr;
            ::inet_pton(AF_INET, ip.c_str(), &imr_multiaddr.s_addr);
            ::setsockopt(soc, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&imr_multiaddr, sizeof(imr_multiaddr));
        }
        break;
    }

    int out = ::sendto(soc, (const char*)data, (int)size, 0, (struct sockaddr*)&addr, (int)sizeof(addr));
    ::closesocket(soc);

    return out;
}

} // namespace Net
}
