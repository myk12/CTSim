#include "cybertwin.h"

#include "cybertwin-packet-header.h"

#include "ns3/applications-module.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Cybertwin");
NS_OBJECT_ENSURE_REGISTERED(Cybertwin);

TypeId
Cybertwin::GetTypeId()
{
    static TypeId tid = TypeId("Cybertwin").SetParent<Application>().SetGroupName("cybertwin");

    return tid;
}

Cybertwin::Cybertwin()
    : localSocket(nullptr),
      globalSocket(nullptr)
{
    NS_LOG_FUNCTION(this);
}

Cybertwin::Cybertwin(InitCybertwinCallback initCallback)
    : localSocket(nullptr),
      globalSocket(nullptr)
{
    NS_LOG_FUNCTION(this);
    initCallback();
}

Cybertwin::Cybertwin(CYBERTWINID_t id, CybertwinInterface local, CybertwinInterface global)
    : cybertwinID(id),
      localInterface(local),
      globalInterface(global)
{
    NS_LOG_INFO("Create new Cybertwin [" << cybertwinID << "]");
}

Cybertwin::~Cybertwin()
{
    NS_LOG_INFO("Delete Cybertwin [" << cybertwinID << "]");
}

void
Cybertwin::DoDispose()
{
    NS_LOG_FUNCTION(this);
    localSocket = nullptr;
    globalSocket = nullptr;
}

void
Cybertwin::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Cybertwin " << cybertwinID << " born.");
    int ret = -1;
    Address addr;
    uint16_t port;

    // init local socket
    addr = localInterface.first;
    port = localInterface.second;
    NS_LOG_DEBUG("Cybertwin: locally binding to : " << addr << ", " << port);
    if (!localSocket)
    {
        localSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        if (Ipv4Address::IsMatchingType(addr))
        {
            Ipv4Address ipv4 = Ipv4Address::ConvertFrom(addr);
            InetSocketAddress inetAddress = InetSocketAddress(ipv4, port);
            NS_LOG_DEBUG("Server bing to " << ipv4 << ":" << port);
            ret = localSocket->Bind(inetAddress);
        }
        else if (Ipv6Address::IsMatchingType(addr))
        {
            Ipv6Address ipv6 = Ipv6Address::ConvertFrom(addr);
            Inet6SocketAddress inetAddress = Inet6SocketAddress(ipv6, port);
            NS_LOG_DEBUG("Server bind to " << ipv6 << ":" << port);
            ret = localSocket->Bind(inetAddress);
        }
        else
        {
            NS_FATAL_ERROR("Cybertwin: no matching address.");
        }
    }

    if (ret == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket.");
    }

    localSocket->SetAcceptCallback(MakeCallback(&Cybertwin::localConnRequestCallback, this),
                                   MakeCallback(&Cybertwin::localNewConnCreatedCallback, this));
    // localSocket->SetRecvCallback(MakeCallback(&Cybertwin::localReceivedDataCallback, this));
    localSocket->SetCloseCallbacks(MakeCallback(&Cybertwin::localNormalCloseCallback, this),
                                   MakeCallback(&Cybertwin::localErrorCloseCallback, this));
    localSocket->Listen();
    NS_LOG_DEBUG("Cybertwin is locally listening.");

    // init global socket
    addr = globalInterface.first;
    port = globalInterface.second;
    if (!globalSocket)
    {
        globalSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        if (Ipv4Address::IsMatchingType(addr))
        {
            Ipv4Address ipv4 = Ipv4Address::ConvertFrom(addr);
            InetSocketAddress inetAddress = InetSocketAddress(ipv4, port);
            NS_LOG_DEBUG("Server bing to " << ipv4 << ":" << port);
            ret = globalSocket->Bind(inetAddress);
        }
        else if (Ipv6Address::IsMatchingType(addr))
        {
            Ipv6Address ipv6 = Ipv6Address::ConvertFrom(addr);
            Inet6SocketAddress inet6Address = Inet6SocketAddress(ipv6, port);
            NS_LOG_DEBUG("Server bind to " << ipv6 << ":" << port);
            ret = globalSocket->Bind(inet6Address);
        }
    }

    if (ret == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket.");
    }
    globalSocket->SetAcceptCallback(MakeCallback(&Cybertwin::globalConnRequestCallback, this),
                                    MakeCallback(&Cybertwin::globalNewConnCreatedCallback, this));
    // globalSocket->SetRecvCallback(MakeCallback(&Cybertwin::globalReceivedDataCallback, this));
    globalSocket->SetCloseCallbacks(MakeCallback(&Cybertwin::globalNormalCloseCallback, this),
                                    MakeCallback(&Cybertwin::globalErrorCloseCallback, this));
    globalSocket->Listen();
    NS_LOG_DEBUG("Cybertwin is globally listening.");

    // Simulator::Schedule(Seconds(0.), &Cybertwin::ouputPackets, this);
}

void
Cybertwin::StopApplication()
{
    NS_LOG_FUNCTION(this);
    // The host might stop and close the connection before the cybertwin stop
    if (localSocket)
    {
        localSocket->Close();
        localSocket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                                       MakeNullCallback<void, Ptr<Socket>>());
    }
    globalSocket->Close();
    globalSocket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                                    MakeNullCallback<void, Ptr<Socket>>());
    NS_LOG_INFO("Cybertwin " << cybertwinID << " stop.");
}

CYBERTWINID_t
Cybertwin::GetCybertwinID() const
{
    return cybertwinID;
}

void
Cybertwin::SetCybertwinID(CYBERTWINID_t cybertwinID)
{
    this->cybertwinID = cybertwinID;
}

void
Cybertwin::SetLocalInterface(Address addr, uint16_t port)
{
    localInterface = std::make_pair(addr, port);
}

