#include "cybertwin-client.h"

#include "ns3/address.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinClient");
NS_OBJECT_ENSURE_REGISTERED(CybertwinClient);
NS_OBJECT_ENSURE_REGISTERED(CybertwinConnClient);
NS_OBJECT_ENSURE_REGISTERED(CybertwinBulkClient);

TypeId
CybertwinClient::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinClient")
            .SetParent<Application>()
            .SetGroupName("cybertwin")
            .AddConstructor<CybertwinClient>()
            .AddAttribute("Socket",
                          "The pointer of the socket connected to the corresponding cybertwin",
                          PointerValue(),
                          MakePointerAccessor(&CybertwinClient::m_socket),
                          MakePointerChecker<Socket>())
            .AddAttribute("LocalCuid",
                          "The CUID of this  device",
                          UintegerValue(),
                          MakeUintegerAccessor(&CybertwinClient::m_localCuid),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("PeerCuid",
                          "The CUID of the peer device",
                          UintegerValue(),
                          MakeUintegerAccessor(&CybertwinClient::m_peerCuid),
                          MakeUintegerChecker<uint64_t>());
    return tid;
}

CybertwinClient::CybertwinClient()
    : m_socket(nullptr)
{
}

void
CybertwinClient::DoDispose()
{
    m_socket = nullptr;
    Application::DoDispose();
}

void
CybertwinClient::RecvPacket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(m_localCuid << socket);
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }
        // TODO
        CybertwinHeader header;
        packet->PeekHeader(header);
        NS_LOG_DEBUG("Client received packet with header " << header << " from " << from);
    }
}

TypeId
CybertwinConnClient::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinConnClient")
            .SetParent<CybertwinClient>()
            .SetGroupName("cybertwin")
            .AddConstructor<CybertwinConnClient>()
            .AddAttribute("LocalAddress",
                          "The address on which to bind the socket",
                          AddressValue(),
                          MakeAddressAccessor(&CybertwinConnClient::m_localAddr),
                          MakeAddressChecker())
            .AddAttribute("LocalPort",
                          "The port on which the application sends data",
                          UintegerValue(80),
                          MakeUintegerAccessor(&CybertwinConnClient::m_localPort),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("EdgeAddress",
                          "The address of the edge server serving this  network",
                          AddressValue(),
                          MakeAddressAccessor(&CybertwinConnClient::m_edgeAddr),
                          MakeAddressChecker())
            .AddAttribute("EdgePort",
                          "The port on which the edge server listens for incoming data",
                          UintegerValue(443),
                          MakeUintegerAccessor(&CybertwinConnClient::m_edgePort),
                          MakeUintegerChecker<uint16_t>());
    return tid;
}

CybertwinConnClient::CybertwinConnClient()
    : m_ctrlSocket(nullptr)
{
}

CybertwinConnClient::~CybertwinConnClient()
{
}

void
CybertwinConnClient::DoDispose()
{
    if (!Simulator::IsFinished())
    {
        StopApplication();
    }
    m_ctrlSocket = nullptr;
    CybertwinClient::DoDispose();
}

void
CybertwinConnClient::StartApplication()
{
    NS_LOG_FUNCTION(m_localCuid << this->GetTypeId());
    if (!m_ctrlSocket)
    {
        m_ctrlSocket =
            Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::TcpSocketFactory"));
        DoSocketMethod(&Socket::Bind, m_ctrlSocket, m_localAddr, m_localPort);

        m_ctrlSocket->SetConnectCallback(
            MakeCallback(&CybertwinConnClient::ControllerConnectSucceededCallback, this),
            MakeCallback(&CybertwinConnClient::ControllerConnectFailedCallback, this));

        DoSocketMethod(&Socket::Connect, m_ctrlSocket, m_edgeAddr, m_edgePort);
        NS_LOG_DEBUG("--[#" << m_localCuid << "-Conn]: connecting to CybertwinController");
    }
}

void
CybertwinConnClient::StopApplication()
{
    NS_LOG_FUNCTION(m_localCuid);

    if (m_socket)
    {
        DisconnectCybertwin();
        m_socket->Close();
    }

    if (m_ctrlSocket)
    {
        m_ctrlSocket->Close();
    }
}

void
CybertwinConnClient::SetCertificate(const CybertwinCertTag& cert)
{
    m_cert = cert;
}

void
CybertwinConnClient::ControllerConnectSucceededCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(m_localCuid << socket);
    m_ctrlSocket->SetRecvCallback(
        MakeCallback(&CybertwinConnClient::RecvFromControllerCallback, this));
    Simulator::ScheduleNow(&CybertwinConnClient::Authenticate, this);
}

