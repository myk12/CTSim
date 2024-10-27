#ifndef CYBERTWIN_NAME_RESOLUTION_SERVICE_H
#define CYBERTWIN_NAME_RESOLUTION_SERVICE_H
#include "../cybertwin-common.h"
#include "../cybertwin-header.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ns3
{
typedef std::pair<Ptr<Socket>, Address> PeerInfo_t;

class NameResolutionService : public Application
{
  public:
    NameResolutionService();
    NameResolutionService(Ipv4Address superior);
    ~NameResolutionService();
    static TypeId GetTypeId();

    void SetSuperior(Ipv4Address superior);
    void DefaultGetInterfaceCallback(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t ifs);

    /**
     * @brief get cybertwin interface by name call by application
     *
     * @param name  cybertwin name
     * @param callback  callback when get result
     *
     * @return int32_t 0: success, -1: fail
     */
    int32_t GetCybertwinInterfaceByName(CYBERTWINID_t name,
                                        Callback<void, CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t>);
    int32_t InsertCybertwinInterfaceName(CYBERTWINID_t name, CYBERTWIN_INTERFACE_LIST_t& interface);

  private:
    void StartApplication() override;
    void StopApplication() override;

    void LoadDatabase();
    void InitNameResolutionServer();
    void InitSuperior();
    int32_t InitClientUDPSocket();

    void ServiceRecvHandler(Ptr<Socket> socket);
    void ClientRecvHandler(Ptr<Socket> socket);

    void ProcessQuery(CNRSHeader& rcvHeader, Ptr<Socket> socekt, Address& from);
    void QueryResponseHandler(bool status, CNRSHeader& rcvHeader);
    void ProcessInsert(CNRSHeader& rcvHeader, Ptr<Socket> socket);
    void ReportName2Superior(CYBERTWINID_t id, CYBERTWIN_INTERFACE_LIST_t interfaces);

    void InformCNRSResult(CYBERTWINID_t id, QUERY_ID_t qId, Ptr<Socket> socket, Address& from);

    /**
     * @brief query superior for cybertwin interface
     *
     * @param id cybertwin id
     * @param qid query id
     *
     */
    int32_t QuerySuperior(CYBERTWINID_t id, QUERY_ID_t qid);
    void QueryResponseCallback(CYBERTWINID_t, QUERY_ID_t, CYBERTWIN_INTERFACE_LIST_t);
    void QuerySuperiorFail(CYBERTWINID_t id, QUERY_ID_t qid);
    // void QuerySuperiorSuccess(QUERY_ID qid, CYBERTWIN_INTERFACE_LIST_t interfaces);
    void QueryResponse(PeerInfo_t peerInfo, CYBERTWINID_t, QUERY_ID_t, CYBERTWIN_INTERFACE_LIST_t);

    QUERY_ID_t GetQueryID();

    Ptr<Socket> serviceSocket;
    Ptr<Socket> clientSocket;
    uint16_t m_port;
    Ipv4Address m_superior;
    std::string databaseName;
    std::unordered_map<CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t> itemCache;

    std::unordered_map<QUERY_ID_t, Callback<void, CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t>>
        m_queryCache;
    std::unordered_map<QUERY_ID_t, PeerInfo_t> m_queryClientCache;
    Ptr<UniformRandomVariable> m_rand;

    std::string m_nodeName;
    
    bool m_isCNRSRoot;
};
} // namespace ns3

#endif