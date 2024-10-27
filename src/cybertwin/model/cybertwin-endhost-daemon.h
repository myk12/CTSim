#ifndef _CYBERTWIN_ENDHOST_DAEMON_H_
#define _CYBERTWIN_ENDHOST_DAEMON_H_

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-header.h"
#include "ns3/cybertwin-node.h"
#include "ns3/cybertwin-tag.h"
#include "ns3/end-host-bulk-send.h"

namespace ns3
{
class CybertwinEndHostDaemon : public Application
{
  public:
    CybertwinEndHostDaemon();
    ~CybertwinEndHostDaemon();

    static TypeId GetTypeId();

    Ipv4Address GetManagerAddr();
    uint16_t GetManagerPort();
    uint16_t GetCybertwinPort();

    bool IsRegisteredToCybertwin();

  private:
    void StartApplication();
    void StopApplication();

    // --------------------------------------------------------
    //        Reigister a cybertiwn to Cybertwin Manager
    // --------------------------------------------------------
    void RegisterCybertwin();
    void ConnectCybertwinManager();

    // callbacks
    void ConnectCybertwinManangerSucceededCallback(Ptr<Socket>);
    void ConnectCybertwinManangerFailedCallback(Ptr<Socket>);

    void RecvFromCybertwinManangerCallback(Ptr<Socket>);

    // cybertwin registration
    void RegisterSuccessHandler(Ptr<Socket>, Ptr<Packet>);
    void RegisterFailureHandler(Ptr<Socket>, Ptr<Packet>);
    void Authenticate();

    //private member variables
    Ipv4Address m_managerAddr;
    uint16_t m_managerPort;
    Ptr<Socket> m_proxySocket;
    Ptr<Socket> m_cybertwinSocket;

    bool m_isConnectedToCybertwinManager;
    bool m_isRegisteredToCybertwin;
    bool m_isConnectedToCybertwin;

    std::string m_nodeName;

    CYBERTWINID_t m_cybertwinId;
    uint16_t m_cybertwinPort;
};
    
} // namespace ns

#endif // _CYBERTWIN_ENDHOST_DAEMON_H_
