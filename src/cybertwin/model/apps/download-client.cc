#include "ns3/download-client.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("DownloadClient");
NS_OBJECT_ENSURE_REGISTERED(DownloadClient);

#define SECURITY_ENABLED 0

//******************************************************************************
//*                         download client                                    *
//******************************************************************************

TypeId
DownloadClient::GetTypeId()
{
    static TypeId tid = TypeId("ns3::DownloadClient")
                            .SetParent<Application>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<DownloadClient>()
                            .AddAttribute("MaxOfflineTime",
                                          "Maximum offline time.",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&DownloadClient::m_maxOfflineTime),
                                          MakeUintegerChecker<uint8_t>());
    return tid;
}

DownloadClient::DownloadClient()
{
    NS_LOG_DEBUG("[DownloadClient] create DownloadClient.");
}

DownloadClient::~DownloadClient()
{
}

void
DownloadClient::AddTargetServer(CYBERTWINID_t cuid, uint8_t rate)
{
    m_targetServers.push_back(std::make_pair(cuid, rate));
}

void
DownloadClient::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[DownloadClient] Starting DownloadClient.");
   
    m_endHost = DynamicCast<CybertwinEndHost>(GetNode());
    if (m_endHost == nullptr)
    {
        NS_FATAL_ERROR("Node is not a CybertwinEndHost.");
        return;
    }

     //StartOnOffDownloadStreams();

    StartNaiveDownloadStreams();

    // create log stream
    std::string logFile = "download-client.log";
    std::string logFileName = m_endHost->GetLogDir() + "/" + logFile;
    std::ofstream file(logFileName);
    m_logStream.open(logFileName, std::ios::app);
    m_logStream << "Download client start at " << Simulator::Now().GetSeconds() << "(s)." << std::endl;
}

void
DownloadClient::StartDownloadStreams(uint8_t offlineTime)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[DownloadClient] Starting DownloadClient.");

    if (!m_endHost->isCybertwinCreated())
    {
        NS_LOG_DEBUG("[DownloadClient] Cybertwin not created yet.");
        Simulator::Schedule(Seconds(1.0), &DownloadClient::StartDownloadStreams, this, offlineTime);
        return;
    }

    Ipv4Address cybertwinAddress = m_endHost->GetUpperNodeAddress();
    uint16_t cybertwinPort = m_endHost->GetCybertwinPort();
    NS_LOG_DEBUG("[DownloadClient] Cybertwin created, address: " << cybertwinAddress << ":" << cybertwinPort << " Start download streams.");

    for (auto tar : m_targetServers)
    {
        NS_LOG_DEBUG("[DownloadClient] Create download stream to " << tar.first << ".");
        Ptr<DownloadStream> stream = CreateObject<DownloadStream>();
        stream->SetNode(GetNode());
        stream->SetStreamID(m_streamID++);
        stream->SetTargetID(tar.first);
#if SECURITY_TEST_ENABLED
        stream->SetRate(static_cast<uint8_t>(TrustRateMapping(tar.second)*100));
#else
        stream->SetRate(tar.second);
#endif
        stream->SetCUID(0);
        stream->SetCybertwin(cybertwinAddress, cybertwinPort);
        stream->SetLogDir(m_endHost->GetLogDir());
        stream->SetOfflineTime(offlineTime);
        stream->Activate();

        m_streams.push_back(stream);
    }
}

void
DownloadClient::StartOnOffDownloadStreams()
{

    for (int32_t i=0; i<m_maxOfflineTime; i++)
    {
        Simulator::ScheduleNow(&DownloadClient::StartDownloadStreams, this, i+1);
    }
}

void
DownloadClient::StartNaiveStreams(uint8_t offlineTime, uint8_t streamID)
{
    NS_LOG_DEBUG("[DownloadClient] Create naive download stream " << streamID);
    NaiveStreamInfo_s* stream = m_naiveStreams[streamID];
    // create socket and connect to server
    stream->m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
    stream->m_socket->Bind();
    stream->m_socket->Connect(InetSocketAddress(stream->m_serverAddr, stream->m_serverPort));

    stream->m_startTime = Simulator::Now();

    // set recv callback
    NS_LOG_DEBUG("[DownloadClient] Set recv callback for stream " << stream->streamID);
    m_socketToStream[stream->m_socket] = stream;
    stream->m_socket->SetRecvCallback(MakeCallback(&DownloadClient::NaiveStreamRecvCallback, this));
    stream->m_socket->SetCloseCallbacks(MakeCallback(&DownloadClient::NaiveStreamCloseCallback, this),
                                        MakeNullCallback<void, Ptr<Socket>>());

#if 0
    // schedule offline(stop) events
    for (int32_t i=0; i<offlineTime; i++)
    {
        // schedule stop event
        NS_LOG_DEBUG("[DownloadClient] Schedule stop event for stream " << stream->streamID);
        Simulator::Schedule(MilliSeconds(100*(i+1)), &DownloadClient::NaiveStreamClose, this, streamID);

        // schedule start event
        NS_LOG_DEBUG("[DownloadClient] Schedule start event for stream " << stream->streamID);
        Simulator::Schedule(MilliSeconds(100*(i+1) + 10), &DownloadClient::NaiveStreamReconnect, this, streamID);
    }
#endif
}

