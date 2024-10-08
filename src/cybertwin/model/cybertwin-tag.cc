#include "cybertwin-tag.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinCert");
NS_OBJECT_ENSURE_REGISTERED(CybertwinTag);
NS_OBJECT_ENSURE_REGISTERED(CybertwinCreditTag);
NS_OBJECT_ENSURE_REGISTERED(CybertwinCertTag);

CybertwinTag::CybertwinTag(CYBERTWINID_t cuid)
    : m_cuid(cuid)
{
}

TypeId
CybertwinTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinTag")
                            .SetParent<Tag>()
                            .SetGroupName("cybertwin")
                            .AddConstructor<CybertwinTag>();
    return tid;
}

TypeId
CybertwinTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
CybertwinTag::GetSerializedSize() const
{
    return sizeof(m_cuid);
}

void
CybertwinTag::Serialize(TagBuffer i) const
{
    i.WriteU64(m_cuid);
}

void
CybertwinTag::Deserialize(TagBuffer i)
{
    m_cuid = i.ReadU64();
}

void
CybertwinTag::Print(std::ostream& os) const
{
    os << "cybertwin=" << m_cuid;
}

std::string
CybertwinTag::ToString() const
{
    std::ostringstream oss;
    Print(oss);
    return oss.str();
}

void
CybertwinTag::SetCybertwin(CYBERTWINID_t val)
{
    m_cuid = val;
}

CYBERTWINID_t
CybertwinTag::GetCybertwin() const
{
    return m_cuid;
}

CybertwinCreditTag::CybertwinCreditTag(CYBERTWINID_t cuid, uint16_t credit, CYBERTWINID_t peer)
    : CybertwinTag(cuid),
      m_credit(credit),
      m_peer(peer)
{
}

TypeId
CybertwinCreditTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinCreditTag")
                            .SetParent<CybertwinTag>()
                            .SetGroupName("cybertwin")
                            .AddConstructor<CybertwinCreditTag>();
    return tid;
}

TypeId
CybertwinCreditTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
CybertwinCreditTag::GetSerializedSize() const
{
    return sizeof(m_cuid) + sizeof(m_credit) + sizeof(m_peer);
}

void
CybertwinCreditTag::Serialize(TagBuffer i) const
{
    i.WriteU64(m_cuid);
    i.WriteU16(m_credit);
    i.WriteU64(m_peer);
}

void
CybertwinCreditTag::Deserialize(TagBuffer i)
{
    m_cuid = i.ReadU64();
    m_credit = i.ReadU16();
    m_peer = i.ReadU64();
}

void
CybertwinCreditTag::Print(std::ostream& os) const
{
    os << "cybertwin=" << m_cuid << ", credit=" << m_credit << ", peer=" << m_peer;
}

void
CybertwinCreditTag::SetCredit(uint16_t val)
{
    m_credit = val;
}

uint16_t
CybertwinCreditTag::GetCredit() const
{
    return m_credit;
}

void
CybertwinCreditTag::SetPeer(CYBERTWINID_t val)
{
    m_peer = val;
}

CYBERTWINID_t
CybertwinCreditTag::GetPeer() const
{
    return m_peer;
}

CybertwinCertTag::CybertwinCertTag(CYBERTWINID_t cuid,
                                   uint16_t initialCredit,
                                   uint16_t ingressCredit,
                                   bool isUserRequired,
                                   bool isCertValid,
                                   CYBERTWINID_t usr,
                                   uint16_t usrInitialCredit)
    : CybertwinTag(cuid),
      m_initialCredit(initialCredit),
      m_ingressCredit(ingressCredit),
      m_isUserRequired(isUserRequired),
      m_isCertValid(isCertValid),
      m_usr(usr),
      m_usrInitialCredit(usrInitialCredit)
{
}

TypeId
CybertwinCertTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinCertTag")
                            .SetParent<Tag>()
                            .SetGroupName("cybertwin")
                            .AddConstructor<CybertwinCertTag>();
    return tid;
}

TypeId
CybertwinCertTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
CybertwinCertTag::GetSerializedSize() const
{
    return sizeof(m_cuid) + sizeof(m_initialCredit) + sizeof(m_ingressCredit) +
           sizeof(m_isUserRequired) + sizeof(m_isCertValid) +
           (m_isUserRequired ? sizeof(m_usr) + sizeof(m_usrInitialCredit) : 0);
}

void
CybertwinCertTag::Serialize(TagBuffer i) const
{
    i.WriteU64(m_cuid);
    i.WriteU16(m_initialCredit);
    i.WriteU16(m_ingressCredit);
    i.WriteU8(m_isUserRequired);
    i.WriteU8(m_isCertValid);
    if (m_isUserRequired)
    {
        i.WriteU64(m_usr);
        i.WriteU16(m_usrInitialCredit);
    }
}

