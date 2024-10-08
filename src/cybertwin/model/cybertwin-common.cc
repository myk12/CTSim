#include "cybertwin-common.h"

namespace ns3
{

void
DoSocketMethod(int (Socket::*Method)(const Address&),
               Ptr<Socket> socket,
               const Address& address,
               uint16_t port)
{
    int ret = -1;
    if (Ipv4Address::IsMatchingType(address))
    {
        const Ipv4Address ipv4 = Ipv4Address::ConvertFrom(address);
        const InetSocketAddress inetSocket = InetSocketAddress(ipv4, port);
        ret = ((*socket).*Method)(inetSocket);
    }
    else if (Ipv6Address::IsMatchingType(address))
    {
        const Ipv6Address ipv6 = Ipv6Address::ConvertFrom(address);
        const Inet6SocketAddress inet6Socket = Inet6SocketAddress(ipv6, port);
        ret = ((*socket).*Method)(inet6Socket);
    }else{
        NS_FATAL_ERROR("Unsupported address type");
    }

    if (ret == -1)
    {
        NS_FATAL_ERROR("Failed to execute socket method");
    }
}

uint16_t
DoSocketBind(Ptr<Socket> socket, const Address& address)
{
    uint16_t bindPort = GetBindPort(socket);
    DoSocketMethod(&Socket::Bind, socket, address, bindPort);
    return bindPort;
}

uint16_t
GetBindPort(Ptr<Socket> socket)
{
    int flag = socket->Bind();
    NS_ASSERT_MSG(flag != -1, "Failed to allocate a port");
    Address sockAddr;
    socket->GetSockName(sockAddr);
    uint16_t ret = 0;
    if (InetSocketAddress::IsMatchingType(sockAddr))
    {
        ret = InetSocketAddress::ConvertFrom(sockAddr).GetPort();
    }
    else if (Inet6SocketAddress::IsMatchingType(sockAddr))
    {
        ret = Inet6SocketAddress::ConvertFrom(sockAddr).GetPort();
    }
    return ret;
}

void NotifyCybertwinConfiguration()
{
    NS_LOG_UNCOND("\n\n======= CYBERTWIN CONFIGURATION =======\n");
#if MDTP_ENABLED
    NS_LOG_UNCOND(" - MDTP:\t\t ON");
#else
    NS_LOG_UNCOND(" - MDTP:\t\t OFF");
#endif
    NS_LOG_UNCOND("\n\n======= CYBERTWIN CONFIGURATION =======\n");
}

uint64_t StringToUint64(std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    uint64_t ret = 0;
    for (int i = 0; i < 8; i++)
    {
        ret += hash[i];
        ret <<= 8;
    }
    return ret;
}

//k 控制曲线的斜率，斜率越大曲线在中心点的附近变化越快。
//x0 是函数在斜率最大的点的横坐标位置。
double TrustRateMapping(double trust)
{
    if (trust < 0)
        trust = 0;
    if (trust > 100)
        trust = 100;

    double k = 0.2;
    double x0 = 60;

    return 1 / (1 + exp(-k * (trust - x0)));
}

} // namespace ns3