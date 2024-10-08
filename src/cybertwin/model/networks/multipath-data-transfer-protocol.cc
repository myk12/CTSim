#include "ns3/multipath-data-transfer-protocol.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CybertwinMultipathTransfer");
NS_OBJECT_ENSURE_REGISTERED(CybertwinDataTransferServer);
NS_OBJECT_ENSURE_REGISTERED(MultipathConnection);
NS_OBJECT_ENSURE_REGISTERED(SinglePath);

//*****************************************************************************
//*                    Cybertwin Data Transfer Server                         *
//*****************************************************************************

TypeId
CybertwinDataTransferServer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinDataTransferServer")
            .SetParent<Object>()
            .SetGroupName("Cybertwin")
            .AddConstructor<CybertwinDataTransferServer>()
            .AddAttribute("MpConnCreatedCallback",
                          "Callback for new connection created",
                          CallbackValue(),
                          MakeCallbackAccessor(&CybertwinDataTransferServer::m_notifyNewConnection),
                          MakeCallbackChecker());
    return tid;
}

TypeId
CybertwinDataTransferServer::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinDataTransferServer::CybertwinDataTransferServer()
    : m_node(nullptr),
      m_localCybertwinID(0)
{
}

void
CybertwinDataTransferServer::Setup(Ptr<Node> node,
                                   CYBERTWINID_t cyberid,
                                   CYBERTWIN_INTERFACE_LIST_t ifs)
{
    NS_LOG_DEBUG(node->GetId() << cyberid);
    m_node = node;
    m_localCybertwinID = cyberid;
    m_cybertwinIfs = ifs;
}

void
CybertwinDataTransferServer::Listen()
{
    // get cybertwin interface
    Ptr<NetDevice> netDevice = nullptr;
    Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
    NS_ASSERT_MSG(ipv4, "Ipv4 is null.");

    for (auto interface : m_cybertwinIfs)
    {
        // create listen socket
        Ptr<Socket> cyberEar = Socket::CreateSocket(m_node, TcpSocketFactory::GetTypeId());

        // bind socket to netdevice
        netDevice = ipv4->GetNetDevice(ipv4->GetInterfaceForAddress(interface.first));
        NS_ASSERT_MSG(netDevice, "NetDevice is null.");
        cyberEar->BindToNetDevice(netDevice);
        NS_LOG_DEBUG("DTServer socket bind to netdevice: " << netDevice);

        // bind socket
        InetSocketAddress inetAddress = InetSocketAddress(interface.first, interface.second);
        NS_ASSERT(cyberEar->Bind(inetAddress) >= 0);

        cyberEar->SetAcceptCallback(
            MakeCallback(&CybertwinDataTransferServer::PathRequestCallback, this),
            MakeCallback(&CybertwinDataTransferServer::PathCreatedCallback, this));

        cyberEar->Listen();
        
        NS_LOG_DEBUG("Start listen in " << interface.first << ":" << interface.second);
    }
}

void
CybertwinDataTransferServer::InsertCNRSCache(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t ifs)
{
    CNRSCache[id] = ifs;
}

bool
CybertwinDataTransferServer::PathRequestCallback(Ptr<Socket> sock, const Address& addr)
{
    NS_LOG_FUNCTION(this << sock << addr);
    return true; // accept all TCP Connection request
}

void
CybertwinDataTransferServer::PathCreatedCallback(Ptr<Socket> sock, const Address& addr)
{
    NS_LOG_FUNCTION(this << sock << addr);

    // create new SinglePath
    NS_LOG_DEBUG("Server born new path.");
    SinglePath* path = new SinglePath();
    path->SetSocket(sock);
    path->SetServer(this);
    path->SetPeerAddress(addr);
    path->SetLocalKey(GenerateKey());
    path->SetLocalCybertwinID(m_localCybertwinID);
    path->SetPathState(SinglePath::SINGLE_PATH_LISTEN);

    path->PathListen();
}

MP_CONN_KEY_t
CybertwinDataTransferServer::GenerateKey()
{
    MP_CONN_KEY_t key;
    if (rand == nullptr)
    {
        rand = CreateObject<UniformRandomVariable>();
    }

    do
    {
        NS_LOG_WARN("gen rand val.");
        key = (MP_CONN_KEY_t)(rand->GetValue() * 10000);
    } while (key == 0);

    return key;
}

void
CybertwinDataTransferServer::NewConnectionBuilt(SinglePath* path)
{
    NS_LOG_DEBUG("new connection built.");
    NS_ASSERT_MSG(path, "path is null.");

    // create new connection with path
    MultipathConnection* new_conn = new MultipathConnection(path);
    NS_ASSERT_MSG(new_conn, "new_conn is null.");

    // add to data server
    m_connections.insert(new_conn);
    m_connectionIDs[new_conn->GetConnID()] = new_conn;

    // aggravate connection to path
    path->SetConnection(new_conn);

    // inform application
    NS_LOG_DEBUG("inform application new connection created.");
    if (!m_notifyNewConnection.IsNull())
    {
        m_notifyNewConnection(new_conn);
    }
}

bool
CybertwinDataTransferServer::ValidConnectionID(MP_CONN_ID_t connid)
{
    NS_LOG_DEBUG("Vaildating the connectionID");
    return m_connectionIDs.find(connid) != m_connectionIDs.end();
}

