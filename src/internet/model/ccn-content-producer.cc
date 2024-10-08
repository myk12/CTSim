#include "ns3/core-module.h"
#include "ns3/log.h"

#include "ccn-content-producer.h"
#include "ccn-header.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CCNContentProducer");

NS_OBJECT_ENSURE_REGISTERED(CCNContentProducer);

TypeId
CCNContentProducer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CCNContentProducer")
                            .SetParent<Object>()
                            .SetGroupName("Internet")
                            .AddConstructor<CCNContentProducer>();
    return tid;
}

CCNContentProducer::CCNContentProducer()
{
    NS_LOG_FUNCTION(this);
}

CCNContentProducer::~CCNContentProducer()
{
    NS_LOG_FUNCTION(this);
}

void
CCNContentProducer::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
    m_ccnl4 = m_node->GetObject<CCNL4Protocol>();
    NS_ASSERT_MSG(m_ccnl4, "CCNL4Protocol not found");
}

void
CCNContentProducer::SetCCNL4(Ptr<CCNL4Protocol> ccnl4)
{
    NS_LOG_FUNCTION(this << ccnl4);
    m_ccnl4 = ccnl4;
}

std::string
CCNContentProducer::GetContentName() const
{
    NS_LOG_FUNCTION(this);
    return m_content_name;
}

void
CCNContentProducer::SetContentName(std::string content_name)
{
    NS_LOG_FUNCTION(this << content_name);
    m_content_name = content_name;
}

void
CCNContentProducer::SetContentFile(std::string content_file)
{
    NS_LOG_FUNCTION(this << content_file);
    m_content_file = content_file;
}

void
CCNContentProducer::SendContentResponse(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr);

    // read content from file and send it in the packet
    std::ifstream file(m_content_file, std::ios::binary);

    if (!file.is_open())
    {
        NS_LOG_DEBUG("Failed to open file " << m_content_file);
        return;
    }

    NS_LOG_DEBUG("Opened file " << m_content_file);
    while (!file.eof())
    {
        char buffer[16];
        file.read(buffer, sizeof(buffer));
        Ptr<Packet> contentPacket = Create<Packet>((uint8_t*)buffer, file.gcount());
        
        NS_LOG_DEBUG("Sending content packet");
        m_ccnl4->SendData(contentPacket, m_content_name);
    }
}

} // namespace ns3
