#include "cybertwin-topology-reader.h"

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

std::string
CybertwinTopologyReader::MaskNumberToIpv4Address(std::string mask)
{
    NS_LOG_FUNCTION(this);
    std::string result;
    uint32_t maskNumber = std::stoi(mask);
    uint32_t maskValue = 0xFFFFFFFF << (32 - maskNumber);
    result = std::to_string((maskValue >> 24) & 0xFF) + "." +
             std::to_string((maskValue >> 16) & 0xFF) + "." +
             std::to_string((maskValue >> 8) & 0xFF) + "." + std::to_string(maskValue & 0xFF);
    return result;
}

// Determine  the type of nodes
NodeType_e
CybertwinTopologyReader::GetNodeType(const std::string& type)
{
    if (type == "host_server")
    {
        return NodeType_e::HOST_SERVER;
    }
    else if (type == "end_cluster")
    {
        return NodeType_e::END_CLUSTER;
    }
    else
    {
        NS_LOG_ERROR("Unknown node type: " << type);
        return NodeType_e::HOST_SERVER;
    }
}

NodeInfo_t*
CybertwinTopologyReader::CreateCloudNodeInfo(const YAML::Node& node)
{
    NodeInfo_t* nodeInfo = new NodeInfo_t();
    nodeInfo->name = node["name"].as<std::string>();
    nodeInfo->type = GetNodeType(node["type"].as<std::string>());
    nodeInfo->links = ParseLinks(node["connections"]);
    nodeInfo->position = Vector(node["position"][0].as<double>(),
                                node["position"][1].as<double>(),
                                node["position"][2].as<double>());

    // insert to the node info map
    m_nodeInfoMap[nodeInfo->name] = nodeInfo;
    return nodeInfo;
}

NodeInfo_t*
CybertwinTopologyReader::CreateEndClusterNodeInfo(const YAML::Node& node)
{
    NodeInfo_t* nodeInfo = new NodeInfo_t();
    nodeInfo->name = node["name"].as<std::string>();
    nodeInfo->type = GetNodeType(node["type"].as<std::string>());
    nodeInfo->num_nodes = node["num_nodes"].as<int>();
    nodeInfo->local_network = node["local_network"].as<std::string>();
    nodeInfo->network_type = node["network_type"].as<std::string>();
    nodeInfo->gateways = ParseGateways(node["gateways"]);
    nodeInfo->position = Vector(node["position"][0].as<double>(),
                                node["position"][1].as<double>(),
                                node["position"][2].as<double>());

    // insert to the node info map
    m_nodeInfoMap[nodeInfo->name] = nodeInfo;
    return nodeInfo;
}

//-----------------------------------------------------------------------------
//
//        Parse the topology configuration file
//
//-----------------------------------------------------------------------------
//
// The topology configuration file is in YAML format.
// The file contains the following sections:
// 1. core_cloud
// 2. edge_cloud
// 3. access_net
//
// Each section contains the following information:
// 1. nodes
// 2. links
// 3. gateways
//
// The nodes section contains the following information:
// 1. name
// 2. type
// 3. connections
//
// The connections section contains the following information:
// 1. target
// 2. data_rate
// 3. delay
// 4. network
//
//-----------------------------------------------------------------------------

