#include "ns3/cybertwin.h"

#include "ns3/cybertwin-header.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("Cybertwin");
NS_OBJECT_ENSURE_REGISTERED(Cybertwin);

TypeId
Cybertwin::GetTypeId()
{
    static TypeId tid = TypeId("ns3::Cybertwin")
                            .SetParent<Application>()
                            .SetGroupName("cybertwin")
                            .AddConstructor<Cybertwin>();
    return tid;
}

Cybertwin::Cybertwin()
    : m_cybertwinId(0),
      m_localSocket(nullptr)
{
}

Cybertwin::Cybertwin(CYBERTWINID_t cuid,
                     CYBERTWIN_INTERFACE_t l_interface,
                     CYBERTWIN_INTERFACE_LIST_t g_interfaces)
    : m_cybertwinId(cuid),
      m_localSocket(nullptr),
      m_localInterface(l_interface),
      m_globalInterfaces(g_interfaces),
      m_isStartTrafficOpt(false),
      m_comm_test_total_bytes(0),
      m_comm_test_interval_bytes(0)
{
    NS_LOG_FUNCTION(cuid);
}

Cybertwin::~Cybertwin()
{
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Destroy Cybertwin : " << m_cybertwinId);
}

void
Cybertwin::StartApplication()
{
    // get node name
    m_nodeName = GetNode()->GetObject<CybertwinNode>()->GetName();

    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Start Cybertwin : " << m_cybertwinId);
    // start listen at local port
    Simulator::ScheduleNow(&Cybertwin::LocallyListen, this);

    // start forwarding local packets
    // Simulator::ScheduleNow(&Cybertwin::LocallyForward, this);

    // start listen at global ports
    Simulator::ScheduleNow(&Cybertwin::GloballyListen, this);

    // report interfaces to CNRS
    m_cnrs = DynamicCast<CybertwinNode>(GetNode())->GetCNRSApp();
    NS_ASSERT(m_cnrs != nullptr);
    m_cnrs->InsertCybertwinInterfaceName(m_cybertwinId, m_globalInterfaces);

    // open log
    OpenLogFile(DynamicCast<CybertwinNode>(GetNode())->GetLogDir(), "cybertwin.log");
}

void
Cybertwin::StopApplication()
{
    NS_LOG_FUNCTION(m_cybertwinId);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Stop Cybertwin : " << m_cybertwinId);
    for (auto it = m_streamBuffer.begin(); it != m_streamBuffer.end(); ++it)
    {
        it->first->Close();
    }
    m_streamBuffer.clear();
    if (m_localSocket)
    {
        m_localSocket->Close();
        m_localSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
Cybertwin::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_localSocket = nullptr;
    Application::DoDispose();
}

//***************************************************************************************
//*                     Handle incoming connections from host                           *
//***************************************************************************************

void
Cybertwin::LocallyListen()
{
    NS_LOG_DEBUG("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                     << "]: Cybertwin starts listening at local port " << m_localInterface.second);
    if (!m_localSocket)
    {
        m_localSocket =
            Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::TcpSocketFactory"));
        NS_ASSERT(m_localSocket != nullptr);
    }

    InetSocketAddress localAddr =
        InetSocketAddress(m_localInterface.first, m_localInterface.second);
    if (m_localSocket->Bind(localAddr) < 0)
    {
        NS_LOG_ERROR("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                         << "]: Failed to bind local socket");
        return;
    }

    m_localSocket->SetAcceptCallback(MakeCallback(&Cybertwin::LocalConnRequestCallback, this),
                                     MakeCallback(&Cybertwin::LocalConnCreatedCallback, this));
    m_localSocket->SetCloseCallbacks(MakeCallback(&Cybertwin::LocalNormalCloseCallback, this),
                                     MakeCallback(&Cybertwin::LocalErrorCloseCallback, this));
    m_localSocket->Listen();
}

bool
Cybertwin::LocalConnRequestCallback(Ptr<Socket> socket, const Address&)
{
    NS_LOG_FUNCTION(this);
    return true;
}

void
Cybertwin::LocalConnCreatedCallback(Ptr<Socket> socket, const Address& address)
{
    NS_LOG_FUNCTION(this);
    socket->SetRecvCallback(MakeCallback(&Cybertwin::LocalRecvCallback, this));
}

void
Cybertwin::LocalNormalCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    socket->ShutdownSend();
}

void
Cybertwin::LocalErrorCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket->GetErrno());
}

void
Cybertwin::StartCybertwinDownloadProcess(Ptr<Socket> socket, CYBERTWINID_t targetID)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Start download process for target " << targetID);
    // resolve target address
    m_cnrs->GetCybertwinInterfaceByName(targetID, MakeCallback(&Cybertwin::StartDownloadProcess, this, socket));
}

void
Cybertwin::StartDownloadProcess(Ptr<Socket> endHostSock, CYBERTWINID_t targetID, CYBERTWIN_INTERFACE_LIST_t targetInterfaces)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Get Cybertwin resolved address for target " << targetID);
    
    // create a new connection
    Ptr<Socket> socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::TcpSocketFactory"));
    NS_ASSERT(socket != nullptr);
    m_socketList.push_back(socket);

    InetSocketAddress targetAddr = InetSocketAddress(targetInterfaces[0].first, targetInterfaces[0].second);
    socket->Connect(targetAddr);

    // Set callbacks
    socket->SetAcceptCallback(MakeCallback(&Cybertwin::DownloadSocketAcceptCallback, this),
                                MakeCallback(&Cybertwin::DownloadSocketCreatedCallback, this));
    socket->SetCloseCallbacks(MakeCallback(&Cybertwin::DownloadSocketNormalCloseCallback, this),
                              MakeCallback(&Cybertwin::DownloadSocketErrorCloseCallback, this));
    socket->SetRecvCallback(MakeCallback(&Cybertwin::DownloadSocketRecvCallback, this));

    // Insert to maps
    m_cloud2endSockMap[socket] = endHostSock;
    m_end2cloudSockMap[endHostSock] = socket;
}

