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

#include "cybertwin-topology.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/cybertwin-name-resolution-service.h"
#include "ns3/cybertwin-node-coreserver.h"
#include "ns3/cybertwin-node-edgeserver.h"
#include "ns3/cybertwin-node-endhost.h"
#include "ns3/cybertwin-node.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>

// Network Topology, Cybertwin: v1.0
//
//
#define TOPOLOGY_GRAPH                                                                             \
    ("                             - Cybertwin Network Simulation: version 1 -                   " \
     "    \n"                                                                                      \
     "                                                                                           " \
     "    \n"                                                                                      \
     "                            [core1]                                [core2]                 " \
     "    \n"                                                                                      \
     "                               |                                      |                    " \
     "    \n"                                                                                      \
     "                          [20.1.0.0]                             [20.2.0.0]                " \
     " #point to point#   \n"                                                                      \
     "                    ___________|______________________________________|___________         " \
     "    \n"                                                                                      \
     "                     |                   |                   |                  |          " \
     "    \n"                                                                                      \
     "                     |                   |                   |                  |          " \
     "    \n"                                                                                      \
     "                  [edge1]             [edge2]             [edge3]            [edge4]       " \
     "    \n"                                                                                      \
     "                     |                   |                   |                  |          " \
     "    \n"                                                                                      \
     "             ___[10.1.0.0]___    ___[10.2.0.0]___    ___[10.3.0.0]___   ___[10.4.0.0]___   " \
     " #CSMA#   \n"                                                                                \
     "              |            |      |            |      |            |     |            |    " \
     "    \n"                                                                                      \
     "              |            |      |            |      |            |     |            |    " \
     "    \n"                                                                                      \
     "             [h1]         [h2]   [h3]         [h4]   [h5]         [h6]  [h7]         [h8]  " \
     "    \n")
//
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CybertwinV1");