void
DownloadClient::NaiveStreamRecvCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    //NS_LOG_DEBUG("[DownloadClient] Naive stream recv callback.");

    NaiveStreamInfo_s* streamInfo = m_socketToStream[socket];
    NS_LOG_DEBUG("[DownloadClient] Naive stream " << streamInfo->streamID << " recv callback. Total Mbytes: " << streamInfo->m_totalBytes / 1024.0 / 1024.0);

    Ptr<Packet> packet;
    while ((packet = socket->Recv()))
    {
        //NS_LOG_INFO("[DownloadClient] " << Simulator::Now() << " Received packet from " << streamInfo->m_targetID << ". Size: " << packet->GetSize() << " bytes.");
        streamInfo->m_totalBytes += packet->GetSize();
        streamInfo->m_realBytes += packet->GetSize();
    }

    if ((streamInfo->m_offlineBytes !=0) && (streamInfo->m_realBytes >= streamInfo->m_offlineBytes))
    {
        NS_LOG_DEBUG("[DownloadClient] Naive stream " << streamInfo->streamID << " close.");
        streamInfo->m_offlineBytes = 0;
        streamInfo->m_realBytes = 0;
        streamInfo->m_socket->Close();
        m_logStream << Simulator::Now().GetSeconds() << " (s) Naive stream [" << streamInfo->streamID << "] close with total download " << streamInfo->m_totalBytes / 1024.0 / 1024.0 << " MB." << std::endl;

        // reconnect
        streamInfo->m_socket->Connect(
            InetSocketAddress(streamInfo->m_serverAddr, streamInfo->m_serverPort));
        m_socketToStream[streamInfo->m_socket] = streamInfo;
        streamInfo->m_socket->SetRecvCallback(
            MakeCallback(&DownloadClient::NaiveStreamRecvCallback, this));
        streamInfo->m_socket->SetCloseCallbacks(
            MakeCallback(&DownloadClient::NaiveStreamCloseCallback, this),
            MakeNullCallback<void, Ptr<Socket>>());
    }

    if ((streamInfo->m_realBytes) >= 10*1024*1024)
    {
        NS_LOG_DEBUG("[DownloadClient] Naive stream  " << streamInfo->streamID << " close permenen.");
        streamInfo->m_socket->Close();
        m_logStream << Simulator::Now().GetSeconds() << " (s) Naive stream [" << streamInfo->streamID << "] close with total download " << streamInfo->m_totalBytes / 1024.0 / 1024.0 << " MB." << std::endl;
    }
}

void
DownloadClient::NaiveStreamCloseCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NaiveStreamInfo_s* streamInfo = m_socketToStream[socket];
    NS_LOG_DEBUG("[DownloadClient] Naive stream " << streamInfo->streamID << " closed.");
    NS_LOG_DEBUG("[DownloadClient] Total download " << streamInfo->m_totalBytes / 1024.0 / 1024.0 << " MB.");
    NS_LOG_DEBUG("[DownloadClient] Total time " << (Simulator::Now() - streamInfo->m_startTime).GetSeconds() << " s.");

    m_logStream << Simulator::Now().GetSeconds() << " (s) Naive stream [" << streamInfo->streamID << "] closed with total download " << streamInfo->m_totalBytes / 1024.0 / 1024.0 << " MB." << std::endl;
}

void
DownloadClient::NaiveStreamClose(uint8_t streamID)
{
    NS_LOG_FUNCTION(this);
    NaiveStreamInfo_s* streamInfo = m_naiveStreams[streamID];
    NS_LOG_DEBUG("[DownloadClient] Naive stream " << streamInfo->streamID << " close.");
    streamInfo->m_socket->Close();
    m_logStream << Simulator::Now().GetSeconds() << " (s) Naive stream [" << streamInfo->streamID << "] close with total download " << streamInfo->m_totalBytes / 1024.0 / 1024.0 << " MB." << std::endl;
}

