#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-name-resolution-service.h"

#include <iostream>
#include <vector>
#define TEST_PORT (5353)

// ****************************************************************
//              Test Cybertwin Name Resolution Service
//
// Test topology:
//
//
//                     __ n0 __
//                    |        |  
//                  n1         n2
//                  |            |
//                n3              n4
//
//
//
//******************************************************************
using namespace ns3;
NS_LOG_COMPONENT_DEFINE("CybertwinNameResolutionServiceExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("CybertwinNameResolutionServiceExample", LOG_LEVEL_DEBUG);
    LogComponentEnable("NameResolutionService", LOG_LEVEL_DEBUG);
    //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_FUNCTION);
    
    //******************************************************************
    //*                         Build the topology                     *
    //******************************************************************
    // Create the nodes
    NodeContainer nodes;
    std::vector<CYBERTWIN_INTERFACE_LIST_t> ifsList(5);
    nodes.Create(5);
    
    // Install the internet protocol stack on all nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // Create the CSMA network
    // Assign IP addresses to the interfaces
    CsmaHelper csmaHelper;
    NetDeviceContainer devices;
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer interfaces;

    csmaHelper.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    address.SetBase("10.1.1.0", "255.255.255.0");

    devices = csmaHelper.Install(nodes);
    interfaces = address.Assign(devices);

    for (uint32_t i=0; i<ifsList.size(); i++)
    {
        ifsList[i].push_back(std::make_pair(interfaces.GetAddress(i), TEST_PORT));
    }

    // Print the addresses
    for (uint32_t i = 0; i < ifsList.size(); i++)
    {
        std::cout<<"-------"<<std::endl;
        std::cout << "Node " << i <<std::endl;
        std::cout<<"Interface: " << ifsList[i].size() << std::endl;
        for (auto it = ifsList[i].begin(); it != ifsList[i].end(); it++)
        {
            std::cout << *it << " ";
        }
        std::cout<<std::endl;
    }

    //******************************************************************
    //*                       Install Application                      *
    //******************************************************************
    // Create the application
    NameResolutionService cnrsApp0, cnrsApp1, cnrsApp2, cnrsApp3, cnrsApp4;
    cnrsApp1.SetSuperior(ifsList[0][0].first);
    cnrsApp2.SetSuperior(ifsList[0][0].first);
    cnrsApp3.SetSuperior(ifsList[1][0].first);
    cnrsApp4.SetSuperior(ifsList[2][0].first);

    nodes.Get(0)->AddApplication(&cnrsApp0);
    nodes.Get(1)->AddApplication(&cnrsApp1);
    nodes.Get(2)->AddApplication(&cnrsApp2);
    nodes.Get(3)->AddApplication(&cnrsApp3);
    nodes.Get(4)->AddApplication(&cnrsApp4);

    cnrsApp0.SetStartTime(Seconds(1));
    cnrsApp1.SetStartTime(Seconds(1));
    cnrsApp2.SetStartTime(Seconds(1));
    cnrsApp3.SetStartTime(Seconds(1));
    cnrsApp4.SetStartTime(Seconds(1));


#if 0
    Simulator::Schedule(Seconds(2),
                        &NameResolutionService::InsertCybertwinInterfaceName,
                        &cnrsApp3,
                        3,ifsList[3]);
#endif

    Simulator::Schedule(Seconds(2),
                        &NameResolutionService::InsertCybertwinInterfaceName,
                        &cnrsApp4,
                        4,ifsList[4]);

    Simulator::Schedule(Seconds(3),
                        &NameResolutionService::GetCybertwinInterfaceByName,
                        &cnrsApp3,
                        4,
                        MakeCallback(&NameResolutionService::DefaultGetInterfaceCallback, &cnrsApp1));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