void
CybertwinConnClient::ControllerConnectFailedCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(m_localCuid << socket);
    NS_LOG_ERROR("--[#" << m_localCuid
                        << "-Conn]: failed to connect to CybertwinController, errno: "
                        << socket->GetErrno());
}

void
CybertwinConnClient::RecvFromControllerCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(m_localCuid << socket);
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }
        CybertwinHeader header;
        packet->PeekHeader(header);
        NS_ASSERT_MSG(!header.isDataPacket(),
                      "--[#" << m_localCuid << "-Conn]: Received invalid packet type");
        switch (header.GetCommand())
        {
        case CYBERTWIN_CONNECT_SUCCESS: {
            NS_LOG_DEBUG("--[#" << m_localCuid << "-Conn]: Cybertwin generated successfully");
            m_cybertwinPort = header.GetCybertwinPort();
            Simulator::ScheduleNow(&CybertwinConnClient::ConnectCybertwin, this);
            break;
        }
        case CYBERTWIN_CONNECT_ERROR:
            NS_LOG_INFO("--[#" << m_localCuid << "-Conn]: "
                               << "Failed to generate a cybertwin, retrying");
            // TODO: Simulator::Schedule(Seconds(2.), &CybertwinConnClient::Authenticate, this);
            break;
        default:
            NS_LOG_INFO("--[#" << m_localCuid << "-Conn]: "
                               << "Invalid response from controller, retrying");
            // Simulator::Schedule(Seconds(2.), &CybertwinConnClient::Authenticate, this);
            break;
        }
    }
}

void
CybertwinConnClient::Authenticate()
{
    NS_LOG_FUNCTION(m_localCuid);
    Ptr<Packet> authPacket = Create<Packet>(0);
    authPacket->AddPacketTag(m_cert);

    CybertwinHeader ctrlHeader;
    ctrlHeader.SetCommand(HOST_CONNECT);
    ctrlHeader.SetSelfID(m_localCuid);
    authPacket->AddHeader(ctrlHeader);

    m_ctrlSocket->Send(authPacket);
    NS_LOG_DEBUG("--[#" << m_localCuid << "-Conn]: "
                        << "Certificate(s) sent");
}

void
CybertwinConnClient::ConnectCybertwin()
{
    NS_LOG_FUNCTION(m_localCuid);
    if (!m_socket)
    {
        m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::TcpSocketFactory"));
        DoSocketMethod(&Socket::Bind, m_socket, m_localAddr, m_localPort + 1);
    }
    m_socket->SetConnectCallback(
        MakeCallback(&CybertwinConnClient::CybertwinConnectSucceededCallback, this),
        MakeCallback(&CybertwinConnClient::CybertwinCloseFailedCallback, this));
    m_socket->SetCloseCallbacks(
        MakeCallback(&CybertwinConnClient::CybertwinCloseSucceededCallback, this),
        MakeCallback(&CybertwinConnClient::CybertwinCloseFailedCallback, this));
    DoSocketMethod(&Socket::Connect, m_socket, m_edgeAddr, m_cybertwinPort);
    NS_LOG_DEBUG("--[#" << m_localCuid << "-Conn]: request to connect to the cybertwin sent");
}

void
CybertwinConnClient::DisconnectCybertwin()
{
    NS_LOG_FUNCTION(this);
    CybertwinHeader ctrlHeader;
    ctrlHeader.SetCommand(HOST_DISCONNECT);
    ctrlHeader.SetSelfID(m_localCuid);

    Ptr<Packet> connPacket = Create<Packet>(0);
    connPacket->AddHeader(ctrlHeader);

    m_ctrlSocket->Send(connPacket);
    NS_LOG_DEBUG("--[#" << m_localCuid << "-Conn]: "
                        << "Request to disconnect to the cybertwin sent");
}

void
CybertwinConnClient::CybertwinConnectSucceededCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(m_localCuid << socket);
    socket->SetRecvCallback(MakeCallback(&CybertwinConnClient::RecvPacket, this));
    m_socket = socket;
    // Set attributes of other applications
    uint32_t appNums = GetNode()->GetNApplications();
    for (uint32_t i = 0; i < appNums; ++i)
    {
        Ptr<Application> curApp = GetNode()->GetApplication(i);
        if (curApp != this && curApp->GetInstanceTypeId().IsChildOf(CybertwinClient::GetTypeId()))
        {
            NS_LOG_DEBUG("--[#" << m_localCuid << "-Conn]: setting attributes for "
                                << curApp->GetTypeId());
            curApp->SetAttribute("Socket", PointerValue(socket));
            curApp->SetAttribute("LocalCuid", UintegerValue(m_localCuid));
        }
    }
}