/**
 * @brief A path try to join a connection when it's state changed
 * from SINGLE_PATH_LISTEN to SINGLE_PATH_CONNECTED.
 */
void
CybertwinDataTransferServer::NewPathJoinConnection(SinglePath* path)
{
    NS_LOG_DEBUG("new path join connection.");
    NS_ASSERT_MSG(path, "path is null.");

    MP_CONN_ID_t connid = path->GetConnectionID();
    if (m_connectionIDs.find(connid) == m_connectionIDs.end())
    {
        NS_LOG_ERROR("Connection [" << connid << "] doesn't exist.");
        return;
    }

    MultipathConnection* conn = m_connectionIDs[connid];
    path->SetConnection(conn);
    conn->AddOtherConnectPath(path);
}

void
CybertwinDataTransferServer::SetNewConnectCreatedCallback(
    Callback<void, MultipathConnection*> newConn)
{
    m_notifyNewConnection = newConn;
    m_rxBytes = 0;
}

void
CybertwinDataTransferServer::DtServerBulkSend(MultipathConnection* conn)
{
    NS_LOG_DEBUG("Server bulk send.");
    NS_ASSERT_MSG(conn, "conn is null.");

    if (m_rxBytes < 100 * 1024 * 1024) // 100GB
    {
        Ptr<Packet> packet = Create<Packet>(516);
        conn->Send(packet);
        m_rxBytes++;

        // 1024B/ns = 102.4MBps
        Simulator::Schedule(MicroSeconds(1),
                            &CybertwinDataTransferServer::DtServerBulkSend,
                            this,
                            conn);
    }
}

//*****************************************************************************
//*                     Multipath Connection                                  *
//*****************************************************************************
TypeId
MultipathConnection::GetTypeId()
{
    static TypeId tid = 
        TypeId("ns3::MultipathConnection")
            .SetParent<Object>()
            .SetGroupName("Cybertwin")
            .AddConstructor<MultipathConnection>()
            .AddAttribute("SendReportInterval",
                          "The interval of sending report to peer.",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&MultipathConnection::m_sendReportInterval),
                          MakeTimeChecker())
            .AddAttribute("SendReportCallback",
                          "The callback function when sending report.",
                          CallbackValue(),
                          MakeCallbackAccessor(&MultipathConnection::m_sendReportCallback),
                          MakeCallbackChecker())
            .AddAttribute("RecvReportInterval",
                          "The interval of receiving report from peer.",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&MultipathConnection::m_recvReportInterval),
                          MakeTimeChecker())
            .AddAttribute("RecvReportCallback",
                          "The callback function when receiving report.",
                          CallbackValue(),
                          MakeCallbackAccessor(&MultipathConnection::m_recvReportCallback),
                          MakeCallbackChecker());
        
    return tid;
}

TypeId
MultipathConnection::GetInstanceTypeId() const
{
    return GetTypeId();
}

MultipathConnection::MultipathConnection()
{
    NS_LOG_FUNCTION("MultipathConnection constructor.");
    m_node = nullptr;
    m_localCyberID = 0;
    m_peerCyberID = 0;
    m_localKey = 0;
    m_connID = 0;
    m_connState = MP_CONN_INIT;
    m_pathNum = 0;

    if (!m_sendReportCallback.IsNull())
    {
        Simulator::Schedule(m_sendReportInterval,
                            &MultipathConnection::ConnectionReport,
                            this,
                            true);
    }
    if (!m_recvReportCallback.IsNull())
    {
        Simulator::Schedule(m_recvReportInterval,
                            &MultipathConnection::ConnectionReport,
                            this,
                            false);
    }
}

MultipathConnection::MultipathConnection(SinglePath* path)
{
    NS_LOG_FUNCTION("MultipathConnection constructor.");
    NS_ASSERT_MSG(path, "path is null.");
    NS_ASSERT_MSG(path->GetPathState() == SinglePath::SINGLE_PATH_CONNECTED,
                  "path state is not connected.");
    m_node = nullptr; // currently not used
    m_localCyberID = path->GetLocalCybertwinID();
    m_peerCyberID = path->GetRemoteCybertwinID();
    m_localKey = path->GetLocalKey();
    m_connID = path->GetConnectionID();
    m_sendSeqNum = m_connID;
    m_recvSeqNum = m_connID;
    m_connState = MP_CONN_CONNECT;
    m_pathNum = 0;

    m_paths.push_back(path);

    if (!m_sendReportCallback.IsNull())
    {
        Simulator::Schedule(m_sendReportInterval,
                            &MultipathConnection::ConnectionReport,
                            this,
                            true);
    }
    if (!m_recvReportCallback.IsNull())
    {
        Simulator::Schedule(m_recvReportInterval,
                            &MultipathConnection::ConnectionReport,
                            this,
                            false);
    }
}

