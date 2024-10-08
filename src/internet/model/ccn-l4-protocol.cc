/*
 * Copyright (c) 2005 INRIA
 *
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ccn-l4-protocol.h"
#include "ccn-header.h"

#include "ipv4-l3-protocol.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CCNL4Protocol");

NS_OBJECT_ENSURE_REGISTERED(CCNL4Protocol);

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t CCNL4Protocol::PROT_NUMBER = 147;

TypeId
CCNL4Protocol::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CCNL4Protocol")
                            .SetParent<IpL4Protocol>()
                            .SetGroupName("Internet")
                            .AddConstructor<CCNL4Protocol>();
    return tid;
}

CCNL4Protocol::CCNL4Protocol()
{
    NS_LOG_FUNCTION(this);
}

CCNL4Protocol::~CCNL4Protocol()
{
    NS_LOG_FUNCTION(this);
}

void
CCNL4Protocol::SetNode(Ptr<Node> node)
{
    m_node = node;
}

/*
 * This method is called by AggregateObject and completes the aggregation
 * by setting the node in the udp stack and link it to the ipv4 object
 * present in the node along with the socket factory
 */
void
CCNL4Protocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);
    Ptr<Node> node = this->GetObject<Node>();
    NS_ASSERT_MSG(node, "Node not found");
    Ptr<Ipv4> ipv4 = this->GetObject<Ipv4>();

    if (!m_node)
    {
        m_node = node;
    }

    // We set at least one of our 2 down targets to the IPv4/IPv6 send
    // functions.  Since these functions have different prototypes, we
    // need to keep track of whether we are connected to an IPv4 or
    // IPv6 lower layer and call the appropriate one.

    if (ipv4 && m_downTarget.IsNull())
    {
        ipv4->Insert(this);
        this->SetDownTarget(MakeCallback(&Ipv4::Send, ipv4));
    }

    IpL4Protocol::NotifyNewAggregate();
}

int
CCNL4Protocol::GetProtocolNumber() const
{
    return PROT_NUMBER;
}

void
CCNL4Protocol::DoDispose()
{
    NS_LOG_FUNCTION(this);

    for (auto it = m_contentConsumers.begin(); it != m_contentConsumers.end(); ++it)
    {
        (*it)->Dispose();
    }

    for (auto it = m_contentProducers.begin(); it != m_contentProducers.end(); ++it)
    {
        (*it)->Dispose();
    }

    m_node = nullptr;
    m_downTarget.Nullify();
    IpL4Protocol::DoDispose();
}

void
CCNL4Protocol::ReceiveIcmp(Ipv4Address icmpSource,
                           uint8_t icmpTtl,
                           uint8_t icmpType,
                           uint8_t icmpCode,
                           uint32_t icmpInfo,
                           Ipv4Address payloadSource,
                           Ipv4Address payloadDestination,
                           const uint8_t payload[8])
{
    NS_LOG_FUNCTION(this << icmpSource << icmpTtl << icmpType << icmpCode << icmpInfo
                         << payloadSource << payloadDestination);
    NS_LOG_INFO("Received ICMP message");
}

enum IpL4Protocol::RxStatus
CCNL4Protocol::Receive(Ptr<Packet> packet, const Ipv4Header& header, Ptr<Ipv4Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << header);
    // Content-centric networking:
    // there are two kinds of packets that we can receive:
    // - content requests (from consumers)
    // - content responses (from producers)
    // for the content requests, we need to forward the request to the producer
    // for the content responses, we need to forward the response to the consumer

    CCNHeader ccnheader;
    packet->PeekHeader(ccnheader);

    // print the header
    //ccnheader.Print(std::cout);

    // determine which kind of header
    if (ccnheader.GetMessageType() == CCNHeader::INTEREST)
    {
        // received a content request
        NS_LOG_DEBUG("Received a content request");
        HandleInterestPacket(packet, header.GetSource(), header.GetDestination());
    }
    else if (ccnheader.GetMessageType() == CCNHeader::DATA)
    {
        NS_LOG_DEBUG("Received a content response");
        HandleDataPacket(packet, header.GetSource(), header.GetDestination());
    }
    else
    {
        NS_LOG_DEBUG("Unknown message type");
    }

    return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus
CCNL4Protocol::Receive(Ptr<Packet> packet, const Ipv6Header& header, Ptr<Ipv6Interface> incomingInterface)
{
    NS_LOG_FUNCTION(this << packet << header);
    NS_LOG_DEBUG("Not implemented");
    return IpL4Protocol::RX_OK;
}

void
CCNL4Protocol::Send(Ptr<Packet> packet,
                    Ipv4Address saddr,
                    Ipv4Address daddr)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr);

    m_downTarget(packet, saddr, daddr, PROT_NUMBER, nullptr);
}

void
CCNL4Protocol::Send(Ptr<Packet> packet,
                    Ipv4Address saddr,
                    Ipv4Address daddr,
                    Ptr<Ipv4Route> route)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << route);

    m_downTarget(packet, saddr, daddr, PROT_NUMBER, route);
}

void
CCNL4Protocol::SetDownTarget(IpL4Protocol::DownTargetCallback callback)
{
    NS_LOG_FUNCTION(this);
    m_downTarget = callback;
}

void
CCNL4Protocol::SetDownTarget6(IpL4Protocol::DownTargetCallback6 callback)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Not implemented");
}

IpL4Protocol::DownTargetCallback
CCNL4Protocol::GetDownTarget() const
{
    return m_downTarget;
}

IpL4Protocol::DownTargetCallback6
CCNL4Protocol::GetDownTarget6() const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Not implemented");
    return IpL4Protocol::DownTargetCallback6();
}