void
Cybertwin::DownloadSocketAcceptCallback(Ptr<Socket> socket, const Address& address)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Accept download connection from " << address);
}

void
Cybertwin::DownloadSocketCreatedCallback(Ptr<Socket> socket, const Address& address)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Download connection created with " << address);
}

void
Cybertwin::DownloadSocketNormalCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Download connection closed normally");
}

void
Cybertwin::DownloadSocketErrorCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Download connection closed with error");
}

void
Cybertwin::DownloadSocketRecvCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet;
    Address from;
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                    << "]: Receive packet from target host");
    while ((packet = socket->RecvFrom(from)))
    {
        // Get packet and redirect to end host
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                        << "]: Redirect packet to end host");
        Ptr<Socket> endHostSock = m_cloud2endSockMap[socket];
        endHostSock->Send(packet);
    }
}

void
Cybertwin::LocalRecvCallback(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName << "][Cybertwin" << m_cybertwinId
                    << "]: Receive packet from local host");

    while ((packet = socket->RecvFrom(from)))
    {
        EndHostHeader header;
        packet->RemoveHeader(header);

        EndHostCommand_t cmd = (EndHostCommand_t)header.GetCommand();
        if (cmd == DOWNLOAD_REQUEST)
        {
            NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName << "][" << m_cybertwinId
                            << "]: Receive download request from local host");
            // send download response
            CYBERTWINID_t targetID = header.GetTargetID();
            Simulator::ScheduleNow(&Cybertwin::StartCybertwinDownloadProcess, this, socket, targetID);            
        }
        else
        {
            NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "(s)][" << m_nodeName
                            << "]: Receive unknown command from local host");
        }
#if 0
        if (packet->GetSize() == 0)
        {
            NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: receive empty packet, close socket");
            break;
        }

        Address localAddr, remoteAddr;
        socket->GetSockName(localAddr);
        socket->GetPeerName(remoteAddr);

        // get stream id
        CybertwinHeader header;
        packet->RemoveHeader(header);
        CYBERTWINID_t sender = header.GetSelfID();
        CYBERTWINID_t receiver = header.GetPeerID();
        CybertwinCommand_t cmd = (CybertwinCommand_t)header.GetCommand();

        if (cmd == CREATE_STREAM)
        {
            NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: create a new duplex stream from " << sender
                                     << " to " << receiver);
            CYBERTWINID_t selfID = header.GetSelfID();
            CYBERTWINID_t targetId = header.GetPeerID();
            uint8_t rate = header.GetRecvRate();

            // create a new stream
            NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: create a new duplex stream from " << selfID
                                     << " to " << targetId);
            Ptr<CybertwinFullDuplexStream> stream = CreateObject<CybertwinFullDuplexStream>(m_node, m_cnrs, selfID, targetId);
            m_streams.push_back(stream);
            stream->SetEndSocket(socket);
            stream->SetAttribute("CloudRateLimit", DoubleValue(static_cast<double>(rate)));
            //stream->SetAttributes("EndRateLimit", DoubleValue(m_endRateLimit));
            stream->Activate();
    
            //STREAMID_t streamId = sender;
            continue;
        }

        if (receiver == COMM_TEST_CYBERTWIN_ID && sender == COMM_TEST_CYBERTWIN_ID)
        {
            // only used for comm test
            int32_t recvSize = packet->GetSize();
            Time m_currTime = Simulator::Now();
            m_comm_test_total_bytes +=  recvSize;
            m_comm_test_interval_bytes += recvSize;

            // put into queue
            m_tsPktQueue.push(packet);
            m_tpPktQueue.push(packet);

            if (m_isStartTrafficOpt == false)
            {
                // first packet, start statistics
                NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: start statistics");
                m_logStream << "--[Edge-#" << m_cybertwinId << "]: start statistics" << std::endl;
                m_isStartTrafficOpt = true;
                m_startShapingTime = m_currTime;
                m_lastTime = m_currTime;
                m_lastShapingTime = m_currTime;

                m_statisticalEnd = false;

                Simulator::ScheduleNow(&Cybertwin::CybertwinCommModelStatistical, this);

                // start traffic shaping
                Simulator::ScheduleNow(&Cybertwin::CybertwinCommModelTrafficShaping, this);
                // start traffic policing
                Simulator::ScheduleNow(&Cybertwin::CybertwinCommModelTrafficPolicing, this);
            }
            
            continue;
        }

        STREAMID_t streamId = sender;
        streamId = streamId << 64 | receiver;

        NS_LOG_INFO("--[Edge-#" << m_cybertwinId << "]: received packet from " << sender << " to "
                                 << receiver << " with size " << packet->GetSize() << " bytes");

        if (m_txStreamBuffer.find(streamId) == m_txStreamBuffer.end())
        {
            NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId
                                     << "]: stream buffer not found, create a new one");
            m_txStreamBuffer[streamId] = std::queue<Ptr<Packet>>();
            m_txStreamBufferOrder.push_back(streamId);
        }

        // put into buffer
        std::queue<Ptr<Packet>>& buffer = m_txStreamBuffer[streamId];
        buffer.push(packet);
        if (buffer.size() > MAX_BUFFER_PKT_NUM)
        {
            NS_LOG_WARN("Stream buffer is full, drop the oldest packet");
            buffer.pop();
        }
        Simulator::ScheduleNow(&Cybertwin::SendPendingPackets, this, streamId);
#endif
    }
}

