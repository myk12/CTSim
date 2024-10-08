#include "ccn-header.h"

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(CCNHeader);

CCNHeader::CCNHeader()
    : m_content_name(""),
      m_selector(0),
      m_nonce(0)
{
}

CCNHeader::~CCNHeader()
{
    m_content_name = "";
    m_selector = 0;
    m_nonce = 0;
}

void
CCNHeader::SetContentName(std::string content_name)
{
    m_content_name = content_name;
}

std::string
CCNHeader::GetContentName() const
{
    return m_content_name;
}

void
CCNHeader::SetMessageType(uint8_t type)
{
    m_type = type;
}

uint8_t
CCNHeader::GetMessageType() const
{
    return m_type;
}

TypeId
CCNHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CCNHeader")
                            .SetParent<Header>()
                            .SetGroupName("ccn")
                            .AddConstructor<CCNHeader>();
    return tid;
}

TypeId
CCNHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
CCNHeader::Print(std::ostream& os) const
{
    os << "------- CCNHeader -------" << std::endl;
    if (m_type == 0)
    {
        os << "Type: Interest" << std::endl;
    }
    else
    {
        os << "Type: Data" << std::endl;
    }
    os << "Content Name: " << m_content_name << std::endl;
    os << "-------------------------" << std::endl;
}

uint32_t
CCNHeader::GetSerializedSize() const
{
    // content name + packet type
    return m_name_length + m_type_length;
}

void
CCNHeader::Serialize(Buffer::Iterator start) const
{
    // serialize the content name and the packet type
    for (uint32_t i = 0; i < m_content_name.size(); i++)
    {
        start.WriteU8(m_content_name[i]);
    }

    int padding = m_name_length - m_content_name.size();
    NS_ASSERT_MSG(padding >= 0, "Content name is too long");
    for (int i = 0; i < padding; i++)
    {
        start.WriteU8(0);
    }

    start.WriteU8(m_type);
}

uint32_t
CCNHeader::Deserialize(Buffer::Iterator start)
{
    // deserialize the content name and the packet type
    for (uint32_t i = 0; i < m_name_length; i++)
    {
        m_content_name += start.ReadU8();
    }

    m_type = start.ReadU8();

    return m_name_length + m_type_length;
}



} // namespace ns3
