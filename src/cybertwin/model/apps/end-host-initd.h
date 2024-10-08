#ifndef END_HOST_INITD_H
#define END_HOST_INITD_H

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-header.h"
#include "ns3/cybertwin-node.h"
#include "ns3/cybertwin-tag.h"
#include "ns3/end-host-bulk-send.h"

namespace ns3
{
class EndHostInitd : public Application
{
  public:
    EndHostInitd();
    ~EndHostInitd();

    static TypeId GetTypeId();

  private:
    void StartApplication();
    void StopApplication();

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

  private:
    //private member variables
    Ipv4Address m_proxyAddr;
    uint16_t m_proxyPort;
    Ptr<Socket> m_proxySocket;
    Ptr<Socket> m_cybertwinSocket;

    bool m_isConnectedToCybertwinManager;
    bool m_isRegisteredToCybertwin;
    bool m_isConnectedToCybertwin;
};
    
} // namespace ns

#endif // END_HOST_INITD_H