void
Cybertwin::SetGlobalInterface(Address addr, uint16_t port)
{
    globalInterface = std::make_pair(addr, port);
}

//********************************************************************************
//*                        Define local function                                 *
//********************************************************************************

bool
Cybertwin::localConnRequestCallback(Ptr<Socket> socket, const Address& address)
{
    NS_LOG_DEBUG("* Cybertwin * : receive a local connectiong request.");
    return true;
}

void
Cybertwin::localNewConnCreatedCallback(Ptr<Socket> socket, const Address& addr)
{
    NS_LOG_DEBUG("* Cybertwin * : TCP connection with endhost established.");
    socket->SetRecvCallback(MakeCallback(&Cybertwin::localRecvHandler, this));
}

void
Cybertwin::localRecvHandler(Ptr<Socket> socket)
{
    NS_LOG_DEBUG("* Cybertwin * : Cybertwin received local packet.");

    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom(from)))
    {
        CYBERTWINID_t id;
        CybertwinPacketHeader header;
        std::queue<Ptr<Packet>> packetQueue;

        NS_LOG_DEBUG("Packet size:" << packet->GetSize() << " content:" << packet->ToString()
                                    << " uid:" << packet->GetUid());

        socket->GetSockName(localAddress);
        packet->RemoveHeader(header);
        id = static_cast<CYBERTWINID_t>(header.GetSrc());

        // TODO: it must lock the queue before push data to it.
        if (txPacketBuffer.find(id) == txPacketBuffer.end())
        {
            txPacketBuffer[id] = std::queue<Ptr<Packet>>();
        }

        // push packet to queue
        txPacketBuffer[id].push(packet);
    }
}

void
Cybertwin::localNormalCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_INFO("-[Cybertwin]: Connection closed normally");
    socket->ShutdownSend();
    if (socket == localSocket)
    {
        localSocket = nullptr;
    }
}

void
Cybertwin::localErrorCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_ERROR("A socket error occurs:" << socket->GetErrno());
}

//********************************************************************************
//*                        Define global function                                *
//********************************************************************************

bool
Cybertwin::globalConnRequestCallback(Ptr<Socket> socket, const Address& address)
{
    return true;
}

void
Cybertwin::globalNewConnCreatedCallback(Ptr<Socket> socket, const Address& addr)
{
    NS_LOG_INFO("TCP connection established.");
    socket->SetRecvCallback(MakeCallback(&Cybertwin::localRecvHandler, this));
}

void
Cybertwin::globalRecvHandler(Ptr<Socket> socket)
{
    NS_LOG_INFO("Cybertwin received global packet.");

    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom(from)))
    {
        CYBERTWINID_t id;
        CybertwinPacketHeader header;
        std::queue<Ptr<Packet>> packetQueue;

        socket->GetSockName(localAddress);
        packet->RemoveHeader(header);
        id = static_cast<CYBERTWINID_t>(header.GetSrc());

        // TODO: it must lock the queue before push data to it.
        if (rxPacketBuffer.find(id) == rxPacketBuffer.end())
        {
            rxPacketBuffer[id] = std::queue<Ptr<Packet>>();
        }

        // push packet to rxqueue
        rxPacketBuffer[id].push(packet);
    }
}

void
Cybertwin::globalNormalCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_INFO("Connection normally close.");
    socket->ShutdownSend();
}

void
Cybertwin::globalErrorCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_ERROR("A socket error occurs:" << socket->GetErrno());
}

void
Cybertwin::ouputPackets()
{
    // NS_LOG_DEBUG("Cybertwin outputPakcets proces.");
    // naive output method
    for (auto rxBuffer : rxPacketBuffer)
    {
        CYBERTWINID_t dstGuid = rxBuffer.first;
        std::queue<Ptr<Packet>> packetQueue = rxBuffer.second;
        uint32_t size = packetQueue.size();
        Ptr<Socket> txSocket;
        Ptr<Packet> packet;

        // check if the connection have been established.
        if (globalTxSocket.find(dstGuid) == globalTxSocket.end())
        {
            // establish connection
            if (nameResolutionCache.find(dstGuid) == nameResolutionCache.end())
            {
                // TODO: global name resolution service.
                // nameResolutionCache[dstGuid] = guidResolution(dstGuid);
            }
            CybertwinInterface dstInterface = nameResolutionCache[dstGuid];

            Ptr<Socket> dstSock = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

            if (Ipv4Address::IsMatchingType(dstInterface.first))
            {
                Ipv4Address ipv4 = Ipv4Address::ConvertFrom(dstInterface.first);
                InetSocketAddress inetAddress = InetSocketAddress(ipv4, dstInterface.second);
                dstSock->Bind();
                dstSock->Connect(inetAddress);
            }
            else if (Ipv6Address::IsMatchingType(dstInterface.first))
            {
                Ipv6Address ipv6 = Ipv6Address::ConvertFrom(dstInterface.first);
                Inet6SocketAddress inet6Address = Inet6SocketAddress(ipv6, dstInterface.second);
                dstSock->Bind();
                dstSock->Connect(inet6Address);
            }

            globalTxSocket[dstGuid] = dstSock;
        }

        txSocket = globalTxSocket[dstGuid];

        for (uint32_t i = 0; i < size && i < TX_MAX_NUM; i++)
        {
            packet = packetQueue.front();
            // TODO : add header
            // CybertwinGlobalHeader header();
            // packet->AddHeader(header);
            txSocket->Send(packet);
        }
    }

    Simulator::Schedule(Seconds(2.), &Cybertwin::ouputPackets, this);
}

void
Cybertwin::Start()
{
    StartApplication();
}

void
Cybertwin::Stop()
{
    StopApplication();
}

} // namespace ns3
