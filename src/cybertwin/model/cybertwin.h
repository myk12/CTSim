#ifndef CYBERTWIN_H
#define CYBERTWIN_H

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-header.h"
#include "ns3/cybertwin-node.h"
#include "ns3/multipath-data-transfer-protocol.h"
#include "ns3/cybertwin-name-resolution-service.h"
#include "ns3/cybertwin-tag.h"
#include "ns3/cybertwin-app.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/callback.h"

#include <queue>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

namespace ns3
{
class CybertwinEdgeServer;
class MultipathConnection;
class CybertwinDataTransferServer;
//**********************************************************************
//*                 Cybertwin Full Duplex Stream                       *
//**********************************************************************
class CybertwinFullDuplexStream: public Object
{
  public:
    enum ENDPOINT_STATUS
    {
        ENDPOINT_NONE,
        ENDPOINT_CONNECTING,
        ENDPOINT_CONNECTED,
        ENDPOINT_DISCONNECTED,
        ENDPOINT_DONE,
        ENDPOINT_END_STOP,
    };
    static TypeId GetTypeId();

    CybertwinFullDuplexStream();
    CybertwinFullDuplexStream(Ptr<Node> node,
                            Ptr<NameResolutionService> cnrs,
                            CYBERTWINID_t end,
                            CYBERTWINID_t cloud);
    ~CybertwinFullDuplexStream();
    void Activate();
    void Deactivate();

    void SetEndID(CYBERTWINID_t id);
    void SetCloudID(CYBERTWINID_t id);
    void SetEndSocket(Ptr<Socket> sock);
    void SetCloudSocket(Ptr<Socket> sock);

  private:
    void DuplexStreamEndRecvCallback(Ptr<Socket>);
    // Buffer Output
    void OuputCloudBuffer();
    void OuputEndBuffer();

    // Cloud related
    void DuplexStreamCloudConnect(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t ifs);
    void DuplexStreamCloudConnectCallback(Ptr<Socket>);
    void DuplexStreamCloudConnectErrorCallback(Ptr<Socket>);
    void DuplexStreamCloudRecvCallback(Ptr<Socket>);
    void DuplexStreamCloudNormalCloseCallback(Ptr<Socket>);
    void DuplexStreamCloudErrorCloseCallback(Ptr<Socket>);


    // End related
    //void DuplexStreamEndConnect(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t ifs);
    void DuplexStreamEndNormalCloseCallback(Ptr<Socket>);
    void DuplexStreamEndErrorCloseCallback(Ptr<Socket>);

  private:
    Ptr<Node> m_node;
    Ptr<NameResolutionService> m_cnrs;
    Ptr<Socket> m_endSocket;
    Ptr<Socket> m_cloudSocket;
    CYBERTWINID_t m_endID;
    CYBERTWINID_t m_cloudID;
    ENDPOINT_STATUS m_endStatus;
    ENDPOINT_STATUS m_cloudStatus;
    std::queue<Ptr<Packet>> m_endBuffer;
    std::queue<Ptr<Packet>> m_cloudBuffer;

    EventId m_sendToCloudEvent;
    uint64_t m_sendToCloudBytes;
    Time m_cloudStartTime;
    EventId m_sendToEndEvent;
    uint64_t m_sendToEndBytes;
    Time m_endStartTime;

    double m_cloudRateLimit;
    double m_endRateLimit;
};

//**********************************************************************
//*                 Cybertwin Application                              *
//**********************************************************************
class Cybertwin : public CybertwinApp
{
  public:
    typedef uint128_t STREAMID_t;
#if MDTP_ENABLED
    typedef Callback<int, CYBERTWINID_t, MultipathConnection*, Ptr<const Packet>>
        CybertwinSendCallback;
#else
    typedef Callback<int, CYBERTWINID_t, Ptr<Socket>, Ptr<const Packet>>
        CybertwinSendCallback;
#endif

