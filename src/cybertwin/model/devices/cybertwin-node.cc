#include "cybertwin-node.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinNode");
NS_OBJECT_ENSURE_REGISTERED(CybertwinNode);

TypeId
CybertwinNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinNode")
            .SetParent<Node>()
            .SetGroupName("Cybertwin")
            .AddConstructor<CybertwinNode>()
            .AddAttribute("UpperNodeAddress",
                          "The address of the upper node",
                          Ipv4AddressValue(),
                          MakeIpv4AddressAccessor(&CybertwinNode::m_upperNodeAddress),
                          MakeIpv4AddressChecker())
            .AddAttribute("SelfNodeAddress",
                          "The address of the current node",
                          Ipv4AddressValue(),
                          MakeIpv4AddressAccessor(&CybertwinNode::m_selfNodeAddress),
                          MakeIpv4AddressChecker())
            .AddAttribute("SelfCuid",
                          "The cybertwin id of the current node",
                          UintegerValue(),
                          MakeUintegerAccessor(&CybertwinNode::m_cybertwinId),
                          MakeUintegerChecker<uint64_t>());
    return tid;
};

TypeId
CybertwinNode::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinNode::CybertwinNode()
{
    NS_LOG_DEBUG("Created a CybertwinNode.");
}

CybertwinNode::~CybertwinNode()
{
    NS_LOG_DEBUG("Destroyed a CybertwinNode.");
}

void
CybertwinNode::PowerOn()
{
    NS_LOG_DEBUG("Setup a CybertwinNode.");
}

void
CybertwinNode::SetAddressList(std::vector<Ipv4Address> addressList)
{
    ipv4AddressList = addressList;
}

void
CybertwinNode::SetName(std::string name)
{
    m_name = name;
}

void
CybertwinNode::SetLogDir(std::string logDir)
{
    m_logDir = logDir;
}

std::string
CybertwinNode::GetLogDir()
{
    return m_logDir;
}

std::string
CybertwinNode::GetName()
{
    return m_name;
}

void
CybertwinNode::AddParent(Ptr<Node> parent)
{
    for (auto it = m_parents.begin(); it != m_parents.end(); ++it)
    {
        if (*it == parent)
        {
            return;
        }
    }
    m_parents.push_back(parent);
}

Ipv4Address
CybertwinNode::GetUpperNodeAddress()
{
    return m_upperNodeAddress;
}

void
CybertwinNode::AddLocalIp(Ipv4Address localIp)
{
    m_localAddrList.push_back(localIp);
}

void
CybertwinNode::AddGlobalIp(Ipv4Address globalIp)
{
    m_globalAddrList.push_back(globalIp);
}

std::vector<Ipv4Address>
CybertwinNode::GetLocalIpList()
{
    return m_localAddrList;
}

std::vector<Ipv4Address>
CybertwinNode::GetGlobalIpList()
{
    return m_globalAddrList;
}

void
CybertwinNode::InstallCNRSApp()
{
    NS_LOG_DEBUG("[Load][InstallCNRSApp] Installing CNRS app to " << m_name << ".");
    Ptr<NameResolutionService> cybertwinCNRSApp = nullptr;
    if (!m_upperNodeAddress.IsInitialized() && m_parents.size() == 0)
    {
        NS_LOG_DEBUG("[Load][InstallCNRSApp] CNRS Root node " << m_name << ".");
        cybertwinCNRSApp = CreateObject<NameResolutionService>();
    }
    else
    {
        if (!m_upperNodeAddress.IsInitialized())
        {
            m_upperNodeAddress = m_parents[0]->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
        }

        // install CNRS application
        NS_LOG_DEBUG("[Load][InstallCNRSApp] CNRS Child node " << m_name << ".");
        cybertwinCNRSApp = CreateObject<NameResolutionService>(m_upperNodeAddress);
    }

    this->AddApplication(cybertwinCNRSApp);
    cybertwinCNRSApp->SetStartTime(Simulator::Now());
    m_cybertwinCNRSApp = cybertwinCNRSApp;
}

void
CybertwinNode::InstallCybertwinManagerApp(std::vector<Ipv4Address> localIpList,
                                          std::vector<Ipv4Address> globalIpList)
{
    NS_LOG_DEBUG("Installing Cybertwin Manager app.");
    if (!m_upperNodeAddress.IsInitialized())
    {
        if (m_parents.size() == 0)
        {
            NS_LOG_WARN("No parent node found.");
            return;
        }
        else
        {
            Ptr<Node> upperNode = m_parents[0];
            m_upperNodeAddress = upperNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
        }
    }

    // install Cybertwin Manager application
    Ptr<CybertwinManager> cybertwinManagerApp =
        CreateObject<CybertwinManager>(localIpList, globalIpList);
    this->AddApplication(cybertwinManagerApp);
    cybertwinManagerApp->SetStartTime(Simulator::Now());
}

