#include <stdio.h>
#include <string>
#include <vector>
#include <sys/resource.h>
#include <chrono>
#include <numeric>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include "ns3/point-to-point-module.h"

#define CORE_CLOUD_NODE_NUM 4
#define EDGE_CLOUD_NODE_NUM_RATIO 4
#define END_HOST_NODE_NUM_RATIO 8

#define UDP_ECHO_SERVER_PORT (7777)

#define SYSTEM_MAX_RUNTIME (10) // seconds
#define UDP_ECHO_TIME_INTERVAL (100) // milliseconds

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScabilityTest");

int coreCloudNodeNum = 4;
std::string outpath = "/tmp/";
int logStep = 0;
struct rusage usage;
std::vector<long> memoryUsageVec;

//*************************************************************************************
//***                   Resource Usage Statistics                                   ***
//*************************************************************************************
// 1. Memory Usage
// 2. CPU Usage
void GetResourceUsage()
{
    Time now = Simulator::Now();
    NS_LOG_UNCOND("--------------- " << now.GetSeconds() << " seconds ---------------");
    // get memory usage
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // CPU usage
        double cpuUsage = (double)(usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) +
                          (double)(usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000;
        
        NS_LOG_UNCOND("CPU Usage: " << cpuUsage << " seconds");
        // Get memeory usage
        long memoryUsage = usage.ru_maxrss;
        NS_LOG_UNCOND("Memory Usage: " << memoryUsage << " KB");
        memoryUsageVec.push_back(memoryUsage);
    } else {
        perror("getrusage");
    }

    if (now.GetSeconds() < SYSTEM_MAX_RUNTIME) {
        Simulator::Schedule(Seconds(1.0), &GetResourceUsage);
    }
}

void GenerateEndHostNodesToEdge(Ptr<Node> edgeNode,int edgeTag, int endTag)
{
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Create end host nodes");
    // Create End Host Nodes
    NodeContainer endHostNodes;
    endHostNodes.Create(END_HOST_NODE_NUM_RATIO);

    // Use point-to-point links to connect end host nodes to edge cloud node
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Connect end host nodes to edge cloud node");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // assign IP address to end host nodes
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install network stack and assign IP address to end host nodes");
    Ipv4AddressHelper address;
    std::string baseIp = "30." + std::to_string(edgeTag) + "." + std::to_string(endTag) + ".0";
    address.SetBase(baseIp.c_str(), "255.255.255.0");
    InternetStackHelper stack;
    stack.Install(endHostNodes);

    // Connect end host nodes to edge cloud node
    for (int i=0; i<END_HOST_NODE_NUM_RATIO; i++) {
        NetDeviceContainer endDevices = p2p.Install(edgeNode, endHostNodes.Get(i));
        NS_LOG_DEBUG("Assign IP address to end host nodes");
        address.Assign(endDevices);
    }

    // install UDP echo client in end host nodes
    // Get the IP address of the edge cloud node
    Ipv4Address edgeNodeIp = edgeNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    UdpEchoClientHelper echoClient(edgeNodeIp, UDP_ECHO_SERVER_PORT);
    echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(UDP_ECHO_TIME_INTERVAL)));
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install UDP echo client in end host nodes");
    for (int i=0; i<END_HOST_NODE_NUM_RATIO; i++) {
        ApplicationContainer clientApps = echoClient.Install(endHostNodes.Get(i));
        clientApps.Start(Seconds(2.0));
        clientApps.Stop(Seconds(10.0));
    }
}

void GenerateEdgeNodesToCloud(Ptr<Node> coreNode, int edgeTag)
{
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Create edge cloud nodes");
    // Create Edge Cloud Nodes
    NodeContainer edgeCloudNodes;
    edgeCloudNodes.Create(EDGE_CLOUD_NODE_NUM_RATIO);

    // Use point-to-point links to connect edge cloud nodes to core cloud node
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("500Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // Connect edge cloud nodes to core cloud node
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Connect edge cloud nodes to core cloud node");
    NetDeviceContainer edgeCloudDevices;
    for (int i=0; i<EDGE_CLOUD_NODE_NUM_RATIO; i++) {
        NetDeviceContainer edgeDevices = p2p.Install(coreNode, edgeCloudNodes.Get(i));
        edgeCloudDevices.Add(edgeDevices);
    }

    // install network stack and assign IP address to edge cloud nodes
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install network stack and assign IP address to edge cloud nodes");
    InternetStackHelper stack;
    stack.Install(edgeCloudNodes);

    Ipv4AddressHelper address;
    std::string baseIp = "20." + std::to_string(edgeTag) + ".0.0";
    address.SetBase(baseIp.c_str(), "255.255.0.0");

    Ipv4InterfaceContainer edgeCloudInterfaces;
    edgeCloudInterfaces = address.Assign(edgeCloudDevices);

    // install UDP echo server in edge cloud nodes
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install UDP echo server in edge cloud nodes");
    UdpEchoServerHelper echoServer(UDP_ECHO_SERVER_PORT);
    for (int i=0; i<EDGE_CLOUD_NODE_NUM_RATIO; i++) {
        ApplicationContainer serverApps = echoServer.Install(edgeCloudNodes.Get(i));
        serverApps.Start(Seconds(1.0));
        serverApps.Stop(Seconds(10.0));
    }

    // install UDP echo client in edge cloud nodes
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install UDP echo client in edge cloud nodes");
    Ipv4Address coreNodeIp = coreNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    UdpEchoClientHelper echoClient(coreNodeIp, UDP_ECHO_SERVER_PORT);
    echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(UDP_ECHO_TIME_INTERVAL)));
    for (int i=0; i<EDGE_CLOUD_NODE_NUM_RATIO; i++) {
        ApplicationContainer clientApps = echoClient.Install(edgeCloudNodes.Get(i));
        clientApps.Start(Seconds(2.0));
        clientApps.Stop(Seconds(10.0));
    }

    // Generate end host nodes to edge cloud nodes
    for (int i=0; i<EDGE_CLOUD_NODE_NUM_RATIO; i++) {
        GenerateEndHostNodesToEdge(edgeCloudNodes.Get(i), edgeTag, i);
    }
}