// Parse the Core Cloud Layer
// The core cloud layer contains the core servers
// The core servers are connected to each other using point-to-point links
void
CybertwinTopologyReader::ParseCoreCloud(const YAML::Node& coreCloudConfig)
{
    // parse core cloud
    NS_LOG_INFO("Parsing core cloud...");
    NS_ASSERT(coreCloudConfig["nodes"]);
    Ptr<CybertwinCoreServer> coreServerSrc = nullptr;
    Ptr<CybertwinCoreServer> coreServerDst = nullptr;

    for (const auto& node : coreCloudConfig["nodes"])
    {
        // Parse node information and create the Cybertwin core server
        NodeInfo_t* nodeInfo = CreateCloudNodeInfo(node);
        m_coreNodesList.push_back(nodeInfo);

        // create nodes
        Ptr<Node> n = CreateObject<CybertwinCoreServer>();
        nodeInfo->node = n;
        m_nodes.Add(n);
        m_coreNodes.Add(n);
        m_nodeName2Ptr[nodeInfo->name] = n;

        // configure the Cybertiwn core server
        coreServerSrc = DynamicCast<CybertwinCoreServer>(n);
        coreServerSrc->SetName(nodeInfo->name);

        // Install Internet stack on the node
        InternetStackHelper stack;
        stack.Install(n);

        // Set constant position for the core cloud nodes
        Vector pos = nodeInfo->position;
        AnimationInterface::SetConstantPosition(n, pos.x, pos.y, pos.z);
    }

    // create links between the core cloud nodes
    // Here we go through all the nodes and create point-to-point links
    // between the core cloud nodes
    // We set the first core cloud node as the Central Name Resolution Service (CNRS)
    for (auto nodeInfo : m_coreNodesList)
    {
        // get target
        std::string nodeName = nodeInfo->name;

        // go through all the links and create point-to-point network
        // between the core cloud nodes
        for (auto link : nodeInfo->links)
        {
            std::string target = link.target;
            std::string data_rate = link.data_rate;
            std::string delay = link.delay;

            coreServerSrc = DynamicCast<CybertwinCoreServer>(nodeInfo->node);
            coreServerDst = DynamicCast<CybertwinCoreServer>(m_nodeInfoMap[target]->node);

            if (m_nodeInfoMap.find(target) == m_nodeInfoMap.end())
            {
                NS_LOG_ERROR("Node not found: " << target);
                continue;
            }

            if (m_links.find(nodeName + target) != m_links.end() ||
                m_links.find(target + nodeName) != m_links.end())
            {
                NS_LOG_INFO("Link already exists: " << nodeName << " <-> " << target);
                continue;
            }

            Ipv4InterfaceContainer interfaces =
                CreateP2PLink(coreServerSrc, coreServerDst, data_rate, delay, link.network);

            // configure core server, add global IP address and parent node
            // the m_links set is used to keep track of the links and avoid
            // creating duplicate links and loops
            coreServerSrc->AddGlobalIp(interfaces.GetAddress(0));
            coreServerDst->AddGlobalIp(interfaces.GetAddress(1));
            NS_LOG_INFO("Link: " << nodeName << "(" << interfaces.GetAddress(0) << ")" << " <-> " << target << "("
                                << interfaces.GetAddress(1) << ")");

            m_links.insert(nodeName + target);
            m_links.insert(target + nodeName);
        }
    }
}

// Parse the Edge Cloud Layer
// The edge cloud layer contains the edge servers
// The edge servers are connected to the core cloud nodes using point-to-point links
void
CybertwinTopologyReader::ParseEdgeCloud(const YAML::Node& edgeCloudConfig)
{
    NS_LOG_INFO("Parsing edge cloud...");
    Ptr<CybertwinEdgeServer> edgeServer = nullptr;

    // Here we go through all the nodes in the edge cloud, create
    // the Cybertwin edge server and connect the edge servers to
    // the core cloud nodes

    for (const auto& node : edgeCloudConfig["nodes"])
    {
        // Parse node information and create the Cybertwin edge server
        NodeInfo_t* nodeInfo = CreateCloudNodeInfo(node);
        m_edgeNodesList.push_back(nodeInfo);

        // create nodes
        Ptr<Node> n = CreateObject<CybertwinEdgeServer>();
        nodeInfo->node = n;
        m_nodes.Add(n);
        m_edgeNodes.Add(n);
        m_nodeName2Ptr[nodeInfo->name] = n;

        // configure the Cybertiwn edge server
        edgeServer = DynamicCast<CybertwinEdgeServer>(n);
        edgeServer->SetName(nodeInfo->name);

        // Install Internet stack on the node
        InternetStackHelper stack;
        stack.Install(n);

        // Set constant position for the edge cloud nodes
        Vector pos = nodeInfo->position;
        AnimationInterface::SetConstantPosition(n, pos.x, pos.y, pos.z);

        // connect to the core cloud nodes
        for (auto link : nodeInfo->links)
        {
            Ipv4InterfaceContainer interfaces;
            std::string target = link.target;
            std::string data_rate = link.data_rate;
            std::string delay = link.delay;

            if (m_nodeInfoMap.find(target) == m_nodeInfoMap.end())
            {
                NS_LOG_ERROR("Node not found: " << target);
                continue;
            }

            interfaces = CreateP2PLink(nodeInfo->node,
                                       m_nodeInfoMap[target]->node,
                                       data_rate,
                                       delay,
                                       link.network);

            // Configure edge server
            edgeServer->AddParent(m_nodeInfoMap[target]->node);
            edgeServer->SetAttribute("UpperNodeAddress",
                                     Ipv4AddressValue(interfaces.GetAddress(1)));
            edgeServer->AddGlobalIp(interfaces.GetAddress(0));
        }
    }
}