void
CybertwinConnClient::CybertwinConnectFailedCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket->GetErrno());
}

void
CybertwinConnClient::CybertwinCloseSucceededCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                              MakeNullCallback<void, Ptr<Socket>>());
    m_socket = nullptr;
}

void
CybertwinConnClient::CybertwinCloseFailedCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket->GetErrno());
}

TypeId
CybertwinBulkClient::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinBulkClient")
                            .SetParent<CybertwinClient>()
                            .SetGroupName("cybertwin")
                            .AddConstructor<CybertwinBulkClient>()
                            .AddAttribute("MaxBytes",
                                          "The total number of bytes to send. 0 means no limit",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&CybertwinBulkClient::m_maxBytes),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("PacketSize",
                                          "The size of a single packet",
                                          UintegerValue(516),
                                          MakeUintegerAccessor(&CybertwinBulkClient::m_packetSize),
                                          MakeUintegerChecker<uint32_t>(21));
    return tid;
}

CybertwinBulkClient::CybertwinBulkClient()
    : m_unsentPacket(nullptr)
{
    NS_LOG_FUNCTION(this);
}

CybertwinBulkClient::~CybertwinBulkClient()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinBulkClient::StartApplication()
{
    NS_LOG_FUNCTION(this->GetTypeId());
    WaitForConnection();
}

void
CybertwinBulkClient::StopApplication()
{
    NS_LOG_FUNCTION(GetTypeId() << Simulator::Now());
}

void
CybertwinBulkClient::DoDispose()
{
    NS_LOG_FUNCTION(this->GetTypeId() << Simulator::Now());
    if (m_socket)
    {
        m_socket->Close();
        m_socket = nullptr;
    }
    CybertwinClient::DoDispose();
}

void
CybertwinBulkClient::WaitForConnection()
{
    if (m_socket)
    {
        SendData();
    }
    else
    {
        Simulator::Schedule(MilliSeconds(100), &CybertwinBulkClient::WaitForConnection, this);
    }
}

void
CybertwinBulkClient::SendData()
{
    // NS_LOG_DEBUG("--[#" << m_localCuid << "-Bulk]: started sending data at " <<
    // Simulator::Now());

    while (m_socket && (m_maxBytes == 0 || m_sentBytes < m_maxBytes))
    {
        // uint64_t to allow the comparison later
        uint64_t toSend = m_packetSize;
        if (m_maxBytes > 0)
        {
            toSend = std::min(toSend, m_maxBytes - m_sentBytes);
        }

        Ptr<Packet> packet;
        if (m_unsentPacket)
        {
            // NS_LOG_DEBUG("Resending an unsent packet");
            packet = m_unsentPacket;
            toSend = packet->GetSize();
        }
        else
        {
            CybertwinHeader header;
            header.SetSelfID(m_localCuid);
            header.SetPeerID(m_peerCuid);
            header.SetSize(toSend);
            header.SetCommand(CYBERTWIN_HEADER_DATA);
            NS_ABORT_IF(toSend < header.GetSerializedSize());
            packet = Create<Packet>(toSend - header.GetSerializedSize());
            packet->AddHeader(header);
            CybertwinTag idTag(m_localCuid);
            packet->AddPacketTag(idTag);
        }

        int actual = m_socket->Send(packet);
        if ((unsigned)actual == toSend)
        {
            m_sentBytes += actual;
            m_unsentPacket = nullptr;
        }
        else if (actual == -1)
        {
            // NS_LOG_DEBUG("Unable to send packet; caching for later attempt");
            m_unsentPacket = packet;
            break;
        }
        else if (actual > 0 && (unsigned)actual < toSend)
        {
            // A Linux socket (non-blocking, such as in DCE) may return
            // a quantity less than the packet size.  Split the packet
            // into two, trace the sent packet, save the unsent packet
            NS_LOG_DEBUG("Packet size: " << packet->GetSize() << "; sent: " << actual
                                         << "; fragment saved: " << toSend - (unsigned)actual);
            Ptr<Packet> unsent = packet->CreateFragment(actual, (toSend - (unsigned)actual));
            m_sentBytes += actual;
            m_unsentPacket = unsent;
            break;
        }
        else
        {
            NS_FATAL_ERROR("Unexpected return value from m_socket->Send ()");
        }
    }
    if (m_socket)
    {
        Simulator::Schedule(MilliSeconds(10), &CybertwinBulkClient::SendData, this);
    }
}

} // namespace ns3