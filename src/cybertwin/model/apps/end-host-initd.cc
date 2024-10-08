#include "end-host-initd.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("EndHostInitd");

EndHostInitd::EndHostInitd()
{
    m_isConnectedToCybertwinManager = false;
    m_isRegisteredToCybertwin = false;
    m_isConnectedToCybertwin = false;

    m_proxySocket = nullptr;
    m_cybertwinSocket = nullptr;
    NS_LOG_DEBUG("[EndHostInitd] create EndHostInitd.");
}

EndHostInitd::~EndHostInitd()
{
    NS_LOG_DEBUG("[EndHostInitd] destroy EndHostInitd.");
}

TypeId
EndHostInitd::GetTypeId()
{
    static TypeId tid = TypeId("ns3::EndHostInitd")
                            .SetParent<Application>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<EndHostInitd>()
                            .AddAttribute("ProxyAddr",
                                          "Cybertwin Manager address.",
                                          Ipv4AddressValue(),
                                          MakeIpv4AddressAccessor(&EndHostInitd::m_proxyAddr),
                                          MakeIpv4AddressChecker())
                            .AddAttribute("ProxyPort",
                                          "Proxy port.",
                                          UintegerValue(CYBERTWIN_MANAGER_PROXY_PORT),
                                          MakeUintegerAccessor(&EndHostInitd::m_proxyPort),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

void
EndHostInitd::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Starting EndHostInitd.");

    // Register to Cybertwin
    RegisterCybertwin();
}

void
EndHostInitd::StopApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Stopping EndHostInitd.");

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
EndHostInitd::RegisterCybertwin()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Registering Cybertwin.");

    // Get Node Information
    ConnectCybertwinManager();
}

void
EndHostInitd::ConnectCybertwinManager()
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
        MakeCallback(&EndHostInitd::ConnectCybertwinManangerSucceededCallback, this),
        MakeCallback(&EndHostInitd::ConnectCybertwinManangerFailedCallback, this));
    
    NS_LOG_DEBUG("Connecting to Cybertwin Manager: "<<m_proxyAddr <<":"<<m_proxyPort);
    InetSocketAddress proxyAddr = InetSocketAddress(m_proxyAddr, m_proxyPort);
    m_proxySocket->Connect(proxyAddr);
}

void
EndHostInitd::ConnectCybertwinManangerSucceededCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Connect to CybertwinManager succeeded.");

    m_isConnectedToCybertwinManager = true;
    socket->SetRecvCallback(MakeCallback(&EndHostInitd::RecvFromCybertwinManangerCallback, this));
    Simulator::ScheduleNow(&EndHostInitd::Authenticate, this);
}

void
EndHostInitd::ConnectCybertwinManangerFailedCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_ERROR("Connect to CybertwinManager failed.");
}

void
EndHostInitd::Authenticate()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Authenticating.");

    if (m_isRegisteredToCybertwin)
    {
        NS_LOG_DEBUG("Already registered.");
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
        Simulator::Schedule(Seconds(1), &EndHostInitd::Authenticate, this);
    }
}

void
EndHostInitd::RecvFromCybertwinManangerCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[Initd][EndHost] Receiving from CybertwinManager.");

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
            NS_LOG_ERROR("[Initd][EndHost] Unknown command: " << header.GetCommand());
            break;
        }
        }
    }
}

void
EndHostInitd::RegisterSuccessHandler(Ptr<Socket> socket, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[Initd][EndHost] Cybertwin Registration Success.");

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
    header.Print(std::cout);
    endHost->SetCybertwinId(header.GetCUID());
    endHost->SetCybertwinPort(header.GetPort());
    endHost->SetCybertwinStatus(true);
}

void
EndHostInitd::RegisterFailureHandler(Ptr<Socket> socket, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Handling registration failure.");
}

} // namespace ns3