void
MultipathConnection::Setup(Ptr<Node> node, CYBERTWINID_t cyberid, CYBERTWIN_INTERFACE_LIST_t interfaces)
{
    m_node = node;
    m_localCyberID = cyberid;
    m_interfaces = interfaces;

    if (rand == nullptr)
    {
        rand = CreateObject<UniformRandomVariable>();
    }
    do
    {
        NS_LOG_DEBUG("gen rand val.");
        m_localKey = (MP_CONN_KEY_t)(rand->GetValue() * 10000);
    } while (m_localKey == 0);

    m_connState = MP_CONN_INIT;
}

void
MultipathConnection::Connect(CYBERTWINID_t targetID)
{
    NS_LOG_DEBUG("Connceting to remote Cybertwin : " << targetID);
    m_peerCyberID = targetID;
    // resolve the cyberID
    NS_ASSERT_MSG(m_node, "Connection related with node is not initialized.");
    Ptr<NameResolutionService> cnrs = DynamicCast<CybertwinEdgeServer>(m_node)->GetCNRSApp();
    NS_ASSERT_MSG(cnrs, "CNRS is null.");
    cnrs->GetCybertwinInterfaceByName(
        targetID,
        MakeCallback(&MultipathConnection::OnCybertwinInterfaceResolved, this));
}

void
MultipathConnection::OnCybertwinInterfaceResolved(CYBERTWINID_t peerId,
                                                  CYBERTWIN_INTERFACE_LIST_t interfaces)
{
    NS_LOG_DEBUG("Cybertwin interface resolved, connect.");
    m_pathNum = m_interfaces.size();
    NS_LOG_DEBUG("Cybertwin " << peerId << " contain " << m_pathNum << " interfaces.");
    Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
    NS_ASSERT_MSG(ipv4, "Ipv4 is null.");
    Ptr<NetDevice> netDevice = nullptr;

    uint32_t pathNum = 0;
    for (auto itf:m_interfaces)
    {
        NS_LOG_DEBUG("Using interface : "<<itf);
        SinglePath* path = new SinglePath();
        Ptr<Socket> sock = Socket::CreateSocket(m_node, TcpSocketFactory::GetTypeId());

        // find netdevice by ipaddr and bind socket to it
        netDevice = ipv4->GetNetDevice(ipv4->GetInterfaceForAddress(itf.first));
        NS_ASSERT_MSG(netDevice, "NetDevice is null.");
        sock->BindToNetDevice(netDevice);
        
        path->SetSocket(sock);
        path->SetRemoteInteface(interfaces[pathNum++]);
        path->SetLocalKey(m_localKey);
        path->SetLocalCybertwinID(m_localCyberID);
        path->SetConnection(this);

        path->PathConnect();
        NS_LOG_UNCOND("Conn[" << m_localKey <<"]: born a path with :("<< sock << ", " <<netDevice <<")");
    }
}

/**
 * @brief Send data to remote Cybertwin
 *
 * @param packet
 * @return int32_t
 */
int32_t
MultipathConnection::Send(Ptr<Packet> packet)
{
    int32_t pktSize = packet->GetSize();
    if (pktSize == 0)
    {
        NS_LOG_LOGIC("Packet size is 0, do nothing.");
        return 0;
    }

    // construct header
    MultipathHeaderDSN header;
    header.SetCuid(m_localCyberID);
    header.SetDataSeqNum(m_sendSeqNum);
    header.SetDataLen(pktSize);
    packet->AddHeader(header);

    // update send sequence number
    m_sendSeqNum += pktSize;

    NS_LOG_DEBUG("MpConn[" << m_connID << "] send data to remote Cybertwin : " << m_peerCyberID);
    m_txBuffer.push(packet);
    Simulator::ScheduleNow(&MultipathConnection::SendData, this);

    return pktSize;
}

void
MultipathConnection::SendData()
{
    NS_LOG_FUNCTION(this);
    if (m_txBuffer.empty())
    {
        NS_LOG_DEBUG("TxBuffer is empty.");
        return;
    }

    // choose one path to send
    int32_t pathIndex = ChoosePathRoundRobin();
    if (pathIndex == -1)
    {
        NS_LOG_DEBUG("No path to send.");
        return;
    }

    NS_LOG_DEBUG("Choose No." << pathIndex << " path to send.");

    uint32_t counter = 0;
    while (counter < MULTIPATH_MAXSENT_PACKET_ONCE && !m_txBuffer.empty())
    {
        Ptr<Packet> pkt = m_txBuffer.front();
        m_paths[pathIndex]->Send(pkt);
        m_txBuffer.pop();

        counter++;
        MultipathHeaderDSN header;
        m_txTotalBytes += pkt->GetSize() - header.GetSerializedSize();
    }
}

Ptr<Packet>
MultipathConnection::Recv()
{
    NS_LOG_DEBUG("Recv");
    if (m_rxBuffer.empty())
    {
        NS_LOG_DEBUG("RxBuffer is empty.");
        return nullptr;
    }

    Ptr<Packet> pack = m_rxBuffer.front();
    m_rxBuffer.pop();
    return pack;
}

