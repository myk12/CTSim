#ifndef CYBERTWIN_H
#define CYBERTWIN_H

#include "cybertwin-common.h"

#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/network-module.h"
#include "ns3/socket.h"

#include <queue>
#include <string>
#include <unordered_map>

namespace ns3
{
// class Application;

class Cybertwin : public Application
{
  public:
    Cybertwin();
    Cybertwin(InitCybertwinCallback);
    Cybertwin(CYBERTWINID_t, CybertwinInterface, CybertwinInterface);
    ~Cybertwin();

    // local
    static TypeId GetTypeId();
    bool localConnRequestCallback(Ptr<Socket> socket, const Address& address);
    void localNewConnCreatedCallback(Ptr<Socket> socket, const Address& address);
    // void localReceivedDataCallback(Ptr<Socket> socket);
    void localNormalCloseCallback(Ptr<Socket> socket);
    void localErrorCloseCallback(Ptr<Socket> socket);
    void localRecvHandler(Ptr<Socket> socket);

    // global
    bool globalConnRequestCallback(Ptr<Socket> socket, const Address& address);
    void globalNewConnCreatedCallback(Ptr<Socket> socket, const Address& address);
    // void globalReceivedDataCallback(Ptr<Socket> socket);
    void globalNormalCloseCallback(Ptr<Socket> socket);
    void globalErrorCloseCallback(Ptr<Socket> socket);
    void globalRecvHandler(Ptr<Socket> socket);

    void ouputPackets();

    CYBERTWINID_t GetCybertwinID() const;
    void SetCybertwinID(CYBERTWINID_t cybertwinID);

    void SetLocalInterface(Address address, uint16_t port);
    void SetGlobalInterface(Address address, uint16_t port);

    void Start();
    void Stop();

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    CYBERTWINID_t cybertwinID;

    Ptr<Socket> localSocket; // communicate with end host
    CybertwinInterface localInterface;

    Ptr<Socket> globalSocket; // network socket
    CybertwinInterface globalInterface;

    std::unordered_map<CYBERTWINID_t, std::queue<Ptr<Packet>>> txPacketBuffer;
    std::unordered_map<CYBERTWINID_t, std::queue<Ptr<Packet>>> rxPacketBuffer;

    std::unordered_map<CYBERTWINID_t, Ptr<Socket>> globalTxSocket;
    // TODO: Add traffic logger
    // TODO: Add other functionality
    std::unordered_map<CYBERTWINID_t, CybertwinInterface> nameResolutionCache;
};
} // namespace ns3

#endif /* CYBERTWIN_H */
