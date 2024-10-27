#include "cybertwin-endhost-daemon.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CybertwinEndHostDaemon");

CybertwinEndHostDaemon::CybertwinEndHostDaemon()
{
    m_isConnectedToCybertwinManager = false;
    m_isRegisteredToCybertwin = false;
    m_isConnectedToCybertwin = false;

    m_proxySocket = nullptr;
    m_cybertwinSocket = nullptr;
    NS_LOG_DEBUG("[CybertwinEndHostDaemon] create CybertwinEndHostDaemon.");
}

CybertwinEndHostDaemon::~CybertwinEndHostDaemon()
{
    NS_LOG_DEBUG("[CybertwinEndHostDaemon] destroy CybertwinEndHostDaemon.");
}

TypeId
CybertwinEndHostDaemon::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinEndHostDaemon")
            .SetParent<Application>()
            .SetGroupName("Cybertwin")
            .AddConstructor<CybertwinEndHostDaemon>()
            .AddAttribute("ManagerAddr",
                          "Cybertwin Manager address.",
                          Ipv4AddressValue(),
                          MakeIpv4AddressAccessor(&CybertwinEndHostDaemon::m_managerAddr),
                          MakeIpv4AddressChecker())
            .AddAttribute("ManagerPort",
                          "Manager port.",
                          UintegerValue(CYBERTWIN_MANAGER_PROXY_PORT),
                          MakeUintegerAccessor(&CybertwinEndHostDaemon::m_managerPort),
                          MakeUintegerChecker<uint16_t>());
    return tid;
}

void
CybertwinEndHostDaemon::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // Get Node Information
    Ptr<Node> node = GetNode();
    Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(node);
    m_nodeName = endHost->GetName();

    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Starting CybertwinEndHostDaemon.");

    // Register to Cybertwin
    RegisterCybertwin();
}

void
CybertwinEndHostDaemon::StopApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Stopping CybertwinEndHostDaemon.");

    if (m_proxySocket)
    {
        m_proxySocket->Close();
    }
    if (m_cybertwinSocket)
    {
        m_cybertwinSocket->Close();
    }

    return;
}

void
CybertwinEndHostDaemon::RegisterCybertwin()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Registering to Cybertwin.");
    // Get Node Information
    ConnectCybertwinManager();
}

void
CybertwinEndHostDaemon::ConnectCybertwinManager()
{
    NS_LOG_FUNCTION(this);
    if (!m_proxySocket)
    {
        m_proxySocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
    }

    if (m_proxySocket->Bind() < 0)
    {
        NS_LOG_ERROR("Failed to bind socket.");
        return;
    }

    m_proxySocket->SetConnectCallback(
        MakeCallback(&CybertwinEndHostDaemon::ConnectCybertwinManangerSucceededCallback, this),
        MakeCallback(&CybertwinEndHostDaemon::ConnectCybertwinManangerFailedCallback, this));

    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Connecting to CybertwinManager at "
                    << m_managerAddr << ":" << m_managerPort);
    InetSocketAddress proxyAddr = InetSocketAddress(m_managerAddr, m_managerPort);
    m_proxySocket->Connect(proxyAddr);
}

void
CybertwinEndHostDaemon::ConnectCybertwinManangerSucceededCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Connect to CybertwinManager succeeded.");

    m_isConnectedToCybertwinManager = true;
    socket->SetRecvCallback(
        MakeCallback(&CybertwinEndHostDaemon::RecvFromCybertwinManangerCallback, this));
    Simulator::ScheduleNow(&CybertwinEndHostDaemon::Authenticate, this);
}

void
CybertwinEndHostDaemon::ConnectCybertwinManangerFailedCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Connect to CybertwinManager failed.");
}

void
CybertwinEndHostDaemon::Authenticate()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Authenticating to CybertwinManager.");

    if (m_isRegisteredToCybertwin)
    {
        NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Already registered.");
        return;
    }
    else
    {
        // get node info
        Ptr<Node> node = GetNode();
        Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(node);
        std::string nodeName = endHost->GetName();

        // send to cybertwin manager
        Ptr<Packet> packet = Create<Packet>();
        CybertwinManagerHeader header;
        header.SetCommand(CYBERTWIN_REGISTRATION);
        header.SetCName(nodeName);
        packet->AddHeader(header);

        m_proxySocket->Send(packet);

        // Send again after 1 second
        Simulator::Schedule(Seconds(1), &CybertwinEndHostDaemon::Authenticate, this);
    }
}

void
CybertwinEndHostDaemon::RecvFromCybertwinManangerCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Receiving from CybertwinManager.");

    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        CybertwinManagerHeader header;
        packet->PeekHeader(header);

        switch (header.GetCommand())
        {
        case CYBERTWIN_REGISTRATION_ACK: {
            m_isRegisteredToCybertwin = true;
            RegisterSuccessHandler(socket, packet);
            break;
        }
        case CYBERTWIN_REGISTRATION_ERROR: {
            RegisterFailureHandler(socket, packet);
            break;
        }
        default: {
            NS_LOG_INFO("[" << m_nodeName << "][EndHostDaemon] Received packet from "
                            << InetSocketAddress::ConvertFrom(from).GetIpv4() << " : "
                            << InetSocketAddress::ConvertFrom(from).GetPort());
            break;
        }
        }
    }
}

void
CybertwinEndHostDaemon::RegisterSuccessHandler(Ptr<Socket> socket, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    CybertwinManagerHeader header;
    packet->RemoveHeader(header);

    // get node info
    Ptr<Node> node = GetNode();
    Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(node);
    std::string nodeName = endHost->GetName();
    if (nodeName != header.GetCName())
    {
        NS_LOG_ERROR("Node name mismatch.");
        return;
    }

    // set cybertwin info
    endHost->SetCybertwinId(header.GetCUID());
    endHost->SetCybertwinPort(header.GetPort());
    endHost->SetCybertwinStatus(true);
    m_cybertwinId = header.GetCUID();
    m_cybertwinPort = header.GetPort();
    m_isRegisteredToCybertwin = true;

    NS_LOG_DEBUG("[" << m_nodeName << "][EndHostDaemon] Registered to Cybertwin : " << header.GetCUID() << " at " << header.GetPort());
}

void
CybertwinEndHostDaemon::RegisterFailureHandler(Ptr<Socket> socket, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Handling registration failure.");
}

Ipv4Address
CybertwinEndHostDaemon::GetManagerAddr()
{
    return m_managerAddr;
}

uint16_t
CybertwinEndHostDaemon::GetManagerPort()
{
    return m_managerPort;
}

uint16_t
CybertwinEndHostDaemon::GetCybertwinPort()
{
    return m_cybertwinPort;
}

bool
CybertwinEndHostDaemon::IsRegisteredToCybertwin()
{
    return m_isRegisteredToCybertwin;
}

} // namespace ns3