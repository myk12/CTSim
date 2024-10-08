// Default Network Topology
//           
// |`````|----------[10.1.1.0]-------------|``````|
// | n0  |----------[10.1.2.0]-------------| n1   |         n2         n3   
// |_____|----------[10.1.3.0]-------------|______|         |          | 
//                                         ==============================
//            3 point-to-point links                LAN 10.1.4.0
//          
//         
#include "cybertwin-multipath-datatransfer.h"

#define CYBERTWIN_MULTIPATH_PORT (45670)

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CybertwinMultipathDataTransferExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("CybertwinMultipathDataTransferExample", LOG_LEVEL_DEBUG);
    //LogComponentEnable("CybertwinMultipathTransfer", LOG_LEVEL_INFO);
    LogComponentEnable("NameResolutionService", LOG_LEVEL_DEBUG);
    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_DEBUG);
    //LogComponentEnable("Packet", LOG_LEVEL_DEBUG);
    //LogComponentEnable("PacketTagList", LOG_LEVEL_DEBUG);
    //LogComponentEnable("PacketTagList", LOG_LEVEL_FUNCTION);
    //--------------------------------------------------------------------------
    //-                         build topology                                 -
    //--------------------------------------------------------------------------
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_UNCOND("*                          Building topology                            *");
    NS_LOG_UNCOND("*************************************************************************");
    NodeContainer p2pNodes, lanNodes;
    NetDeviceContainer p2pDevs, csmaDevs;

    p2pNodes.Create(1);
    lanNodes.Create(3);
    p2pNodes.Add(lanNodes.Get(0));

    PointToPointHelper p2pHelper;
    CsmaHelper csmaHelper;
    InternetStackHelper stackHelper;
    Ipv4AddressHelper addrHelper;
    Ipv4InterfaceContainer ipNode0;
    Ipv4InterfaceContainer ipNode1;

    stackHelper.Install(p2pNodes.Get(0));
    stackHelper.Install(lanNodes);

    p2pHelper.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4Address ipbase[3] = {
        "10.1.1.0",
        "10.1.2.0",
        "10.1.3.0"
    };
    for (int i=0; i<3; i++)
    {
        Ipv4InterfaceContainer ipContainer;
        addrHelper.SetBase(ipbase[i], "255.255.255.0");
        p2pDevs = p2pHelper.Install(p2pNodes);
        ipContainer = addrHelper.Assign(p2pDevs);
        //NS_LOG_DEBUG("ipbase" <<ipbase[i]<<" interface: "<<ipContainer.GetAddress(0)<<" "<<ipContainer.GetAddress(1));
        ipNode0.Add(ipContainer.Get(0));
        ipNode1.Add(ipContainer.Get(1));
    }

    csmaHelper.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    csmaDevs = csmaHelper.Install(lanNodes);
    addrHelper.SetBase("10.1.4.0", "255.255.255.0");
    addrHelper.Assign(csmaDevs);
    NS_LOG_DEBUG("\n\n\n");
    NS_LOG_DEBUG("|`````|----------[10.1.1.0]-------------|``````|");
    NS_LOG_DEBUG("| n0  |----------[10.1.2.0]-------------| n1   |         n2         n3   ");
    NS_LOG_DEBUG("|_____|----------[10.1.3.0]-------------|______|         |          | ");
    NS_LOG_DEBUG("                                         ==============================");
    NS_LOG_DEBUG("            3 point-to-point links                LAN 10.1.4.0");
    NS_LOG_DEBUG("\n\n\n");

    //----------------------------------------------------------------------
    //-                     run simulation                                 -
    //----------------------------------------------------------------------
    Ptr<Node> client = p2pNodes.Get(0);
    Ptr<Node> server = p2pNodes.Get(1);
    MultipathDataTransferApp appClient, appServer;
    NameResolutionService nameServiceClient, nameServiceServer;

    // set application
    nameServiceClient.SetSuperior(ipNode1.GetAddress(0));
    nameServiceServer.SetSuperior(ipNode0.GetAddress(1));

    // set app property
    appClient.SetRole(true);
    appServer.SetRole(false);

    // cybertwin interfaces
    CYBERTWIN_INTERFACE_LIST_t clientIfs, serverIfs;
    CYBERTWINID_t clientID, serverID;
    NS_LOG_DEBUG(">> Connecting with point-to-pint link...");
    for (uint32_t i=0; i<ipNode0.GetN(); i++)
    {
        CYBERTWIN_INTERFACE_t ift_client, ift_server;
        ift_client.first = ipNode0.GetAddress(i);
        ift_client.second = CYBERTWIN_MULTIPATH_PORT;
        ift_server.first = ipNode1.GetAddress(i);
        ift_server.second = CYBERTWIN_MULTIPATH_PORT;

        clientIfs.push_back(ift_client);
        serverIfs.push_back(ift_server);
        NS_LOG_DEBUG(" INTERFACE_" << i<< " client: "<<ift_client.first<<" server: "<<ift_server.first);
    }

    // cybertwin id
    Ptr<UniformRandomVariable> randv = CreateObject<UniformRandomVariable>();
    clientID = (CYBERTWINID_t)(randv->GetValue()*1000);
    serverID = (CYBERTWINID_t)(randv->GetValue()*10000);
    NS_LOG_DEBUG("\n>> Generating Cybertwin ID ...\n ClientID :"<<clientID<<"\n ServerID :"<<serverID);

    // set cybertwin related information
    appClient.SetLocalInterface(clientID, clientIfs);
    appClient.SetRemoteInterface(serverID, serverIfs);
    appServer.SetLocalInterface(serverID, serverIfs);
    appServer.SetRemoteInterface(clientID, clientIfs);

    // set app start time
    appClient.SetStartTime(Seconds(2));
    appServer.SetStartTime(Seconds(1));
    nameServiceClient.SetStartTime(Seconds(1));
    nameServiceServer.SetStartTime(Seconds(1));

    // aggrate app to node
    client->AddApplication(&appClient);
    server->AddApplication(&appServer);
    client->AddApplication(&nameServiceClient);
    server->AddApplication(&nameServiceServer);

    NS_LOG_DEBUG("\n\n\n");
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_UNCOND("*                          Run simulation                               *");
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_DEBUG("\n\n\n");
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