void
CybertwinCertTag::Deserialize(TagBuffer i)
{
    m_cuid = i.ReadU64();
    m_initialCredit = i.ReadU16();
    m_ingressCredit = i.ReadU16();
    m_isUserRequired = i.ReadU8();
    m_isCertValid = i.ReadU8();
    if (m_isUserRequired)
    {
        m_usr = i.ReadU64();
        m_usrInitialCredit = i.ReadU16();
    }
}

void
CybertwinCertTag::Print(std::ostream& os) const
{
    os << "cybertwin=" << m_cuid << ", credit=" << m_initialCredit
       << ", ingressCredit=" << m_ingressCredit << ", isUserRequired=" << m_isUserRequired
       << ", isCertValid=" << m_isCertValid;
    if (m_isUserRequired)
    {
        os << ", user=" << m_usr << ", userInitialCredit=" << m_usrInitialCredit;
    }
}

void
CybertwinCertTag::SetInitialCredit(uint16_t val)
{
    m_initialCredit = val;
}

uint16_t
CybertwinCertTag::GetInitialCredit() const
{
    return m_initialCredit;
}

void
CybertwinCertTag::SetIngressCredit(uint16_t val)
{
    m_ingressCredit = std::max(m_ingressCredit, val);
}

uint16_t
CybertwinCertTag::GetIngressCredit() const
{
    return m_ingressCredit;
}

void
CybertwinCertTag::SetIsUserRequired(bool val)
{
    m_isUserRequired = val;
}

bool
CybertwinCertTag::GetIsUserRequired() const
{
    return m_isUserRequired;
}

void
CybertwinCertTag::SetIsValid(bool val)
{
    m_isCertValid = val;
}

bool
CybertwinCertTag::GetIsValid() const
{
    return m_isCertValid;
}

CYBERTWINID_t
CybertwinCertTag::GetUser() const
{
    return m_usr;
}

uint16_t
CybertwinCertTag::GetUserInitialCredit() const
{
    return m_usrInitialCredit;
}


//*****************************************************************************
//*                 Multipath Connection Tag                                  *
//*****************************************************************************
NS_OBJECT_ENSURE_REGISTERED(MultipathTagConn);

TypeId MultipathTagConn::GetTypeId (void)
{
  static TypeId tid = TypeId ("MultipathTagConn")
    .SetParent<Tag> ()
    .AddConstructor<MultipathTagConn> ();
  return tid;
}

TypeId MultipathTagConn::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t MultipathTagConn::GetSerializedSize (void) const
{
  return sizeof(m_pathId) + sizeof(m_cuid) + sizeof(m_senderKey) + sizeof(m_recverKey) + sizeof(m_connId);
}

void MultipathTagConn::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_pathId);
  i.WriteU64 (m_cuid);
  i.WriteU32 (m_senderKey);
  i.WriteU32 (m_recverKey);
  i.WriteU64 (m_connId);
}

void MultipathTagConn::Deserialize (TagBuffer i)
{
  m_pathId = i.ReadU32 ();
  m_cuid = i.ReadU64 ();
  m_senderKey = i.ReadU32 ();
  m_recverKey = i.ReadU32 ();
  m_connId = i.ReadU64 ();
}

void MultipathTagConn::Print (std::ostream &os) const
{
  os << " MultipathTagConn {" ;
  os << "pathId=" << m_pathId << ", ";
  os << "cuid=" << m_cuid << ", ";
  os << "senderKey=" << m_senderKey << ", ";
  os << "recverKey=" << m_recverKey << ", ";
  os << "connId=" << m_connId;
  os << "} ";
}

void MultipathTagConn::SetPathId (uint32_t pathId)
{
  m_pathId = pathId;
}

void MultipathTagConn::SetCuid (uint64_t cuid)
{
  m_cuid = cuid;
}

void MultipathTagConn::SetSenderKey (uint32_t senderKey)
{
  m_senderKey = senderKey;
}

void MultipathTagConn::SetRecverKey (uint32_t recverKey)
{
  m_recverKey = recverKey;
}

void MultipathTagConn::SetConnId (uint64_t connId)
{
  m_connId = connId;
}

uint32_t MultipathTagConn::GetPathId (void) const
{
  return m_pathId;
}

uint64_t MultipathTagConn::GetCuid (void) const
{
  return m_cuid;
}

uint32_t MultipathTagConn::GetSenderKey (void) const
{
  return m_senderKey;
}

uint32_t MultipathTagConn::GetRecverKey (void) const
{
  return m_recverKey;
}

uint64_t MultipathTagConn::GetConnId (void) const
{
  return m_connId;
}


} // namespace ns3