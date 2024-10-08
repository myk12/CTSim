#include "cybertwin-topology-reader.h"

#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/point-to-point-helper.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinTopologyReader");

NS_OBJECT_ENSURE_REGISTERED(CybertwinTopologyReader);

TypeId
CybertwinTopologyReader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinTopologyReader")
                            .SetParent<TopologyReader>()
                            .SetGroupName("TopologyReader")
                            .AddConstructor<CybertwinTopologyReader>();
    return tid;
}

CybertwinTopologyReader::CybertwinTopologyReader()
{
    NS_LOG_FUNCTION(this);
}

CybertwinTopologyReader::~CybertwinTopologyReader()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinTopologyReader::CreateCoreCloud()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Creating core cloud...");
    for (auto nodeInfo : m_coreNodesList) {
        // get target
        std::string nodeName = nodeInfo->name;
        for (auto link : nodeInfo->links) {
            std::string target = link.target;
            std::string data_rate = link.data_rate;
            std::string delay = link.delay;
            NS_LOG_INFO("Target: " << target);

            if (m_nodeInfoMap.find(target) == m_nodeInfoMap.end()) {
                NS_LOG_ERROR("Node not found: " << target);
                continue;
            }

            if (m_links.find(nodeName + target) != m_links.end() ||
                m_links.find(target + nodeName) != m_links.end()) {
                NS_LOG_INFO("Link already exists: " << nodeName << " -> " << target);
                continue;
            }

            NS_LOG_INFO("Creating link: " << nodeName << " <-> " << target);
            // get target node
            Ptr<Node> targetNode = m_nodeInfoMap[target]->node;

            // create point-to-point link
            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", StringValue(data_rate));
            p2p.SetChannelAttribute("Delay", StringValue(delay));

            NetDeviceContainer devices = p2p.Install(nodeInfo->node, targetNode);
            m_coreDevices.Add(devices);

            // add link to the map
            m_links.insert(nodeName + target);
            m_links.insert(target + nodeName);
        }
    }
    
}

void
CybertwinTopologyReader::ParseNodes(const YAML::Node &nodes, const std::string &layerName)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Parsing nodes...");

    for (YAML::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        const YAML::Node &node = *it;

        NodeInfo_t *nodeInfo = new NodeInfo_t();

        if (node["name"] && node["type"]) {
            std::string name = node["name"].as<std::string>();
            std::string type = node["type"].as<std::string>();
            nodeInfo->name = name;
            YAML::Node connections = node["connections"];

            NS_LOG_INFO("Node name: " << name);
            NS_LOG_INFO("Node type: " << type);

            if (type == "host_server") {
                NS_LOG_INFO("Node type: HOST_SERVER");
                Ptr<Node> n = CreateObject<Node>();
                nodeInfo->node = n;
                nodeInfo->type = HOST_SERVER;
                for (YAML::const_iterator it = connections.begin(); it != connections.end(); ++it) {
                    const YAML::Node &connection = *it;

                    Link_t link;
                    link.target = connection["target"].as<std::string>();
                    link.data_rate = connection["data_rate"].as<std::string>();
                    link.delay = connection["delay"].as<std::string>();

                    nodeInfo->links.push_back(link);
                }

            } else if (type == "end_cluster") {
                NS_LOG_INFO("Node type: END_CLUSTER");
                nodeInfo->type = END_CLUSTER;
                nodeInfo->num_nodes = node["num_nodes"].as<int32_t>();
                for (int i = 0; i < nodeInfo->num_nodes; i++) {
                    Ptr<Node> n = CreateObject<Node>();
                    nodeInfo->nodes.Add(n);
                    m_nodes.Add(n);
                }

                for (YAML::const_iterator it = connections.begin(); it != connections.end(); ++it) {
                    const YAML::Node &connection = *it;

                    Gateway_t gateway;
                    gateway.name = connection["name"].as<std::string>();
                    gateway.data_rate = connection["data_rate"].as<std::string>();
                    gateway.delay = connection["delay"].as<std::string>();

                    nodeInfo->gateways.push_back(gateway);
                }

            } else {
                NS_LOG_ERROR("Unknown node type: " << type);
            }
        }

        if (layerName == "Core") {
            m_coreNodesList.push_back(nodeInfo);
        } else if (layerName == "Edge") {
            m_edgeNodesList.push_back(nodeInfo);
        } else if (layerName == "End") {
            m_endNodesList.push_back(nodeInfo);
        }

        m_nodeInfoMap[nodeInfo->name] = nodeInfo;
    }
}

void
CybertwinTopologyReader::ParseLayer(const YAML::Node &layerNode, const std::string& layerName)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Parsing " << layerName << " layer...");

    if (layerNode["nodes"]) {
        ParseNodes(layerNode["nodes"], layerName);
    }else {
        NS_LOG_ERROR("No nodes found in " << layerName << " layer!");
    }
}

NodeContainer
CybertwinTopologyReader::Read()
{
    NS_LOG_FUNCTION(this);

    YAML::Node topology_yaml = YAML::LoadFile(GetFileName());

    // read nodes from the topology file
    if (topology_yaml["cybertwin_network"]) {
        const YAML::Node &cybertwin_network = topology_yaml["cybertwin_network"];

        NS_ASSERT(cybertwin_network["core_layer"] || cybertwin_network["edge_layer"] || cybertwin_network["access_layer"]);
        // parse core, edge and end layers
        ParseLayer(cybertwin_network["core_layer"], "Core");
        ParseLayer(cybertwin_network["edge_layer"], "Edge");
        ParseLayer(cybertwin_network["access_layer"], "End");
    }

    // read applications from applications file

    // create core cloud
    CreateCoreCloud();
    

    return m_nodes;
}

} // namespace ns3