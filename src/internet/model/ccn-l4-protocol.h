/*
 * Copyright (c) 2005,2006,2007 INRIA
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
 * Author: Yuke Ma <mayuke803.gmail.com>
 */

#ifndef CCN_L4_PROTOCOL_H
#define CCN_L4_PROTOCOL_H

#include "ip-l4-protocol.h"
#include "ccn-content-producer.h"
#include "ccn-content-consumer.h"

#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <stdint.h>
#include <unordered_map>
#include <vector>

namespace ns3
{

class Node;
class CCNContentConsumer;
class CCNContentProducer;

/**
 * \ingroup CCN
 * \defgroup ccn CCN
 * 
 * This is an implementation of the Content Centric Networking (CCN) protocol.
 * CCN is a new networking paradigm that leverages the content itself to route
 * and forward packets. This is in contrast to the traditional IP networking
 * paradigm that uses the IP address to route and forward packets.
 * 
 */

/**
 * \ingroup CCN
 * \brief Implementation of the CCN L4 protocol
 */
class CCNL4Protocol : public IpL4Protocol
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    static const uint8_t PROT_NUMBER; //!< protocol number (0x11)

    CCNL4Protocol();
    ~CCNL4Protocol() override;

    // Delete copy constructor and assignment operator to avoid misuse
    CCNL4Protocol(const CCNL4Protocol&) = delete;
    CCNL4Protocol& operator=(const CCNL4Protocol&) = delete;

    int GetProtocolNumber() const override;

    // inherited from IpL4Protocol
    enum IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                        const Ipv4Header& header,
                                        Ptr<Ipv4Interface> interface) override;

    // inherited from IpL4Protocol
    enum IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                          const Ipv6Header& header,
                          Ptr<Ipv6Interface> incomingInterface) override;
    
    void ReceiveIcmp(Ipv4Address icmpSource,
                     uint8_t icmpTtl,
                     uint8_t icmpType,
                     uint8_t icmpCode,
                     uint32_t icmpInfo,
                     Ipv4Address payloadSource,
                     Ipv4Address payloadDestination,
                     const uint8_t payload[8]) override;
    
    // From IpL4Protocol
    void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
    // From IpL4Protocol
    void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
    // From IpL4Protocol
    IpL4Protocol::DownTargetCallback GetDownTarget() const override;
    // From IpL4Protocol
    IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;


    // called by CCNContentProducer
    /**
     * Send a packet to the network
     */
    void Send(Ptr<Packet> packet,
              Ipv4Address saddr,
              Ipv4Address daddr);

    // called by CCNContentProducer
    /**
     * Send a packet to the network
     */
    void Send(Ptr<Packet> packet,
              Ipv4Address saddr,
              Ipv4Address daddr,
              Ptr<Ipv4Route> route);

    /**
     * Set node associated with this stack
     * \param node the node
     */
    void SetNode(Ptr<Node> node);

    /**
     * \brief Batch add content prefix to host address mapping
     */
    void AddContentPrefixToHostAddress(std::vector<std::pair<std::string, Ipv4Address>> contentPrefixToHostAddress);
    /**
     * \brief Add a content prefix to host address mapping
     */
    void AddContentPrefixToHostAddress(std::string contentPrefix, Ipv4Address hostAddress);

    /**
     * \brief Create a new content consumer
     */
    Ptr<CCNContentConsumer> CreateContentConsumer();

    /**
     * \brief Create a new content producer
     */
    Ptr<CCNContentProducer> CreateContentProducer();

    /**
     * \brief Parse the content name and return the host address
     * \param contentName the content name
     * 
     * ContentName format: /L1Name/L2Name/.../LxName/HostAddress
     */
    Ipv4Address GetHostAddressFromContentName(std::string contentName);

    /**
     * \brief Send Interest packet
     */
    void SendInterest(Ptr<Packet> packet, std::string contentName);

    /**
     * \brief Send Data packet
     */
    void SendData(Ptr<Packet> packet, std::string contentName);

    /**
     * \brief Handle received Interest packet
     */
    void HandleInterestPacket(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr);

    /**
     * \brief Handle received Data packet
     */
    void HandleDataPacket(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr);

  protected:
    void DoDispose() override;
    /*
     * This function will notify other components connected to the node that a new stack member is
     * now connected This will be used to notify Layer 3 protocol of layer 4 protocol stack to
     * connect them together.
     */
    void NotifyNewAggregate() override;

  private:
    Ptr<Node> m_node;                //!< the node this stack is associated with
    std::vector<Ptr<CCNContentConsumer>> m_contentConsumers; //!< the content consumers
    std::vector<Ptr<CCNContentProducer>> m_contentProducers; //!< the content producers

    IpL4Protocol::DownTargetCallback m_downTarget;   //!< Callback to send packets over IPv4

    // content prefix to host address mapping
    std::unordered_map<std::string, Ipv4Address> m_contentPrefixToHostAddress;

    // pending Interest table
    std::unordered_map<std::string, Ipv4Address> m_pendingInterestTable;
};

} // namespace ns3

#endif /* UDP_L4_PROTOCOL_H */
