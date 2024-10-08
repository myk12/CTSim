#include "ns3/core-module.h"
#include "ns3/cybertwin-helper.h"
#include "ns3/point-to-point-helper.h"

using namespace ns3;

int
main(int argc, char* argv[])
{
    bool verbose = true;

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell application to log if true", verbose);
/*
    cmd.Parse(argc, argv);
    LogComponentEnable("CybertwinServer", LOG_LEVEL_INFO);
    LogComponentEnable("Socket", LOG_LEVEL_DEBUG);
    NodeContainer hosts;
    hosts.Create(2);

    PointToPointHelper p2pHelper;
    NetDeviceContainer devices;
    InternetStackHelper stack;
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer ifs;
    
    stack.Install(hosts);
    devices = p2pHelper.Install(hosts);
    address.SetBase("10.1.1.0", "255.255.255.0");
    ifs = address.Assign(devices);

    CybertwinServer cyberserverapp;
    Ptr<Node> serverNode = hosts.Get(0);
    serverNode->AddApplication(&cyberserverapp);
    cyberserverapp.Setup();

    CybertwinClient cyberclientapp;
    Ptr<Node> clientNode = hosts.Get(1);
    clientNode->AddApplication(&cyberclientapp);
    cyberclientapp.Setup("10.1.1.1", 8080);

    cyberserverapp.SetStartTime(Seconds(1));
    cyberserverapp.SetStopTime(Seconds(10));
    cyberclientapp.SetStartTime(Seconds(2));
    cyberclientapp.SetStopTime(Seconds(9));

    AsciiTraceHelper ascii;
    p2pHelper.EnableAsciiAll(ascii.CreateFileStream("cybertwin-example.tr"));
*/
    /* ... */

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
