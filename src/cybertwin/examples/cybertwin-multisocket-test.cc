#include "cybertwin-multipath-datatransfer.h"
#include "../model/cybertwin-name-resolution-service.h"
#include "ns3/bulk-send-application.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink-helper.h"
#include "cybertwin-multisocket-test.h"


// Default Network Topology
//           
// |`````|----------[10.1.1.0]-------------|``````|
// | n0  |----------[10.1.2.0]-------------| n1   |         n2         n3   
// |_____|----------[10.1.3.0]-------------|______|         |          | 
//                                         ==============================
//            3 point-to-point links                LAN 10.1.4.0
//          
//         

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CybertwinMultiSocketTest");

int
main(int argc, char* argv[])
{
    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_DEBUG);
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_UNCOND("*                          Building topology                            *");
    NS_LOG_UNCOND("*************************************************************************");
    NodeContainer p2pNodes;
    NetDeviceContainer p2pDevs;

    p2pNodes.Create(2);

    PointToPointHelper p2pHelper;
    InternetStackHelper stackHelper;
    Ipv4AddressHelper addrHelper;
    Ipv4InterfaceContainer ipInterfaces;

    stackHelper.Install(p2pNodes);

    p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));

    p2pDevs = p2pHelper.Install(p2pNodes);

    addrHelper.SetBase("10.1.1.0", "255.255.255.0");
    ipInterfaces = addrHelper.Assign(p2pDevs);
    
    NS_LOG_DEBUG("\n\n\n");
    NS_LOG_DEBUG("|`````|                                   |``````|");
    NS_LOG_DEBUG("| n0  |----------[10.1.1.0]---------------| n1   |");
    NS_LOG_DEBUG("|_____|                                   |______|");
    NS_LOG_DEBUG("            1 point-to-point links            ");
    NS_LOG_DEBUG("\n\n\n");

    //----------------------------------------------------------------------
    //-                     Install Software                               -
    //----------------------------------------------------------------------


    //----------------------------------------------------------------------
    //-                     run simulation                                 -
    //----------------------------------------------------------------------

    NS_LOG_DEBUG("\n\n\n");
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_UNCOND("*                          Run simulation                               *");
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_DEBUG("\n\n\n");
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

namespace ns3
{
TypeId
MultiSocketServer::GetTypeId(void)
{
    static TypeId tid = 
            TypeId("ns3::MultiSocketServer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<MultiSocketServer>();
    return tid;
}
MultiSocketServer::MultiSocketServer()
{
    m_ports.push_back(80);
    m_ports.push_back(90);
    m_ports.push_back(100);
}

MultiSocketServer::~MultiSocketServer()
{
}

void
MultiSocketServer::Setup(Address addr)
{
    m_local = add;
}

void
MultiSocketServer::StartApplication()
{
    
    for (uint32_t i=0; i<m_ports.size(); i++)
    {
        uint16_t port = m_ports[i];
        Ptr<Socket> socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypdId());
        socket->Bind(InetSocketAddress(m_local, port));
        socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&> (),
                                  MakeCallback(&MultiSocketServer::HandleAccept, this));
        socket->SetCloseCallbacks(MakeCallback(&MultiSocketServer::HandlePeerClose, this),
                                  MakeCallback(&MultiSocketServer::HandlePeerError, this));

        m_sockets.push_back(socket);
    }
}

void
MultiSocketServer::HandleAccept(Ptr<Socket> s, const Address& from)
{
    NS_LOG_FUNCTION(this<< s <<from); 
}
    
} // namespace ns3

