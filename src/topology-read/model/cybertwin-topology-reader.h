#ifndef CYBERTWIN_TOPOLOGY_READER_H
#define CYBERTWIN_TOPOLOGY_READER_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "topology-reader.h"
#include "ns3/net-device-container.h"
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
} Link_t;

typedef struct Gateway
{
    std::string name;
    std::string data_rate;
    std::string delay;
} Gateway_t;

// define node info
typedef struct NodeInfo
{
    Ptr<Node> node;
    NodeContainer nodes;
    std::string name;
    NodeType_e type;
    int32_t num_nodes;  // number of nodes in the cluster
    std::vector<Link_t> links;
    std::vector<Gateway_t> gateways;
} NodeInfo_t;

class CybertwinTopologyReader : public TopologyReader
{
  public:
    static TypeId GetTypeId();

    CybertwinTopologyReader();
    ~CybertwinTopologyReader() override;

    CybertwinTopologyReader(const CybertwinTopologyReader &) = delete;
    CybertwinTopologyReader &operator=(const CybertwinTopologyReader &) = delete;

    NodeContainer Read();

  private:
    void ParseLayer(const YAML::Node &layerNode, const std::string &layerName);
    void ParseNodes(const YAML::Node &nodes, const std::string &layerName);

    void CreateCoreCloud();
    void CreateEdgeNodes();
    void CreateEndNodes();

    //void ReadNodes(std::ifstream &inputStream, NodeContainer &nodes);
    //void ReadLinks(std::ifstream &inputStream, NodeContainer &nodes);
    //void ReadLink(std::ifstream &inputStream, NodeContainer &nodes);
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
    std::unordered_map<std::string, NodeInfo_t *> m_nodeInfoMap;

    std::unordered_set<std::string> m_links;
};

}; // namespace ns3



#endif /* CYBERTWIN_TOPOLOGY_READER_H */