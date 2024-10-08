/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <iostream>
#include <string>

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1      n2     n3
//    point-to-point  |      |      |
//                    ===============
//                     LAN 10.1.2.0

#define RECEIVER_ID 7777777
#define END_SWITCH_INTERVAL_MS 100
#define PACKET_SIZE (128)
#define SEND_RATE (1*(1024)*(1024))   // 1Mbps
#define SYSTEM_MAX_RUNTIME 7 // 10s

#define TEST_UDP_PORT 10000

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MobilityTest");

std::string outpath = ".";
uint32_t random_seed = 17;

//******************************************************************************
//*                                 SenderApp
//******************************************************************************

class SenderApp : public Application
{
  public:
    static TypeId GetTypeId();
    SenderApp();
    ~SenderApp() override;

    void SendPacket()
    {
        // send packet at defined rate,
        // rate defined by SEND_RATE
        // using random time interval
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("[SenderApp::SendPacket] send packet to " << target_addr << ":" << target_port);
        Ptr<Packet> packet = Create<Packet>(PACKET_SIZE);
        m_socket->Send(packet);
        // schedule next packet
        double interval = m_exponentialRandomVariable->GetValue();
        m_sendEvent = Simulator::Schedule(Seconds(interval), &SenderApp::SendPacket, this);

        // log packet
        std::ofstream logFile;
        logFile.open(outpath + "sender.log", std::ios::app);
        // format: time, packet_size
        logFile << Simulator::Now().GetSeconds() << "," << packet->GetSize() << std::endl;
    }

    void SetTargetName(uint64_t name)
    {
        target_name = name;
    }

    void SetTargetAddress(Ipv4Address addr)
    {
        target_addr = addr;
    }

    void SetTargetPort(uint16_t port)
    {
        target_port = port;
    }

  private:
    void StartApplication() override;
    void StopApplication() override;

    void DoDispose() override
    {
        NS_LOG_FUNCTION(this);
        m_socket = nullptr;
        Application::DoDispose();
    }

    Ptr<Socket> m_socket;
    uint64_t target_name;
    Ipv4Address target_addr;
    uint16_t target_port;

    // event id
    EventId m_sendEvent;

    // random variable
    Ptr<ExponentialRandomVariable> m_exponentialRandomVariable;
};

TypeId
SenderApp::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SenderApp").SetParent<Application>().SetGroupName("Applications");
    return tid;
}

SenderApp::SenderApp()
{
    NS_LOG_FUNCTION(this);
    // create a exponential random variable
    m_exponentialRandomVariable = CreateObject<ExponentialRandomVariable>();

    // packet size / rate = interval between packet
    double interval = PACKET_SIZE * 8.0 / SEND_RATE;
    m_exponentialRandomVariable->SetAttribute("Mean", DoubleValue(interval));

    // get current time as the random seed
    for (uint32_t i=0; i<random_seed; i++) {
        NS_LOG_DEBUG("[SenderApp::SenderApp] interval: " << m_exponentialRandomVariable->GetValue());
    }
}

SenderApp::~SenderApp()
{
    NS_LOG_FUNCTION(this);
}

void
SenderApp::StartApplication()
{
    NS_LOG_DEBUG("[SenderApp::SenderApp] start application");
    NS_LOG_FUNCTION(this);
    // send UDP packet
    if (!m_socket)
    {
        NS_LOG_DEBUG("[SenderApp::SenderApp] create socket");
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
    }

    // bind to any port
    NS_LOG_DEBUG("[SenderApp::SenderApp] bind to any port");
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 0);
    if (m_socket->Bind(local) == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket");
    }

    // set target address and port
    InetSocketAddress remote = InetSocketAddress(target_addr, target_port);
    if (m_socket->Connect(remote) == -1)
    {
        NS_FATAL_ERROR("Failed to connect socket");
    }
    NS_LOG_DEBUG("[SenderApp::SenderApp] connect to " << target_addr << ":" << target_port);

    SendPacket();
}

void
SenderApp::StopApplication()
{
    NS_LOG_INFO("SenderApp::StopApplication");
    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }

    m_socket = nullptr;

    NS_LOG_INFO("SenderApp::StopApplication DONE");
}

//******************************************************************************
//*                                 ReceiverApp
//******************************************************************************

class ReceiverApp : public Application
{
  public:
    static TypeId GetTypeId();
    ReceiverApp();
    ~ReceiverApp() override;

    void HandleRead(Ptr<Socket> socket);

