#include "ns3/cybertwin-app-download-client.h"
#include "ns3/cybertwin-header.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CybertwinAppDownloadClient");

NS_OBJECT_ENSURE_REGISTERED(CybertwinAppDownloadClient);

TypeId
CybertwinAppDownloadClient::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinAppDownloadClient")
                            .SetParent<Application>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinAppDownloadClient>()
                            .AddAttribute("TargetCybertwinId",
                                          "The target cybertwin id.",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&CybertwinAppDownloadClient::m_targetCybertwinId),
                                          MakeUintegerChecker<uint64_t>());
    return tid;
}

CybertwinAppDownloadClient::CybertwinAppDownloadClient()
{
    NS_LOG_FUNCTION(this);
}

CybertwinAppDownloadClient::~CybertwinAppDownloadClient()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinAppDownloadClient::StartApplication()
{
    NS_LOG_FUNCTION(this);
    // Get the end host
    Ptr<Node> node = GetNode();
    Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(node);
    NS_ASSERT(endHost);
    m_nodeName = endHost->GetName();
    m_selfCybertwinId = endHost->GetCybertwinId();

    NS_LOG_INFO("[" << m_nodeName << "][download-client] Starting CybertwinAppDownloadClient.");

    // Get Cybertwin EndHost Daemon
    Ptr<CybertwinEndHostDaemon> endHostDaemon = endHost->GetEndHostDaemon();
    NS_ASSERT(endHostDaemon);

    // wait until the endhost daemon is ready
    uint32_t wait_times = 0;
    while (!endHostDaemon->IsRegisteredToCybertwin() && wait_times < 10)
    {
        NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Waiting for the endhost daemon to be ready.");
        Simulator::Schedule(Seconds(1), &CybertwinAppDownloadClient::StartApplication, this);
        wait_times++;
    }

    NS_LOG_INFO("[" << m_nodeName << "][download-client] Endhost daemon is ready.");

    // Get Cybertwin address
    m_cybertwinAddr = endHostDaemon->GetManagerAddr();
    m_cybertwinPort = endHostDaemon->GetCybertwinPort();

    // Create socket and connect to cybertwin
    m_socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
    m_socket->Bind();

    m_socket->SetConnectCallback(
        MakeCallback(&CybertwinAppDownloadClient::ConnectSucceededCallback, this),
        MakeCallback(&CybertwinAppDownloadClient::ConnectFailedCallback, this));
    
    m_socket->SetCloseCallbacks(
        MakeCallback(&CybertwinAppDownloadClient::NormalCloseCallback, this),
        MakeCallback(&CybertwinAppDownloadClient::ErrorCloseCallback, this));
    
    InetSocketAddress managerAddr = InetSocketAddress(m_cybertwinAddr, m_cybertwinPort);
 
    NS_LOG_INFO("[" << m_nodeName << "][download-client] connecting to " << m_cybertwinAddr << ":" << m_cybertwinPort);
    m_socket->Connect(managerAddr);
}

void
CybertwinAppDownloadClient::StopApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Stopping CybertwinAppDownloadClient.");
    if (m_socket)
    {
        m_socket->Close();
    }
}

void
CybertwinAppDownloadClient::ConnectSucceededCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Connect to Cybertwin Manager succeeded.");
    socket->SetRecvCallback(MakeCallback(&CybertwinAppDownloadClient::RecvCallback, this));

    // Send Download Request
    SendDownloadRequest(socket);
}

void
CybertwinAppDownloadClient::ConnectFailedCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_ERROR("[" << m_nodeName << "][download-client] Connect to Cybertwin Manager failed.");
}

void
CybertwinAppDownloadClient::NormalCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Normal close.");
}

void
CybertwinAppDownloadClient::ErrorCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Error close.");
}

void
CybertwinAppDownloadClient::RecvCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Receiving from CybertwinManager.");

    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Received packet from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " : " << InetSocketAddress::ConvertFrom(from).GetPort());
    }
}

void
CybertwinAppDownloadClient::SendDownloadRequest(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[" << m_nodeName << "][download-client] Sending download request.");

    // Create a packet
    Ptr<Packet> packet = Create<Packet>();
    EndHostHeader endHostHeader;
    endHostHeader.SetCommand(DOWNLOAD_REQUEST);
    endHostHeader.SetTargetID(m_targetCybertwinId);
    packet->AddHeader(endHostHeader);

    packet->AddPaddingAtEnd(SYSTEM_PACKET_SIZE - endHostHeader.GetSerializedSize());

    // Send the packet
    socket->Send(packet);
}

} // namespace ns3