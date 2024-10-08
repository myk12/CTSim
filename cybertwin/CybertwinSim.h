
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/cybertwin-name-resolution-service.h"
#include "ns3/cybertwin-node.h"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define CORE_CLOUD_CONF_PATH "./cybertwin/system/core_cloud/"
#define EDGE_CLOUD_CONF_PATH "./cybertwin/system/edge_cloud/"
#define ACCESS_NET_CONF_PATH "./cybertwin/system/access_net/"

#define SYS_CONF_DIR_NAME "/etc/"
#define SYS_LOG_DIR_NAME "/log/"

#define CORE_CLOUD_NODE_PREFIX "core_cloud_"
#define EDGE_CLOUD_NODE_PREFIX "edge_cloud_"
#define ACCESS_NET_NODE_PREFIX "end_host_"

#define CYBERTWIN_TOPOLOGY_FILE "./cybertwin/system/topology.json"

#define CORE_CLOUD_DEFAULT_IP_FORMAT "10.%d.0.0"
#define CORE_CLOUD_DEFAULT_MASK "255.255.255.0"

#define EDGE_CLOUD_DEFAULT_IP_FORMAT "20.%d.0.0"
#define EDGE_CLOUD_DEFAULT_MASK "255.255.255.0"

#define ACCESS_NET_DEFAULT_IP_FORMAT "30.%d.0.0"
#define ACCESS_NET_DEFAULT_MASK "255.255.255.0"

#define CORE_LINE_RATE "1000Mbps"
#define EDGE_LINE_RATE "1000Mbps"

namespace ns3
{
    
class CybertwinNodeContainer
{
  public:
    CybertwinNodeContainer();
    ~CybertwinNodeContainer();

    struct NodeInfo
    {
        std::string nameStr;
        Ptr<Node> nodePtr;
    };

    Ptr<Node> GetNodeByName(std::string name);
    int32_t AddNode(std::string name, Ptr<Node> node);

  private:
    std::vector<NodeInfo> m_nodes;
};

class CybertwinSim
{
  public:
    CybertwinSim();
    ~CybertwinSim();

    int32_t SetConfigPath(std::string path);

    int32_t Input();
    //int32_t Driver();
    //int32_t Simulate();
    //int32_t Output();

    int32_t Compiler();
    int32_t Run();

  private:
    // private methods
    int32_t InitTopology();
    int32_t ParseNodes();
    int32_t ParseNodeConfig(Ptr<CybertwinNode> node, std::string nodeConfigPath);

  private:

    // private members
    CybertwinNodeContainer m_nodes;
    NodeContainer m_coreCloudNodes;
    NodeContainer m_edgeCloudNodes;
    NodeContainer m_endHostNodes;

    // config file path
    std::string m_configFilePath;
};
} // namespace ns3