void
MultipathConnection::PathRecvedData(SinglePath* path)
{
    NS_LOG_FUNCTION(this);

    // extract data from path
    NS_LOG_DEBUG("MpConnection[" << m_connID <<" ] PathRecvedData, m_recvSeqNum =" <<
                m_recvSeqNum << "path->m_rxHeadSeqNum = "<< path->HeadPacketSeqNum());
    while (m_recvSeqNum == path->HeadPacketSeqNum() || path->HeadPacketSeqNum() != MpDataSeqNum(0))
    {
        Ptr<Packet> pack = nullptr;
        pack = path->Recv();
        NS_ASSERT_MSG(pack != nullptr, "pack is null.");

        m_rxBuffer.push(pack);

        m_recvSeqNum += pack->GetSize(); // renew seqnum
        NS_LOG_DEBUG("MpConnection[" << m_connID << "] received data, renewed m_recvSeqNum = " << m_recvSeqNum);
        m_rxTotalBytes += pack->GetSize();
    }

    // notify upper layer
    if (!m_rxBuffer.empty() && !m_recvCallback.IsNull())
    {
        NS_LOG_DEBUG("MpConnection[" << m_connID << "] received data, notify upper layer.");
        m_recvCallback(this);
    }
}

/**
 * @brief Close the connection
 */
int32_t
MultipathConnection::Close()
{
    NS_LOG_FUNCTION(this);
    // if rxBuffer is empty then close
    // else wait for 10ms and check again
    int closedNum = 0;
    if (m_rxBuffer.empty())
    {
        m_connState = MP_CONN_CLOSING;
        int32_t num = m_paths.size();
        for (int32_t i = 0; i < num; i++)
        {
            if (m_paths[i] == nullptr)
            {
                closedNum++;
                continue;
            }

            if (m_paths[i]->m_pathState == SinglePath::SINGLE_PATH_CLOSED)
            {
                // delete path
                m_paths[i]->PathClean();
                delete m_paths[i];
            }

            m_paths[i]->PathClose();
        }
    }

    if (closedNum == (int32_t)m_paths.size())
    {
        NS_LOG_DEBUG("Connection Closed.");
        m_connState = MP_CONN_CLOSED;
        if (!m_closeCallback.IsNull())
        {
            NS_LOG_DEBUG("Connection Closed, notify upper layer.");
            m_closeCallback(this); // notify upper layer
        }
    }
    else
    {
        NS_LOG_DEBUG("Connection Closing.");
        Simulator::Schedule(MilliSeconds(10), &MultipathConnection::Close, this);
    }

    return 0;
}

void
MultipathConnection::AddRawPath(SinglePath* path, bool ready)
{
    NS_LOG_DEBUG("Connection try to add new path");
    NS_ASSERT(path);
    if (ready)
    {
        NS_LOG_DEBUG("Add new ready path");
        m_rawReadyPath.push(path);
    }
    else
    {
        NS_LOG_DEBUG("Add new fail path");
        m_rawFailPath.push(path);
    }

    NS_LOG_DEBUG("m_rawReadyPath.size() = " << m_rawReadyPath.size() << ", m_rawFailPath.size() = " << m_rawFailPath.size() << ", m_pathNum = " << m_pathNum);
    if ((int32_t)(m_rawReadyPath.size() + m_rawFailPath.size()) == m_pathNum)
    {
        // start build connection
        NS_LOG_DEBUG("All " << m_pathNum << " path initialized, start build connection.");
        Simulator::Schedule(TimeStep(1), &MultipathConnection::BuildConnection, this);
    }
}

// choose one path from the rawpath set
// and use this path to init connection
void
MultipathConnection::BuildConnection()
{
    NS_LOG_DEBUG("Building connection.");
    if (m_rawReadyPath.size() <= 0)
    {
        NS_LOG_ERROR("Error, no ready path exists.");
        return;
    }
    SinglePath* path = m_rawReadyPath.front();
    m_rawReadyPath.pop();
    path->InitConnection();
}

// called by the first connected path
void
MultipathConnection::AddInitConnectPath(SinglePath* path)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Add init connect path.");
    m_connID = path->GetConnectionID();
    NS_ASSERT(m_connID != 0);
    m_sendSeqNum = m_connID; // init send seq num
    m_recvSeqNum = m_connID; // init recv seq num
    m_remoteKey = path->GetRemoteKey();
    m_readyPath.push(path);
    m_paths.push_back(path);

    m_connState = MP_CONN_CONNECT;

    // connect to the other path
    Simulator::Schedule(TimeStep(1), &MultipathConnection::ConnectOtherPath, this);
    // notify the upper layer
    if (!m_connectSucceedCallback.IsNull())
    {
        m_connectSucceedCallback(this);
    }
}

void
MultipathConnection::ConnectOtherPath()
{
    NS_LOG_DEBUG("Connect other path.");
    while (m_rawReadyPath.size() > 0)
    {
        // aggreate each path in the raw path set to connection
        SinglePath* path = m_rawReadyPath.front();
        m_rawReadyPath.pop();
        path->SetConnectionID(m_connID);
        path->SetRemoteKey(m_remoteKey);
        path->SetLocalCybertwinID(m_localCyberID);

        path->JoinConnection();
    }
}

// called by the subsequent connected path
void
MultipathConnection::AddOtherConnectPath(SinglePath* path)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(path->GetConnectionID() == m_connID);

    m_paths.push_back(path);
    m_connState = MP_CONN_CONNECT;
}

