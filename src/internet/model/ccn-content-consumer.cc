#include "ccn-content-consumer.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CCNContentConsumer");

NS_OBJECT_ENSURE_REGISTERED(CCNContentConsumer);

TypeId
CCNContentConsumer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CCNContentConsumer")
                            .SetParent<Object>()
                            .SetGroupName("Internet")
                            .AddConstructor<CCNContentConsumer>();
    return tid;
}

CCNContentConsumer::CCNContentConsumer()
{
    NS_LOG_FUNCTION(this);
}

CCNContentConsumer::~CCNContentConsumer()
{
    NS_LOG_FUNCTION(this);
}

void
CCNContentConsumer::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
    m_ccnl4 = m_node->GetObject<CCNL4Protocol>();
}

void
CCNContentConsumer::SetCCNL4(Ptr<CCNL4Protocol> ccn_l4)
{
    NS_LOG_FUNCTION(this << ccn_l4);
    m_ccnl4 = ccn_l4;
}

void
CCNContentConsumer::SetContentName(std::string content_name)
{
    NS_LOG_FUNCTION(this << content_name);
    m_content_name = content_name;
}

std::string
CCNContentConsumer::GetContentName() const
{
    NS_LOG_FUNCTION(this);
    return m_content_name;
}

void
CCNContentConsumer::SetRecvCallback(Callback<void, Ptr<Packet>> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_recvCb = callback;
}

void
CCNContentConsumer::GetContent()
{
    // create a new packet
    Ptr<Packet> packet = Create<Packet>();

    // send through the L4 protocol
    m_ccnl4->SendInterest(packet, m_content_name);
}

void
CCNContentConsumer::NotifyRecv(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    if (m_recvCb.IsNull()) {
        NS_LOG_WARN("No callback function set");
        return;
    }

    // remove the header
    CCNHeader ccnheader;
    packet->RemoveHeader(ccnheader);

    m_recvCb(packet);
}

} // namespace ns3