TypeId
MultipathDataTransferApp::GetTypeId()
{
    static TypeId tid = TypeId("MultipathDataTransferApp")
                        .SetParent<Application>();

    return tid;
}

void
MultipathDataTransferApp::StartApplication()
{
    if (is_client)
    {
        NS_LOG_DEBUG(">> Start Client.");
        test_client();
    }else
    {
        NS_LOG_DEBUG(">> Start Server.");
        test_server();
    }

}

void
MultipathDataTransferApp::StopApplication()
{
    if (m_dataServer)
    {
        delete m_dataServer;
    }
}

void
MultipathDataTransferApp::SetRole(bool client)
{
    is_client = client;
}

void
MultipathDataTransferApp::RecvHandler(MultipathConnection* conn)
{
    Ptr<Packet> pack = nullptr;
    while ((pack = conn->Recv()))
    {
        NS_LOG_UNCOND("Recv packet.");
    }
}

void 
MultipathDataTransferApp::ConnectSucceedHandler(MultipathConnection* conn)
{
    NS_LOG_UNCOND("Build connection succeed! ID: " << conn->GetConnID());
}

void
MultipathDataTransferApp::ClientConnectSucceedHandler(MultipathConnection* conn)
{
    NS_LOG_UNCOND("------ Client Build connection succeed! ----------");
    NS_LOG_UNCOND(" ID: " << conn->GetConnID());
    NS_LOG_UNCOND("--------------------------------------------------");
    int sec = 2;
    Simulator::Schedule(Seconds(sec++), &MultipathDataTransferApp::PeriodicSender, this, conn);
    Simulator::Schedule(Seconds(sec++), &MultipathDataTransferApp::PeriodicSender, this, conn);
    Simulator::Schedule(Seconds(sec++), &MultipathDataTransferApp::PeriodicSender, this, conn);
    Simulator::Schedule(Seconds(sec++), &MultipathDataTransferApp::PeriodicSender, this, conn);
    Simulator::Schedule(Seconds(sec++), &MultipathDataTransferApp::PeriodicSender, this, conn);
}

void
MultipathDataTransferApp::PeriodicSender(MultipathConnection* conn)
{
    Ptr<Packet> pkt = Create<Packet>(100);
    conn->Send(pkt);
}

void
MultipathDataTransferApp::ServerConnectSucceedHandler(MultipathConnection* conn)
{
    NS_LOG_UNCOND("------ Server Build connection succeed! ----------");
    NS_LOG_UNCOND(" ID: " << conn->GetConnID());
    NS_LOG_UNCOND("--------------------------------------------------");
    //conn->Send(Create<Packet>(100));
    conn->SetRecvCallback(MakeCallback(&MultipathDataTransferApp::RecvHandler, this));
}

void
MultipathDataTransferApp::ConnectFailedHandler(MultipathConnection* conn)
{
    NS_LOG_UNCOND("Failed to connect...");
}

void
MultipathDataTransferApp::ConnectCloseHandler(MultipathConnection* conn)
{
    NS_LOG_UNCOND("Connection closed...");
    delete  conn;
}

void
MultipathDataTransferApp::test_client()
{
    MultipathConnection* conn_client;
    conn_client = new MultipathConnection();
    conn_client->Setup(GetNode(), m_localCyberID, m_localIfs);
    conn_client->Connect(m_remoteCyberID);

    conn_client->SetConnectCallback(MakeCallback(&MultipathDataTransferApp::ClientConnectSucceedHandler, this),
                                    MakeCallback(&MultipathDataTransferApp::ConnectFailedHandler, this));
    conn_client->SetRecvCallback(MakeCallback(&MultipathDataTransferApp::RecvHandler, this));
}

void
MultipathDataTransferApp::test_server()
{
    m_dataServer = new CybertwinDataTransferServer();
    m_dataServer->Setup(GetNode(), m_localCyberID, m_localIfs);
    m_dataServer->Listen();

    m_dataServer->SetNewConnectCreatedCallback(MakeCallback(&MultipathDataTransferApp::ServerConnectSucceedHandler, this));
}

void
MultipathDataTransferApp::SetRemoteInterface(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t ifs)
{
    m_remoteCyberID = id;
    m_remoteIfs = ifs;
}

void
MultipathDataTransferApp::SetLocalInterface(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t ifs)
{
    m_localCyberID = id;
    m_localIfs = ifs;
}