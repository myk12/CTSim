#ifndef _CYBERTWIN_APP_DOWNLOAD_CLIENT_H_
#define _CYBERTWIN_APP_DOWNLOAD_CLIENT_H_

#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include "ns3/cybertwin-node.h"
#include "ns3/cybertwin-endhost-daemon.h"

namespace ns3
{

class CybertwinAppDownloadClient : public Application
{
  public:
    static TypeId GetTypeId();

    CybertwinAppDownloadClient();
    ~CybertwinAppDownloadClient();

  private:
    void StartApplication();
    void StopApplication();

    void ConnectSucceededCallback(Ptr<Socket>);
    void ConnectFailedCallback(Ptr<Socket>);

    void NormalCloseCallback(Ptr<Socket>);
    void ErrorCloseCallback(Ptr<Socket>);

    void RecvCallback(Ptr<Socket>);
    void SendDownloadRequest(Ptr<Socket>);

    Ptr<CybertwinEndHost> m_endHost;
    Ipv4Address m_cybertwinAddr;
    uint16_t m_cybertwinPort;

    Ptr<Socket> m_socket;
    std::string m_nodeName;

    CYBERTWINID_t m_targetCybertwinId;
    CYBERTWINID_t m_selfCybertwinId;
};

} // namespace ns3

#endif // _CYBERTWIN_APP_DOWNLOAD_CLIENT_H_