#include "CybertwinSim.h"

#include <filesystem>
#include <iostream>

namespace ns3
{
namespace fs = std::filesystem;

NS_LOG_COMPONENT_DEFINE("CybertwinSim");

CybertwinSim::CybertwinSim()
{
    m_coreCloudNodes.Create(0);
    m_edgeCloudNodes.Create(0);
    m_endHostNodes.Create(0);
}

CybertwinSim::~CybertwinSim()
{
}

int32_t
CybertwinSim::SetConfigPath(std::string path)
{
    m_configFilePath = path;
    return 0;
}

int32_t
CybertwinSim::Input()
{
    NS_LOG_INFO("\n======= INPUT =======\n\n");

    // check config file path
    if (!fs::exists(m_configFilePath) || !fs::is_directory(m_configFilePath))
    {
        NS_LOG_ERROR("Config file path does not exist or is not a directory");
        return -1;
    }

    // parse Nodes
    ParseNodes();

    return 0;
}

int32_t
CybertwinSim::Compiler()
{
    InitTopology();
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    return 0;
}

int32_t
CybertwinSim::Run()
{
    NS_LOG_INFO("\n======= Cybertwin::Run Simulator =======\n\n");
    // power on all core nodes
    for (uint32_t i = 0; i < m_coreCloudNodes.GetN(); i++)
    {
        Ptr<CybertwinCoreServer> node = DynamicCast<CybertwinCoreServer>(m_coreCloudNodes.Get(i));
        node->PowerOn();
    }

    // power on all edge nodes
    for (uint32_t i = 0; i < m_edgeCloudNodes.GetN(); i++)
    {
        Ptr<CybertwinEdgeServer> node = DynamicCast<CybertwinEdgeServer>(m_edgeCloudNodes.Get(i));
        node->PowerOn();
    }

    // power on all end hosts
    for (uint32_t i = 0; i < m_endHostNodes.GetN(); i++)
    {
        Ptr<CybertwinEndHost> node = DynamicCast<CybertwinEndHost>(m_endHostNodes.Get(i));
        node->PowerOn();
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

int32_t
CybertwinSim::InitTopology()
{
    NS_LOG_INFO("\n======= Cybertwin::InitTopology() =======\n\n");
    InternetStackHelper stack;
    // install network stack for each nodes
    stack.Install(m_coreCloudNodes);
    stack.Install(m_edgeCloudNodes);
    stack.Install(m_endHostNodes);

    // parse topolgoy
    nlohmann::json topology;
    nlohmann::json edgeConf;
    nlohmann::json accessConf;
    std::ifstream topologyFile(CYBERTWIN_TOPOLOGY_FILE);
    topologyFile >> topology;
    topologyFile.close();

    edgeConf = topology["edge_cloud"];
    accessConf = topology["access_net"];

    // connect core servers
    NS_LOG_INFO("STEP[1]: Connecting core servers.");
    NetDeviceContainer devices;
    PointToPointHelper p2pHelper;
    Ipv4AddressHelper address;
    char ipBase[128] = {0};

    p2pHelper.SetDeviceAttribute("DataRate", StringValue(CORE_LINE_RATE));
    p2pHelper.SetChannelAttribute("Delay", StringValue("1ms"));

    for (uint32_t i = 1; i < m_coreCloudNodes.GetN(); i++)
    {
        Ipv4InterfaceContainer interfaces;
        Ptr<CybertwinNode> prevNode = DynamicCast<CybertwinNode>(m_coreCloudNodes.Get(i - 1));
        Ptr<CybertwinNode> curNode = DynamicCast<CybertwinNode>(m_coreCloudNodes.Get(i));

        devices = p2pHelper.Install(prevNode, curNode);

        bzero(ipBase, sizeof(ipBase));
        sprintf(ipBase, CORE_CLOUD_DEFAULT_IP_FORMAT, i);
        address.SetBase(ipBase, CORE_CLOUD_DEFAULT_MASK);
        interfaces = address.Assign(devices);

        // set ip address
        prevNode->AddGlobalIp(interfaces.GetAddress(0));
        curNode->AddGlobalIp(interfaces.GetAddress(1));
        curNode->AddParent(prevNode);
    }

    // connect edge servers
    NS_LOG_INFO("STEP[2]: Connecting edge servers.");
    p2pHelper.SetDeviceAttribute("DataRate", StringValue(EDGE_LINE_RATE));
    p2pHelper.SetChannelAttribute("Delay", StringValue("1ms"));

    // do for each edge server
    int net_id = 0;
    for (auto& it : edgeConf.items())
    {
        nlohmann::json server = it.value();
        // get parent node
        std::string nodeName = EDGE_CLOUD_NODE_PREFIX + it.key();
        std::string linkType = server["link_type"];

        Ptr<Node> edgeNode = m_nodes.GetNodeByName(nodeName);
        if (edgeNode == nullptr)
        {
            NS_LOG_ERROR("Edge node does not exist");
            return -1;
        }

        // connect to parent node
        nlohmann::json parents = server["parents"];
        for (auto parent : parents)
        {
            std::string parentName = parent.get<std::string>();
            Ptr<Node> parentNode = m_nodes.GetNodeByName(parentName);
            Ipv4InterfaceContainer interfaces;
            if (parentNode == nullptr)
            {
                NS_LOG_ERROR("Parent node does not exist");
                return -1;
            }

            if (linkType == "p2p")
            {
                NS_LOG_DEBUG("link type: p2p");
                devices = p2pHelper.Install(edgeNode, parentNode);
                bzero(ipBase, sizeof(ipBase));
                snprintf(ipBase, sizeof(ipBase), EDGE_CLOUD_DEFAULT_IP_FORMAT, net_id++);
                address.SetBase(ipBase, EDGE_CLOUD_DEFAULT_MASK);
                interfaces = address.Assign(devices);
            }
            else if (linkType == "csma")
            {
                NS_LOG_ERROR("CSMA is not supported yet");
                return -1;
            }
            else
            {
                NS_LOG_ERROR("Unknown link type");
                return -1;
            }

            Ptr<CybertwinEdgeServer> edgeServer = DynamicCast<CybertwinEdgeServer>(edgeNode);
            edgeServer->AddParent(parentNode);
            edgeServer->SetAttribute("UpperNodeAddress",
                                     Ipv4AddressValue(interfaces.GetAddress(1)));
            edgeServer->AddGlobalIp(interfaces.GetAddress(0));
        }
    }

    NS_LOG_INFO("STEP[3]: Connecting end hosts.");
    // do for each end host
    net_id = 0;
    for (auto host : accessConf.items())
    {
        // get parent node
        nlohmann::json endhost = host.value();
        nlohmann::json parents = endhost["parents"];
        std::string linkType = endhost["link_type"];
        std::string nodeName = ACCESS_NET_NODE_PREFIX + host.key();

        Ptr<Node> endHost = m_nodes.GetNodeByName(nodeName);
        if (endHost == nullptr)
        {
            NS_LOG_ERROR("End host" << nodeName << " does not exist");
            return -1;
        }

        // connect to parent node
        for (auto parent : parents)
        {
            std::string parentName = parent.get<std::string>();
            Ptr<Node> parentNode = m_nodes.GetNodeByName(parentName);
            Ipv4InterfaceContainer interfaces;
            if (parentNode == nullptr)
            {
                NS_LOG_ERROR("Parent node does not exist");
                return -1;
            }

            if (linkType == "p2p")
            {
                devices = p2pHelper.Install(endHost, parentNode);
                bzero(ipBase, sizeof(ipBase));
                snprintf(ipBase, sizeof(ipBase), ACCESS_NET_DEFAULT_IP_FORMAT, net_id++);
                address.SetBase(ipBase, ACCESS_NET_DEFAULT_MASK);
                interfaces = address.Assign(devices);

                // set uppernode address
                DynamicCast<CybertwinNode>(endHost)->SetAttribute(
                    "UpperNodeAddress",
                    Ipv4AddressValue(interfaces.GetAddress(1)));
                DynamicCast<CybertwinNode>(parentNode)->AddLocalIp(interfaces.GetAddress(1));
                DynamicCast<CybertwinNode>(endHost)->AddLocalIp(interfaces.GetAddress(0));
                NS_LOG_INFO("+ Connecting " << nodeName << " to parent " << parentName << ": "
                                            << interfaces.GetAddress(0) << " -> "
                                            << interfaces.GetAddress(1));
            }
            else if (linkType == "csma")
            {
                NS_LOG_ERROR("CSMA is not supported yet");
                return -1;
            }
            else
            {
                NS_LOG_ERROR("Unknown link type");
                return -1;
            }

            DynamicCast<CybertwinEndHost>(endHost)->AddParent(parentNode);
        }
    }

    return 0;
}

int32_t
CybertwinSim::ParseNodeConfig(Ptr<CybertwinNode> node, std::string nodeConfigPath)
{
    // parse node configuration
    std::string confPath = nodeConfigPath + SYS_CONF_DIR_NAME;
    for (const auto& entry : fs::directory_iterator(confPath))
    {
        std::string fileName = entry.path().filename().string();
        NS_LOG_INFO("Config file: " << fileName);
        nlohmann::json conf;
        std::ifstream confFile(entry.path());
        confFile >> conf;
        confFile.close();

        node->AddConfigFile(fileName, conf);
    }
    return 0;
}

int32_t
CybertwinSim::ParseNodes()
{
    NS_LOG_INFO("\n======= Cybertwin::ParseNodes() =======\n\n");

    // parse nodes
    // ********************************  [1] Core Cloud **********************************************
    std::string core_cloud_conf_path = m_configFilePath + "/core_cloud/";
    std::string edge_cloud_conf_path = m_configFilePath + "/edge_cloud/";
    std::string access_net_conf_path = m_configFilePath + "/access_net/";

    if (!fs::exists(core_cloud_conf_path) || !fs::is_directory(core_cloud_conf_path))
    {
        NS_LOG_ERROR("Core cloud configuration path does not exist or is not a directory");
        return -1;
    }

    for (const auto& entry : fs::directory_iterator(core_cloud_conf_path))
    {
        if (fs::is_directory(entry))
        {
            std::string nodeName = CORE_CLOUD_NODE_PREFIX + entry.path().filename().string();
            NS_LOG_INFO("Core cloud node: " << nodeName);

            // create core server node
            Ptr<CybertwinCoreServer> coreServer = CreateObject<CybertwinCoreServer>();
            m_nodes.AddNode(nodeName, coreServer);
            m_coreCloudNodes.Add(coreServer);

            coreServer->SetName(nodeName);
            // parse node configuration
            ParseNodeConfig(coreServer, entry.path().string());

            // get log dir sbsolute path
            std::string logDir = entry.path().string() + SYS_LOG_DIR_NAME;
            coreServer->SetLogDir(logDir);
        }
    }

    // ********************************  [2] Edge Cloud **********************************************
    if (!fs::exists(EDGE_CLOUD_CONF_PATH) || !fs::is_directory(EDGE_CLOUD_CONF_PATH))
    {
        NS_LOG_ERROR("Edge cloud configuration path does not exist or is not a directory");
        return -1;
    }

    for (const auto& entry : fs::directory_iterator(EDGE_CLOUD_CONF_PATH))
    {
        if (fs::is_directory(entry))
        {
            std::string nodeName = EDGE_CLOUD_NODE_PREFIX + entry.path().filename().string();
            NS_LOG_INFO("Edge cloud node: " << nodeName);
            Ptr<CybertwinEdgeServer> edgeServer = CreateObject<CybertwinEdgeServer>();

            m_nodes.AddNode(nodeName, edgeServer);
            m_edgeCloudNodes.Add(edgeServer);

            edgeServer->SetName(nodeName);

            // parse node configuration
            ParseNodeConfig(edgeServer, entry.path().string());

            // get log dir sbsolute path
            std::string logDir = entry.path().string() + SYS_LOG_DIR_NAME;
            edgeServer->SetLogDir(logDir);
        }
    }

    // ********************************  [3] Access Network **********************************************
    if (!fs::exists(ACCESS_NET_CONF_PATH) || !fs::is_directory(ACCESS_NET_CONF_PATH))
    {
        NS_LOG_ERROR("Access network configuration path does not exist or is not a directory");
        return -1;
    }

    for (const auto& entry : fs::directory_iterator(ACCESS_NET_CONF_PATH))
    {
        if (fs::is_directory(entry))
        {
            std::string nodeName = ACCESS_NET_NODE_PREFIX + entry.path().filename().string();
            NS_LOG_INFO("Access network node: " << nodeName);
            Ptr<CybertwinEndHost> endHost = CreateObject<CybertwinEndHost>();

            m_nodes.AddNode(nodeName, endHost);
            m_endHostNodes.Add(endHost);

            endHost->SetName(nodeName);

            // parse node configuration
            ParseNodeConfig(endHost, entry.path().string());

            // get log dir sbsolute path
            std::string logDir = entry.path().string() + SYS_LOG_DIR_NAME;
            endHost->SetLogDir(logDir);
        }
    }

    return 0;
}

//**********************************************************************
//*                       CybertwinNodeContainer                       *
//**********************************************************************
CybertwinNodeContainer::CybertwinNodeContainer()
{
}

CybertwinNodeContainer::~CybertwinNodeContainer()
{
}

Ptr<Node>
CybertwinNodeContainer::GetNodeByName(std::string name)
{
    for (auto node : m_nodes)
    {
        if (node.nameStr == name)
        {
            return node.nodePtr;
        }
    }

    return nullptr;
}

int32_t
CybertwinNodeContainer::AddNode(std::string name, Ptr<Node> node)
{
    for (auto node : m_nodes)
    {
        if (node.nameStr == name)
        {
            NS_LOG_ERROR("Node " << name << " already exists");
            return -1;
        }
    }

    NodeInfo nodeInfo;
    nodeInfo.nameStr = name;
    nodeInfo.nodePtr = node;
    m_nodes.push_back(nodeInfo);

    return 0;
}

} // namespace ns3