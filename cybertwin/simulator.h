#ifndef CYBERTWIN_SIMULATOR_H
#define CYBERTWIN_SIMULATOR_H

#include "ns3/core-module.h"
#include "ns3/cybertwin-topology-reader.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/node-container.h"
#include "ns3/simulator.h"

#define CORE_CLOUD_NODE_SIZE (7)
#define EDGE_CLOUD_NODE_SIZE (5)
#define END_HOST_NODE_SIZE (5)
#define AP_NODE_SIZE (5)
#define STA_NODE_SIZE (5)

namespace ns3
{
class CybertwinNetworkSimulator : public Object
{
  public:
    static TypeId GetTypeId();

    CybertwinNetworkSimulator();
    ~CybertwinNetworkSimulator() override;

    CybertwinNetworkSimulator(const CybertwinNetworkSimulator&) = delete;
    CybertwinNetworkSimulator& operator=(const CybertwinNetworkSimulator&) = delete;

    void InputInit();
    void DriverCompile();
    void AnimationInit();
    void DriverInstallApps();
    void DriverBoot();
    void RunSimulator();
    void Output();

  private:
    NodeContainer m_nodes;
    Ptr<CybertwinTopologyReader> m_topologyReader;
    AnimationInterface* m_animInterface;
};

}; // namespace ns3

#endif /* CYBERTWIN_SIMULATOR_H */