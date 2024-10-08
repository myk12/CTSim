#include "ns3/core-module.h"
#include "ns3/cybertwin-module.h"
#include "ns3/cybertwin-tag.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestEndToEdge");

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

#define TEST_LOG_LEVEL LOG_LEVEL_ALL

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Packet::EnablePrinting();

    Time::SetResolution(Time::NS);
    LogComponentEnable("CybertwinEdge", TEST_LOG_LEVEL);
    LogComponentEnable("CybertwinClient", TEST_LOG_LEVEL);
    LogComponentEnable("Cybertwin", TEST_LOG_LEVEL);
    LogComponentEnable("CybertwinCert", TEST_LOG_LEVEL);
    LogComponentEnable("TestEndToEdge", TEST_LOG_LEVEL);
    // LogComponentEnable("CybertwinHeader", TEST_LOG_LEVEL);

    NodeContainer nodes;
    nodes.Create(4);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("40Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer lan1Devices, lan2Devices, e2eDevices;
    lan1Devices = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    e2eDevices = pointToPoint.Install(nodes.Get(1), nodes.Get(3));
    lan2Devices = pointToPoint.Install(nodes.Get(2), nodes.Get(3));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper lan1Addr, lan2Addr, edgeAddr;
    lan1Addr.SetBase("10.1.1.0", "255.255.255.0");
    lan2Addr.SetBase("10.1.2.0", "255.255.255.0");
    edgeAddr.SetBase("10.1.3.0", "255.255.255.0");

    Ipv4InterfaceContainer lan1Interfaces, lan2Interfaces, edgeInterfaces;
    lan1Interfaces = lan1Addr.Assign(lan1Devices);
    lan2Interfaces = lan2Addr.Assign(lan2Devices);
    edgeInterfaces = edgeAddr.Assign(e2eDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    CybertwinCertTag dev1Cert(1000, 1000, 500, true, true, 2000, 1000),
        dev2Cert(1001, 500, 1000, false, true);

    CybertwinConnHelper connClient1;
    connClient1.SetAttribute("LocalAddress", AddressValue(lan1Interfaces.GetAddress(0)));
    connClient1.SetAttribute("EdgeAddress", AddressValue(lan1Interfaces.GetAddress(1)));
    connClient1.SetAttribute("LocalCuid", UintegerValue(1000));
    ApplicationContainer clientConn1App = connClient1.Install(nodes.Get(0));
    connClient1.SetCertificate(nodes.Get(0), dev1Cert);

    CybertwinConnHelper connClient2;
    connClient2.SetAttribute("LocalAddress", AddressValue(lan2Interfaces.GetAddress(0)));
    connClient2.SetAttribute("EdgeAddress", AddressValue(lan2Interfaces.GetAddress(1)));
    connClient2.SetAttribute("LocalCuid", UintegerValue(1001));
    ApplicationContainer clientConn2App = connClient2.Install(nodes.Get(2));
    connClient2.SetCertificate(nodes.Get(2), dev2Cert);

    CybertwinHelper bulkClient1("ns3::CybertwinBulkClient");
    bulkClient1.SetAttribute("PeerCuid", UintegerValue(1001));
    ApplicationContainer clientBulk1App = bulkClient1.Install(nodes.Get(0));
    clientConn1App.Start(Seconds(1.0));
    clientBulk1App.Start(Seconds(1.1));

    CybertwinHelper lan1EdgeCtrl("ns3::CybertwinController");
    lan1EdgeCtrl.SetAttribute("LocalAddress", AddressValue(lan1Interfaces.GetAddress(1)));
    ApplicationContainer lan1EdgeApp = lan1EdgeCtrl.Install(nodes.Get(1));
    lan1EdgeApp.Start(Seconds(0.0));

    CybertwinHelper lan2Edge("ns3::CybertwinController");
    lan2Edge.SetAttribute("LocalAddress", AddressValue(lan2Interfaces.GetAddress(1)));
    ApplicationContainer lan2EdgeApp = lan2Edge.Install(nodes.Get(3));
    lan2EdgeApp.Start(Seconds(0.0));

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
