/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

// Network Topology, Cybertwin: v1.0
//          
//   Core Cloud with 2 hosts
//   Edge Cloud with 4 hosts
//   End Devices with 8 hosts
//
using namespace ns3;

#define ECHO_SERVER_PORT (80)
#define NUM_CORE_HOST   2
#define NUM_EDGE_HOST   4
#define NUM_END_HOST    8
#define NUM_TOTOAL_NODE 22

typedef std::pair<std::string, std::string> IPaddrBase;

// Define label for each node in order to cite node in container
enum NodeLabel
{
  END_HOST1, END_HOST2, END_SWITCH1, // LAN1
  END_HOST3, END_HOST4, END_SWITCH2, // LAN2
  END_HOST5, END_HOST6, END_SWITCH3, // LAN3
  END_HOST7, END_HOST8, END_SWITCH4, // LAN4
  EDGE_SERVER1, EDGE_SERVER2, EDGE_ROUTER1, // Edge cloud1
  EDGE_SERVER3, EDGE_SERVER4, EDGE_ROUTER2, // Edge cloud2
  CORE_SERVER1, CORE_SERVER2, CORE_ROUTER1, CORE_ROUTER2,// Core cloud
  MAX_NODE_NUM,
};

const std::vector<uint32_t> 
edgeRouter1Neighbors = {
  END_SWITCH1,
  END_SWITCH2,
  EDGE_SERVER1,
  EDGE_SERVER2, 
};

std::vector<IPaddrBase> edgeRouter1NeighborIPBase = {
  std::make_pair("10.0.0.0", "255.255.0.0"),
  std::make_pair("10.1.0.0", "255.255.0.0"),
  std::make_pair("10.2.0.0", "255.255.0.0"),
  std::make_pair("10.3.0.0", "255.255.0.0")
};

const std::vector<uint32_t> 
edgeRouter2Neighbors = {
  END_SWITCH3,
  END_SWITCH4,
  EDGE_SERVER3,
  EDGE_SERVER4
};

std::vector<IPaddrBase> edgeRouter2NeighborIPBase = {
  std::make_pair("80.0.0.0", "255.255.0.0"),
  std::make_pair("80.1.0.0", "255.255.0.0"),
  std::make_pair("80.2.0.0", "255.255.0.0"),
  std::make_pair("80.3.0.0", "255.255.0.0")
};

std::vector<uint32_t>
coreRouter1Neighbors = {
  EDGE_ROUTER1,
  CORE_SERVER1,
  EDGE_ROUTER2
};

std::vector<IPaddrBase> coreRouter1NeighborIPBase = {
  std::make_pair("20.0.0.0", "255.0.0.0"),
  std::make_pair("40.0.0.0", "255.0.0.0"),
  std::make_pair("60.0.0.0", "255.0.0.0")
};

const std::vector<uint32_t>
coreRouter2Neighbors = {
  EDGE_ROUTER1,
  CORE_SERVER2,
  EDGE_ROUTER2
};

std::vector<IPaddrBase> coreRouter2NeighborIPBase = {
  std::make_pair("30.0.0.0", "255.0.0.0"),
  std::make_pair("50.0.0.0", "255.0.0.0"),
  std::make_pair("70.0.0.0", "255.0.0.0")
};

// define end lan node ID set
enum {
  END_LAN1, // 10.0.0.0/16
  END_LAN2, // 10.1.0.0/16
  END_LAN3, // 80.0.0.0/16
  END_LAN4  // 80.1.0.0/16
};

const std::vector<std::vector<uint32_t> > endLANNodeIDs = {
  {END_HOST1, END_HOST2, END_SWITCH1},
  {END_HOST3, END_HOST4, END_SWITCH2},
  {END_HOST5, END_HOST6, END_SWITCH3},
  {END_HOST7, END_HOST8, END_SWITCH4}
};

std::vector<IPaddrBase> endLANIPBases = {
  std::make_pair("10.0.0.0", "255.255.0.0"),
  std::make_pair("10.1.0.0", "255.255.0.0"),
  std::make_pair("80.0.0.0", "255.255.0.0"),
  std::make_pair("80.1.0.0", "255.255.0.0")
};