void
Cybertwin::LocallyForward()
{
    NS_LOG_FUNCTION(this);
    m_lastTxStreamBufferOrder = 0;

    if (m_txStreamBufferOrder.size() == 0)
    {
        // no stream buffer, wait for 10ms
        if (m_noneDataCnt++ < CYBERTWIN_FORWARD_MAX_WAIT_NUM)
        {
            Simulator::Schedule(MilliSeconds(10), &Cybertwin::LocallyForward, this);
        }
        return;
    }

    // pick a stream
    int32_t streamIdx = -1;
    int32_t idx = m_lastTxStreamBufferOrder;
    for (uint32_t i = 0; i < m_txStreamBufferOrder.size(); i++)
    {
        if (m_txStreamBuffer[idx].size() > 0)
        {
            streamIdx = idx;
            break;
        }
        idx = (idx + 1) % m_txStreamBufferOrder.size();
    }

    if (streamIdx == -1)
    {
        // no packets in all stream buffers, wait for 1ms
        Simulator::Schedule(MilliSeconds(1), &Cybertwin::LocallyForward, this);
        return;
    }

    // schedule to send packets
    NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: schedule to send packets in stream "
                             << streamIdx << " at " << Simulator::Now());
    STREAMID_t streamId = m_txStreamBufferOrder[streamIdx];
    Simulator::ScheduleNow(&Cybertwin::SendPendingPackets, this, streamId);
    // Simulator::Schedule(MilliSeconds(10), &Cybertwin::LocallyForward, this);
}

void
Cybertwin::SendPendingPackets(STREAMID_t streamId)
{
    // get connection by peer cuid
#if MDTP_ENABLED
    MultipathConnection* conn = nullptr;
#else
    Ptr<Socket> conn = nullptr;
#endif

    if (m_txConnections.find(streamId) != m_txConnections.end())
    {
        NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId
                                 << "]: connection is established, output packets");
        // case[1]: connection is established, output packets
        conn = m_txConnections[streamId];

        // TODO: limit the number of packets to send
        while (!m_txStreamBuffer[streamId].empty())
        {
            NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: send packet at " << Simulator::Now());
            Ptr<Packet> packet = m_txStreamBuffer[streamId].front();
            m_txStreamBuffer[streamId].pop();
            conn->Send(packet);
        }
    }
    else if (m_pendingConnections.find(streamId) != m_pendingConnections.end())
    {
        // case[2]: connection is establishing, wait for establishment
        NS_LOG_DEBUG("--[Edge-#"
                     << m_cybertwinId
                     << "]: connection is establishing, wait for the connection to be created at "
                     << Simulator::Now());
    }
    else
    {
        // case[3]: connection not created yet, request to create.
        NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId
                                 << "]: connection not created yet, initiate a new connection at "
                                 << Simulator::Now());
        // connection not created yet, initiate a new connection
#if MDTP_ENABLED
        conn = new MultipathConnection();
        conn->Setup(GetNode(), m_cybertwinId, m_globalInterfaces);
        conn->Connect(peerCuid);
        // send pending packets by callback after connection is created
        conn->SetConnectCallback(MakeCallback(&Cybertwin::NewMpConnectionCreatedCallback, this),
                                 MakeCallback(&Cybertwin::NewMpConnectionErrorCallback, this));
#else
        conn = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        conn->Bind();
        m_cnrs->GetCybertwinInterfaceByName(
            GET_PEERID_FROM_STREAMID(streamId),
            MakeCallback(&Cybertwin::SocketConnectWithResolvedCybertwinName, this, conn));

        m_pendingConnectionsReverse[conn] = streamId;
#endif

        // insert to pending set
        m_pendingConnections[streamId] = conn;
    }
}

void
Cybertwin::SocketConnectWithResolvedCybertwinName(Ptr<Socket> socket,
                                                  CYBERTWINID_t cuid,
                                                  CYBERTWIN_INTERFACE_LIST_t ifs)
{
    NS_LOG_FUNCTION(this << socket << cuid);
    if (ifs.size() == 0)
    {
        NS_LOG_ERROR("[Cybertwin] No interface found for " << cuid);
        return;
    }

    // TODO: a reasonable way to select an interface
    InetSocketAddress peeraddr = InetSocketAddress(ifs.at(0).first, ifs.at(0).second);
    NS_LOG_DEBUG("--[Edge-#" << m_cybertwinId << "]: connecting to " << ifs.at(0).first << ":"
                             << ifs.at(0).second);
    socket->SetConnectCallback(MakeCallback(&Cybertwin::NewSpConnectionCreatedCallback, this),
                               MakeCallback(&Cybertwin::NewSpConnectionErrorCallback, this));
    socket->SetRecvCallback(MakeCallback(&Cybertwin::SpConnectionRecvCallback, this));
    if (socket->Connect(peeraddr) < 0)
    {
        NS_LOG_ERROR("[Cybertwin] Failed to connect to " << peeraddr << " with error "
                                                         << socket->GetErrno());
    }
}

void
Cybertwin::CybertwinCommModelStatistical()
{
    NS_LOG_FUNCTION(this);
    uint64_t intervalBytes = m_comm_test_interval_bytes;
    if ((Simulator::Now() - m_startShapingTime).GetSeconds() >= END_HOST_BULK_SEND_TEST_TIME)
    {
        // stop test
        NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: stop test");
        m_logStream << "Cybertwin[" << m_cybertwinId << "]: stop test" << std::endl;
        m_statisticalEnd = true;
        return;
    }

    m_comm_test_interval_bytes = 0;

    Time now = Simulator::Now();
    Time interval = now - m_lastTime;
    m_lastTime = now;

    double speed = intervalBytes * 8.0 / interval.GetSeconds() / 1000000.0; // Mbps

    // report thoughput
    m_logStream << "Cybertwin[" << m_cybertwinId << "]: at "
                << (now - m_startShapingTime).GetMilliSeconds() << " ms, throughput = " << speed
                << " Mbps" << std::endl;
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: at "
                              << (now - m_startShapingTime).GetMilliSeconds()
                              << " ms, throughput = " << speed << " Mbps");

    // schedule next report
    Simulator::Schedule(MilliSeconds(STATISTIC_INTERVAL_MILLISECONDS),
                        &Cybertwin::CybertwinCommModelStatistical,
                        this);
}

#if MDTP_ENABLED
// Multipath connection Callbacks
void
Cybertwin::NewMpConnectionErrorCallback(MultipathConnection* conn)
{
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: connection error with "
                              << conn->m_peerCyberID);
    // TODO: failed to create a connection, how to handle the pending data?
}

