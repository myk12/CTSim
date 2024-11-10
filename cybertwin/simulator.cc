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
    std::string topologyFile = "cybertwin/topology.yaml";
    std::string appFiles = "cybertwin/applications.yaml";
    m_topologyReader.SetFileName(topologyFile);
    m_topologyReader.SetAppFiles(appFiles);
}

CybertwinNetworkSimulator::~CybertwinNetworkSimulator()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinNetworkSimulator::InputInit()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[0] Initializing the simulator...");
    // Initialize the simulator
    // create netanim object
}

void
CybertwinNetworkSimulator::DriverCompileTopology()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[1] Reading the topology file...");
    m_nodes = m_topologyReader.Read();

    // populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("[1] Topology file read successfully!");
}

void
CybertwinNetworkSimulator::DriverInstallApps()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[2] Configuring the nodes and applications...");
    m_topologyReader.InstallApplications();

    // Start all installed applications
    NodeContainer coreNodes = m_topologyReader.GetCoreCloudNodes();
    for (uint32_t i = 0; i < coreNodes.GetN(); i++)
    {
        Ptr<CybertwinCoreServer> node = DynamicCast<CybertwinCoreServer>(coreNodes.Get(i));
        node->StartAllAggregatedApps();
    }

    NodeContainer edgeNodes = m_topologyReader.GetEdgeCloudNodes();
    for (uint32_t i = 0; i < edgeNodes.GetN(); i++)
    {
        Ptr<CybertwinEdgeServer> node = DynamicCast<CybertwinEdgeServer>(edgeNodes.Get(i));
        node->StartAllAggregatedApps();
    }

    NodeContainer endNodes = m_topologyReader.GetEndClusterNodes();
    for (uint32_t i = 0; i < endNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(endNodes.Get(i));
        node->StartAllAggregatedApps();
    }

    NS_LOG_INFO("[2] Nodes and applications configured successfully!");
}

void
CybertwinNetworkSimulator::DriverBootSimulator()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[3] Running the simulation...");
    // boot the Cybertwin Network
    // 1. Power on Cloud Nodes
    // 2. Power on Edge Nodes
    // 3. Power on End Hosts

    // power on the nodes
    NodeContainer coreNodes = m_topologyReader.GetCoreCloudNodes();
    for (uint32_t i = 0; i < coreNodes.GetN(); i++)
    {
        Ptr<CybertwinCoreServer> node = DynamicCast<CybertwinCoreServer>(coreNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), CORE_CLOUD_NODE_SIZE, CORE_CLOUD_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 0);
    }

    NodeContainer edgeNodes = m_topologyReader.GetEdgeCloudNodes();
    for (uint32_t i = 0; i < edgeNodes.GetN(); i++)
    {
        Ptr<CybertwinEdgeServer> node = DynamicCast<CybertwinEdgeServer>(edgeNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), EDGE_CLOUD_NODE_SIZE, EDGE_CLOUD_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 1);
    }

    NodeContainer endhostNodes = m_topologyReader.GetEndHostNodes();
    for (uint32_t i = 0; i < endhostNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(endhostNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), END_HOST_NODE_SIZE, END_HOST_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 2);
    }

    NodeContainer apNodes = m_topologyReader.GetApNodes();
    for (uint32_t i = 0; i < apNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(apNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), AP_NODE_SIZE, AP_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 3);
    }

    NodeContainer staNodes = m_topologyReader.GetStaNodes();
    for (uint32_t i = 0; i < staNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(staNodes.Get(i));
        node->PowerOn();
        m_animInterface->UpdateNodeSize(node->GetId(), STA_NODE_SIZE, STA_NODE_SIZE);
        m_animInterface->UpdateNodeImage(node->GetId(), 4);
    }


    NS_LOG_INFO("[3] Simulation completed!");
}

void
CybertwinNetworkSimulator::RunSimulator()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[4] Running the simulation...");
    Simulator::Stop(Seconds(10));
    Simulator::Run();
    NS_LOG_INFO("[4] Simulation completed!");
}

void
CybertwinNetworkSimulator::Output()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[5] Output the simulation results...");
    Simulator::Destroy();
    NS_LOG_INFO("[5] Simulation results outputted successfully!");
}

void
CybertwinNetworkSimulator::SetAnimationInterface(AnimationInterface* animInterface)
{
    NS_LOG_FUNCTION(this);
    m_animInterface = animInterface;

}

}; // namespace ns3

using namespace ns3;

int
main(int argc, char* argv[])
{
    LogComponentEnable("CybertwinNetworkSimulator", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinTopologyReader", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinNode", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinAppDownloadClient", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinAppDownloadServer", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinEndHostDaemon", LOG_LEVEL_DEBUG);
    LogComponentEnable("Cybertwin", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinManager", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinHeader", LOG_LEVEL_INFO);
    LogComponentEnable("NameResolutionService", LOG_LEVEL_INFO);
    NS_LOG_INFO("-*-*-*-*-*-*- Starting Cybertwin Network Simulator -*-*-*-*-*-*-");

    // create the simulator
    Ptr<ns3::CybertwinNetworkSimulator> simulator = CreateObject<ns3::CybertwinNetworkSimulator>();

    // init the simulator
    simulator->InputInit();

    // read the topology file
    simulator->DriverCompileTopology();

    // the animation interface must define after the topology is read
    AnimationInterface anim("cybertwin.xml");
    anim.AddResource("doc/netanim_icon/core_server.png");
    anim.AddResource("doc/netanim_icon/edge_server.png");
    anim.AddResource("doc/netanim_icon/endhost_station.png");
    anim.AddResource("doc/netanim_icon/wireless_ap.png");
    anim.AddResource("doc/netanim_icon/wireless_sta.png");
    simulator->SetAnimationInterface(&anim);

    // boot the simulator
    simulator->DriverBootSimulator();

    // configure the nodes and applications
    simulator->DriverInstallApps();

    // run the simulator
    simulator->RunSimulator();

    // output the simulation results
    simulator->Output();

    return 0;

}; // namespace ns3