    Cybertwin();
    Cybertwin(CYBERTWINID_t,
              CYBERTWIN_INTERFACE_t l_interface,
              CYBERTWIN_INTERFACE_LIST_t g_interfaces);

    ~Cybertwin();

    static TypeId GetTypeId();
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    // Cybertwin v2.0
    void StartCybertwinDownloadProcess(Ptr<Socket> sock, CYBERTWINID_t id);
    void StartDownloadProcess(Ptr<Socket> sock, CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t interfaces);

    // locally
    void LocallyListen();
    bool LocalConnRequestCallback(Ptr<Socket>, const Address&);
    void LocalConnCreatedCallback(Ptr<Socket>, const Address&);
    void LocalNormalCloseCallback(Ptr<Socket>);
    void LocalErrorCloseCallback(Ptr<Socket>);

    void LocalRecvCallback(Ptr<Socket>);
    void LocallyForward();

    // globally
    void GloballyListen();

#if MDTP_ENABLED
    void NewMpConnectionCreatedCallback(MultipathConnection* conn);
    void NewMpConnectionErrorCallback(MultipathConnection* conn);
    void MpConnectionRecvCallback(MultipathConnection* conn);
    void MpConnectionClosedCallback(MultipathConnection* conn);

    void DownloadFileServer(MultipathConnection* conn);
#else
    bool NewSpConnectionRequestCallback(Ptr<Socket> sock);
    void NewSpConnectionCreatedCallback(Ptr<Socket> sock);
    void NewSpConnectionErrorCallback(Ptr<Socket> sock);
    void SpConnectionRecvCallback(Ptr<Socket> sock);
    void SpNormalCloseCallback(Ptr<Socket> sock);
    void SpErrorCloseCallback(Ptr<Socket> sock);
#endif

    void SendPendingPackets(STREAMID_t streamid);
    void SocketConnectWithResolvedCybertwinName(Ptr<Socket> sock,
                                                CYBERTWINID_t cyberid,
                                                CYBERTWIN_INTERFACE_LIST_t ifs);

    CybertwinSendCallback SendPacket;

    // buffer for handling packet fragments; also used for recording all accepted sockets
    std::unordered_map<Ptr<Socket>, Ptr<Packet>> m_streamBuffer;
    std::unordered_map<CYBERTWINID_t, Ptr<Socket>> m_txBuffer;

    // data transmission between cybertwins
    std::unordered_map<CYBERTWINID_t, std::queue<Ptr<Packet>>> m_txPendingBuffer;
#if MDTP_ENABLED // Multipath Connection
    std::unordered_map<CYBERTWINID_t, MultipathConnection*> m_txConnections;
    std::unordered_map<CYBERTWINID_t, MultipathConnection*> m_pendingConnections;
    std::unordered_map<CYBERTWINID_t, MultipathConnection*> m_rxConnections;
    std::unordered_map<CYBERTWINID_t, std::queue<Ptr<Packet>>> m_rxPendingBuffer;
    std::unordered_map<CYBERTWINID_t, TracedValue<uint64_t>> m_rxSizePerSecond;
#else  // Naive Socket
    //std::unordered_map<STREAMID_t, Ptr<Socket>> m_txConnections;
    std::unordered_map<Ptr<Socket>, CYBERTWINID_t> m_txConnectionsReverse;
    //std::unordered_map<CYBERTWINID_t, Ptr<Socket>> m_pendingConnections;
    //std::unordered_map<Ptr<Socket>, CYBERTWINID_t> m_pendingConnectionsReverse;
    std::unordered_set<Ptr<Socket>> m_rxConnections;
    std::unordered_map<Ptr<Socket>, Address> m_rxConnectionsReverse;
    std::unordered_map<Ptr<Socket>, std::queue<Ptr<Packet>>> m_rxPendingBuffer;
    std::unordered_map<Ptr<Socket>, TracedValue<uint64_t>> m_rxSizePerSecond;
#endif
private:
    // tx buffer
    std::unordered_map<STREAMID_t, std::queue<Ptr<Packet>>> m_txStreamBuffer;
    std::vector<STREAMID_t> m_txStreamBufferOrder;
    uint32_t m_lastTxStreamBufferOrder;

