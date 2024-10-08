#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/bulk-send-application.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink-helper.h"
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
    //LogComponentEnable("BulkSendApplication", LOG_LEVEL_LOGIC);
    LogComponentEnable("PacketSink", LOG_LEVEL_DEBUG);
    //LogComponentEnable("Ipv4EndPointDemux", LOG_LEVEL_DEBUG);
    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_FUNCTION);
    LogComponentEnable("CybertwinMultiSocketTest", LOG_LEVEL_FUNCTION);
    //--------------------------------------------------------------------------
    //-                         build topology                                 -
    //--------------------------------------------------------------------------
    NS_LOG_UNCOND("*************************************************************************");
    NS_LOG_UNCOND("*                          Building topology                            *");
    NS_LOG_UNCOND("*************************************************************************");
    NodeContainer p2pNodes;
    NodeContainer three;
    NetDeviceContainer p2pDevs;

    p2pNodes.Create(2);
    three.Create(1);

    PointToPointHelper p2pHelper;
    InternetStackHelper stackHelper;
    Ipv4AddressHelper addrHelper;
    Ipv4InterfaceContainer ipNode0;
    Ipv4InterfaceContainer ipNode1;

    stackHelper.Install(p2pNodes);

    p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4Address ipbase[3] = {
        "10.1.1.0",
        "10.1.2.0",
        "10.1.3.0"
    };


    for (int i=0; i<2; i++)
    {
        Ipv4InterfaceContainer ipContainer;
        p2pDevs = p2pHelper.Install(p2pNodes);
        ipContainer = addrHelper.Assign(p2pDevs);
        //NS_LOG_DEBUG("ipbase" <<ipbase[i]<<" interface: "<<ipContainer.GetAddress(0)<<" "<<ipContainer.GetAddress(1));
        ipNode0.Add(ipContainer.Get(0));
        addrHelper.SetBase(ipbase[i], "255.255.255.0");
    
        ipNode1.Add(ipContainer.Get(1));
    }

    NS_LOG_UNCOND("Node device number: "<< p2pNodes.Get(0)->GetNDevices());

    NS_LOG_DEBUG("\n\n\n");
    NS_LOG_DEBUG("|`````|----------[10.1.1.0]-------------|``````|");
    NS_LOG_DEBUG("| n0  |----------[10.1.2.0]-------------| n1   |");
    NS_LOG_DEBUG("|_____|----------[10.1.3.0]-------------|______|");
    NS_LOG_DEBUG("            3 point-to-point links            ");
    NS_LOG_DEBUG("\n\n\n");

    //----------------------------------------------------------------------
    //-                     Install Software                               -
    //----------------------------------------------------------------------
    ApplicationContainer appContainer;

    BulkSendHelper bulkSendClient_0("ns3::TcpSocketFactory", InetSocketAddress(ipNode1.GetAddress(0), 80));
    bulkSendClient_0.SetAttribute("Local", AddressValue(InetSocketAddress(ipNode0.GetAddress(0), 8000)));
    bulkSendClient_0.SetAttribute("DevIndex", UintegerValue(2));
    BulkSendHelper bulkSendClient_1("ns3::TcpSocketFactory", InetSocketAddress(ipNode1.GetAddress(1), 80));
    bulkSendClient_1.SetAttribute("Local", AddressValue(InetSocketAddress(ipNode0.GetAddress(1), 8000)));
    bulkSendClient_1.SetAttribute("DevIndex", UintegerValue(1));

    appContainer =  bulkSendClient_0.Install(p2pNodes.Get(0));
    appContainer.Start(Seconds(2.0));
    //appClient.Stop(Seconds(7.0));
    appContainer =  bulkSendClient_1.Install(p2pNodes.Get(0));
    appContainer.Start(Seconds(3.0));
    //appContainer.Stop(Seconds(10.0));
    
    PacketSinkHelper packetSinkServer_0("ns3::TcpSocketFactory", InetSocketAddress(ipNode1.GetAddress(0), 80));
    packetSinkServer_0.SetAttribute("DevIdx", UintegerValue(1));
    appContainer =  packetSinkServer_0.Install(p2pNodes.Get(1));
    appContainer.Start(Seconds(1.0));
    //appServer.Stop(Seconds(8.0));
    PacketSinkHelper packetSinkServer_1("ns3::TcpSocketFactory", InetSocketAddress(ipNode1.GetAddress(1), 80));
    packetSinkServer_1.SetAttribute("DevIdx", UintegerValue(2));
    appContainer =  packetSinkServer_1.Install(p2pNodes.Get(1));
    appContainer.Start(Seconds(1.0));
    //appContainer.Stop(Seconds(10.0));

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