    void Online()
    {
        m_isOnline = true;
#ifdef FISIM_NAME_FIRST_ROUTING
        // declare name and address
        Ipv4GlobalRouting::m_name2addr[1234567] = m_nodeAddr;
        NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                      << "] announce name and address");
#endif
    }

    void Offline()
    {
        m_isOnline = false;
    }

    void CloseSocket()
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                      << "] close socket");
        // shutdown receive before close socket
        m_socket->ShutdownRecv();
        if (m_socket)
        {
            NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                          << "] really close socket");
            int ret = m_socket->Close();
            if (ret == -1)
            {
                NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                              << "] close socket failed");
            }
            NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                          << "] close socket done");
        }
    }

    void SetPort(uint16_t port)
    {
        m_port = port;
    }

    void SetRecvId(uint32_t id)
    {
        m_recvId = id;
    }

    void SetTestApp(Ptr<Application> app)
    {
        m_testApp = app;
    }

    Ptr<Application> GetTestApp()
    {
        return m_testApp;
    }

  private:
    void StartApplication() override;
    void StopApplication() override;

    void DoDispose() override
    {
        NS_LOG_FUNCTION(this);
        m_socket = nullptr;
        Application::DoDispose();
    }

    Ptr<Socket> m_socket;
    uint16_t m_port;
    uint32_t m_recvId;

    uint32_t m_nPackets;

    Ptr<Application> m_testApp;
    uint32_t m_nodeId;
    Ipv4Address m_nodeAddr;

    std::string m_logFileName;
    bool m_isOnline;
};

TypeId
ReceiverApp::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ReceiverApp").SetParent<Application>().SetGroupName("Applications");
    return tid;
}

ReceiverApp::ReceiverApp()
{
    NS_LOG_FUNCTION(this);
}

ReceiverApp::~ReceiverApp()
{
    NS_LOG_FUNCTION(this);
}

void
ReceiverApp::StartApplication()
{
    NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s]"
                                  << " ReceiverApp::StartApplication");
    // get node ID and address
    m_nodeId = GetNode()->GetId();
    m_nodeAddr =
        Ipv4Address::ConvertFrom(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
    NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s]"
                                  << " node ID: " << m_nodeId << " node address: " << m_nodeAddr);

    // create log file and write header
    m_logFileName = outpath + "receiver_" + std::to_string(m_nodeId) + ".log";
    std::ofstream logFile;
    logFile.open(m_logFileName, std::ios::trunc);
    logFile << "n_packets,node_id,node_addr,time,packet_size" << std::endl;

    // lestin to UDP packet
    if (!m_socket)
    {
        NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                      << "] create socket");
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
    }

    // bind to port
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), TEST_UDP_PORT);
    if (m_socket->Bind(local) == -1)
    {
        // get error message
        uint32_t errorno = m_socket->GetErrno();
        NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                      << "] errorno: " << errorno);
        NS_FATAL_ERROR("Failed to bind socket");
    }

    // set recv callback
    m_nPackets = 0;
    m_socket->SetRecvCallback(MakeCallback(&ReceiverApp::HandleRead, this));
}

void
ReceiverApp::StopApplication()
{
    NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                  << "] ReceiverApp::StopApplication");

    if (m_socket)
    {
        NS_LOG_DEBUG("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                      << "] close socket");
        m_socket->Close();
    }

    m_socket = nullptr;
}

void
ReceiverApp::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_INFO("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                 << "] ReceiverApp::HandleRead");
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        NS_LOG_INFO("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                     << "] received packet from "
                                     << InetSocketAddress::ConvertFrom(from).GetIpv4() << ":"
                                     << InetSocketAddress::ConvertFrom(from).GetPort());
        if (packet->GetSize() > 0)
        {
            m_nPackets++;
            NS_LOG_INFO("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                         << "] received packet size: " << packet->GetSize());
            // log packet
            if (m_isOnline)
            {
                // log to file
                NS_LOG_INFO("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                             << "] online");
                std::ofstream logFile;
                logFile.open(m_logFileName, std::ios::app);
                // format: n_packets, node_id, node_addr, time, packet_size
                logFile << m_nPackets << "," << m_nodeId << "," << m_nodeAddr << ","
                        << Simulator::Now().GetSeconds() << "," << packet->GetSize() << std::endl;
                logFile.close();
            }
            else
            {
                // Drop packet
                NS_LOG_INFO("[ReceiverApp][" << Simulator::Now().GetSeconds() << "s][" << m_nodeId
                                             << "] not online");
            }
        }
    }
}

