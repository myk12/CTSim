#ifndef _CCN_CONTENT_PRODUCER_H_
#define _CCN_CONTENT_PRODUCER_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/node.h"

#include "ccn-l4-protocol.h"

#include <unordered_map>

namespace ns3
{

class Node;
class CCNL4Protocol;

class CCNContentProducer : public Object
{
    public:
        /**
         * \brief Get the type ID
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        CCNContentProducer();
        ~CCNContentProducer() override;
        
        /**
         * \brief Send Content Response
         * \param packet the packet to send
         */
        void SendContentResponse(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr);

        /**
         * \brief Set the node
         */
        void SetNode(Ptr<Node> node);

        /**
         * \brief Set the L4 protocol
         */
        void SetCCNL4(Ptr<CCNL4Protocol> ccnl4);

        /**
         * \brief Get the content name
         */
        std::string GetContentName() const;

        /**
         * \brief Set the content name
         */
        void SetContentName(std::string content_name);

        /**
         * \brief Set the content file
         */
        void SetContentFile(std::string content_file);

    private:
        Ptr<Node> m_node;
        std::string m_content_name;
        std::string m_content_file;

        Ptr<CCNL4Protocol> m_ccnl4;
};

}




#endif