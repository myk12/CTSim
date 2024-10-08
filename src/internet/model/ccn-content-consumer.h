#ifndef     _CCN_CONTENT_CONSUMER_H_
#define     _CCN_CONTENT_CONSUMER_H_

#include "ns3/object.h"
#include "ns3/node.h"
#include "ccn-l4-protocol.h"

namespace ns3
{

class Node;
class CCNL4Protocol;

class CCNContentConsumer : public Object
{
    public:
        /**
         * \brief Get the type ID
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        CCNContentConsumer();
        ~CCNContentConsumer() override;

        /**
         * \brief Set the node
         */

        void SetNode(Ptr<Node> node);

        /**
         * \brief Set the L4 protocol
         */
        void SetCCNL4(Ptr<CCNL4Protocol> ccnl4);

        /**
         * \brief Get Content
         */
        void GetContent();

        /**
         * \brief Set the content name
         */
        void SetContentName(std::string content_name);

        /**
         * \brief Get the content name
         */
        std::string GetContentName() const;

        /**
         * \brief Set Callback function to be called when content is received
         */
        void SetRecvCallback(Callback<void, Ptr<Packet>> callback);

        /**
         * \brief Notify packet reception
         */
        void NotifyRecv(Ptr<Packet> packet);

    private:
        Ptr<Node> m_node;
        Ptr<CCNL4Protocol> m_ccnl4;

        std::string m_content_name;

        // callback functions
        Callback<void, Ptr<Packet>> m_recvCb;
};

}

#endif