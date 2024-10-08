#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/cybertwin-topology-reader.h"
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
    m_topologyReader.SetFileName("./FIA/Cybertwin/topology.yaml");
}

CybertwinNetworkSimulator::~CybertwinNetworkSimulator()
{
    NS_LOG_FUNCTION(this);
}

void CybertwinNetworkSimulator::InputReadTopology()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[1] Reading the topology file...");
    m_nodes = m_topologyReader.Read();
    NS_LOG_INFO("[1] Topology file read successfully!");
}

void CybertwinNetworkSimulator::InputConfigureNodes()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[2] Configuring the nodes and applications...");
    //ApplicationConfigurator appConfigurator;
    //appConfigurator.Configure(m_nodes);
    NS_LOG_INFO("[2] Nodes and applications configured successfully!");
}

void CybertwinNetworkSimulator::Boot()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[3] Running the simulation...");
    //Simulator::Run();
    NS_LOG_INFO("[3] Simulation completed!");
}

void CybertwinNetworkSimulator::Run()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[4] Running the simulation...");
    //Simulator::Run();
    NS_LOG_INFO("[4] Simulation completed!");
}

void CybertwinNetworkSimulator::Output()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("[5] Output the simulation results...");
    NS_LOG_INFO("[5] Simulation results outputted successfully!");
}
    
} // namespace ns3

using namespace ns3;
int main(int argc, char *argv[])
{
    LogComponentEnable("CybertwinNetworkSimulator", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinTopologyReader", LOG_LEVEL_INFO);
    NS_LOG_INFO("-*-*-*-*-*-*- Starting Cybertwin Network Simulator -*-*-*-*-*-*-");

    // create the simulator
    Ptr<ns3::CybertwinNetworkSimulator> simulator = CreateObject<ns3::CybertwinNetworkSimulator>();

    // read the topology file
    simulator->InputReadTopology();

    // configure the nodes and applications
    simulator->InputConfigureNodes();

    // boot the simulator
    simulator->Boot();

    // run the simulator
    simulator->Run();

    // output the simulation results
    simulator->Output();

    return 0;

}; // namespace ns3