void buildCsmaNetwork(NodeContainer &allnodes,
                      const std::vector<uint32_t> &nodeIds,
                      IPaddrBase &addrBase,
                      std::vector<std::vector<Ipv4Address> > &allNodesIPv4,
                      const char* datarate,
                      const uint32_t delay);

void p2pConnectToNeighbors(NodeContainer &allNodes,
                          const uint32_t centerID,
                          const std::vector<uint32_t> &neighborIDs,
                          std::vector<IPaddrBase> &neighborsIPBase,
                          std::vector<std::vector<Ipv4Address> > &allNodesIPv4,
                          const char* datarate,
                          const char* delay);

NS_LOG_COMPONENT_DEFINE ("Cybertwinv1");

int 
main (int argc, char *argv[])
{
  bool verbose = true;

  // parse command line arguements
  //CommandLine cmd (__FILE__);
  //cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  //cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  //cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
  LogComponentEnable("Cybertwinv1", LOG_LEVEL_DEBUG);
  // ************** Create Nodes **************
  NS_LOG_UNCOND("-> Creating Nodes.");
  NodeContainer allNodesContainer;
  std::vector<std::vector<Ipv4Address> > allNodesIPAddress(MAX_NODE_NUM);
  
  allNodesContainer.Create(MAX_NODE_NUM);

  // ************** Network Stack **************
  // install network stack for every nodes
  NS_LOG_UNCOND("-> Install Network Stack.");
  InternetStackHelper stack;
  stack.Install(allNodesContainer);

  // ************** Install Devices & Assign IPaddr ***********
  NS_LOG_UNCOND("-> Install Devices & Assign IPaddr.");
  // build access network
  // LAN1 
  buildCsmaNetwork(allNodesContainer,
                  endLANNodeIDs[END_LAN1],
                  endLANIPBases[END_LAN1],
                  allNodesIPAddress,
                  "5Mbps", 6560);
  // LAN2
  buildCsmaNetwork(allNodesContainer,
                  endLANNodeIDs[END_LAN2],
                  endLANIPBases[END_LAN2],
                  allNodesIPAddress,
                  "5Mbps", 6560);
  // LAN3
  buildCsmaNetwork(allNodesContainer,
                  endLANNodeIDs[END_LAN3],
                  endLANIPBases[END_LAN3],
                  allNodesIPAddress,
                  "5Mbps", 6560);
  // LAN4
  buildCsmaNetwork(allNodesContainer,
                  endLANNodeIDs[END_LAN4],
                  endLANIPBases[END_LAN4],
                  allNodesIPAddress,
                  "5Mbps", 6560);

  // Connect edge routers with neighbors
  p2pConnectToNeighbors(allNodesContainer,
                        EDGE_ROUTER1,
                        edgeRouter1Neighbors,
                        edgeRouter1NeighborIPBase,
                        allNodesIPAddress,
                        "10Mbps", "2ms");

  p2pConnectToNeighbors(allNodesContainer,
                        EDGE_ROUTER2,
                        edgeRouter2Neighbors,
                        edgeRouter2NeighborIPBase,
                        allNodesIPAddress,
                        "10Mbps", "2ms");
  
  // Connect core routers with neighbors
  p2pConnectToNeighbors(allNodesContainer,
                        CORE_ROUTER1,
                        coreRouter1Neighbors,
                        coreRouter1NeighborIPBase,
                        allNodesIPAddress,
                        "20Mbps", "1ms");

  p2pConnectToNeighbors(allNodesContainer,
                        CORE_ROUTER2,
                        coreRouter2Neighbors,
                        coreRouter2NeighborIPBase,
                        allNodesIPAddress,
                        "20Mbps", "1ms");

//xxxxxxxxxxxxxxxxxxxxxxxxxx TODO xxxxxxxxxxxxxxxxxxxxxxxxxxx

  // ************** Install Application ********
  NS_LOG_UNCOND("-> Install Application.");
  Ptr<Node> serverNode, clientNode;
  Ptr<NetDevice> serverDev;
  serverNode = allNodesContainer.Get(END_HOST1);
  clientNode = allNodesContainer.Get(END_HOST3);

  UdpEchoServerHelper echoServer (ECHO_SERVER_PORT);
  ApplicationContainer serverApps = echoServer.Install (serverNode);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (allNodesIPAddress[END_HOST1][0], ECHO_SERVER_PORT);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (clientNode);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // ************* Routing *************
  NS_LOG_UNCOND("-> Routing.");

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // ################## Tracing ####################
  //p2pHelper.EnablePcap ("second", accessCloud.Get(0)->GetId(), 0);
  //p2pHelper.EnablePcapAll("cybertwin");
  PointToPointHelper p2pHelper;
  CsmaHelper csmaHelper;
  AsciiTraceHelper ascii;
  p2pHelper.EnableAsciiAll(ascii.CreateFileStream("cybertwin1.tr"));
  csmaHelper.EnableAsciiAll(ascii.CreateFileStream("cybertwin2.tr"));

  NS_LOG_UNCOND("-> Run simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

/**
 * @brief 
 * 
 * @param allnodes 
 * @param nodeIds 
 * @param addrBase 
 * @param allNodesIPv4 
 * @param datarate 
 * @param delay 
 */
void buildCsmaNetwork(NodeContainer &allnodes,
                      const std::vector<uint32_t> &nodeIds,
                      IPaddrBase &addrBase,
                      std::vector<std::vector<Ipv4Address> > &allNodesIPv4,
                      const char* datarate,
                      const uint32_t delay)
{
  NodeContainer nodesCon;
  CsmaHelper csma;
  NetDeviceContainer csmaDevices;
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer ipcontainer;
  
  for (uint32_t i=0; i<nodeIds.size(); i++)
  {
    nodesCon.Add(allnodes.Get(nodeIds[i]));
  }

  // set channel attribute
  csma.SetChannelAttribute ("DataRate", StringValue (datarate));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (delay)));

  csmaDevices = csma.Install(nodesCon);

  // assign ip
  address.SetBase(addrBase.first.c_str(), addrBase.second.c_str(), "0.0.0.100");
  ipcontainer = address.Assign(csmaDevices);

  for (uint32_t i=0; i<nodeIds.size(); i++)
  {
    allNodesIPv4[nodeIds[i]].push_back(ipcontainer.GetAddress(i));
  }
}