void
Cybertwin::NewMpConnectionCreatedCallback(MultipathConnection* conn)
{
    NS_LOG_DEBUG("New connection created: " << conn->GetConnID());
    NS_ASSERT_MSG(conn != nullptr, "Connection is null");
    if (m_pendingConnections.find(conn->m_peerCyberID) != m_pendingConnections.end())
    {
        // case1: txPendingBuffer not empty means this cybertwin have initiated a connection request
        NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: connection with " << conn->m_peerCyberID
                                  << " is successfully established");

        // erase the connection from pendingConnections, and insert it to txConnections
        m_pendingConnections.erase(conn->m_peerCyberID);
        m_txConnections[conn->m_peerCyberID] = conn;

        // after connection created, we schedule a send event to send pending packets
        Simulator::ScheduleNow(&Cybertwin::SendPendingPackets, this, conn->m_peerCyberID);
        Simulator::Schedule(MilliSeconds(STATISTIC_TIME_INTERVAL),
                            &Cybertwin::UpdateRxSizePerSecond,
                            this,
                            conn->m_peerCyberID);
    }
    else
    {
        // case2: conn not found in txConnections means DataServer received a connection request and
        //  successfully created a connection
        NS_LOG_DEBUG(
            "Cybertwin[" << m_cybertwinId << "]: DataServer received a connection request from "
                         << conn->m_peerCyberID << " and successfully created a connection");
        m_rxConnections[conn->m_peerCyberID] = conn;
        m_rxSizePerSecond[conn->m_peerCyberID] = 0;
        Simulator::Schedule(MilliSeconds(STATISTIC_TIME_INTERVAL),
                            &Cybertwin::UpdateRxSizePerSecond,
                            this,
                            conn->m_peerCyberID);
        Simulator::ScheduleNow(&Cybertwin::CybertwinServerBulkSend, this, conn);
    }

    conn->SetRecvCallback(MakeCallback(&Cybertwin::MpConnectionRecvCallback, this));
    conn->SetCloseCallback(MakeCallback(&Cybertwin::MpConnectionClosedCallback, this));
}

void
Cybertwin::MpConnectionRecvCallback(MultipathConnection* conn)
{
    NS_ASSERT_MSG(conn != nullptr, "Connection is null");
    Ptr<Packet> packet;
    while ((packet = conn->Recv()))
    {
        NS_LOG_INFO("--[Edge" << GetNode()->GetId() << "-#" << m_cybertwinId
                              << "]: received packet from " << conn->m_peerCyberID << " with size "
                              << packet->GetSize() << " bytes");
        // TODO: what to do next? Send to client or save to buffer.
        m_rxSizePerSecond[conn->m_peerCyberID] += packet->GetSize();
    }
}

void
Cybertwin::MpConnectionClosedCallback(MultipathConnection* conn)
{
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: connection closed with "
                              << conn->m_peerCyberID);
    if (m_txConnections.find(conn->m_peerCyberID) != m_txConnections.end())
    {
        m_txConnections.erase(conn->m_peerCyberID);
    }
    else if (m_rxConnections.find(conn->m_peerCyberID) != m_rxConnections.end())
    {
        m_rxConnections.erase(conn->m_peerCyberID);
    }
    else
    {
        NS_LOG_ERROR("Cybertwin[" << m_cybertwinId << "]: connection closed with "
                                  << conn->m_peerCyberID << " but no such connection found");
    }
}

void
Cybertwin::DownloadFileServer(MultipathConnection* conn)
{
    NS_LOG_FUNCTION(this << conn->m_peerCyberID);
}

#else
// Naive Socket Callbacks
void
Cybertwin::NewSpConnectionErrorCallback(Ptr<Socket> sock)
{
    Address peerAddr;
    sock->GetPeerName(peerAddr);
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: connection error with " << peerAddr);
    // TODO: failed to create a connection, how to handle the pending data?
    // one way is to set a timer to retry or discard the pending data
}

bool
Cybertwin::NewSpConnectionRequestCallback(Ptr<Socket> sock)
{
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: received a connection request");
    return true;
}

void
Cybertwin::NewSpConnectionCreatedCallback(Ptr<Socket> sock)
{
    NS_ASSERT_MSG(sock != nullptr, "Connection is null");
    if (m_pendingConnectionsReverse.find(sock) != m_pendingConnectionsReverse.end())
    {
        // case[1]: socket find in tx pending connections means this cybertwin have
        //          initiated a connection, and now it is successfully established
        STREAMID_t streamId = m_pendingConnectionsReverse[sock];
        NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: stream is successfully established");

        // erase the connection from pendingConnections, and insert it to txConnections
        m_pendingConnections.erase(streamId);
        m_pendingConnectionsReverse.erase(sock);
        m_txConnections[streamId] = sock;
        m_txConnectionsReverse[sock] = streamId;

        // after connection created, we schedule a send event to send pending packets
        Simulator::ScheduleNow(&Cybertwin::SendPendingPackets, this, streamId);
    }
    else
    {
        // case[2]: socket not found in pending connections means DataServer received a
        //          request and successfully created a connection
        Address peeraddr;
        sock->GetPeerName(peeraddr);

        NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId
                                  << "]: DataServer received a connection request from " << peeraddr
                                  << " and successfully created a connection");
        m_rxConnections.insert(sock);
        m_rxConnectionsReverse[sock] = peeraddr;
        m_rxSizePerSecond[sock] = 0;

        // Simulator::ScheduleNow(&Cybertwin::CybertwinServerBulkSend, this, sock);
    }

    sock->SetRecvCallback(MakeCallback(&Cybertwin::SpConnectionRecvCallback, this));
    sock->SetCloseCallbacks(MakeCallback(&Cybertwin::SpNormalCloseCallback, this),
                            MakeCallback(&Cybertwin::SpErrorCloseCallback, this));
}

