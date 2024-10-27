#include "ns3/cybertwin-app-download-server.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CybertwinAppDownloadServer");
NS_OBJECT_ENSURE_REGISTERED(CybertwinAppDownloadServer);

TypeId
CybertwinAppDownloadServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinAppDownloadServer")
                            .SetParent<Application>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinAppDownloadServer>()
                            .AddAttribute("CybertwinID",
                                          "Cybertwin ID of the server.",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&CybertwinAppDownloadServer::m_cybertwinID),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("ListenPort",
                                          "Port to listen on.",
                                          UintegerValue(80),
                                          MakeUintegerAccessor(&CybertwinAppDownloadServer::m_serverPort),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("MaxBytes",
                                          "Maximum bytes to send.",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&CybertwinAppDownloadServer::m_maxBytes),
                                          MakeUintegerChecker<uint64_t>());
    return tid;
}

CybertwinAppDownloadServer::CybertwinAppDownloadServer()
{
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] create CybertwinAppDownloadServer.");
}

CybertwinAppDownloadServer::CybertwinAppDownloadServer(CYBERTWINID_t cybertwinID, CYBERTWIN_INTERFACE_LIST_t interfaces)
{
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] create CybertwinAppDownloadServer.");
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Cybertwin ID: " << cybertwinID);
    for (auto it = interfaces.begin(); it != interfaces.end(); ++it)
    {
        NS_LOG_DEBUG("[CybertwinAppDownloadServer] Interface: " << it->first << " " << it->second);
    }
}

CybertwinAppDownloadServer::~CybertwinAppDownloadServer()
{
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] destroy CybertwinAppDownloadServer.");
}

void
CybertwinAppDownloadServer::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Starting CybertwinAppDownloadServer.");

    // Init interface list: self ipv4 address and listening port
    for (auto globalIp : GetNode()->GetObject<CybertwinNode>()->GetGlobalIpList())
    {
        m_interfaces.push_back(std::make_pair(globalIp, m_serverPort));
    }

    // Get CNRS app and insert the cybertwin ID
    Ptr<CybertwinNode> node = GetNode()->GetObject<CybertwinNode>();
    Ptr<NameResolutionService> cnrs = node->GetCNRSApp();
    NS_ASSERT(cnrs);
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Inserting Cybertwin " << m_cybertwinID << " into CNRS.");
    cnrs->InsertCybertwinInterfaceName(m_cybertwinID, m_interfaces);

    // Init data server and listen for incoming connections
    m_serverSocket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::TcpSocketFactory"));
    m_serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_serverPort));
    m_serverSocket->SetAcceptCallback(MakeCallback(&CybertwinAppDownloadServer::ConnRequestCallback, this),
                                      MakeCallback(&CybertwinAppDownloadServer::ConnCreatedCallback, this));
    m_serverSocket->SetCloseCallbacks(MakeCallback(&CybertwinAppDownloadServer::NormalCloseCallback, this),
                                        MakeCallback(&CybertwinAppDownloadServer::ErrorCloseCallback, this));
    m_serverSocket->Listen();

    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Server is listening on " << m_serverPort);
}

void
CybertwinAppDownloadServer::StopApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Stopping CybertwinAppDownloadServer.");

    if (m_serverSocket)
    {
        m_serverSocket->Close();
    }
}

bool
CybertwinAppDownloadServer::ConnRequestCallback(Ptr<Socket> socket, const Address& from)
{
    NS_LOG_FUNCTION(this << socket << from);
    // accept all connections
    return true;
}

void
CybertwinAppDownloadServer::ConnCreatedCallback(Ptr<Socket> socket, const Address& from)
{
    NS_LOG_FUNCTION(this << socket << from);
    // put into the map
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] New connection with "
                 << InetSocketAddress::ConvertFrom(from).GetIpv4() << ":"
                 << InetSocketAddress::ConvertFrom(from).GetPort());
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Send " << m_maxBytes << " bytes to "
                                               << InetSocketAddress::ConvertFrom(from).GetIpv4() << ":"
                                               << InetSocketAddress::ConvertFrom(from).GetPort());
    socket->SetRecvCallback(MakeCallback(&CybertwinAppDownloadServer::RecvCallback, this));
    
    // send data
    m_sendBytes[socket] = 0;
    Simulator::ScheduleNow(&CybertwinAppDownloadServer::SendData, this, socket);
}

void
CybertwinAppDownloadServer::RecvCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("[CybertwinAppDownloadServer] RecvCallback");
}

void
CybertwinAppDownloadServer::NormalCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Address peername;
    socket->GetPeerName(peername);
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Normal Connection with "
                 << InetSocketAddress::ConvertFrom(peername).GetIpv4() << ":"
                 << InetSocketAddress::ConvertFrom(peername).GetPort() << " closed.");
}

void
CybertwinAppDownloadServer::ErrorCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_DEBUG("[CybertwinAppDownloadServer] Error Connection closed.");
}

void
CybertwinAppDownloadServer::SendData(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("[CybertwinAppDownloadServer] SendData");
    if (!socket)
    {
        NS_LOG_ERROR("[CybertwinAppDownloadServer] Connection closed.");
        return;
    }

    if (m_sendBytes[socket] >= m_maxBytes)
    {
        NS_LOG_DEBUG("[CybertwinAppDownloadServer] Send "
                     << m_sendBytes[socket] << " bytes");
        socket->Close();
        return;
    }

    // send data
    Ptr<Packet> packet = Create<Packet>(SYSTEM_PACKET_SIZE);
    int32_t sendSize = socket->Send(packet);
    if (sendSize <= 0)
    {
        NS_LOG_ERROR("[CybertwinAppDownloadServer] Send failed.");
        return;
    }

    m_sendBytes[socket] += sendSize;
    Simulator::Schedule(MilliSeconds(1), &CybertwinAppDownloadServer::SendData, this, socket);
}

} // namespace ns3
