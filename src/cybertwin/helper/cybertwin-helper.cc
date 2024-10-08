#include "cybertwin-helper.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinHelper");

CybertwinHelper::CybertwinHelper()
{
}

CybertwinHelper::CybertwinHelper(std::string name)
{
    m_factory.SetTypeId(name);
}

void
CybertwinHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
CybertwinHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
CybertwinHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
CybertwinHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);

    return app;
}

CybertwinConnHelper::CybertwinConnHelper()
{
    m_factory.SetTypeId(CybertwinConnClient::GetTypeId());
}

void
CybertwinConnHelper::SetCertificate(Ptr<Node> node, const CybertwinCertTag& cert)
{
    for (uint32_t j = 0; j < node->GetNApplications(); j++)
    {
        Ptr<CybertwinConnClient> connClient =
            DynamicCast<CybertwinConnClient>(node->GetApplication(j));
        if (connClient)
        {
            connClient->SetCertificate(cert);
        }
    }
}

} // namespace ns3