void
Cybertwin::SpConnectionRecvCallback(Ptr<Socket> sock)
{
    Ptr<Packet> packet;
    while ((packet = sock->Recv()))
    {
        NS_LOG_INFO("--[Edge" << GetNode()->GetId() << "-#" << m_cybertwinId
                              << "]: received packet from " << m_rxConnectionsReverse[sock]
                              << " with size " << packet->GetSize() << " bytes");
        // TODO: what to do next? Send to client or save to buffer.
        m_rxSizePerSecond[sock] += packet->GetSize();
    }
}

void
Cybertwin::SpNormalCloseCallback(Ptr<Socket> sock)
{
    Address peeraddr;
    sock->GetPeerName(peeraddr);
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: connection closed with " << peeraddr);
    if (m_txConnectionsReverse.find(sock) != m_txConnectionsReverse.end())
    {
        m_txConnectionsReverse.erase(sock);
    }
    else if (m_rxConnectionsReverse.find(sock) != m_rxConnectionsReverse.end())
    {
        m_rxConnectionsReverse.erase(sock);
        m_rxConnections.erase(sock);
    }
    else
    {
        NS_LOG_ERROR("Cybertwin[" << m_cybertwinId << "]: connection closed with " << peeraddr
                                  << " but no such connection found");
    }
}

void
Cybertwin::SpErrorCloseCallback(Ptr<Socket> sock)
{
    Address peeraddr;
    sock->GetPeerName(peeraddr);
    NS_LOG_WARN("Cybertwin[" << m_cybertwinId << "]: connection error with " << peeraddr);
}

#endif

//***************************************************************************************
//*                     Handle incoming connections from cloud                          *
//***************************************************************************************
void
Cybertwin::GloballyListen()
{
#if MDTP_ENABLED
    // init data server and listen for incoming connections
    m_dtServer = new CybertwinDataTransferServer();
    m_dtServer->Setup(GetNode(), m_cybertwinId, m_globalInterfaces);
    m_dtServer->Listen();
    m_dtServer->SetNewConnectCreatedCallback(
        MakeCallback(&Cybertwin::NewMpConnectionCreatedCallback, this));
#else
    // init data server and listen for incoming connections
    m_dtServer = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::TcpSocketFactory"));
    // FIXME: attention, the port number is hard-coded here
    m_dtServer->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_globalInterfaces[0].second));
    m_dtServer->SetAcceptCallback(MakeCallback(&Cybertwin::NewSpConnectionRequestCallback, this),
                                  MakeCallback(&Cybertwin::NewSpConnectionCreatedCallback, this));
    m_dtServer->Listen();
#endif
}

//***************************************************************************************
//*                    Cybertwin Comm model Traffic Shaping                             *
//***************************************************************************************

/**
 * @brief Cybertwin::CybertwinCommModelTraffic
 * @param speed in Mbps
 */
void
Cybertwin::CybertwinCommModelTrafficShaping()
{
    double thoughput = TRAFFIC_SHAPING_LIMIT_THROUGHPUT;
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: traffic shaping with speed " << thoughput
                              << " Mbps");
    // schedule a timer to generate token
    double interval_millisecond =
        (1 / ((thoughput * 1000000.0) / (515.0 * 8))) * 1000000.0; // 计算token生成间隔
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: token generate interval is "
                              << interval_millisecond << " us");
    Simulator::ScheduleNow(&Cybertwin::GenerateToken, this, interval_millisecond);

    // schedule a timer to consume token
    Simulator::ScheduleNow(&Cybertwin::ConsumeToken, this);

    // schedule a statistic timer
    Simulator::Schedule(MilliSeconds(STATISTIC_INTERVAL_MILLISECONDS),
                        &Cybertwin::TrafficShapingStatistical,
                        this);
}

void
Cybertwin::GenerateToken(double interval)
{
    m_tokenBucket++;
    NS_LOG_LOGIC("Cybertwin[" << m_cybertwinId << "]: token bucket size is " << m_tokenBucket);
    m_tokenGeneratorEvent =
        Simulator::Schedule(MicroSeconds(interval), &Cybertwin::GenerateToken, this, interval);
}

void
Cybertwin::ConsumeToken()
{
    if (m_statisticalEnd && m_tsPktQueue.empty())
    {
        StopTrafficShaping();
        return;
    }
    if (m_tokenBucket > 0 && !m_tsPktQueue.empty())
    {
        m_tokenBucket--;
        NS_LOG_LOGIC("Cybertwin[" << m_cybertwinId << "]: token bucket size is " << m_tokenBucket);
        Ptr<Packet> pkt = m_tsPktQueue.front();
        m_consumeBytes += pkt->GetSize();
        m_tsPktQueue.pop();
    }
    else
    {
        NS_LOG_LOGIC("Cybertwin[" << m_cybertwinId << "]: token bucket size is " << m_tokenBucket
                                  << ", pkt queue size is " << m_tsPktQueue.size());
    }

    // try to consume every 1us
    m_consumerEvent = Simulator::Schedule(MicroSeconds(10), &Cybertwin::ConsumeToken, this);
}

void
Cybertwin::TrafficShapingStatistical()
{
    Time now = Simulator::Now();
    Time interval = now - m_lastShapingTime;
    m_lastShapingTime = now;

    double thoughput = m_consumeBytes * 8 / interval.GetSeconds() / 1000000.0; // Mbps
    m_logStream << "Cybertwin[" << m_cybertwinId << "]: at "
                << (now - m_startShapingTime).GetMilliSeconds()
                << " ms, traffic shaping thoughput is " << thoughput << " Mbps" << std::endl;
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: at "
                              << (now - m_startShapingTime).GetMilliSeconds()
                              << " ms, traffic shaping thoughput is " << thoughput << " Mbps");
    m_consumeBytes = 0;
    m_statisticalEvent = Simulator::Schedule(MilliSeconds(STATISTIC_INTERVAL_MILLISECONDS),
                                             &Cybertwin::TrafficShapingStatistical,
                                             this);
}