// Parse the Access Network Layer
// The access network layer contains the end hosts (IoT devices)
// These devices are connected to the edge servers using point-to-point links
// The end hosts are connected to the edge servers using CSMA or WiFi
void
CybertwinTopologyReader::ParseAccessNetwork(const YAML::Node& accessLayerConfig)
{
    NS_LOG_INFO("Parsing access network...");
    for (const auto& node : accessLayerConfig["nodes"])
    {
        // print node
        NodeInfo_t* nodeInfo = CreateEndClusterNodeInfo(node);
        m_nodeInfoMap[nodeInfo->name] = nodeInfo;

        // Create different access network according to the network type
        // For now, we only support CSMA and WiFi
        // The leader node is the first node in the list and it is
        // connected to the edge cloud
        Ptr<Node> leader = nullptr;
        Ptr<Node> gateway = nullptr;
        NodeContainer endNodes;
        if (node["network_type"].as<std::string>() == "csma")
        {
            NS_LOG_INFO("Creating CSMA network...");
            endNodes = CreateCsmaNetwork(nodeInfo, leader);
        }
        else if (node["network_type"].as<std::string>() == "wifi")
        {
            NS_LOG_INFO("Creating WiFi network...");
            endNodes = CreateWifiNetwork(nodeInfo, leader);
        }
        else
        {
            NS_LOG_ERROR("Unknown network type: " << node["network_type"].as<std::string>());
        }

        // TODO: add multiple gateways support
        // connect end cluster to the edge cloud
        // currently we only support one gateway
        NS_ASSERT(nodeInfo->gateways.size() > 0);
        gateway = m_nodeInfoMap[nodeInfo->gateways[0].name]->node;
        Ipv4InterfaceContainer interfaces = CreateP2PLink(leader,
                                                          gateway,
                                                          nodeInfo->gateways[0].data_rate,
                                                          nodeInfo->gateways[0].delay,
                                                          nodeInfo->gateways[0].network);

        // configure the end cluster
        for (int i = 0; i < nodeInfo->num_nodes; i++)
        {
            Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(endNodes.Get(i));
            endHost->AddParent(gateway);
            endHost->SetAttribute("UpperNodeAddress", Ipv4AddressValue(interfaces.GetAddress(1)));
        }

        // configure the gateway
        Ptr<CybertwinEdgeServer> edgeServer = DynamicCast<CybertwinEdgeServer>(gateway);
        edgeServer->AddGlobalIp(interfaces.GetAddress(1));
    }
}

std::vector<Link_t>
CybertwinTopologyReader::ParseLinks(const YAML::Node& linksNode)
{
    std::vector<Link_t> links;
    for (const auto& link : linksNode)
    {
        Link_t linkInfo;
        linkInfo.target = link["target"].as<std::string>();
        linkInfo.network = link["network"].as<std::string>();
        linkInfo.data_rate = link["data_rate"].as<std::string>();
        linkInfo.delay = link["delay"].as<std::string>();
        links.push_back(linkInfo);
    }
    return links;
}

std::vector<Gateway_t>
CybertwinTopologyReader::ParseGateways(const YAML::Node& gateways)
{
    std::vector<Gateway_t> gatewaysList;
    for (const auto& gateway : gateways)
    {
        Gateway_t gatewayInfo;
        gatewayInfo.name = gateway["target"].as<std::string>();
        gatewayInfo.network = gateway["network"].as<std::string>();
        gatewayInfo.data_rate = gateway["data_rate"].as<std::string>();
        gatewayInfo.delay = gateway["delay"].as<std::string>();
        gatewaysList.push_back(gatewayInfo);
    }
    return gatewaysList;
}

