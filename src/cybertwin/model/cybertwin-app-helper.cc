#include "ns3/cybertwin-app-helper.h"

#include "ns3/cybertwin-node.h"
#include "ns3/cybertwin-app-download-client.h"
#include "ns3/cybertwin-app-download-server.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("CybertwinAppHelper");

    CybertwinAppHelper::CybertwinAppHelper()
    {
        NS_LOG_FUNCTION(this);
    }

    CybertwinAppHelper::~CybertwinAppHelper()
    {
        NS_LOG_FUNCTION(this);
    }

    /**
     * Install download client on a node
     * @param parameters : YAML::Node
     * @param node : Ptr<Node>
     * @return void
     */
    void CybertwinAppHelper::InstallDownloadClient(YAML::Node parameters, Ptr<Node> node)
    {
        NS_LOG_INFO("Installing download client on node " << node->GetId());
        uint32_t startDelay = parameters["start-delay"].as<uint32_t>();
        CYBERTWINID_t targetId = parameters["target-cybertwin-id"].as<CYBERTWINID_t>();

        Ptr<CybertwinAppDownloadClient> downloadClient = CreateObject<CybertwinAppDownloadClient>();
        downloadClient->SetAttribute("TargetCybertwinId", UintegerValue(targetId));
        node->AddApplication(downloadClient);

        Ptr<CybertwinNode> cybertwinNode = DynamicCast<CybertwinNode>(node);
        cybertwinNode->AddInstalledApp(downloadClient, Seconds(startDelay));
    }

    /**
     * Install download server on a node
     * @param parameters : YAML::Node
     * @param node : Ptr<Node>
     */
    void CybertwinAppHelper::InstallDownloadServer(YAML::Node parameters, Ptr<Node> node)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Installing download server on node " << node->GetId());

        uint32_t startDelay = parameters["start-delay"].as<uint32_t>();
        CYBERTWINID_t cybertwinId = parameters["cybertwin-id"].as<CYBERTWINID_t>();
        uint16_t cybertwinPort = parameters["cybertwin-port"].as<uint16_t>();
        std::string maxSize = parameters["max-size"].as<std::string>();

        Ptr<CybertwinNode> cybertwinNode = node->GetObject<CybertwinNode>();
        std::vector<Ipv4Address> m_globalAddrList = cybertwinNode->GetGlobalIpList();

        CYBERTWIN_INTERFACE_LIST_t interfaces;
        for (auto& ip : m_globalAddrList)
        {
            interfaces.push_back(std::make_pair(ip, cybertwinPort));
            NS_LOG_DEBUG("interface: " << ip << ":" << cybertwinPort);
        }

        Ptr<CybertwinAppDownloadServer> downloadServer = CreateObject<CybertwinAppDownloadServer>(cybertwinId, interfaces);
        downloadServer->SetAttribute("CybertwinID", UintegerValue(cybertwinId));
        // Convert a human-readable string representation of size into 
        // an integer representing the size in bytes.
        uint32_t maxSizeBytes = 0;
        if (maxSize.find("MB") != std::string::npos)
        {
            maxSizeBytes = std::stoi(maxSize) * 1024 * 1024;
        }
        else if (maxSize.find("KB") != std::string::npos)
        {
            maxSizeBytes = std::stoi(maxSize) * 1024;
        }
        else
        {
            maxSizeBytes = std::stoi(maxSize);
        }
        downloadServer->SetAttribute("MaxBytes", UintegerValue(maxSizeBytes));
        node->AddApplication(downloadServer);
        cybertwinNode->AddInstalledApp(downloadServer, Seconds(startDelay));
    }

    /**
     * Install applications on a node
     * @param appname : std::string
     * @param paramters : YAML::Node
     */
    void CybertwinAppHelper::InstallApplications(std::string appname, YAML::Node paramters, Ptr<Node> node)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Installing app "<< appname << " on node " << node->GetId());

        if (appname == APPTYPE_DOWNLOAD_CLIENT) 
        {
            NS_LOG_INFO("Installing download client on node " << node->GetId());
            InstallDownloadClient(paramters, node);
        }
        else if (appname == APPTYPE_DOWNLOAD_SERVER)
        {
            NS_LOG_INFO("Installing download server on node " << node->GetId());
            InstallDownloadServer(paramters, node);
        }else {
            NS_LOG_ERROR("Unknown app name: " << appname);
        }
    }

    /**
     * Install applications on a list of nodes
     * @param appname : std::string
     * @param appParamters : YAML::Node
     * @param nodes : NodeContainer
     * @return void
     */
    void CybertwinAppHelper::InstallApplications(std::string appname, YAML::Node appParamters, NodeContainer nodes)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Installing app "<< appname << " on " << nodes.GetN() << " nodes...");
        for (uint32_t i=0; i<nodes.GetN(); i++)
        {
            Ptr<Node> node = nodes.Get(i);
            InstallApplications(appname, appParamters, node);
        }
    }
} // namespace ns3
