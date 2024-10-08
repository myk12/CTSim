#include "ccn-producer-app.h"

#include "ns3/ccn-l4-protocol.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CCNProducerApp");

NS_OBJECT_ENSURE_REGISTERED(CCNProducerApp);

TypeId
CCNProducerApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CCNProducerApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<CCNProducerApp>()
                            .AddAttribute("ContentName", "The name of the content to produce",
                                          StringValue(""),
                                          MakeStringAccessor(&CCNProducerApp::m_contentName),
                                          MakeStringChecker())
                            .AddAttribute("ContentFile", "The file to produce",
                                            StringValue(""),
                                            MakeStringAccessor(&CCNProducerApp::m_contentFile),
                                            MakeStringChecker());
                                        
    return tid;
}

CCNProducerApp::CCNProducerApp()
{
    NS_LOG_FUNCTION(this);
}

CCNProducerApp::~CCNProducerApp()
{
    NS_LOG_FUNCTION(this);
}

void
CCNProducerApp::Install(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
    node->AddApplication(this);
}

void
CCNProducerApp::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // Create a CCN Producer
    Ptr<CCNL4Protocol> ccnl4 = m_node->GetObject<CCNL4Protocol>();
    NS_ASSERT_MSG(ccnl4, "CCNL4Protocol not found");
    Ptr<CCNContentProducer> producer = ccnl4->CreateContentProducer();

    // get node name
    std::string nodeName = Names::FindName(m_node);

    // set the content name
    producer->SetContentName(nodeName + "/" + m_contentName);
    producer->SetContentFile(m_contentFile);
}

void
CCNProducerApp::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3