int
main(int argc, char* argv[])
{
    Packet::EnablePrinting();
    // Time::SetResolution(Time::NS);

    //LogComponentEnable("CybertwinV1", LOG_LEVEL_INFO);
    // LogComponentEnable("V4Ping", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinEdgeServer", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinCoreServer", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinEndHost", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinClient", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinEdge", LOG_LEVEL_DEBUG);
    LogComponentEnable("Cybertwin", LOG_LEVEL_DEBUG);
    // LogComponentEnable("NameResolutionService", LOG_LEVEL_DEBUG);
    //LogComponentEnable("CybertwinMultipathTransfer", LOG_LEVEL_DEBUG);

    //*************************************************************************************************
    //*                           Building Topology *
    //*************************************************************************************************
    NS_LOG_UNCOND(
        "\n\n[1] ************************ Building Topology ****************************\n\n");
    NS_LOG_UNCOND(TOPOLOGY_GRAPH);
    //read config file
    std::ifstream conf_file("cybertwin/config.json");
    NS_ASSERT_MSG(conf_file.is_open(), "Open config file failed.");

    // parse config
    nlohmann::json json_conf;
    conf_file>>json_conf;
    conf_file.close();
    std::string corelink1 = std::to_string(int(json_conf["topology"]["link_speed"]["core_server1"]));
    std::string corelink2 = std::to_string(int(json_conf["topology"]["link_speed"]["core_server2"]));

    NodeContainer allNodesContainer;
    std::vector<std::vector<Ipv4Address>> allNodesIpv4Addresses;

    for (int32_t i = 0; i < MAX_NODE_NUM; i++)
    {
        if (END_HOST1 <= i && i <= END_HOST8)
        { // create end host
            Ptr<CybertwinEndHost> endhost = CreateObject<CybertwinEndHost>();
            allNodesContainer.Add(endhost);
        }
        else if (EDGE_SERVER1 <= i && i <= EDGE_SERVER4)
        { // create edge server
            Ptr<CybertwinEdgeServer> edgeServer = CreateObject<CybertwinEdgeServer>();
            allNodesContainer.Add(edgeServer);
        }
        else if (CORE_SERVER1 <= i && i <= CORE_SERVER2)
        { // create core server
            Ptr<CybertwinCoreServer> coreServer = CreateObject<CybertwinCoreServer>();
            allNodesContainer.Add(coreServer);
        }
        else
        { // create router
            Ptr<CybertwinNode> router = CreateObject<CybertwinNode>();
            allNodesContainer.Add(router);
        }
    }

    // Install Network Stack
    NS_LOG_INFO("-> Install Network Stack.");
    InternetStackHelper stack;
    stack.Install(allNodesContainer);

    // Install Devices and Assign Ipaddress
    NS_LOG_INFO("-> Install Devices & Assign IPaddr.");
    // LAN1
    buildCsmaNetwork(allNodesContainer,
                     endLANNodeIDs[END_LAN1],
                     endLANIPBases[END_LAN1],
                     "5Mbps",
                     6560);
    // LAN2
    buildCsmaNetwork(allNodesContainer,
                     endLANNodeIDs[END_LAN2],
                     endLANIPBases[END_LAN2],
                     "5Mbps",
                     6560);
    // LAN3
    buildCsmaNetwork(allNodesContainer,
                     endLANNodeIDs[END_LAN3],
                     endLANIPBases[END_LAN3],
                     "5Mbps",
                     6560);
    // LAN4
    buildCsmaNetwork(allNodesContainer,
                     endLANNodeIDs[END_LAN4],
                     endLANIPBases[END_LAN4],
                     "5Mbps",
                     6560);

    // ======= Connect core server with edge server =======
    // core server1
    p2pConnectToNeighbors(allNodesContainer,
                          CORE_SERVER1,
                          coreServer1Neighbors,
                          coreServer1NeighborIPBase,
                          (corelink1 += "Mbps").c_str(),
                          "2ms");
    // core server2
    p2pConnectToNeighbors(allNodesContainer,
                          CORE_SERVER2,
                          coreServer2Neighbors,
                          coreServer2NeighborIPBase,
                          (corelink2 += "Mbps").c_str(),
                          "2ms");

    allNodesIpv4Addresses = getNodesIpv4List(allNodesContainer);

    NotifyCybertwinConfiguration();

    // Routing
    NS_LOG_INFO("-> Routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //*************************************************************************************************
    //*                           Testing Connectivity *
    //*************************************************************************************************
    NS_LOG_UNCOND(
        "\n\n[2] ************************ Testing Connectivity ****************************\n\n");
    // testNodesConnectivity(allNodesContainer.Get(END_HOST1), allNodesContainer.Get(END_HOST8));
    // testNodesConnectivity(allNodesContainer.Get(END_HOST8), allNodesContainer.Get(END_HOST1));

    //*************************************************************************************************
    //*                           Installing Software *
    //*************************************************************************************************
    NS_LOG_UNCOND(
        "\n\n[3] ************************ Installing Software ****************************\n\n");

    std::vector<uint64_t> allCybertwinId;
    for (int32_t i = 0; i < MAX_NODE_NUM; i++)
    {
        Ptr<CybertwinNode> node = DynamicCast<CybertwinNode>(allNodesContainer.Get(i));

        // Set Node IP Address list
        node->SetAddressList(allNodesIpv4Addresses[i]);
        NS_LOG_UNCOND("Node " << i << " IP Address number: " << allNodesIpv4Addresses[i].size());

        // Init Cybertwin ID
        uint64_t simCuid;
        if (i < EDGE_SERVER1)
        {
            // end hosts
            simCuid = 1000 + i * 100 + i % 2;
        }
        else if (i < CORE_SERVER1)
        {
            // edge servers
            simCuid = 30000 + (i - EDGE_SERVER1) * 1000 + i % 2;
        }
        else
        {
            // core servers
            simCuid = 900000 + (i - CORE_SERVER1) * 10000 + i % 2;
        }
        allCybertwinId.push_back(simCuid);
        node->SetAttribute("SelfCuid", UintegerValue(simCuid));
        node->SetAttribute("SelfNodeAddress", Ipv4AddressValue(allNodesIpv4Addresses[i][0]));

        switch (i)
        {
        case END_HOST1:
        case END_HOST2:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[EDGE_SERVER1][0]));
            break;
        case END_HOST3:
        case END_HOST4:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[EDGE_SERVER2][0]));
            break;
        case END_HOST5:
        case END_HOST6:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[EDGE_SERVER3][0]));
            break;
        case END_HOST7:
        case END_HOST8:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[EDGE_SERVER4][0]));
            break;
        case EDGE_SERVER1:
        case EDGE_SERVER2:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[CORE_SERVER1][0]));
            break;
        case EDGE_SERVER3:
        case EDGE_SERVER4:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[CORE_SERVER2][0]));
            break;
        case CORE_SERVER1:
            break;
        case CORE_SERVER2:
            node->SetAttribute("UpperNodeAddress",
                               Ipv4AddressValue(allNodesIpv4Addresses[CORE_SERVER1][0]));
            break;
        default:
            break;
        }
        node->Setup();
    }
    Ptr<CybertwinEndHost> endHost1 =
        DynamicCast<CybertwinEndHost>(allNodesContainer.Get(END_HOST1));
    endHost1->Connect(
        CybertwinCertTag(allCybertwinId.at(END_HOST1), 1000, 500, true, true, 6000, 1000));

    Ptr<CybertwinEndHost> endHost8 =
        DynamicCast<CybertwinEndHost>(allNodesContainer.Get(END_HOST8));
    endHost8->Connect(CybertwinCertTag(allCybertwinId.at(END_HOST8), 500, 1000, false, true));

    endHost1->SendTo(allCybertwinId.at(END_HOST8));

    //*************************************************************************************************
    //*                           Starting Simulation *
    //*************************************************************************************************
    NS_LOG_UNCOND("\n\n[4] ************************ Starting Simulation "
                  "****************************\n\n");
    Simulator::Run();
    Simulator::Stop(Seconds(10));
    Simulator::Destroy();
    return 0;
}