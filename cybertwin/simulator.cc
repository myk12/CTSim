#include "simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinNetworkSimulator");

NS_OBJECT_ENSURE_REGISTERED(CybertwinNetworkSimulator);

TypeId
CybertwinNetworkSimulator::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinNetworkSimulator")
                            .SetParent<Object>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinNetworkSimulator>();
    return tid;
}

CybertwinNetworkSimulator::CybertwinNetworkSimulator()
{
    NS_LOG_FUNCTION(this);
}

CybertwinNetworkSimulator::~CybertwinNetworkSimulator()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinNetworkSimulator::InputInit()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[INPUT] Initializing the simulator...");
    // Initialize the simulator
    // create netanim object
    std::string topologyFile = "cybertwin/topology.yaml";
    std::string appFiles = "cybertwin/applications.yaml";
    m_topologyReader = CreateObject<CybertwinTopologyReader>();
    m_topologyReader->SetFileName(topologyFile);
    m_topologyReader->SetAppFiles(appFiles);
}

void
CybertwinNetworkSimulator::DriverCompile()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[DRIVER][COMPILE] Reading the topology file...");
    m_nodes = m_topologyReader->Read();

    // populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void
CybertwinNetworkSimulator::DriverBoot()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[DRIVER][BOOT] Booting the Cybertwin Network...");
    // boot the Cybertwin Network
    // 1. Power on Cloud Nodes
    // 2. Power on Edge Nodes
    // 3. Power on End Hosts

    // power on the nodes
    NodeContainer coreNodes = m_topologyReader->GetCoreCloudNodes();
    for (uint32_t i = 0; i < coreNodes.GetN(); i++)
    {
        Ptr<CybertwinCoreServer> node = DynamicCast<CybertwinCoreServer>(coreNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), CORE_CLOUD_NODE_SIZE, CORE_CLOUD_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 0);
    }

    NodeContainer edgeNodes = m_topologyReader->GetEdgeCloudNodes();
    for (uint32_t i = 0; i < edgeNodes.GetN(); i++)
    {
        Ptr<CybertwinEdgeServer> node = DynamicCast<CybertwinEdgeServer>(edgeNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), EDGE_CLOUD_NODE_SIZE, EDGE_CLOUD_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 1);
    }

    NodeContainer endhostNodes = m_topologyReader->GetEndHostNodes();
    for (uint32_t i = 0; i < endhostNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(endhostNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), END_HOST_NODE_SIZE, END_HOST_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 2);
    }

    NodeContainer apNodes = m_topologyReader->GetApNodes();
    for (uint32_t i = 0; i < apNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(apNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), AP_NODE_SIZE, AP_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 3);
    }

    NodeContainer staNodes = m_topologyReader->GetStaNodes();
    for (uint32_t i = 0; i < staNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(staNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), STA_NODE_SIZE, STA_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 4);
    }
}

void
CybertwinNetworkSimulator::DriverInstallApps()
{
    NS_LOG_INFO("[DRIVER][INSTALL] Installing applications...");
    m_topologyReader->InstallApplications();

    // Start all installed applications
    NodeContainer coreNodes = m_topologyReader->GetCoreCloudNodes();
    for (uint32_t i = 0; i < coreNodes.GetN(); i++)
    {
        Ptr<CybertwinCoreServer> node = DynamicCast<CybertwinCoreServer>(coreNodes.Get(i));
        node->StartAllAggregatedApps();
    }

    NodeContainer edgeNodes = m_topologyReader->GetEdgeCloudNodes();
    for (uint32_t i = 0; i < edgeNodes.GetN(); i++)
    {
        Ptr<CybertwinEdgeServer> node = DynamicCast<CybertwinEdgeServer>(edgeNodes.Get(i));
        node->StartAllAggregatedApps();
    }

    NodeContainer endNodes = m_topologyReader->GetEndClusterNodes();
    for (uint32_t i = 0; i < endNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(endNodes.Get(i));
        node->StartAllAggregatedApps();
    }
}

void
CybertwinNetworkSimulator::RunSimulator()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[CORE][RUN] Running the Cybertwin Network Simulator...");
    Simulator::Stop(Seconds(10));
    Simulator::Run();
}

void
CybertwinNetworkSimulator::Output()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[OUTPUT] Outputting the simulation results to cybertwin.xml...");
    Simulator::Destroy();
}

void
CybertwinNetworkSimulator::AnimationInit()
{
    NS_LOG_FUNCTION(this);
    m_animInterface = new AnimationInterface("cybertwin.xml");
    m_animInterface->AddResource("doc/netanim_icon/core_server.png");
    m_animInterface->AddResource("doc/netanim_icon/edge_server.png");
    m_animInterface->AddResource("doc/netanim_icon/endhost_station.png");
    m_animInterface->AddResource("doc/netanim_icon/wireless_ap.png");
    m_animInterface->AddResource("doc/netanim_icon/wireless_sta.png");
}

}; // namespace ns3

using namespace ns3;

int
main(int argc, char* argv[])
{
    LogComponentEnable("CybertwinNetworkSimulator", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinTopologyReader", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinNode", LOG_LEVEL_DEBUG);
    //LogComponentEnable("CybertwinAppDownloadClient", LOG_LEVEL_WARN);
    //LogComponentEnable("CybertwinAppDownloadServer", LOG_LEVEL_WARN);
    LogComponentEnable("CybertwinEndHostDaemon", LOG_LEVEL_DEBUG);
    LogComponentEnable("Cybertwin", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinManager", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinHeader", LOG_LEVEL_INFO);
    LogComponentEnable("NameResolutionService", LOG_LEVEL_DEBUG);
    NS_LOG_INFO("-*-*-*-*-*-*- Starting Cybertwin Network Simulator -*-*-*-*-*-*-");

    // create the simulator
    Ptr<ns3::CybertwinNetworkSimulator> simulator = CreateObject<ns3::CybertwinNetworkSimulator>();

    // init the simulator
    simulator->InputInit();

    // read the topology file
    simulator->DriverCompile();

    // the animation interface must define after the topology is read
    simulator->AnimationInit();

    // boot the simulator
    simulator->DriverBoot();

    // configure the nodes and applications
    simulator->DriverInstallApps();

    // run the simulator
    simulator->RunSimulator();

    // output the simulation results
    simulator->Output();

    return 0;

}; // namespace ns3