void
EndTransmission(Ptr<Node> fromNode, Ptr<Node> toNode)
{
    if (fromNode != nullptr)
    {
        NS_LOG_DEBUG("[EndTransmission][" << Simulator::Now().GetSeconds() << "s]"
                                          << " end transmission fromNode: " << fromNode->GetId());
        // find and stop old app
        Ptr<Application> app = nullptr;
        for (uint32_t i = 0; i < fromNode->GetNApplications(); i++)
        {
            app = fromNode->GetApplication(i);
            if (app->GetInstanceTypeId() == TypeId::LookupByName("ns3::ReceiverApp"))
            {
                NS_LOG_INFO("[EndTransmission][" << Simulator::Now().GetSeconds() << "s]"
                                                 << " found old app");
                break;
            }
        }
        if (app)
        {
            NS_LOG_DEBUG("[EndTransmission][" << Simulator::Now().GetSeconds() << "s]"
                                              << " stop old app");
            // immediately stop old app
            DynamicCast<ReceiverApp>(app)->Offline();
        }
    }

    if (toNode != nullptr)
    {
        NS_LOG_DEBUG("[EndTransmission][" << Simulator::Now().GetSeconds() << "s]"
                                          << " start new transmission toNode: " << toNode->GetId());

        // find and start new app
        Ptr<Application> app = nullptr;
        for (uint32_t i = 0; i < toNode->GetNApplications(); i++)
        {
            app = toNode->GetApplication(i);
            if (app->GetInstanceTypeId() == TypeId::LookupByName("ns3::ReceiverApp"))
            {
                NS_LOG_INFO("[EndTransmission][" << Simulator::Now().GetSeconds() << "s]"
                                                 << " found new app");
                break;
            }
        }
        if (app)
        {
            NS_LOG_DEBUG("[EndTransmission][" << Simulator::Now().GetSeconds() << "s]"
                                              << " start new app");
            // delay 100ms to start new app
            Simulator::Schedule(MilliSeconds(END_SWITCH_INTERVAL_MS),
                                &ReceiverApp::Online,
                                DynamicCast<ReceiverApp>(app));
        }
    }

    NS_LOG_UNCOND("[EndTransmission] DONE");
}

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("\n======= ======= ======= MobilityTest ======= ======= =======\n");
    CommandLine cmd(__FILE__);
    cmd.AddValue("outpath", "Path to output files", outpath);
    cmd.AddValue("random_seed", "Random seed", random_seed);
    cmd.Parse(argc, argv);

    NS_LOG_UNCOND("Output path: " << outpath);
    NS_LOG_UNCOND("Random seed: " << random_seed);

    // set random seed
    //int seed = time(NULL) % 177;
    //RngSeedManager::SetSeed(seed);
    //RngSeedManager::SetRun(seed);

    uint32_t nCsma = 2;
    // LogComponentEnable("Ipv4Header", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("TcpL4Protocol", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("UdpL4Protocol", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("Ipv4GlobalRouting", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("UdpSocketImpl", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("UdpL4Protocol", LOG_LEVEL_FUNCTION);
    // LogComponentEnable("Ipv4GlobalRouting", LOG_LEVEL_INFO);
    LogComponentEnable("MobilityTest", LOG_LEVEL_DEBUG);

    NS_LOG_UNCOND("\n******* [1] Constructing Topology *******\n");
    NS_LOG_UNCOND("- Creating nodes...");
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));
    csmaNodes.Create(nCsma);

    NS_LOG_UNCOND("- Creating channels...");
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    NS_LOG_UNCOND("- Installing internet stack...");
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));
    stack.Install(csmaNodes);

    NS_LOG_UNCOND("- Assigning IP addresses...");
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    // install applications: sender and receiver

    Ptr<Node> sender = p2pNodes.Get(0);
    Ptr<Node> receiver1 = csmaNodes.Get(1);
    Ptr<Node> receiver2 = csmaNodes.Get(2);

    // install sender app
    NS_LOG_UNCOND("\n******* [2] Installing SenderApp *******\n");
    NS_LOG_UNCOND("- Creating SenderApp...");
    Ptr<SenderApp> senderApp = CreateObject<SenderApp>();
    // start sender appliaction
    senderApp->SetTargetAddress(csmaInterfaces.GetAddress(1));
    senderApp->SetTargetPort(TEST_UDP_PORT);
    senderApp->SetStartTime(Seconds(0.0));
    senderApp->SetStopTime(Seconds(SYSTEM_MAX_RUNTIME));

    // install sender app
    NS_LOG_UNCOND("- Installing SenderApp...");
    sender->AddApplication(senderApp);

    // install receiver app
    Ptr<ReceiverApp> receiverApp1 = CreateObject<ReceiverApp>();
    receiverApp1->SetPort(TEST_UDP_PORT);
    receiverApp1->SetRecvId(1);
    receiverApp1->SetStartTime(Seconds(0.0));
    receiverApp1->SetStopTime(Seconds(SYSTEM_MAX_RUNTIME));
    receiver1->AddApplication(receiverApp1);

    Ptr<ReceiverApp> receiverApp2 = CreateObject<ReceiverApp>();
    receiverApp2->SetPort(TEST_UDP_PORT);
    receiverApp2->SetRecvId(2);
    receiverApp2->SetStartTime(Seconds(0.0));
    receiverApp2->SetStopTime(Seconds(SYSTEM_MAX_RUNTIME));
    receiver2->AddApplication(receiverApp2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // schedule app transmission
    NS_LOG_UNCOND("\n******* [3] Scheduling receiver transmission *******\n");
    Simulator::Schedule(Seconds(0.0), &EndTransmission, nullptr, receiver1);
    Simulator::Schedule(Seconds(1), &EndTransmission, receiver1, receiver2);
    Simulator::Schedule(Seconds(2), &EndTransmission, receiver2, receiver1);
    Simulator::Schedule(Seconds(3), &EndTransmission, receiver1, receiver2);
    Simulator::Schedule(Seconds(4), &EndTransmission, receiver2, receiver1);
    Simulator::Schedule(Seconds(5), &EndTransmission, receiver1, nullptr);

    // Simulator::Schedule(Seconds(3), &EndTransmission, receiver1, nullptr);

    // pointToPoint.EnablePcapAll("second");

    NS_LOG_UNCOND("\n******* [4] Running Simulation *******\n");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