Ipv4InterfaceContainer
CybertwinTopologyReader::CreateP2PLink(Ptr<Node> sourceNode,
                                       Ptr<Node> targetNode,
                                       std::string& data_rate,
                                       std::string& delay,
                                       std::string& network)
{
    NS_LOG_FUNCTION(this);
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(data_rate));
    p2p.SetChannelAttribute("Delay", StringValue(delay));

    NetDeviceContainer devices = p2p.Install(sourceNode, targetNode);
    m_coreDevices.Add(devices);

    // assign IP addresses
    Ipv4InterfaceContainer interfaces = AssignIPAddresses(devices, network);

    return interfaces;
}

/**
 * Create a CSMA network
 *
 * @param csma - node information
 * @param leader - leader node
 */
NodeContainer
CybertwinTopologyReader::CreateCsmaNetwork(NodeInfo* csma, Ptr<Node>& leader)
{
    // Create nodes
    NodeContainer nodes;
    for (int i = 0; i < csma->num_nodes; i++)
    {
        Ptr<Node> n = CreateObject<CybertwinEndHost>();
        nodes.Add(n);
        m_nodes.Add(n);
        m_endNodes.Add(n);

        // configure the Cybertiwn end host
        std::string nodeName = csma->name + "_" + std::to_string(i);
        Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(n);
        endHost->SetName(nodeName);
        m_nodeName2Ptr[nodeName] = n;

        // Set constant position for the ethernet nodes
        Vector pos = csma->position;
        // generate random position
        double x = pos.x + (rand() % 10);
        double y = pos.y + (rand() % 10);
        double z = pos.z;
        AnimationInterface::SetConstantPosition(n, x, y, z);
    }

    // Add endhost to endhostNodes Container
    m_endhostNodes.Add(nodes);

    // Install CSMA devices
    CsmaHelper csmaHelper;
    NetDeviceContainer devices;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    devices = csmaHelper.Install(nodes);

    // Install Internet stack on the node
    InternetStackHelper stack;
    stack.Install(nodes);

    // assign IP addresses
    Ipv4InterfaceContainer interfaces = AssignIPAddresses(devices, csma->local_network);

    // configure the end host nodes
    for (int i = 0; i < csma->num_nodes; i++)
    {
        Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(nodes.Get(i));
        endHost->AddLocalIp(interfaces.GetAddress(i));
    }

    // Return Leader node
    leader = nodes.Get(0);

    return nodes;
}

/**
 * Create a WiFi network
 *
 * @param wifi - node information
 * @param leader - leader node
 *
 */
NodeContainer
CybertwinTopologyReader::CreateWifiNetwork(NodeInfo* wifi, Ptr<Node>& leader)
{
    // Create nodes
    NodeContainer apNode;
    NodeContainer staNodes;
    NodeContainer allNodes;
    for (int i = 0; i < wifi->num_nodes; i++)
    {
        Ptr<Node> n = CreateObject<CybertwinEndHost>();
        m_nodes.Add(n);
        m_endNodes.Add(n);
        (i == 0) ? apNode.Add(n) : staNodes.Add(n);
        allNodes.Add(n);

        // configure the Cybertiwn end host
        std::string nodeName = wifi->name + "_" + std::to_string(i);
        Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(n);
        endHost->SetName(nodeName);
        m_nodeName2Ptr[nodeName] = n;
    }

    // Add nodes to the container
    m_apNodes.Add(apNode);
    m_staNodes.Add(staNodes);

    // Install WiFi devices
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid(wifi->name);

    NS_LOG_INFO("Access network node: " << wifi->name);
    WifiHelper wifiHelper;
    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifiHelper.Install(phy, mac, staNodes);

    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifiHelper.Install(phy, mac, apNode);
    NS_LOG_INFO("Ssid " << ssid << "\n"
                        << "AP devices: " << apDevices.GetN() << "\n"
                        << "STA devices: " << staDevices.GetN());

    // Set random mobility model for the nodes
    // and position
    Vector pos = wifi->position;
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(pos.x - 10),
                                  "MinY",
                                  DoubleValue(pos.y - 10),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(5.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.SetMobilityModel(
        "ns3::RandomWalk2dMobilityModel",
        "Bounds",
        RectangleValue(Rectangle(pos.x - 10, pos.x + 10, pos.y - 10, pos.y + 10)));
    mobility.Install(staNodes);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNode);

    // Install Internet stack on the node
    InternetStackHelper stack;
    stack.Install(allNodes);

    // Assign IP addresses
    NetDeviceContainer devices = apDevices.Get(0);
    devices.Add(staDevices);

    Ipv4InterfaceContainer interfaces = AssignIPAddresses(devices, wifi->local_network);

    // configure the end host nodes
    for (int i = 0; i < wifi->num_nodes; i++)
    {
        Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(allNodes.Get(i));
        endHost->AddLocalIp(interfaces.GetAddress(i));
    }

    // Return Leader node
    leader = apNode.Get(0);

    return allNodes;
}

