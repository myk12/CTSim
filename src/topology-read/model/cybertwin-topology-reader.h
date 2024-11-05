#ifndef CYBERTWIN_TOPOLOGY_READER_H
#define CYBERTWIN_TOPOLOGY_READER_H

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/topology-read-module.h"
#include "ns3/netanim-module.h"

#include "ns3/cybertwin-manager.h"
#include "ns3/cybertwin-node.h"

#include "ns3/cybertwin-app-helper.h"

// using yaml-cpp
#include "yaml-cpp/yaml.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace ns3
{

//----------------------------------------------------------
//          Node Information
//----------------------------------------------------------
// define node type
typedef enum NodeType
{
    HOST_SERVER,
    END_CLUSTER,
} NodeType_e;

typedef struct Link
{
    std::string target;
    std::string data_rate;
    std::string delay;
    std::string network;
} Link_t;

typedef struct Gateway
{
    std::string name;
    std::string data_rate;
    std::string delay;
    std::string network;
} Gateway_t;

// define node info
typedef struct NodeInfo
{
    Ptr<Node> node;     // main node pointer
    std::string name;   // main node name
    Vector position;    // position in NetAnim
    NodeType_e type;    // indicating whether the node is a host server or an end cluster
    std::vector<Link_t> links;  // p2p links

    // for CSMA or Wifi network
    NodeContainer nodes;    // cluster nodes
    int32_t num_nodes;      // number of nodes in the cluster
    std::string network_type;   // indicating the network type: csma or wifi
    std::vector<Gateway_t> gateways;    // gateways
    std::string local_network;      // end cluster local area network
} NodeInfo_t;

//----------------------------------------------------------
//         Application Information
//----------------------------------------------------------
typedef struct ApplicationInfo
{
    std::string appName;
    std::vector<std::string> targetNodes;
} ApplicationInfo_t;

class CybertwinTopologyReader : public TopologyReader
{
  public:
    static TypeId GetTypeId();

    CybertwinTopologyReader();
    ~CybertwinTopologyReader() override;

    CybertwinTopologyReader(const CybertwinTopologyReader &) = delete;
    CybertwinTopologyReader &operator=(const CybertwinTopologyReader &) = delete;

    //----------------------------------------------------------
    //          Topology Construction
    //----------------------------------------------------------
    NodeContainer Read();

    NodeInfo_t* CreateCloudNodeInfo(const YAML::Node &node);
    NodeInfo_t* CreateEndClusterNodeInfo(const YAML::Node &node);
    NodeContainer GetCoreCloudNodes();
    NodeContainer GetEdgeCloudNodes();
    NodeContainer GetEndClusterNodes();
    NodeContainer GetEndHostNodes();
    NodeContainer GetApNodes();
    NodeContainer GetStaNodes();

    //----------------------------------------------------------
    //          Application Installation
    //----------------------------------------------------------
    void SetAppFiles(const std::string &files);
    void InstallApplications();

    //----------------------------------------------------------
    //          Animation
    //----------------------------------------------------------

  private:
    NodeType_e GetNodeType(const std::string &type);
    Ptr<Node> GetNodeByName(const std::string &name);
    std::vector<Link_t> ParseLinks(const YAML::Node &connections);
    std::vector<Gateway_t> ParseGateways(const YAML::Node &gateways);

    Ipv4InterfaceContainer CreateP2PLink(Ptr<Node> sourceNode, Ptr<Node> targetNode, std::string &data_rate, std::string &delay, std::string &network);
    NodeContainer CreateCsmaNetwork(NodeInfo *csma, Ptr<Node> &leader);
    NodeContainer CreateWifiNetwork(NodeInfo *wifi, Ptr<Node> &leader);
    Ipv4InterfaceContainer AssignIPAddresses(const NetDeviceContainer &devices, const std::string &network);

    void ParseCoreCloud(const YAML::Node &coreLayer);
    void ParseEdgeCloud(const YAML::Node &edgeLayer);
    void ParseAccessNetwork(const YAML::Node &accessLayer);
    void ConfigCNRS(const YAML::Node &cnrsConfig);

    void ShowNetworkTopology();

    std::string MaskNumberToIpv4Address(std::string mask);

    std::vector<NodeInfo_t *> m_coreNodesList;
    std::vector<NodeInfo_t *> m_edgeNodesList;
    std::vector<NodeInfo_t *> m_endNodesList;
  
    NodeContainer m_nodes;
    NetDeviceContainer m_devices;
    NodeContainer m_coreNodes;
    NetDeviceContainer m_coreDevices;
    NodeContainer m_edgeNodes;
    NetDeviceContainer m_edgeDevices;
    NodeContainer m_endNodes;
    NetDeviceContainer m_endDevices;
    NodeContainer m_apNodes;
    NodeContainer m_staNodes;
    NodeContainer m_endhostNodes;
    std::unordered_map<std::string, NodeInfo_t *> m_nodeInfoMap;
    std::unordered_map<std::string, Ptr<Node>> m_nodeName2Ptr;
    std::unordered_set<std::string> m_links;

    // applications
    std::string m_appFils;

    // Cybertwin Name Resolution Service
    std::string m_cnrsNodeName;
};

}; // namespace ns3

#endif /* CYBERTWIN_TOPOLOGY_READER_H */