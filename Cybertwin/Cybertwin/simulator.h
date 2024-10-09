#ifndef CYBERTWIN_SIMULATOR_H
#define CYBERTWIN_SIMULATOR_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/cybertwin-topology-reader.h"

namespace ns3
{
class CybertwinNetworkSimulator : public Object
{
public:
    static TypeId GetTypeId();

    CybertwinNetworkSimulator();
    ~CybertwinNetworkSimulator() override;

    CybertwinNetworkSimulator(const CybertwinNetworkSimulator &) = delete;
    CybertwinNetworkSimulator &operator=(const CybertwinNetworkSimulator &) = delete;

    void InputReadTopology();
    void InputConfigureNodes();
    void Boot();
    void Run();
    void Output();

private:
    NodeContainer m_nodes;
    CybertwinTopologyReader m_topologyReader;
};

}; // namespace ns3

#endif /* CYBERTWIN_SIMULATOR_H */