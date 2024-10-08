#ifndef _CCN_CONSUMER_APP_H_
#define _CCN_CONSUMER_APP_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class CCNConsumerApp : public Application
{
    public:
        /**
         * \brief Get the type ID
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        CCNConsumerApp();
        ~CCNConsumerApp() override;
        
        /**
         * \brief Install to node
         */
        void Install(Ptr<Node> node);

        /**
         * \brief Recv callback
         */
        void RecvCallback(Ptr<Packet> packet);

    private:
        void StartApplication() override;
        void StopApplication() override;

        Ptr<Node> m_node;
        std::string m_contentName;
};

}

#endif /* _CCN_CONSUMER_APP_H_ */