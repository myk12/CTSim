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
#include "ns3/internet-apps-module.h"
#include "cybertwin-topology.h"

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

NS_LOG_COMPONENT_DEFINE ("Cybertwinv1");

int 
main (int argc, char *argv[])
{
  // parse command line arguements
  //CommandLine cmd (__FILE__);
  //cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  //cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  //cmd.Parse (argc,argv);
  LogComponentEnable("Cybertwinv1", LOG_LEVEL_INFO);
  LogComponentEnable ("V4Ping", LOG_LEVEL_DEBUG);

  //-------------------------------------------------------------
  //                     Create Nodes                           -
  //-------------------------------------------------------------
  NS_LOG_INFO("-> Creating Nodes.");
  NodeContainer allNodesContainer;
  allNodesContainer.Create(MAX_NODE_NUM);

  //-------------------------------------------------------------
  //                     Install Network Stack                  -
  //-------------------------------------------------------------
  NS_LOG_INFO("-> Install Network Stack.");
  InternetStackHelper stack;
  stack.Install(allNodesContainer);

  //-------------------------------------------------------------
  //              Install Devices & Assign IPaddress            -
  //-------------------------------------------------------------
  NS_LOG_INFO("-> Install Devices & Assign IPaddr.");
  // ======= build access network =======
  // LAN1
  buildCsmaNetwork(allNodesContainer, endLANNodeIDs[END_LAN1], endLANIPBases[END_LAN1], "5Mbps", 6560);
  // LAN2
  buildCsmaNetwork(allNodesContainer, endLANNodeIDs[END_LAN2], endLANIPBases[END_LAN2], "5Mbps", 6560);
  // LAN3
  buildCsmaNetwork(allNodesContainer, endLANNodeIDs[END_LAN3], endLANIPBases[END_LAN3], "5Mbps", 6560);
  // LAN4
  buildCsmaNetwork(allNodesContainer, endLANNodeIDs[END_LAN4], endLANIPBases[END_LAN4], "5Mbps", 6560);

  // ======= Connect edge routers with neighbors =======
  // edge router1
  p2pConnectToNeighbors(allNodesContainer, EDGE_ROUTER1, edgeRouter1Neighbors, edgeRouter1NeighborIPBase, "10Mbps", "2ms");
  // edge router2
  p2pConnectToNeighbors(allNodesContainer, EDGE_ROUTER2, edgeRouter2Neighbors, edgeRouter2NeighborIPBase, "10Mbps", "2ms");
  
  // ======= Connect core routers with neighbors =======
  // core router1
  p2pConnectToNeighbors(allNodesContainer, CORE_ROUTER1, coreRouter1Neighbors, coreRouter1NeighborIPBase, "20Mbps", "1ms");
  // core router2
  p2pConnectToNeighbors(allNodesContainer, CORE_ROUTER2, coreRouter2Neighbors, coreRouter2NeighborIPBase, "20Mbps", "1ms");

  //-------------------------------------------------------------
  //                          Routing                           -
  //-------------------------------------------------------------
  NS_LOG_INFO("-> Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //-------------------------------------------------------------
  //              Test Connectivity using V4Ping                -
  //-------------------------------------------------------------
  NS_LOG_INFO("-> Testing Connectivity.");
  testNodesConnectivity(allNodesContainer.Get(END_HOST1), allNodesContainer.Get(END_HOST8));
  testNodesConnectivity(allNodesContainer.Get(END_HOST8), allNodesContainer.Get(END_HOST1));
  //p2pHelper.EnablePcap ("second", accessCloud.Get(0)->GetId(), 0);
  //p2pHelper.EnablePcapAll("cybertwin");
  //PointToPointHelper p2pHelper;
  //CsmaHelper csmaHelper;
  //AsciiTraceHelper ascii;
  //p2pHelper.EnableAsciiAll(ascii.CreateFileStream("cybertwin1.tr"));
  //csmaHelper.EnableAsciiAll(ascii.CreateFileStream("cybertwin2.tr"));
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}