void
CybertwinNode::AddConfigFile(std::string filename, nlohmann::json config)
{
    m_configFiles[filename] = config;
}

Ptr<NameResolutionService>
CybertwinNode::GetCNRSApp()
{
    return m_cybertwinCNRSApp;
}

void
CybertwinNode::InstallUserApps()
{
    NS_LOG_DEBUG("Installing user applications.");
    nlohmann::json appConfig;
    if (m_configFiles.find(APP_CONF_FILE_NAME) == m_configFiles.end())
    {
        NS_LOG_WARN("No app.json found.");
        return;
    }

    appConfig = m_configFiles[APP_CONF_FILE_NAME];
    NS_LOG_DEBUG("appConfig: " << appConfig.dump(4));

    for (auto& app : appConfig["app-list"])
    {
        std::string type = app["type"];
        if (type == APPTYPE_ENDHOST_BULK_SEND)
        {
            InstallEndHostBulkSend(app);
        }
        else if (type == APPTYPE_ENDHOST_INITD)
        {
            NS_LOG_DEBUG("end-host-initd is not implemented yet.");
        }
        else if (type == APPTYPE_DOWNLOAD_SERVER)
        {
            InstallDownloadServer(app);
        }
        else if (type == APPTYPE_DOWNLOAD_CLIENT)
        {
            InstallDownloadClient(app);
        }
        else
        {
            NS_LOG_WARN("Unknown app type.");
        }
    }
}

void
CybertwinNode::InstallEndHostBulkSend(nlohmann::json config)
{
    NS_LOG_DEBUG(m_name << " - Installing EndHostBulkSend.");
    int32_t enable = config["enabled"];
    if (enable == 0)
    {
        NS_LOG_DEBUG("EndHostBulkSend is disabled.");
        return;
    }

    uint32_t startDelay = config["start-delay"];

    Ptr<EndHostBulkSend> endHostBulkSend = CreateObject<EndHostBulkSend>();
    this->AddApplication(endHostBulkSend);
    endHostBulkSend->SetStartTime(Simulator::Now() + Seconds(startDelay));
}

void
CybertwinNode::InstallDownloadServer(nlohmann::json config)
{
    NS_LOG_DEBUG(m_name <<" - Installing Download Server.");
    int32_t enable = config["enabled"];
    if (enable == 0)
    {
        NS_LOG_DEBUG("DownloadServer is disabled.");
        return;
    }

    CYBERTWINID_t cybertwinId = config["cybertwin-id"];
    uint16_t cybertwinPort = config["cybertwin-port"];
    uint32_t startDelay = config["start-delay"];
    nlohmann::json trafficParams = config["traffic-params"];
    uint32_t maxBytes = config["file-size-MB"];

    CYBERTWIN_INTERFACE_LIST_t interfaces;
    for (auto& ip : m_globalAddrList)
    {
        interfaces.push_back(std::make_pair(ip, cybertwinPort));
        NS_LOG_DEBUG("interface: " << ip << ":" << cybertwinPort);
    }

    NS_LOG_DEBUG("cybertwinId: " << cybertwinId);
    Ptr<DownloadServer> downloadServer = CreateObject<DownloadServer>(cybertwinId, interfaces);
    this->AddApplication(downloadServer);

    downloadServer->SetAttribute("CybertwinID", UintegerValue(cybertwinId));
    downloadServer->SetAttribute("Pattern", StringValue(trafficParams["pattern"]));
    downloadServer->SetAttribute("Duration", TimeValue(MilliSeconds(trafficParams["duration"])));
    downloadServer->SetAttribute("Rate", DoubleValue(trafficParams["rate"]));
    downloadServer->SetAttribute("MaxBytes", UintegerValue(maxBytes));
    downloadServer->SetStartTime(Simulator::Now() + Seconds(startDelay));
}

