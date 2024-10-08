#ifndef CYBERTWIN_CLIENT_H
#define CYBERTWIN_CLIENT_H

#include "cybertwin-common.h"
#include "cybertwin-header.h"
#include "cybertwin-tag.h"

#include "ns3/address.h"
#include "ns3/application.h"

namespace ns3
{

class Socket;
class Packet;

class CybertwinClient : public Application
{
  public:
    CybertwinClient();
    static TypeId GetTypeId();

  protected:
    void DoDispose() override;
    virtual void RecvPacket(Ptr<Socket>);

    // socket connected to the cybertwin
    Ptr<Socket> m_socket;
    CYBERTWINID_t m_localCuid;
    CYBERTWINID_t m_peerCuid;
};

class CybertwinConnClient : public CybertwinClient
{
  public:
    CybertwinConnClient();
    ~CybertwinConnClient();

    void SetCertificate(const CybertwinCertTag&);

    static TypeId GetTypeId();

  protected:
    void DoDispose() override;

  private:
    // CybertwinController
    void ControllerConnectSucceededCallback(Ptr<Socket>);
    void ControllerConnectFailedCallback(Ptr<Socket>);
    void RecvFromControllerCallback(Ptr<Socket>);

    // Cybertwin
    void CybertwinConnectSucceededCallback(Ptr<Socket>);
    void CybertwinConnectFailedCallback(Ptr<Socket>);
    // Need these callbacks because the edge server might stop before the client
    void CybertwinCloseSucceededCallback(Ptr<Socket>);
    void CybertwinCloseFailedCallback(Ptr<Socket>);

    // Basic Functions
    void Authenticate();
    void ConnectCybertwin();
    void DisconnectCybertwin();

    void StartApplication() override;
    void StopApplication() override;

    Address m_localAddr;
    uint16_t m_localPort;

    Address m_edgeAddr;
    uint16_t m_edgePort;

    Ptr<Socket> m_ctrlSocket;
    uint16_t m_cybertwinPort;
    CybertwinCertTag m_cert;
};

class CybertwinBulkClient : public CybertwinClient
{
  public:
    CybertwinBulkClient();
    ~CybertwinBulkClient();

    static TypeId GetTypeId();

  protected:
    // void DoInitialize() override;
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void WaitForConnection();
    void SendData();

    uint64_t m_maxBytes;
    uint64_t m_sentBytes;
    uint32_t m_packetSize;

    Ptr<Packet> m_unsentPacket;
};

} // namespace ns3

#endif