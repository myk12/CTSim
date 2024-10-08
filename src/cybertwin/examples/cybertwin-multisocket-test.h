#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <vector>

namespace ns3
{
class MultiSocketServer: public Application
{
 public:
  static TypeId GetTypeId(void);
  MultiSocketServer();
  virtual ~MultiSocketServer();
  void Setup(Address addr);
  void StartApplication();
  void StopApplication();

 private:
  void HandleAccept(Socket socket, const Address& from);
  void HandleRead(Ptr<Socket> socket);
  void HandlePeerClose(Ptr<Socket> socket);
  void HandlePeerError(Ptr<Socket> socket);

  Address m_local;
  std::vector<uint16_t> m_ports;
  std::vector<Ptr<Socket>> m_sockets;
};

class MultiSocketClient: public Application
{
public:
    static  TypeId GetTypeId(void);
    MultiSocketClient();
    virtual ~MultiSocketClient();

}

}