Ipv4InterfaceContainer
CybertwinTopologyReader::AssignIPAddresses(const NetDeviceContainer& devices,
                                           const std::string& network)
{
    Ipv4AddressHelper ipv4;
    std::string networkPrefix = network.substr(0, network.find('/'));
    std::string networkMask = MaskNumberToIpv4Address(network.substr(network.find('/') + 1));

    ipv4.SetBase(networkPrefix.c_str(), networkMask.c_str());
    NS_LOG_INFO("Assigned IP addresses for network " << networkPrefix);
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    return interfaces;
}

Ptr<Node>
CybertwinTopologyReader::GetNodeByName(const std::string& nodeName)
{
    auto it = m_nodeName2Ptr.find(nodeName);
    if (it == m_nodeName2Ptr.end())
    {
        NS_LOG_ERROR("Node not found: " << nodeName);
        throw std::runtime_error("Node not found: " + nodeName);
    }
    return it->second;
}

// Show the network topology
void
CybertwinTopologyReader::ShowNetworkTopology()
{
    NS_LOG_INFO("Network Topology:");
    NS_LOG_INFO("Core Cloud:");
    for (auto node : m_coreNodesList)
    {
        NS_LOG_INFO("Node: " << node->name);
        for (auto link : node->links)
        {
            NS_LOG_INFO("  Link: " << link.target << " " << link.data_rate << " " << link.delay
                                   << " " << link.network);
        }
    }

    NS_LOG_INFO("Edge Cloud:");
    for (auto node : m_edgeNodesList)
    {
        NS_LOG_INFO("Node: " << node->name);
        for (auto link : node->links)
        {
            NS_LOG_INFO("  Link: " << link.target << " " << link.data_rate << " " << link.delay
                                   << " " << link.network);
        }
    }

    NS_LOG_INFO("End Cluster:");
    for (uint32_t i = 0; i < m_endNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> endHost = DynamicCast<CybertwinEndHost>(m_endNodes.Get(i));
        NS_LOG_INFO("Node: " << endHost->GetName());
    }
}

NodeContainer
CybertwinTopologyReader::GetCoreCloudNodes()
{
    return m_coreNodes;
}

NodeContainer
CybertwinTopologyReader::GetEdgeCloudNodes()
{
    return m_edgeNodes;
}

NodeContainer
CybertwinTopologyReader::GetEndClusterNodes()
{
    return m_endNodes;
}

NodeContainer
CybertwinTopologyReader::GetEndHostNodes()
{
    return m_endhostNodes;
}

NodeContainer
CybertwinTopologyReader::GetApNodes()
{
    return m_apNodes;
}

NodeContainer
CybertwinTopologyReader::GetStaNodes()
{
    return m_staNodes;
}