void
Cybertwin::StopTrafficShaping()
{
    Simulator::Cancel(m_tokenGeneratorEvent);
    Simulator::Cancel(m_consumerEvent);
    Simulator::Cancel(m_statisticalEvent);
}

//***************************************************************************************
//*                    Cybertwin Comm model Traffic Policing                            *
//***************************************************************************************

void
Cybertwin::CybertwinCommModelTrafficPolicing()
{
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: traffic policing");
    m_startPolicingTime = Simulator::Now();
    m_lastTpStartTime = Simulator::Now();
    m_lastTpStaticTime = Simulator::Now();

    Simulator::ScheduleNow(&Cybertwin::CCMTrafficPolicingConsumePacket, this);
    Simulator::ScheduleNow(&Cybertwin::CCMTrafficPolicingStatistical, this);
}

void
Cybertwin::CCMTrafficPolicingConsumePacket()
{
    // calculate if out of limit
    Time currTime = Simulator::Now();
    if ((currTime - m_lastTpStartTime).GetMilliSeconds() >= TRAFFIC_POLICING_INTERVAL_MILLISECONDS)
    {
        // renew policing
        m_lastTpStartTime = currTime;
        m_intervalTpBytes = 0;

        m_tpConsumeEvent = Simulator::Schedule(MicroSeconds(10),
                                               &Cybertwin::CCMTrafficPolicingConsumePacket,
                                               this);
        return;
    }

    // calculate current average thoughput
    if (m_tpPktQueue.empty())
    {
        m_tpConsumeEvent = Simulator::Schedule(MicroSeconds(10),
                                               &Cybertwin::CCMTrafficPolicingConsumePacket,
                                               this);
        return;
    }

    Ptr<Packet> pkt = m_tpPktQueue.front();
    m_tpPktQueue.pop();
    uint32_t pktSize = pkt->GetSize();
    Time interval = Simulator::Now() - m_lastTpStartTime;

    double intervalThroughput =
        ((m_intervalTpBytes + pktSize) * 8) / (interval.GetSeconds()) / 1000000.0; // Mbps
    if (intervalThroughput < TRAFFIC_POLICING_LIMIT_THROUGHPUT)
    {
        // consume
        m_tpTotalConsumeBytes += pktSize;
        m_intervalTpBytes += pktSize;
    }
    else
    {
        // drop pkt
        m_tpTotalDropedBytes += pktSize;
    }

    // Schedule next consume
    m_tpConsumeEvent =
        Simulator::Schedule(MicroSeconds(10), &Cybertwin::CCMTrafficPolicingConsumePacket, this);
}

void
Cybertwin::CCMTrafficPolicingStatistical()
{
    Time now = Simulator::Now();

    if ((now - m_startPolicingTime).GetSeconds() >= END_HOST_BULK_SEND_TEST_TIME)
    {
        CCMTrafficPolicingStop();
        return;
    }

    double throughput =
        (m_tpTotalConsumeBytes * 8) / ((now - m_lastTpStaticTime).GetSeconds()) / 1000000.0; // Mbps
    NS_LOG_DEBUG("Cybertwin[" << m_cybertwinId << "]: at "
                              << (now - m_startPolicingTime).GetMilliSeconds()
                              << " ms, traffic policing thoughput is " << throughput << " Mbps");
    m_logStream << "Cybertwin[" << m_cybertwinId << "]: at "
                << (now - m_startPolicingTime).GetMilliSeconds()
                << " ms, traffic policing thoughput is " << throughput << " Mbps" << std::endl;
    m_lastTpStaticTime = now;
    m_tpTotalConsumeBytes = 0;

    // schedule next
    m_tpStatisticEvent =
        Simulator::Schedule(MilliSeconds(CYBERTWIN_COMM_MODEL_STAT_INTERVAL_MILLISECONDS),
                            &Cybertwin::CCMTrafficPolicingStatistical,
                            this);
}

void
Cybertwin::CCMTrafficPolicingStop()
{
    if (m_tpConsumeEvent.IsRunning())
    {
        Simulator::Cancel(m_tpConsumeEvent);
    }

    if (m_tpStatisticEvent.IsRunning())
    {
        Simulator::Cancel(m_tpStatisticEvent);
    }
}

//********************************************************************
//*                     Cybertwin Full Duplex Stream                 *
//********************************************************************
TypeId
CybertwinFullDuplexStream::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::CybertwinFullDuplexStream")
            .SetParent<Object>()
            .SetGroupName("Cybertwin")
            .AddConstructor<CybertwinFullDuplexStream>()
            .AddAttribute("CloudRateLimit",
                          "The rate limit of cloud to end",
                          DoubleValue(100),
                          MakeDoubleAccessor(&CybertwinFullDuplexStream::m_cloudRateLimit),
                          MakeDoubleChecker<double>())
            .AddAttribute("EndRateLimit",
                          "The rate limit of end to cloud",
                          DoubleValue(100),
                          MakeDoubleAccessor(&CybertwinFullDuplexStream::m_endRateLimit),
                          MakeDoubleChecker<double>());
    return tid;
}

CybertwinFullDuplexStream::CybertwinFullDuplexStream()
{
    NS_LOG_FUNCTION(this);
}

CybertwinFullDuplexStream::CybertwinFullDuplexStream(Ptr<Node> node,
                                                     Ptr<NameResolutionService> cnrs,
                                                     CYBERTWINID_t end,
                                                     CYBERTWINID_t cloud)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
    m_cnrs = cnrs;
    m_endID = end;
    m_cloudID = cloud;
    m_endStatus = ENDPOINT_NONE;
    m_cloudStatus = ENDPOINT_NONE;
}

