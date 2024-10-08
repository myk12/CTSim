#ifndef CYBERTWIN_NODE_H
#define CYBERTWIN_NODE_H

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-manager.h"
#include "ns3/cybertwin-name-resolution-service.h"
#include "ns3/download-server.h"
#include "ns3/download-client.h"
#include "ns3/end-host-initd.h"
#include "ns3/end-host-bulk-send.h"

#include <unordered_map>
#include <vector>

namespace ns3
{
class CybertwinManager;

class CybertwinNode : public Node
{
  public:
    CybertwinNode();
    ~CybertwinNode();

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;

    //setters
    virtual void SetAddressList(std::vector<Ipv4Address> addressList);
    virtual void SetName(std::string name);
    virtual void SetLogDir(std::string logDir);

    //getters
    virtual std::string GetLogDir();
    virtual std::string GetName();

    virtual Ipv4Address GetUpperNodeAddress();

    virtual void PowerOn();

    void AddParent(Ptr<Node> parent);
    void AddConfigFile(std::string filename, nlohmann::json config);
    void InstallCNRSApp();
    void InstallCybertwinManagerApp(std::vector<Ipv4Address> localIpv4AddrList,
                                    std::vector<Ipv4Address> globalIpv4AddrList);
    
    //*******************************************************
    //*             App parser and installer                *
    //*******************************************************
    void InstallUserApps();
    void InstallDownloadServer(nlohmann::json config);
    void InstallDownloadClient(nlohmann::json config);
    void InstallEndHostBulkSend(nlohmann::json config);

    Ptr<NameResolutionService> GetCNRSApp();

    void AddLocalIp(Ipv4Address);
    void AddGlobalIp(Ipv4Address);
    std::vector<Ipv4Address> GetLocalIpList();
    std::vector<Ipv4Address> GetGlobalIpList();

  protected:
    std::vector<Ipv4Address> ipv4AddressList;
    Ipv4Address m_upperNodeAddress; // ip address of default cybertwin controller
    Ipv4Address m_selfNodeAddress;  // ip address of the current node
    CYBERTWINID_t m_cybertwinId;

    std::vector<Ipv4Address> m_localAddrList;
    std::vector<Ipv4Address> m_globalAddrList;

    Ptr<NameResolutionService> m_cybertwinCNRSApp;
    std::string m_name;
    std::string m_logDir;
    std::vector<Ptr<Node>> m_parents;
    std::unordered_map<std::string, nlohmann::json> m_configFiles;
};

//**********************************************************************
//*               edge server node                                     *
//**********************************************************************
class CybertwinEdgeServer : public CybertwinNode
{
  public:
    CybertwinEdgeServer();
    ~CybertwinEdgeServer();

    static TypeId GetTypeId();

    void PowerOn();

    Ptr<CybertwinManager> GetCtrlApp();

  private:
    Ptr<NameResolutionService> m_cybertwinCNRSApp;
    Ptr<CybertwinManager> m_CybertwinManagerApp;
};

//**********************************************************************
//*               core server node                                     *
//**********************************************************************
class CybertwinCoreServer : public CybertwinNode
{
  public:
    CybertwinCoreServer();
    ~CybertwinCoreServer();

    static TypeId GetTypeId();

    void PowerOn() override;

  private:
    Ipv4Address CNRSUpNodeAddress;
    Ptr<Application> cybertwinCNRSApp;
};

//**********************************************************************
//*               end host node                                        *
//**********************************************************************

class CybertwinEndHost : public CybertwinNode
{
  public:
    CybertwinEndHost();
    ~CybertwinEndHost();

    static TypeId GetTypeId();

    void PowerOn() override;
    // void Connect(const CybertwinCertTag&);
    // void SendTo(CYBERTWINID_t, uint32_t size = 0);
    void SetCybertwinId(CYBERTWINID_t);
    CYBERTWINID_t GetCybertwinId();
    void SetCybertwinPort(uint16_t);
    uint16_t GetCybertwinPort();

    void SetCybertwinStatus(bool);
    bool isCybertwinCreated();

  private:
    // private member variables
    CYBERTWINID_t m_cybertwinId;
    uint16_t m_cybertwinPort;
    bool m_isConnected;

    Ptr<Socket> m_cybertwinSocket;
};

} // namespace ns3

#endif