void
DownloadClient::NaiveStreamReconnect(uint8_t streamID)
{
    NS_LOG_FUNCTION(this);
    NaiveStreamInfo_s* streamInfo = m_naiveStreams[streamID];
    NS_LOG_DEBUG("[DownloadClient] Naive stream " << streamInfo->streamID << " reconnect to " << streamInfo->m_serverAddr << ":" << streamInfo->m_serverPort << ".");
    streamInfo->m_socket->Connect(InetSocketAddress(streamInfo->m_serverAddr, streamInfo->m_serverPort));
}

void
DownloadClient::StartNaiveDownloadStreams()
{
    NS_LOG_DEBUG("[DownloadClient] Start naive download streams.");
    for (int32_t i=0; i<m_maxOfflineTime; i++)
    {
        NaiveStreamInfo_s* streamInfo = new NaiveStreamInfo_s();
        streamInfo->streamID = i;
        streamInfo->m_serverAddr = Ipv4Address("30.0.0.2");
        streamInfo->m_serverPort = 8080;
        streamInfo->m_offlineBytes = i*1024*1024;
        m_naiveStreams.push_back(streamInfo);

        Simulator::ScheduleNow(&DownloadClient::StartNaiveStreams, this, i+1, i);
    }
}

void
DownloadClient::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

//******************************************************************************
//*                         download stream                                    *
//******************************************************************************
TypeId
DownloadStream::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::DownloadStream")
            .SetParent<Object>()
            .SetGroupName("Cybertwin")
            .AddConstructor<DownloadStream>();
    return tid;
}

DownloadStream::DownloadStream()
{
    m_socket = nullptr;
}

DownloadStream::~DownloadStream()
{
}

void
DownloadStream::SetNode(Ptr<Node> node)
{
    m_node = node;
}

void
DownloadStream::SetStreamID(uint32_t streamID)
{
    m_streamID = streamID;
}

void
DownloadStream::SetTargetID(CYBERTWINID_t targetID)
{
    m_targetID = targetID;
}

void
DownloadStream::SetCybertwin(Ipv4Address cybertwinAddress, uint16_t cybertwinPort)
{
    m_cybertwinAddress = cybertwinAddress;
    m_cybertwinPort = cybertwinPort;
}

void
DownloadStream::SetCUID(CYBERTWINID_t cuid)
{
    m_cuid = cuid;
}

void
DownloadStream::SetLogDir(std::string logDir)
{
    m_logDir = logDir;
}

void
DownloadStream::SetRate(uint8_t rate)
{
    m_rate = rate;
}

void
DownloadStream::Activate()
{
    NS_LOG_FUNCTION(this);  
    NS_LOG_DEBUG("[DownloadStream] Activate stream " << m_streamID << " to " << m_targetID);

    // create socket
    if (!m_socket)
    {
        m_socket = Socket::CreateSocket(m_node, TcpSocketFactory::GetTypeId());
        m_socket->Bind();
        m_socket->Connect(InetSocketAddress(m_cybertwinAddress, m_cybertwinPort));
        m_socket->SetConnectCallback(MakeCallback(&DownloadStream::ConnectionSucceeded, this),
                                     MakeCallback(&DownloadStream::ConnectionFailed, this));
        NS_LOG_DEBUG("[DownloadStream] Connecting to " << m_targetID);
    }

    // open log file
    if (!m_logStream.is_open())
    {
        std::string logFile = "download-stream-" + std::to_string(m_streamID) + ".log";
        std::string logFileName = m_logDir + "/" + logFile;
        std::ofstream file(logFileName);
        m_logStream.open(m_logDir + "/" + logFile, std::ios::app);
    }
}

void
DownloadStream::ConnectionSucceeded(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[DownloadStream] Connection succeeded.");
    NS_ASSERT_MSG(m_socket == socket, "Socket mismatch.");

    // set recv callback
    socket->SetRecvCallback(MakeCallback(&DownloadStream::RecvCallback, this));
    socket->SetCloseCallbacks(MakeCallback(&DownloadStream::ConnectionNormalClosed, this),
                              MakeCallback(&DownloadStream::ConnectionErrorClosed, this));

    // schedule offline(stop) events
    if (m_offlineTime > 0)
    {
        for (int32_t i=0; i<m_offlineTime; i++)
        {
            // schedule stop event
            Simulator::Schedule(MilliSeconds(100*(i+1)), &DownloadStream::SendStopRequest, this);

            // schedule start event
            Simulator::Schedule(MilliSeconds(100*(i+1) + 10), &DownloadStream::SendStartRequest, this);
        }
    }

    // send create request
    Simulator::Schedule(TimeStep(100), &DownloadStream::SendCreateRequest, this);
}

void
DownloadStream::RecvCallback(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet;
    while ((packet = socket->Recv()))
    {
        
        NS_LOG_INFO("[DownloadStream] " << Simulator::Now() << " Received packet from " << m_targetID << ". Size: " << packet->GetSize() << " bytes.");
        m_intervalBytes += packet->GetSize();
        m_totalBytes += packet->GetSize();
    }
}