Ptr<CCNContentConsumer>
CCNL4Protocol::CreateContentConsumer()
{
    Ptr<CCNContentConsumer> consumer = CreateObject<CCNContentConsumer>();
    consumer->SetNode(m_node);
    consumer->SetCCNL4(this);

    m_contentConsumers.push_back(consumer);
    return consumer;
}

Ptr<CCNContentProducer>
CCNL4Protocol::CreateContentProducer()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_node, "Node not set");
    Ptr<CCNContentProducer> producer = CreateObject<CCNContentProducer>();
    producer->SetNode(m_node);
    producer->SetCCNL4(this);

    m_contentProducers.push_back(producer);
    return producer;
}

void
CCNL4Protocol::AddContentPrefixToHostAddress(std::vector<std::pair<std::string, Ipv4Address>> contentPrefixToHostAddress)
{
    NS_LOG_FUNCTION(this);
    for (auto it = contentPrefixToHostAddress.begin(); it != contentPrefixToHostAddress.end(); ++it)
    {
        m_contentPrefixToHostAddress[it->first] = it->second;
    }
}

void
CCNL4Protocol::AddContentPrefixToHostAddress(std::string contentPrefix, Ipv4Address hostAddress)
{
    NS_LOG_FUNCTION(this << contentPrefix << hostAddress);
    m_contentPrefixToHostAddress[contentPrefix] = hostAddress;
}

Ipv4Address
CCNL4Protocol::GetHostAddressFromContentName(std::string contentName)
{
    NS_LOG_FUNCTION(this << contentName);
    // parse the content name
    // the content name is in the format /L1Name/L2Name/.../LxName/HostAddress
    // we need to extract the host address comply with the longest prefix match
    Ipv4Address hostAddress;

    // start with layer 1 until to whole content name
    std::string prefix = "";
    std::string::size_type pos = 0;

    while (pos != std::string::npos)
    {
        pos = contentName.find("/", pos + 1);
        if (pos != std::string::npos)
        {
            prefix = contentName.substr(0, pos);
            if (m_contentPrefixToHostAddress.find(prefix) != m_contentPrefixToHostAddress.end())
            {
                hostAddress = m_contentPrefixToHostAddress[prefix];
            }
        }
    }

    NS_LOG_DEBUG("Host address for content name " << contentName << " is " << hostAddress);

    return hostAddress;
}

void
CCNL4Protocol::SendInterest(Ptr<Packet> packet, std::string contentName)
{
    NS_LOG_FUNCTION(this << packet << contentName);

    // create a new header
    CCNHeader ccnheader;
    ccnheader.SetMessageType(CCNHeader::INTEREST);
    ccnheader.SetContentName(contentName);

    // add the header to the packet
    packet->AddHeader(ccnheader);

    // get destination address
    Ipv4Address destAddress = GetHostAddressFromContentName(contentName);
    if (destAddress == Ipv4Address::GetZero())
    {
        NS_LOG_DEBUG("No host address found for content name " << contentName);
        return;
    }

    // get this node's address
    Ipv4Address sourceAddress = m_node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

    NS_LOG_DEBUG("Sending interest packet to " << destAddress);

    // send the packet
    Send(packet, sourceAddress, destAddress);
}

void
CCNL4Protocol::SendData(Ptr<Packet> packet, std::string contentName)
{
    NS_LOG_FUNCTION(this << packet << contentName);

    // create a new header
    CCNHeader ccnheader;
    ccnheader.SetMessageType(CCNHeader::DATA);
    ccnheader.SetContentName(contentName);

    // add the header to the packet
    packet->AddHeader(ccnheader);

    // get destination address from pending interest table
    Ipv4Address destAddress = m_pendingInterestTable[contentName];
    if (destAddress == Ipv4Address::GetZero())
    {
        NS_LOG_DEBUG("No pending interest found for content name " << contentName);
        return;
    }

    // get this node's address
    Ipv4Address sourceAddress = m_node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

    NS_LOG_DEBUG("Sending data packet to " << destAddress);

    // send the packet
    Send(packet, sourceAddress, destAddress);
}

void
CCNL4Protocol::HandleInterestPacket(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr);
    // handle the interest packet
    // find the content producer that can serve the content
    
    // get packet header
    CCNHeader ccnheader;
    packet->PeekHeader(ccnheader);

    NS_LOG_INFO("Received interest packet for content " << ccnheader.GetContentName());

    // get the content name
    std::string contentName = ccnheader.GetContentName();
    // remove all null characters from the content names
    contentName.erase(contentName.find('\0'));

    // insert into pending interest table
    m_pendingInterestTable[contentName] = saddr;

    // find the content producer
    for (auto it = m_contentProducers.begin(); it != m_contentProducers.end(); ++it)
    {
        std::string producerName = (*it)->GetContentName();

        if (producerName == contentName)
        {
            // found the content producer
            // forward the interest packet to the producer
            NS_LOG_INFO("Found content producer for content name " << contentName);
            (*it)->SendContentResponse(packet, saddr, daddr);
            return;
        }
    }

    NS_LOG_DEBUG("No content producer found for content name " << contentName);
}

void
CCNL4Protocol::HandleDataPacket(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr);
    // handle the data packet
    // forward the data packet to the consumer

    // get packet header
    CCNHeader ccnheader;
    packet->PeekHeader(ccnheader);

    // get the content name
    std::string contentName = ccnheader.GetContentName();
    // remove all null characters from the content names
    contentName.erase(contentName.find('\0'));

    // find the content consumer
    for (auto it = m_contentConsumers.begin(); it != m_contentConsumers.end(); ++it)
    {
        if ((*it)->GetContentName() == contentName)
        {
            // found the content consumer
            // forward the data packet to the consumer
            (*it)->NotifyRecv(packet);
            return;
        }
    }
}

} // namespace ns3