uint32_t
MultipathConnection::ChoosePathRoundRobin()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_paths.size() > 0);
    uint32_t idx = -1;
    do
    {
        idx = (m_lastPathIndex++) % m_paths.size();
        if (m_paths[idx]->GetPathState() == SinglePath::SINGLE_PATH_CONNECTED)
        {
            NS_LOG_DEBUG("Choose path " << idx);
            return idx;
        }
    } while (true);

    return idx;
}

void
MultipathConnection::SetLocalKey(MP_CONN_KEY_t key)
{
    m_localKey = key;
}

void
MultipathConnection::SetRemoteKey(MP_CONN_KEY_t key)
{
    m_remoteKey = key;
}

void
MultipathConnection::SetConnState(MP_CONN_STATE st)
{
    NS_LOG_DEBUG("Connection state change to " << st);
    m_connID = st;
}

void
MultipathConnection::InitIdentity(Ptr<Node> node,
                                  CYBERTWINID_t localid,
                                  CYBERTWINID_t remoteid,
                                  MP_CONN_ID_t connid)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
    m_localCyberID = localid;
    m_peerCyberID = remoteid;
    m_connID = connid;
}

void
MultipathConnection::SetNode(Ptr<Node> node)
{
    m_node = node;
}

void
MultipathConnection::SetLocalCybertwinID(CYBERTWINID_t id)
{
    m_localCyberID = id;
}

void
MultipathConnection::SetPeerCybertwinID(CYBERTWINID_t id)
{
    m_peerCyberID = id;
}

void
MultipathConnection::SetConnID(MP_CONN_ID_t id)
{
    m_connID = id;
}

MP_CONN_ID_t
MultipathConnection::GetConnID()
{
    return m_connID;
}

void
MultipathConnection::SetConnectCallback(Callback<void, MultipathConnection*> succeedCb,
                                        Callback<void, MultipathConnection*> failCb)
{
    m_connectSucceedCallback = succeedCb;
    m_connectFailedCallback = failCb;
}

void
MultipathConnection::SetRecvCallback(Callback<void, MultipathConnection*> recvCb)
{
    m_recvCallback = recvCb;
}

void
MultipathConnection::SetCloseCallback(Callback<void, MultipathConnection*> closeCb)
{
    m_closeCallback = closeCb;
}

void
MultipathConnection::PathJoinResult(SinglePath* path, bool success)
{
    NS_LOG_DEBUG("Path join result call.");
    // TODO: a lot more things to do
    if (success)
    {
        // add path to ready set
        NS_LOG_DEBUG("Path successfully joined the connection");
        NS_ASSERT_MSG(path->GetConnectionID() == m_connID, "Error connection id.");
        m_readyPath.push(path);
        m_paths.push_back(path);
    }
    else
    {
        NS_LOG_DEBUG("Path failed to join the connection");
        m_errorPath.insert(path);
    }

    // report connection status
    if (m_readyPath.size() + m_errorPath.size() == (uint32_t)m_pathNum)
    {
        // all path init join
        NS_LOG_DEBUG("All paths have tried to join the connection.");
        if (m_readyPath.size() == (uint32_t)m_pathNum)
        {
            // all path successfully joined
            NS_LOG_DEBUG("All paths successfully joined the connection.");
        }
        else
        {
            // some path failed to join
            NS_LOG_DEBUG("Successfully joined " << m_readyPath.size() << " paths."
                                                << m_errorPath.size() << " paths failed to join.");
        }
    }
    else
    {
        NS_LOG_DEBUG("Still waiting for " << m_pathNum - m_readyPath.size() - m_errorPath.size()
                                         << " paths to join the connection.");
    }
}

void
MultipathConnection::ConnectionReport(bool send)
{
    MpConnId_s connId;
    connId.connid = m_connID;
    connId.local = m_localCyberID;
    connId.peer = m_peerCyberID;

    if (send)
    {
        //send report
        MpSendReport_s sendReport;
        sendReport.conn = connId;
        sendReport.connTxBytes = m_txTotalBytes;
        for (auto path:m_paths)
        {
            sendReport.pathTxBytes.push_back(
                std::make_pair(path->m_pathId, path->m_txTotalBytes));
        }

        m_sendReportCallback(sendReport);

        //schedule next report
        Simulator::Schedule(m_sendReportInterval, &MultipathConnection::ConnectionReport, this, true);
    }else
    {
        //recv report
        MpRecvReport_s recvReport;
        recvReport.conn = connId;
        recvReport.connRxBytes = m_rxTotalBytes;
        for (auto path:m_paths)
        {
            recvReport.pathRxBytes.push_back(
                std::make_pair(path->m_pathId, path->m_rxTotalBytes));
        }

        m_recvReportCallback(recvReport);

        //schedule next report
        Simulator::Schedule(m_recvReportInterval, &MultipathConnection::ConnectionReport, this, false);
    }
}

//*****************************************************************************
//*                              Single Path                                  *
//*****************************************************************************
TypeId
SinglePath::GetTypeId()
{
    static TypeId tid = 
        TypeId("ns3::SinglePath")
            .SetParent<Object>()
            .SetGroupName("Cybertwin")
            .AddConstructor<SinglePath>();
    
    return tid;
}

TypeId
SinglePath::GetInstanceTypeId() const
{
    return GetTypeId();
}