CybertwinFullDuplexStream::~CybertwinFullDuplexStream()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinFullDuplexStream::Activate()
{
    NS_LOG_FUNCTION(this);
    m_sendToCloudBytes = 0;
    m_sendToEndBytes = 0;
    m_endStartTime = Simulator::Now();
    m_cloudStartTime = Simulator::Now();

    NS_LOG_DEBUG("[CybertwinFullDuplexStream] Activate full duplex stream from "
                 << m_endID << " to " << m_cloudID << " with cloud rate limit " << m_cloudRateLimit
                 << " Mbps and end rate limit " << m_endRateLimit << " Mbps");

    if (m_endSocket == nullptr)
    {
        m_endStatus = ENDPOINT_DISCONNECTED;
        NS_LOG_ERROR("[CybertwinFullDuplexStream] End socket is null");
        // TODO: connect to end
        return;
    }
    else
    {
        m_endStatus = ENDPOINT_CONNECTED;
        m_endSocket->SetRecvCallback(
            MakeCallback(&CybertwinFullDuplexStream::DuplexStreamEndRecvCallback, this));
        m_endSocket->SetCloseCallbacks(
            MakeCallback(&CybertwinFullDuplexStream::DuplexStreamEndNormalCloseCallback, this),
            MakeCallback(&CybertwinFullDuplexStream::DuplexStreamEndErrorCloseCallback, this));
    }

    if (m_cloudSocket == nullptr)
    {
        m_cloudStatus = ENDPOINT_CONNECTING;
        NS_LOG_DEBUG("[CybertwinFullDuplexStream] Cloud socket is null, try to resolve "
                     << m_cloudID);
        m_cnrs->GetCybertwinInterfaceByName(
            m_cloudID,
            MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudConnect, this));
    }
    else
    {
        m_cloudStatus = ENDPOINT_CONNECTED;
        m_cloudSocket->SetRecvCallback(
            MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudRecvCallback, this));
    }
}

void
CybertwinFullDuplexStream::DuplexStreamEndNormalCloseCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinFullDuplexStream] End connection closed");
    m_endStatus = ENDPOINT_DISCONNECTED;

    // if end is closed, the cloud must be closed
    if (m_cloudSocket != nullptr)
    {
        m_cloudSocket->Close();
    }
}

void
CybertwinFullDuplexStream::DuplexStreamEndErrorCloseCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_ERROR("[CybertwinFullDuplexStream] End connection error");
    m_endStatus = ENDPOINT_DISCONNECTED;
}

void
CybertwinFullDuplexStream::DuplexStreamCloudConnect(CYBERTWINID_t cuid,
                                                    CYBERTWIN_INTERFACE_LIST_t itfs)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinFullDuplexStream] Resolve " << cuid << " to " << itfs.size()
                                                        << " interfaces");
    // now choose the first interface
    if (itfs.size() == 0)
    {
        NS_LOG_ERROR("[CybertwinFullDuplexStream] No interface found for " << cuid);
        return;
    }
    CYBERTWIN_INTERFACE_t cloudIf = itfs.at(0);

    if (m_cloudSocket == nullptr)
    {
        m_cloudSocket = Socket::CreateSocket(m_node, TypeId::LookupByName("ns3::TcpSocketFactory"));
    }
    m_cloudSocket->Bind();
    m_cloudSocket->SetConnectCallback(
        MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudConnectCallback, this),
        MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudConnectErrorCallback, this));
    m_cloudSocket->Connect(InetSocketAddress(cloudIf.first, cloudIf.second));
}

void
CybertwinFullDuplexStream::DuplexStreamCloudConnectCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinFullDuplexStream] Cloud connected");
    m_cloudStatus = ENDPOINT_CONNECTED;
    m_cloudSocket->SetRecvCallback(
        MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudRecvCallback, this));
    m_cloudSocket->SetCloseCallbacks(
        MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudNormalCloseCallback, this),
        MakeCallback(&CybertwinFullDuplexStream::DuplexStreamCloudErrorCloseCallback, this));
}

void
CybertwinFullDuplexStream::DuplexStreamCloudConnectErrorCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_ERROR("[CybertwinFullDuplexStream] Cloud connection error");
    m_cloudStatus = ENDPOINT_DISCONNECTED;
}

void
CybertwinFullDuplexStream::DuplexStreamCloudNormalCloseCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinFullDuplexStream] Cloud connection closed");
    m_cloudStatus = ENDPOINT_DONE;

    if (m_cloudSocket != nullptr)
    {
        m_cloudSocket->Close();
    }

    if (m_cloudBuffer.empty() && m_endSocket != nullptr)
    {
        m_endSocket->Close();
    }
}

void
CybertwinFullDuplexStream::DuplexStreamCloudErrorCloseCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_ERROR("[CybertwinFullDuplexStream] Cloud connection error");
    m_cloudStatus = ENDPOINT_DISCONNECTED;
}

void
CybertwinFullDuplexStream::DuplexStreamEndRecvCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> pkt;
    while ((pkt = sock->Recv()))
    {
        // check if is stop request
        CybertwinHeader header;
        if (pkt->RemoveHeader(header))
        {
            // recev stop request
            if (ENDHOST_STOP_STREAM == header.GetCommand())
            {
                NS_LOG_INFO("[CybertwinFullDuplexStream] Received stop request from end");
                m_endStatus = ENDPOINT_END_STOP;
                // cancel send to end
                if (m_sendToEndEvent.IsRunning())
                {
                    Simulator::Cancel(m_sendToEndEvent);
                }
            }
            else if (ENDHOST_START_STREAM == header.GetCommand())
            {
                NS_LOG_INFO("[CybertwinFullDuplexStream] Received start request from end");
                m_endStatus = ENDPOINT_CONNECTED;
                m_sendToEndEvent =
                    Simulator::ScheduleNow(&CybertwinFullDuplexStream::OuputCloudBuffer, this);
            }

            continue;
        }

        NS_LOG_DEBUG("[CybertwinFullDuplexStream] Received packet from end with size "
                     << pkt->GetSize());
        if (m_endBuffer.size() > 100000)
        {
            NS_LOG_ERROR("[CybertwinFullDuplexStream] End buffer is full, drop packet");
            continue;
        }
        m_endBuffer.push(pkt);
    }

    // send to cloud
    if (!m_sendToCloudEvent.IsRunning())
    {
        m_sendToCloudEvent =
            Simulator::ScheduleNow(&CybertwinFullDuplexStream::OuputEndBuffer, this);
    }
}

