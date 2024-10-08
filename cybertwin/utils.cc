#include "cybertwin-topology.h"

#include "ns3/applications-module.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/net-device-container.h"
#include "ns3/v4ping-helper.h"
namespace ns3
{

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
void
buildCsmaNetwork(NodeContainer& allnodes,
                 const std::vector<uint32_t>& nodeIds,
                 IPaddrBase& addrBase,
                 const char* datarate,
                 const uint32_t delay)
{
    NodeContainer nodesCon;
    CsmaHelper csma;
    NetDeviceContainer csmaDevices;
    Ipv4AddressHelper address;

    for (uint32_t i = 0; i < nodeIds.size(); i++)
    {
        nodesCon.Add(allnodes.Get(nodeIds[i]));
    }

    // set channel attribute
    csma.SetChannelAttribute("DataRate", StringValue(datarate));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));

    csmaDevices = csma.Install(nodesCon);

    // assign ip
    address.SetBase(addrBase.first.c_str(), addrBase.second.c_str());
    address.Assign(csmaDevices);
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
void
p2pConnectToNeighbors(NodeContainer& allNodes,
                      const uint32_t centerID,
                      const std::vector<uint32_t>& neighborIDs,
                      std::vector<IPaddrBase>& neighborsIPBase,
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

    p2pHelper.SetDeviceAttribute("DataRate", StringValue(datarate));
    p2pHelper.SetChannelAttribute("Delay", StringValue(delay));

    for (uint32_t i = 0; i < nNeighbors; i++)
    {
        uint32_t neighborID = neighborIDs[i];
        neighbor = allNodes.Get(neighborID);
        addrBase = neighborsIPBase[i];

        // install p2p channel & devices
        devices = p2pHelper.Install(center, neighbor);

        // assign ip
        address.SetBase(addrBase.first.c_str(), addrBase.second.c_str());
        address.Assign(devices);
    }
}

void
testNodesConnectivity(Ptr<Node> srcNode, Ptr<Node> dstNode)
{
    std::vector<Ipv4Address> srcIPList = getNodeIpv4List(dstNode);
    std::vector<Ipv4Address> dstIPList = getNodeIpv4List(dstNode);
    uint32_t i = 0;
    for (Ipv4Address dstIp : dstIPList)
    {
        V4PingHelper pingHelper(dstIp);
        std::cout << "-> -> Ping Test Nodes Connectivity:\n\t from " << srcIPList[0] << " to "
                  << dstIp << std::endl;
        pingHelper.SetAttribute("Interval", TimeValue(Seconds(1)));
        pingHelper.SetAttribute("Size", UintegerValue(100));
        ApplicationContainer pingContainer = pingHelper.Install(srcNode);
        pingContainer.Start(Seconds(10 * (i + 1)));
        pingContainer.Stop(Seconds(10 * (i + 1) + 1));
        i++;
    }
}

std::vector<Ipv4Address>
getNodeIpv4List(Ptr<Node> node)
{
    std::vector<Ipv4Address> ipList;
    Ptr<Ipv4L3Protocol> ipv4 = node->GetObject<Ipv4L3Protocol>();
    uint32_t nIf = ipv4->GetNInterfaces();
    for (uint32_t i = 1; i < nIf; i++) // iteration start with 1 to skip 127.0.0.1
    {
        Ptr<Ipv4Interface> ipv4If = ipv4->GetInterface(i);
        uint32_t nAddr = ipv4If->GetNAddresses();
        for (uint32_t j = 0; j < nAddr; j++)
        {
            Ipv4InterfaceAddress ifAddr = ipv4If->GetAddress(j);
            ipList.push_back(ifAddr.GetAddress());
        }
    }
    return ipList;
}

std::vector<std::vector<Ipv4Address>>
getNodesIpv4List(NodeContainer nodes)
{
    std::vector<std::vector<Ipv4Address>> retval;
    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); i++)
    {
        retval.push_back(getNodeIpv4List(*i));
    }

    return retval;
}

} // namespace ns3