// Configure the Central Name Resolution Service (CNRS)
// Currently we need to manually set the centralized 
// node at the core cloud
void
CybertwinTopologyReader::ConfigCNRS(const YAML::Node& cnrsConfig)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(cnrsConfig["central_node"]);
    std::string centralNode = cnrsConfig["central_node"].as<std::string>();
    Ptr<Node> node = GetNodeByName(centralNode);
    Ptr<CybertwinCoreServer> coreServer = DynamicCast<CybertwinCoreServer>(node);
    // Set this node as the root of CNRS
    coreServer->SetCNRSRoot();

    NS_ASSERT(coreServer->GetGlobalIpList().size() > 0);
    Ipv4Address centralNodeAddress = coreServer->GetGlobalIpList().at(0);

    // for each other node, set the parent node
    for (const auto& node : m_coreNodesList)
    {
        if (node->name != centralNode)
        {
            Ptr<Node> n = GetNodeByName(node->name);
            Ptr<CybertwinCoreServer> coreServer = DynamicCast<CybertwinCoreServer>(n);
            coreServer->SetUpperNodeAddress(centralNodeAddress);
        }
    }
}


// Read the topology configuration file
NodeContainer
CybertwinTopologyReader::Read()
{
    NS_LOG_FUNCTION(this);

    YAML::Node topology_yaml = YAML::LoadFile(GetFileName());

    // read nodes from the topology file
    NS_ASSERT(topology_yaml["cybertwin_network"]);
    const YAML::Node& cybertwin_network = topology_yaml["cybertwin_network"];
    NS_ASSERT(cybertwin_network["core_layer"] || cybertwin_network["edge_layer"] ||
              cybertwin_network["access_layer"]);

    // parse core, edge and end layers
    ParseCoreCloud(cybertwin_network["core_layer"]);
    ParseEdgeCloud(cybertwin_network["edge_layer"]);
    ParseAccessNetwork(cybertwin_network["access_layer"]);

    // config CNRS
    NS_ASSERT(cybertwin_network["cnrs"]);
    ConfigCNRS(cybertwin_network["cnrs"]);

    // Output Nodes
    ShowNetworkTopology();

    return m_nodes;
}

// Install applications
void
CybertwinTopologyReader::SetAppFiles(const std::string& files)
{
    m_appFils = files;
}

void
CybertwinTopologyReader::InstallApplications()
{
    NS_LOG_FUNCTION(this);

    // Parse the application configuration file
    // The application configuration file is in YAML format
    // The file contains the following sections:
    // 1. applications
    // The applications section contains all the applications that
    // need to be install on the nodes
    //
    // Each application contains the following information:
    // 1. name : the name of the application
    // 2. type : the type of the application
    // 3. enabled : whether the application is enabled or not
    // 4. nodes : the list of nodes where the application is installed
    // 5. parameters : the parameters of the application

    YAML::Node app_yaml = YAML::LoadFile(m_appFils);

    // read applications from the application file
    NS_ASSERT(app_yaml["applications"]);
    const YAML::Node& applications = app_yaml["applications"];
    NS_LOG_INFO("Installing applications...");

    for (const auto& app : applications)
    {
        std::string appName = app["name"].as<std::string>();
        std::string appType = app["type"].as<std::string>();
        bool enabled = app["enabled"].as<bool>();
        NS_LOG_INFO("Application: " << appName << " type: " << appType << " enabled: " << enabled);
        // Parse the application configuration like this:
        //  - name: download-server
        //    description: Download Server Application
        //    type: download-server
        //    enabled: true
        //    start_delay: 0
        //    target_nodes:
        //      - core_node1
        //    parameters:
        //      - start-delay: 0
        //        cybertwin-id: 1000
        //        cybertwin-port: 1000
        std::vector<std::string> nodes;
        for (const auto& node : app["target_nodes"])
        {
            nodes.push_back(node.as<std::string>());
        }

        // install the application on the nodes
        NodeContainer targetNodes;
        for (const auto& nodeName : nodes)
        {
            Ptr<Node> node = GetNodeByName(nodeName);
            targetNodes.Add(node);
        }

        YAML::Node newParams;
        for (const auto& param : app["parameters"])
        {
            for (YAML::const_iterator it = param.begin(); it != param.end(); ++it)
            {
                std::string key = it->first.as<std::string>();
                YAML::Node value = it->second;
                newParams[key] = value;
            }
        }

        CybertwinAppHelper appHelper;
        appHelper.InstallApplications(appName, newParams, targetNodes);
    }
}

} // namespace ns3