int
main(int argc, char *argv[])
{
    auto timeStart = std::chrono::steady_clock::now();
    CommandLine cmd;
    cmd.AddValue("outpath", "Output file path", outpath);
    cmd.AddValue("coreCloudNodeNum", "Number of core cloud nodes", coreCloudNodeNum);
    cmd.Parse(argc, argv);
    LogComponentEnable("ScabilityTest", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);

    //***************************************************************
    //*          build core cloud nodes and connect them            *
    //***************************************************************
    // Create Core Cloud Nodes
    NodeContainer coreCloudNodes;
    coreCloudNodes.Create(coreCloudNodeNum);
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Create "<< coreCloudNodeNum << " core cloud nodes");

    // Use point-to-point links to connect core cloud nodes
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1000Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    // full-connected core cloud nodes
    NetDeviceContainer coreCloudDevices;
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Connect core cloud nodes");
    for (int i=0; i<coreCloudNodeNum; i++) {
        for (int j=i+1; j<coreCloudNodeNum; j++) {
            NetDeviceContainer coreDevices = p2p.Install(coreCloudNodes.Get(i), coreCloudNodes.Get(j));
            coreCloudDevices.Add(coreDevices);
        }
    }

    // install network stack and assign IP address to core cloud nodes
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install network stack and assign IP address to core cloud nodes");
    InternetStackHelper stack;
    stack.Install(coreCloudNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.0.0");

    Ipv4InterfaceContainer coreCloudInterfaces;
    coreCloudInterfaces = address.Assign(coreCloudDevices);

    // install UDP echo server in core cloud nodes
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Install UDP echo server in core cloud nodes");
    UdpEchoServerHelper echoServer(UDP_ECHO_SERVER_PORT);
    for (int i=0; i<coreCloudNodeNum; i++) {
        ApplicationContainer serverApps = echoServer.Install(coreCloudNodes.Get(i));
        serverApps.Start(Seconds(1.0));
        serverApps.Stop(Seconds(10.0));
    }

    //***************************************************************
    //*          build edge cloud nodes and connect them            *
    //***************************************************************
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Create edge cloud nodes");
    for (int i=0; i<coreCloudNodeNum; i++) {
        GenerateEdgeNodesToCloud(coreCloudNodes.Get(i), i);
    }

    // populate routing tables
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Populate routing tables");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //***************************************************************
    //*         Schedule simulation events                         *
    //***************************************************************
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Schedule simulation events");
    Simulator::Schedule(Seconds(1.0), &GetResourceUsage);

    auto timeDriver = std::chrono::steady_clock::now();

    //***************************************************************
    //*          run simulation                                     *
    //***************************************************************
    NS_LOG_UNCOND("-- Step [" << logStep++ << "] : Run Simulation");
    Simulator::Run();
    Simulator::Destroy();

    auto timeEnd = std::chrono::steady_clock::now();

    std::chrono::duration<double> timeDriverElapsed = timeEnd - timeDriver;
    std::chrono::duration<double> timeElapsed = timeEnd - timeStart;

    NS_LOG_UNCOND("Driver Time Elapsed: " << timeDriverElapsed.count() << " seconds");
    NS_LOG_UNCOND("Total Time Elapsed: " << timeElapsed.count() << " seconds");

    //***************************************************************
    //*          Output  Performance Statistics                     *
    //***************************************************************
    int nodeNum = coreCloudNodeNum + coreCloudNodeNum * EDGE_CLOUD_NODE_NUM_RATIO + 
                  coreCloudNodeNum * EDGE_CLOUD_NODE_NUM_RATIO * END_HOST_NODE_NUM_RATIO;
    std::string fileName = outpath + std::to_string(nodeNum) + ".csv";
    FILE *fp = fopen(fileName.c_str(), "w");
    if (fp == NULL) {
        NS_LOG_UNCOND("Failed to open file " << fileName);
        return 1;
    }

    fprintf(fp, "Time(s),Memory(KB)\n");
    // calculate average memory usage
    long sum = std::accumulate(memoryUsageVec.begin(), memoryUsageVec.end(), 0);
    long average = sum / memoryUsageVec.size();
    fprintf(fp, "%f,%ld\n", timeElapsed.count(), average);
}