/**
 * @brief 
 * 
 * @param allNodes 
 * @param center 
 * @param neighborIDs 
 * @param neighborsIPBase 
 * @param allNodesIPv4 
 * @param datarate 
 * @param delay 
 */
void p2pConnectToNeighbors(NodeContainer &allNodes,
                          const uint32_t centerID,
                          const std::vector<uint32_t> &neighborIDs,
                          std::vector<IPaddrBase> &neighborsIPBase,
                          std::vector<std::vector<Ipv4Address> > &allNodesIPv4,
                          const char* datarate,
                          const char* delay)
{
  uint32_t nNeighbors = neighborIDs.size();
  if (nNeighbors <= 0)
  {
    return;
  }

  PointToPointHelper p2pHelper;
  Ipv4AddressHelper address;
  Ptr<Node> center = allNodes.Get(centerID);
  Ptr<Node> neighbor = nullptr;
  IPaddrBase addrBase;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer ipcontainer;

  p2pHelper.SetDeviceAttribute("DataRate", StringValue(datarate));
  p2pHelper.SetChannelAttribute("Delay", StringValue(delay));

  for (uint32_t i=0; i<nNeighbors; i++)
  {
    uint32_t neighborID = neighborIDs[i];
    neighbor = allNodes.Get(neighborID);
    addrBase = neighborsIPBase[i];
    
    // install p2p channel & devices
    devices = p2pHelper.Install(center, neighbor);

    // assign ip
    address.SetBase(addrBase.first.c_str(), addrBase.second.c_str());
    ipcontainer = address.Assign(devices);

    allNodesIPv4[centerID].push_back(ipcontainer.GetAddress(0));
    allNodesIPv4[neighborID].push_back(ipcontainer.GetAddress(1));
  }
}

