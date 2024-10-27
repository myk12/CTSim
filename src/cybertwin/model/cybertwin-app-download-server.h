#ifndef _CYBERTWIN_APP_DOWNLOAD_SERVER_H_
#define _CYBERTWIN_APP_DOWNLOAD_SERVER_H_

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-node.h"

namespace ns3
{

class CybertwinAppDownloadServer : public Application
{
public:
    CybertwinAppDownloadServer();
    CybertwinAppDownloadServer(CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t);
    ~CybertwinAppDownloadServer();

    static TypeId GetTypeId();

private:
    void StartApplication();
    void StopApplication();

    // callbacks
    bool ConnRequestCallback(Ptr<Socket>, const Address &);
    void ConnCreatedCallback(Ptr<Socket>, const Address &);
    void NormalCloseCallback(Ptr<Socket>);
    void ErrorCloseCallback(Ptr<Socket>);

    void RecvCallback(Ptr<Socket>);
    void SendData(Ptr<Socket>);

    uint64_t m_cybertwinID;
    CYBERTWIN_INTERFACE_LIST_t m_interfaces;

    uint64_t m_maxBytes;

    // server listener
    Ptr<Socket> m_serverSocket;
    uint16_t m_serverPort;
    
    std::unordered_map<Ptr<Socket>, uint64_t> m_sendBytes;
};

} // namespace ns3

#endif // _CYBERTWIN_APP_DOWNLOAD_SERVER_H_