void
CybertwinNode::InstallDownloadClient(nlohmann::json config)
{
    NS_LOG_DEBUG(m_name << " - Installing Download Client.");
    int32_t enable = config["enabled"];
    if (enable == 0)
    {
        NS_LOG_DEBUG("DownloadClient is disabled.");
        return;
    }

    uint32_t startDelay = config["start-delay"];
    nlohmann::json targetLists = config["target-cybertwins"];
    uint8_t maxOfflineTime = config["max-offline-time"];

    Ptr<DownloadClient> downloadClient = CreateObject<DownloadClient>();
    this->AddApplication(downloadClient);
    for (auto& target : targetLists)
    {
        CYBERTWINID_t targetId = target["cybertwinID"];
        int rate = target["recvRate"];
        NS_LOG_DEBUG("targetId: " << targetId << " rate: " << rate);
        downloadClient->AddTargetServer(targetId, rate);
    }

    downloadClient->SetAttribute("MaxOfflineTime", UintegerValue(maxOfflineTime));
    downloadClient->SetStartTime(Simulator::Now() + Seconds(startDelay));
}

//***************************************************************
//*               edge server node                              *
//***************************************************************

NS_OBJECT_ENSURE_REGISTERED(CybertwinEdgeServer);

TypeId
CybertwinEdgeServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinEdgeServer")
                            .SetParent<CybertwinNode>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinEdgeServer>();

    return tid;
}

CybertwinEdgeServer::CybertwinEdgeServer()
    : m_cybertwinCNRSApp(nullptr),
      m_CybertwinManagerApp(nullptr)
{
}

CybertwinEdgeServer::~CybertwinEdgeServer()
{
}

void
CybertwinEdgeServer::PowerOn()
{
    NS_LOG_DEBUG("-------------- Powering on " << m_name);
    if (m_parents.size() == 0)
    {
        NS_LOG_ERROR("Edge server should have parents.");
        return;
    }

    InstallCNRSApp();

    // install Cybertwin Controller application
    InstallCybertwinManagerApp(m_localAddrList, m_globalAddrList);

    InstallUserApps();
}

Ptr<CybertwinManager>
CybertwinEdgeServer::GetCtrlApp()
{
    return m_CybertwinManagerApp;
}

//***************************************************************
//*               core server node                              *
//***************************************************************
NS_OBJECT_ENSURE_REGISTERED(CybertwinCoreServer);

TypeId
CybertwinCoreServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinCoreServer")
                            .SetParent<Node>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinCoreServer>();
    return tid;
}

CybertwinCoreServer::CybertwinCoreServer()
    : cybertwinCNRSApp(nullptr)
{
    NS_LOG_DEBUG("Creating a CybertwinCoreServer.");
}

void
CybertwinCoreServer::PowerOn()
{
    NS_LOG_DEBUG("************** Powering on " << m_name);

    InstallCNRSApp();
}

CybertwinCoreServer::~CybertwinCoreServer()
{
    NS_LOG_DEBUG("[CybertwinCoreServer] destroy CybertwinCoreServer.");
}

//***************************************************************
//*               end host node                                 *
//***************************************************************
NS_OBJECT_ENSURE_REGISTERED(CybertwinEndHost);

TypeId
CybertwinEndHost::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinEndHost")
                            .SetParent<CybertwinNode>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinEndHost>();
    return tid;
}

CybertwinEndHost::CybertwinEndHost()
{
    NS_LOG_DEBUG("[CybertwinEndHost] create CybertwinEndHost.");
    m_isConnected = false;
}

CybertwinEndHost::~CybertwinEndHost()
{
    NS_LOG_DEBUG("[CybertwinEndHost] destroy CybertwinEndHost.");
}

void
CybertwinEndHost::PowerOn()
{
    NS_LOG_DEBUG("-+-+-+-+-+-+ Powering on " << m_name);

    // Create initd
    Ptr<EndHostInitd> initd = CreateObject<EndHostInitd>();
    initd->SetAttribute("ProxyAddr", Ipv4AddressValue(m_upperNodeAddress));
    // initd->SetAttribute("ProxyPort", UintegerValue(m_proxyPort));
    AddApplication(initd);
    initd->SetStartTime(Seconds(0.0));

    InstallUserApps();
}

void
CybertwinEndHost::SetCybertwinId(CYBERTWINID_t id)
{
    m_cybertwinId = id;
}

CYBERTWINID_t
CybertwinEndHost::GetCybertwinId()
{
    return m_cybertwinId;
}

void
CybertwinEndHost::SetCybertwinPort(uint16_t port)
{
    NS_LOG_DEBUG("[CybertwinEndHost] SetCybertwinPort: " << port);
    m_cybertwinPort = port;
}

uint16_t
CybertwinEndHost::GetCybertwinPort()
{
    return m_cybertwinPort;
}

void
CybertwinEndHost::SetCybertwinStatus(bool stat)
{
    m_isConnected = stat;
}

bool
CybertwinEndHost::isCybertwinCreated()
{
    return m_isConnected;
}

} // namespace ns3
