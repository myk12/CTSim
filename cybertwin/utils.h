#ifndef CYBERTWIN_TOPOLOGY_H
#define CYBERTWIN_TOPOLOGY_H

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"

#include <string>
#include <vector>

namespace ns3
{

//*************************************************************************
//*                       Cybertwin Topology                              *
//*************************************************************************

typedef std::pair<std::string, std::string> IPaddrBase;

// Define label for each node in order to cite node in container
enum NodeLabel
{
    END_HOST1,
    END_HOST2, // LAN1
    END_HOST3,
    END_HOST4, // LAN2
    END_HOST5,
    END_HOST6, // LAN3
    END_HOST7,
    END_HOST8, // LAN4
    EDGE_SERVER1,
    EDGE_SERVER2,
    //EDGE_ROUTER1, // Edge cloud1
    EDGE_SERVER3,
    EDGE_SERVER4,
    //EDGE_ROUTER2, // Edge cloud2
    CORE_SERVER1,
    CORE_SERVER2,
    //CORE_ROUTER1,
    //CORE_ROUTER2, // Core cloud
    MAX_NODE_NUM,
};

//-------------------------------------------------------------
//    define end LAN topology informations                    -
//-------------------------------------------------------------
enum
{
    END_LAN1, // 10.1.0.0/24
    END_LAN2, // 10.2.0.0/24
    END_LAN3, // 10.3.0.0/24
    END_LAN4  // 10.4.0.0/24
};

const std::vector<std::vector<uint32_t>> endLANNodeIDs = {
    {END_HOST1, END_HOST2, EDGE_SERVER1},
    {END_HOST3, END_HOST4, EDGE_SERVER2},
    {END_HOST5, END_HOST6, EDGE_SERVER3},
    {END_HOST7, END_HOST8, EDGE_SERVER4},
};

static std::vector<IPaddrBase> endLANIPBases = {std::make_pair("10.1.0.0", "255.255.255.0"),
                                                std::make_pair("10.2.0.0", "255.255.255.0"),
                                                std::make_pair("10.3.0.0", "255.255.255.0"),
                                                std::make_pair("10.4.0.0", "255.255.255.0")};

//-------------------------------------------------------------
//    define core server1 neighborhoods information           -
//-------------------------------------------------------------
const std::vector<uint32_t> coreServer1Neighbors = {
    EDGE_SERVER1,
    EDGE_SERVER2,
    EDGE_SERVER3,
    EDGE_SERVER4,
};

static std::vector<IPaddrBase> coreServer1NeighborIPBase = {
    std::make_pair("20.1.1.0", "255.255.255.0"),
    std::make_pair("20.1.2.0", "255.255.255.0"),
    std::make_pair("20.1.3.0", "255.255.255.0"),
    std::make_pair("20.1.4.0", "255.255.255.0"),
};

//-------------------------------------------------------------
//    define core server2 neighborhoods information           -
//-------------------------------------------------------------
const std::vector<uint32_t> coreServer2Neighbors = {
    EDGE_SERVER1,
    EDGE_SERVER2,
    EDGE_SERVER3,
    EDGE_SERVER4,
};

static std::vector<IPaddrBase> coreServer2NeighborIPBase = {
    std::make_pair("20.2.1.0", "255.255.255.0"),
    std::make_pair("20.2.2.0", "255.255.255.0"),
    std::make_pair("20.2.3.0", "255.255.255.0"),
    std::make_pair("20.2.4.0", "255.255.255.0"),
};

void buildCsmaNetwork(NodeContainer&,
                      const std::vector<uint32_t>&,
                      IPaddrBase&,
                      const char*,
                      const uint32_t);
void p2pConnectToNeighbors(NodeContainer&,
                           const uint32_t,
                           const std::vector<uint32_t>&,
                           std::vector<IPaddrBase>&,
                           const char*,
                           const char*);
void testNodesConnectivity(Ptr<Node> srcNode, Ptr<Node> dstNode);
std::vector<Ipv4Address> getNodeIpv4List(Ptr<Node> node);
std::vector<std::vector<Ipv4Address>> getNodesIpv4List(NodeContainer nodes);

} // namespace ns3
#endif