SinglePath::SinglePath()
    : m_socket(nullptr),
      m_localKey(0),
      m_remoteKey(0),
      m_localCybertwinID(0),
      m_remoteCybertwinID(0),
      m_connection(nullptr),
      m_server(nullptr),
      m_pathState(SINGLE_PATH_INIT),
      m_connID(0)
{
#if CYBERTWIN_MDTP_LOG_ENABLE
    m_pathCreatTime = Simulator::Now();
#endif
}

int32_t
SinglePath::Send(Ptr<Packet> pkt)
{
    NS_LOG_FUNCTION(this);
    if (m_socket == nullptr)
    {
        NS_LOG_ERROR("Socket not initialized.");
        return -1;
    }

    if (m_pathState != SINGLE_PATH_CONNECTED)
    {
        NS_LOG_ERROR("Path not established.");
        return -1;
    }

    int32_t ret = 0;
    //NS_LOG_DEBUG("SinglePath[" << m_pathId << "] send packet. Size: " << pkt->GetSize());
    //NS_LOG_UNCOND("SinglePath[" << m_pathId << "] send packet. Size: " << pkt->GetSize());
    ret = m_socket->Send(pkt);
    if (ret <= 0)
    {
        //NS_LOG_ERROR("SinglePath[" << m_pathId << "] send packet failed.");
        //NS_LOG_DEBUG("SinglePath[" << m_pathId << "] send packet fail. Size: " << m_txTotalBytes);
    }else
    {
        m_txTotalBytes += ret;
        NS_LOG_DEBUG("SinglePath[" << m_pathId << "] send packet success. Size: " << m_txTotalBytes);
    }

    return ret;
}

Ptr<Packet>
SinglePath::Recv()
{
    NS_LOG_FUNCTION(this);
    if (m_rxBuffer.empty())
    {
        NS_LOG_DEBUG("No packet in rx buffer.");
        return nullptr;
    }

    Ptr<Packet> packet = m_rxBuffer.front();
    m_rxBuffer.pop();
    MultipathHeaderDSN pktHeader;
    packet->RemoveHeader(pktHeader);

    return packet;
}

MpDataSeqNum
SinglePath::HeadPacketSeqNum()
{
    NS_LOG_DEBUG("SinglePath[" << m_pathId << "] Getting HeadPacketSeqNum.");
    MpDataSeqNum headSeq(0);
    if (m_rxBuffer.empty())
    {
        NS_LOG_DEBUG("SinglePath[" << m_pathId << "] rx buffer is empty.");
        return headSeq;
    }

    Ptr<Packet> pack = m_rxBuffer.front();
    MultipathHeaderDSN header;
    NS_LOG_DEBUG("SinglePath[" << m_pathId << "] rx buffer size: " << m_rxBuffer.size());
    pack->PeekHeader(header);
    headSeq = header.GetDataSeqNum();

    return headSeq;
}

int32_t
SinglePath::PathBind(Address remote)
{
    if (m_socket == nullptr)
    {
        NS_LOG_ERROR("Socket not initialized.");
        return -1;
    }

    m_socket->Bind(remote);
    return 0;
}

/**
 * @brief connect to peer endpoint to establish a path
 */
int32_t
SinglePath::PathConnect()
{
    if (m_socket == nullptr)
    {
        NS_LOG_ERROR("Socket not initialized.");
        return -1;
    }

    //m_socket->Bind();
    NS_LOG_DEBUG("SinglePath[" << m_pathId << "] connecting to " << m_remoteIf.first << ":" << m_remoteIf.second);
    m_socket->Connect(InetSocketAddress(m_remoteIf.first, m_remoteIf.second));

    m_socket->SetConnectCallback(MakeCallback(&SinglePath::PathConnectSucceeded, this),
                                 MakeCallback(&SinglePath::PathConnectFailed, this));
    m_socket->SetCloseCallbacks(MakeCallback(&SinglePath::PathCloseSucceeded, this),
                                MakeCallback(&SinglePath::PathCloseFailed, this));
    return 0;
}

int32_t
SinglePath::PathListen()
{
    if (m_socket == nullptr)
    {
        NS_LOG_ERROR("Socket not initialized.");
        return -1;
    }

    m_pathState = SINGLE_PATH_LISTEN;
    m_socket->SetRecvCallback(MakeCallback(&SinglePath::PathRecvHandler, this));

    return 0;
}

int32_t
SinglePath::PathClose()
{
    if (m_socket == nullptr)
    {
        NS_LOG_ERROR("Socket not initialized.");
        return -1;
    }

    m_socket->Close();
    return 0;
}

void
SinglePath::PathClean()
{
    // TODO: clean all the data, currently just close the socket
}

void
SinglePath::PathConnectSucceeded(Ptr<Socket> sock)
{
    NS_LOG_DEBUG("Path connect success.");
    // set receive handler
    sock->SetRecvCallback(MakeCallback(&SinglePath::PathRecvHandler, this));
    // m_socket = sock;

    // change state
    m_pathState = SINGLE_PATH_READY;

    // Send join request
    m_connection->AddRawPath(this, true);
}

void
SinglePath::PathConnectFailed(Ptr<Socket> sock)
{
    NS_LOG_DEBUG("Path connect failed.");
    m_pathState = SINGLE_PATH_ERROR;

    m_connection->AddRawPath(this, false);
}