void
DownloadStream::SendCreateRequest()
{
    // send request
    CybertwinHeader header;
    header.SetCommand(CREATE_STREAM);
    header.SetSelfID(m_cuid);
    header.SetPeerID(m_targetID);
    header.SetRecvRate(m_rate);
    header.Print(std::cout);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(header);
    packet->AddPaddingAtEnd(SYSTEM_PACKET_SIZE - header.GetSerializedSize());
    int32_t ret = m_socket->Send(packet);
    if (ret <= 0)
    {
        NS_LOG_ERROR("[DownloadStream] Send request failed.");
        return;
    }
    m_logStream << Simulator::Now().GetSeconds() << "(s) Send create stream request to " << m_targetID << " with rate " << m_rate << std::endl;

    // set statistical event
    m_lastTime = Simulator::Now();
    m_intervalBytes = 0;
    m_totalBytes = 0;
    m_statisticalEvent = Simulator::Schedule(MilliSeconds(10), &DownloadStream::DownloadThroughputStatistical, this);
}

void
DownloadStream::SendStopRequest()
{
    // send stop request
    CybertwinHeader header;
    Ptr<Packet> packet = nullptr;

    header.SetCommand(ENDHOST_STOP_STREAM);
    header.SetSelfID(m_cuid);
    header.SetPeerID(m_targetID);
    header.Print(std::cout);

    packet = Create<Packet>();
    packet->AddHeader(header);
    packet->AddPaddingAtEnd(SYSTEM_PACKET_SIZE - header.GetSerializedSize());

    if (m_socket->Send(packet) <= 0)
    {
        NS_LOG_ERROR("[DownloadStream] Send stop stream request failed.");
        return;
    }

    m_logStream << Simulator::Now().GetSeconds() << "(s) Send stop stream request to " << m_targetID << std::endl;
}

void
DownloadStream::SendStartRequest()
{
    // send start request
    CybertwinHeader header;
    Ptr<Packet> packet = nullptr;

    header.SetCommand(ENDHOST_START_STREAM);
    header.SetSelfID(m_cuid);
    header.SetPeerID(m_targetID);
    header.Print(std::cout);

    packet = Create<Packet>();
    packet->AddHeader(header);
    packet->AddPaddingAtEnd(SYSTEM_PACKET_SIZE - header.GetSerializedSize());

    if (m_socket->Send(packet) <= 0)
    {
        NS_LOG_ERROR("[DownloadStream] Send start stream request failed.");
        return;
    }

    m_logStream << Simulator::Now().GetSeconds() << "(s) Send start stream request to " << m_targetID << std::endl;
    m_logStream << Simulator::Now().GetSeconds() << "(s) Total download " << m_totalBytes / 1024.0 / 1024.0 << " MB." << std::endl;
}

void
DownloadStream::SetOfflineTime(uint8_t offlineTime)
{
    m_offlineTime = offlineTime;
}

void
DownloadStream::ConnectionFailed(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_WARN("[DownloadStream] Connection failed.");
}

void
DownloadStream::ConnectionNormalClosed(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[DownloadStream] Connection closed.");

    if (m_statisticalEvent.IsRunning())
    {
        Simulator::Cancel(m_statisticalEvent);
    }

    if (m_logStream.is_open())
    {
        m_logStream.close();
    }

    if (m_socket)
    {
        m_socket->Close();
        m_socket = nullptr;
    }
}

void
DownloadStream::ConnectionErrorClosed(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_WARN("[DownloadStream] Connection error closed.");
}

void
DownloadStream::DownloadThroughputStatistical()
{
    NS_LOG_FUNCTION(this);
    //NS_LOG_DEBUG("[DownloadStream] Download throughput statistical.");

    Time now = Simulator::Now();
    Time interval = now - m_lastTime;

    double throughput = (double)m_intervalBytes * 8.0 / interval.GetSeconds() / 1024.0 / 1024.0;

    NS_LOG_DEBUG("[DownloadStream][" << m_streamID <<"] " << now.GetSeconds() << " Download throughput " << throughput << " Mbps.");

    m_logStream << now.GetSeconds() << "(s) Download throughput " << throughput << " Mbps." << std::endl;
    m_logStream << now.GetSeconds() << "(s) Total download " << m_totalBytes / 1024.0 / 1024.0 << " MB." << std::endl;

    m_intervalBytes = 0;
    m_lastTime = now;

    m_statisticalEvent = Simulator::Schedule(MilliSeconds(10), &DownloadStream::DownloadThroughputStatistical, this);
}

} // namespace ns3