void
CybertwinFullDuplexStream::DuplexStreamCloudRecvCallback(Ptr<Socket> sock)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(sock, "Socket is null");

    Ptr<Packet> pkt;
    while ((pkt = sock->Recv()))
    {
        NS_LOG_INFO("[CybertwinFullDuplexStream] Received packet from cloud with size "
                    << pkt->GetSize());
        if (m_cloudBuffer.size() > 100000)
        {
            NS_LOG_ERROR("[CybertwinFullDuplexStream] Cloud buffer is full, drop packet");
            continue;
        }
        m_cloudBuffer.push(pkt);
    }

    // send to end
    if (!m_sendToEndEvent.IsRunning())
    {
        m_sendToEndEvent =
            Simulator::ScheduleNow(&CybertwinFullDuplexStream::OuputCloudBuffer, this);
    }
}

void
CybertwinFullDuplexStream::OuputEndBuffer()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinFullDuplexStream] End buffer size is " << m_endBuffer.size());
    if (m_cloudSocket == nullptr)
    {
        NS_LOG_ERROR("[CybertwinFullDuplexStream] Cloud socket is null.");
        return;
    }

    if (m_endBuffer.empty())
    {
        NS_LOG_INFO("[CybertwinFullDuplexStream] End buffer is empty");
        return;
    }

    if (m_cloudStatus != ENDPOINT_CONNECTED)
    {
        NS_LOG_INFO("[CybertwinFullDuplexStream] Cloud is not connected.");
        return;
    }

    Ptr<Packet> pkt = m_endBuffer.front();
    uint32_t pktSize = pkt->GetSize();

    // calculate thoughput must less than 10Mbps
    double tryThroughput = (m_sendToCloudBytes + pktSize) * 8 /
                           (Simulator::Now() - m_endStartTime).GetSeconds() / 1000000.0;

    if (tryThroughput >= m_cloudRateLimit)
    {
        NS_LOG_INFO("[CybertwinFullDuplexStream] End thoughput is " << tryThroughput
                                                                    << " Mbps, wait to send.");
        m_sendToCloudEvent =
            Simulator::Schedule(MicroSeconds(10), &CybertwinFullDuplexStream::OuputEndBuffer, this);
    }
    else
    {
        int32_t sendSize = m_cloudSocket->Send(pkt);
        if (sendSize <= 0)
        {
            NS_LOG_ERROR("[CybertwinFullDuplexStream] Send to cloud error");
        }
        else
        {
            NS_LOG_INFO("[CybertwinFullDuplexStream] Send to cloud " << sendSize << " bytes at "
                                                                     << Simulator::Now());
            m_sendToCloudBytes += m_cloudSocket->Send(pkt);
            m_endBuffer.pop();
        }
        m_sendToCloudEvent =
            Simulator::Schedule(MicroSeconds(10), &CybertwinFullDuplexStream::OuputEndBuffer, this);
    }
}

void
CybertwinFullDuplexStream::OuputCloudBuffer()
{
    NS_LOG_FUNCTION(this);
    if (m_cloudBuffer.empty())
    {
        if (m_cloudStatus == ENDPOINT_DONE)
        {
            NS_LOG_INFO("[CybertwinFullDuplexStream] Cloud buffer is empty and cloud is done, stop "
                        "sending.");
            if (m_endSocket != nullptr)
            {
                m_endSocket->Close();
            }
        }
        return;
    }

    if (m_endStatus == ENDPOINT_END_STOP)
    {
        // end stop to receive, stop sending
        NS_LOG_INFO("[CybertwinFullDuplexStream] End stop to receive, stop sending.");
        return;
    }

    // send to end
    Ptr<Packet> pkt = m_cloudBuffer.front();
    uint32_t pktSize = pkt->GetSize();

    // calculate thoughput must less than 10Mbps
    double tryThroughput = (m_sendToEndBytes + pktSize) * 8 /
                           (Simulator::Now() - m_cloudStartTime).GetSeconds() / 1000000.0;

    if (tryThroughput >= m_cloudRateLimit)
    {
        NS_LOG_INFO("[CybertwinFullDuplexStream] Cloud thoughput is " << tryThroughput
                                                                      << " Mbps, wait to send.");
    }
    else
    {
        int32_t sendSize = m_endSocket->Send(pkt);
        if (sendSize <= 0)
        {
            NS_LOG_LOGIC("[CybertwinFullDuplexStream] Send to end error");
        }
        else
        {
            NS_LOG_INFO("[CybertwinFullDuplexStream] Send to end " << sendSize << " bytes at "
                                                                   << Simulator::Now());
            m_sendToEndBytes += sendSize;
            m_cloudBuffer.pop();
        }
    }

    m_sendToEndEvent =
        Simulator::Schedule(MicroSeconds(10), &CybertwinFullDuplexStream::OuputCloudBuffer, this);
}

void
CybertwinFullDuplexStream::Deactivate()
{
    NS_LOG_FUNCTION(this);
    if (m_endSocket != nullptr)
    {
        m_endSocket->Close();
    }

    if (m_cloudSocket != nullptr)
    {
        m_cloudSocket->Close();
    }
}

void
CybertwinFullDuplexStream::SetEndID(CYBERTWINID_t endID)
{
    NS_LOG_FUNCTION(this);
    m_endID = endID;
}

void
CybertwinFullDuplexStream::SetCloudID(CYBERTWINID_t cloudID)
{
    NS_LOG_FUNCTION(this);
    m_cloudID = cloudID;
}

void
CybertwinFullDuplexStream::SetEndSocket(Ptr<Socket> endSocket)
{
    NS_LOG_FUNCTION(this);
    m_endSocket = endSocket;
}

void
CybertwinFullDuplexStream::SetCloudSocket(Ptr<Socket> cloudSocket)
{
    NS_LOG_FUNCTION(this);
    m_cloudSocket = cloudSocket;
}

} // namespace ns3