void
SinglePath::PathRecvHandler(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    StateProcesser();
}

void
SinglePath::PathCloseSucceeded(Ptr<Socket> socket)
{
    NS_LOG_DEBUG("Path close normally.");
    m_pathState = SINGLE_PATH_CLOSED;
    StateProcesser();
}

void
SinglePath::PathCloseFailed(Ptr<Socket> socket)
{
    NS_LOG_DEBUG("Path close with error.");
    m_pathState = SINGLE_PATH_ERROR;
    StateProcesser();
}

// first path to build the connection
void
SinglePath::InitConnection()
{
    MultipathHeader connHeader;
    connHeader.SetPathId(m_pathId);
    connHeader.SetCuid(m_localCybertwinID);
    connHeader.SetSenderKey(m_localKey);
    connHeader.SetRecverKey(0);
    connHeader.SetConnId(0);

    SendPacketWithHeader(connHeader);

    m_pathState = SINGLE_PATH_BUILD_SENT;
}

// other path to join the connection
void
SinglePath::JoinConnection()
{
    NS_LOG_DEBUG("Join connection. " << m_localCybertwinID << " " << m_remoteCybertwinID << " "
                                     << m_localKey << " " << m_remoteKey << " " << m_connID);
    MultipathHeader connHeader;
    connHeader.SetPathId(m_pathId);
    connHeader.SetCuid(m_localCybertwinID);
    connHeader.SetSenderKey(m_localKey);
    connHeader.SetRecverKey(m_remoteKey);
    connHeader.SetConnId(m_connID);

    SendPacketWithHeader(connHeader);

    m_pathState = SINGLE_PATH_JOIN_SENT;
}

int32_t
SinglePath::SendPacketWithHeader(MultipathHeader header)
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(header);

    return m_socket->Send(packet);
}

void
SinglePath::StateProcesser()
{
    switch (m_pathState)
    {
    case SINGLE_PATH_INIT:
        ProcessInit();
        break;
    case SINGLE_PATH_BUILD_SENT:
        ProcessBuildSent();
        break;
    case SINGLE_PATH_LISTEN:
        ProcessListen();
        break;
    case SINGLE_PATH_JOIN_SENT:
        ProcessJoinSent();
        break;
    case SINGLE_PATH_CONNECTED:
        ProcessConnected();
        break;
    case SINGLE_PATH_CLOSED:
        ProcessClosed();
        break;
    case SINGLE_PATH_ERROR:
        ProcessError();
        break;
    default:
        break;
    }
}

void
SinglePath::ProcessBuildSent()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("ProcessBuildSent");
    Ptr<Packet> packet;
    Address from;
    while ((packet = m_socket->RecvFrom(from)))
    {
        MultipathHeader rcvHeader;
        if (packet->PeekHeader(rcvHeader) == 0)
        {
            NS_LOG_DEBUG("Unknown packet, except multipath header.");
            continue;
        }

        MP_CONN_ID_t connID = rcvHeader.GetConnId();
        MP_CONN_KEY_t remoteKey = rcvHeader.GetSenderKey();
        // naive check
        NS_ASSERT(connID != 0 && remoteKey != 0);
        m_connID = connID;
        m_remoteKey = remoteKey;

        m_pathState = SINGLE_PATH_CONNECTED;
    }

    if (m_pathState == SINGLE_PATH_CONNECTED)
    {
        Simulator::Schedule(TimeStep(1),
                            &MultipathConnection::AddInitConnectPath,
                            m_connection,
                            this); // inform connection
    }
}

void
SinglePath::ProcessInit()
{
    NS_LOG_DEBUG("Process Init.");
}

void
SinglePath::ProcessError()
{
    NS_LOG_DEBUG("Process Error.");
}

void
SinglePath::ProcessClosed()
{
    NS_LOG_DEBUG("Process Closed.");
}

void
SinglePath::ProcessConnected()
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet;
    Address from;
    while ((packet = m_socket->RecvFrom(from)))
    {
        MultipathHeaderDSN dsnHeader;
        packet->PeekHeader(dsnHeader);

        // check whether the packet Header and payload match
        //NS_ASSERT_MSG(dsnHeader.GetDataLen() + dsnHeader.GetSerializedSize() == packet->GetSize(),
        //              "Path Header and Payload don't match.");
        
        //dsnHeader.Print(std::cout);

        Ptr<Packet> rcvPacket = packet->Copy();
        m_rxBuffer.push(rcvPacket);
        m_rxTotalBytes += rcvPacket->GetSize() - dsnHeader.GetSerializedSize();
    }

    // Notify connection
    Simulator::Schedule(TimeStep(1), &MultipathConnection::PathRecvedData, m_connection, this);
}

