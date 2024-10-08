#ifndef _CCN_PRODUCER_APP_H_
#define _CCN_PRODUCER_APP_H_


#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class CCNProducerApp : public Application
{
    public:
        /**
         * \brief Get the type ID
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        CCNProducerApp();
        ~CCNProducerApp() override;
        
        /**
         * \brief Install to node
         */
        void Install(Ptr<Node> node);

    private:
        void StartApplication() override;
        void StopApplication() override;

        Ptr<Node> m_node;
        std::string m_contentName;
        std::string m_contentFile;
};
}

#endif /* _CCN_PRODUCER_APP_H_ */