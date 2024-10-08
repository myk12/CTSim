#include "ccn-consumer-app.h"

#include "ns3/ccn-l4-protocol.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CCNConsumerApp");

NS_OBJECT_ENSURE_REGISTERED(CCNConsumerApp);

TypeId
CCNConsumerApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CCNConsumerApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<CCNConsumerApp>()
                            .AddAttribute("ContentNames",
                                          "The list of content names to be fetched",
                                          StringValue(""),
                                          MakeStringAccessor(&CCNConsumerApp::m_contentName),
                                          MakeStringChecker());
    return tid;
}

CCNConsumerApp::CCNConsumerApp()
{
    NS_LOG_FUNCTION(this);
}

CCNConsumerApp::~CCNConsumerApp()
{
    NS_LOG_FUNCTION(this);
}

void
CCNConsumerApp::Install(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
    node->AddApplication(this);
}

void
CCNConsumerApp::RecvCallback(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    NS_LOG_INFO("Received content packet");
    // tranfer the packet content to a string
    uint8_t buffer[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string content((char*)buffer, packet->GetSize());
    NS_LOG_INFO("Content: " << content);
}

void
CCNConsumerApp::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // Create a CCN Consumer
    Ptr<CCNL4Protocol> ccnl4 = m_node->GetObject<CCNL4Protocol>();
    Ptr<CCNContentConsumer> consumer = ccnl4->CreateContentConsumer();

    // Initialize the consumer
    // set the callback function
    consumer->SetRecvCallback(MakeCallback(&CCNConsumerApp::RecvCallback, this));
    consumer->SetContentName(m_contentName);

    // get the content
    consumer->GetContent();
}

void
CCNConsumerApp::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3