void
SinglePath::ProcessJoinSent()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Process Join Sent.");
    Ptr<Packet> packet;
    Address from;
    while ((packet = m_socket->RecvFrom(from)))
    {
        MultipathHeader recvHeader;
        MP_CONN_ID_t connID;
        bool joinResult = false;
        if (packet->PeekHeader(recvHeader) == 0)
        {
            NS_LOG_DEBUG("Error packet header.");
            continue;
        }

        // check key
        connID = recvHeader.GetConnId();
        if (connID != m_connID)
        {
            NS_LOG_DEBUG("Error, wrong connection id.");
            joinResult = false;
            m_pathState = SINGLE_PATH_ERROR;
        }else
        {
            NS_LOG_DEBUG("Join success.");
            joinResult = true;
            m_pathState = SINGLE_PATH_CONNECTED;
#if CYBERTWIN_MDTP_LOG_ENABLE
            m_joinConnTime = Simulator::Now();
#endif
        }

        // Inform Connection of the result
        Simulator::Schedule(TimeStep(1),
                            &MultipathConnection::PathJoinResult,
                            m_connection,
                            this,
                            joinResult);
    }
}

void
SinglePath::ProcessListen()
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet;
    Address from;
    while ((packet = m_socket->RecvFrom(from)))
    {
        MultipathHeader rcvHeader;
        MultipathHeader rspHeader;
        MP_CONN_ID_t connID;
        if (packet->PeekHeader(rcvHeader) == 0)
        {
            NS_LOG_DEBUG("ProcessListen - Unknown packet, except conn header.");
            continue;
        }
        rcvHeader.Print(std::cout);

        // check key
        connID = rcvHeader.GetConnId();
        if (connID == 0)
        {
            // first path
            m_remoteKey = rcvHeader.GetSenderKey();
            m_remoteCybertwinID = rcvHeader.GetCuid();
            SP_KEYS_TO_CONNEID(connID, m_remoteKey, m_localKey);
            m_connID = connID;
            NS_LOG_DEBUG("ProcessListen - First path, connID: " << connID);

            rspHeader.SetSenderKey(m_localKey);
            rspHeader.SetRecverKey(m_remoteKey);
            rspHeader.SetConnId(m_connID);

            SendPacketWithHeader(rspHeader);

            m_pathState = SINGLE_PATH_CONNECTED;
            m_server->NewConnectionBuilt(this);
            return;
        }

        // join connection
        if (m_server->ValidConnectionID(connID))
        {
            // connection exist
            NS_LOG_DEBUG("ProcessListen - Join connection, connID: " << connID);
            m_remoteKey = rcvHeader.GetSenderKey();
            m_connID = connID;

            // m_server->AddPath2Connection(this);
            m_server->NewPathJoinConnection(this);

            rspHeader.SetSenderKey(m_localKey);
            rspHeader.SetRecverKey(m_remoteKey);
            rspHeader.SetConnId(m_connID);

            SendPacketWithHeader(rspHeader);

            m_pathState = SINGLE_PATH_CONNECTED;
        }
        else
        {
            // connection do not exist
            NS_LOG_ERROR("Attempt to join a non-exist connection.");
            return;
        }
    }
}

int32_t
SinglePath::SendPacketWithJoinTag(MultipathTagConn tag)
{
    Ptr<Packet> rspPacket = Create<Packet>(128);
    rspPacket->AddPacketTag(tag);
    if (m_socket->Send(rspPacket) < 0)
    {
        NS_LOG_ERROR("Response error.");
        return -1;
    }

    return 0;
}

MP_CONN_KEY_t
SinglePath::GetLocalKey()
{
    return m_localKey;
}

void
SinglePath::SetLocalKey(MP_CONN_KEY_t key)
{
    m_localKey = key;
}

void
SinglePath::SetRemoteKey(MP_CONN_KEY_t key)
{
    m_remoteKey = key;
}

MP_CONN_KEY_t
SinglePath::GetRemoteKey()
{
    return m_remoteKey;
}

void
SinglePath::SetRemoteInteface(CYBERTWIN_INTERFACE_t ift)
{
    m_remoteIf = ift;
}

void
SinglePath::SetSocket(Ptr<Socket> sock)
{
    m_socket = sock;
}

void
SinglePath::SetServer(CybertwinDataTransferServer* server)
{
    m_server = server;
}

void
SinglePath::SetPathId(MP_PATH_ID_t id)
{
    m_pathId = id;
}

MP_PATH_ID_t
SinglePath::GetPathId()
{
    return m_pathId;
}

void
SinglePath::SetPathState(PathStatus stat)
{
    m_pathState = stat;
}

SinglePath::PathStatus
SinglePath::GetPathState()
{
    return m_pathState;
}

void
SinglePath::SetConnectionID(MP_CONN_ID_t id)
{
    m_connID = id;
}

MP_CONN_ID_t
SinglePath::GetConnectionID()
{
    return m_connID;
}

void
SinglePath::SetPeerAddress(Address addr)
{
    m_peerAddr = addr;
}

void
SinglePath::SetConnection(MultipathConnection* conn)
{
    m_connection = conn;
}

void
SinglePath::SetLocalCybertwinID(CYBERTWINID_t id)
{
    m_localCybertwinID = id;
}

CYBERTWINID_t
SinglePath::GetLocalCybertwinID()
{
    return m_localCybertwinID;
}

void
SinglePath::SetRemoteCybertwinID(CYBERTWINID_t id)
{
    m_remoteCybertwinID = id;
}

CYBERTWINID_t
SinglePath::GetRemoteCybertwinID()
{
    return m_remoteCybertwinID;
}

} //namespace ns3