    std::unordered_map<STREAMID_t, Ptr<Socket>> m_txConnections;    //established connections
    std::unordered_map<STREAMID_t, Ptr<Socket>> m_pendingConnections;   //pending connections
    std::unordered_map<Ptr<Socket>, STREAMID_t> m_pendingConnectionsReverse;

    std::string m_nodeName;

private:
    // private member data    
    CYBERTWINID_t m_cybertwinId;
    Address m_address;

    // end related
    Ptr<Socket> m_localSocket;
    CYBERTWIN_INTERFACE_t m_localInterface;

    // cloud related
    CYBERTWIN_INTERFACE_LIST_t m_globalInterfaces;

    Ptr<NameResolutionService> m_cnrs;
    std::unordered_map<CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t> nameResolutionCache;
    // Cybertwin multiple interfaces

    std::vector<Ptr<CybertwinFullDuplexStream>> m_streams;

#if MDTP_ENABLED
    // Cybertwin Connections
    CybertwinDataTransferServer* m_dtServer;
#else
    Ptr<Socket> m_dtServer;
#endif
    uint32_t m_noneDataCnt;

    uint64_t m_serverTxBytes; //number of sent times

    std::ofstream m_MpLogFile;
    std::string m_MpLogFileName;

    // statistic
    void CybertwinCommModelStatistical();
    //************************************************************
    //*                     traffic shaping                      *
    //************************************************************
    std::queue<Ptr<Packet>> m_tsPktQueue; // received packet queue
    void CybertwinCommModelTrafficShaping();
    void GenerateToken(double);
    void ConsumeToken();
    void TrafficShapingStatistical();
    Time m_lastShapingTime;
    void StopTrafficShaping();
    EventId m_tokenGeneratorEvent;
    EventId m_consumerEvent;
    EventId m_statisticalEvent;
    uint64_t m_tokenBucket;
    uint64_t m_consumeBytes;
    bool m_statisticalEnd;

    //************************************************************
    //*                     traffic policing                     *
    //************************************************************
    std::queue<Ptr<Packet>> m_tpPktQueue; // received packet queue
    void CybertwinCommModelTrafficPolicing();
    void CCMTrafficPolicingConsumePacket();
    void CCMTrafficPolicingStatistical();
    void CCMTrafficPolicingStop();
    Time m_startPolicingTime;
    Time m_lastTpStartTime;
    Time m_lastTpStaticTime;
    uint64_t m_tpTotalConsumeBytes;
    uint64_t m_tpTotalDropedBytes;
    uint64_t m_intervalTpBytes;
    EventId m_tpConsumeEvent;
    EventId m_tpStatisticEvent;



    bool m_isStartTrafficOpt;
    uint64_t m_comm_test_total_bytes;
    uint64_t m_comm_test_interval_bytes;
    Time m_startShapingTime;
    Time m_lastTime;
    Time m_endTime;

    // CybertwinV2 download request
    std::unordered_map<Ptr<Socket>, Ptr<Socket>> m_cloud2endSockMap;
    std::unordered_map<Ptr<Socket>, Ptr<Socket>> m_end2cloudSockMap;
    void DownloadSocketAcceptCallback(Ptr<Socket>, const Address&);
    void DownloadSocketCreatedCallback(Ptr<Socket>, const Address&);
    void DownloadSocketRecvCallback(Ptr<Socket>);
    void DownloadSocketNormalCloseCallback(Ptr<Socket>);
    void DownloadSocketErrorCloseCallback(Ptr<Socket>);

    std::vector<Ptr<Socket>> m_socketList;
};

}; // namespace ns3

#endif