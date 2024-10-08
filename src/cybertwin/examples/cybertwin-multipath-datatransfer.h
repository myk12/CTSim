#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/cybertwin-common.h"
#include "ns3/random-variable-stream.h"

#include "ns3/multipath-data-transfer-protocol.h"
#include "ns3/cybertwin-name-resolution-service.h"

namespace ns3
{

class MultipathDataTransferApp: public Application
{
public:
    static TypeId GetTypeId();
    void SetRemoteInterface(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t interfaces);
    void SetLocalInterface(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t interfaces);
    void SetRole(bool client);
    void RecvHandler(MultipathConnection* conn);
    void ConnectSucceedHandler(MultipathConnection* conn);
    void ConnectFailedHandler(MultipathConnection* conn);
    void ConnectCloseHandler(MultipathConnection* conn);
    void ClientConnectSucceedHandler(MultipathConnection* conn);
    void ServerConnectSucceedHandler(MultipathConnection* conn);


    //test data transportation
    void PeriodicSender(MultipathConnection* conn);

private:
    void StartApplication();
    void StopApplication();

    void test_client();
    void test_server();

    bool is_client;

    CYBERTWINID_t m_localCyberID;
    CYBERTWINID_t m_remoteCyberID;
    CYBERTWIN_INTERFACE_LIST_t m_localIfs;
    CYBERTWIN_INTERFACE_LIST_t m_remoteIfs;

    Ptr<UniformRandomVariable> rand;
    CybertwinDataTransferServer* m_dataServer;
};

} // namespace ns3