#ifndef CYBERTWIN_HELPER_H
#define CYBERTWIN_HELPER_H

#include "../model/cybertwin-client.h"
#include "../model/cybertwin-tag.h"

#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/string.h"

namespace ns3
{

class CybertwinHelper
{
  public:
    CybertwinHelper();
    CybertwinHelper(std::string);

    void SetAttribute(std::string, const AttributeValue& value);

    ApplicationContainer Install(NodeContainer) const;
    ApplicationContainer Install(Ptr<Node>) const;

  protected:
    Ptr<Application> InstallPriv(Ptr<Node>) const;
    ObjectFactory m_factory;
};

class CybertwinConnHelper : public CybertwinHelper
{
  public:
    CybertwinConnHelper();

    void SetCertificate(Ptr<Node>, const CybertwinCertTag&);
};

} // namespace ns3

#endif /* CYBERTWIN_HELPER_H */
