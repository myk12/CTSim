#include "cybertwin-packet-tags.h"

using namespace ns3;

//*********************************************************************
//*                   Cybertwin Multipath Tag                         *
//*********************************************************************

TypeId
CybertwinMpTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinMpTag")
                            .SetParent<Tag>()
                            .AddConstructor<CybertwinMpTag>();

    return tid;
}

TypeId
CybertwinMpTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
CybertwinMpTag::GetSerializedSize() const
{
    uint32_t size = 0;
    size += sizeof(CYBERTWINID_t)
        + sizeof(uint8_t);
    return size;
}

void
CybertwinMpTag::Serialize(TagBuffer i) const
{
    i.WriteU64(cyberID);
    //i.WriteU8(kind);
}

void
CybertwinMpTag::Deserialize(TagBuffer i)
{
    cyberID = i.ReadU64();
    //kind = i.ReadU8();
}

void
CybertwinMpTag::Print(std::ostream& os) const
{
    os << "{CyberID: "<< cyberID
        //<< ", kind = " <<(uint32_t)kind
        << "}";
}

void
CybertwinMpTag::SetCybertwinID(CYBERTWINID_t id)
{
    cyberID = id;
}

CYBERTWINID_t
CybertwinMpTag::GetCybertwinID()
{
    return cyberID;
}

//*********************************************************************
//*                   Cybertwin Multipath Tag -- Join                 *
//*********************************************************************
TypeId
CybertwinMpTagJoin::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinMpTagJoin")
                            .SetParent<Tag>()
                            .AddConstructor<CybertwinMpTagJoin>();

    return tid;
}

TypeId
CybertwinMpTagJoin::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinMpTagJoin::CybertwinMpTagJoin():
    senderKey(0),
    recverKey(0),
    connectionID(0)
{
    cyberID = 0;
    kind = CONN_JOIN;
}

uint32_t
CybertwinMpTagJoin::GetSerializedSize() const
{
    uint32_t size = 0;
    size += sizeof(CYBERTWINID_t)
        + sizeof(uint8_t)
        + sizeof(MP_CONN_KEY_t)
        + sizeof(MP_CONN_KEY_t)
        + sizeof(MP_CONN_ID_t);
    NS_LOG_UNCOND("size " << size);
    return size;
}

void
CybertwinMpTagJoin::Serialize(TagBuffer i) const
{
    i.WriteU64(cyberID);
    i.WriteU8(kind);
    i.WriteU32(senderKey);
    i.WriteU64(recverKey);
    i.WriteU64(connectionID);
}

void
CybertwinMpTagJoin::Deserialize(TagBuffer i)
{
    cyberID = i.ReadU64();
    kind = i.ReadU8();
    senderKey = i.ReadU32();
    recverKey = i.ReadU32();
    connectionID = i.ReadU64();
}

void
CybertwinMpTagJoin::Print(std::ostream& os) const
{
    os << "{CyberID: "<< cyberID
        //<< ", kind = " <<(uint32_t)kind
        << ", senderKey = "<< senderKey
        << ", recverKey = "<< recverKey
        << "}";
}

void 
CybertwinMpTagJoin::SetCybertwinID(CYBERTWINID_t id)
{
    cyberID = id;
}

CYBERTWINID_t
CybertwinMpTagJoin::GetCybertwinID()
{
    return cyberID;
}

uint8_t
CybertwinMpTagJoin::GetKind()
{
    return  (uint8_t)CONN_JOIN;
}

void
CybertwinMpTagJoin::SetSenderKey(MP_CONN_KEY_t key)
{
    senderKey = key;
}

MP_CONN_KEY_t
CybertwinMpTagJoin::GetSenderKey()
{
    return senderKey;
}

void
CybertwinMpTagJoin::SetRecverKey(MP_CONN_KEY_t key)
{
    recverKey = key;
}

MP_CONN_KEY_t
CybertwinMpTagJoin::GetRecverKey()
{
    return recverKey;
}

void
CybertwinMpTagJoin::SetConnectionID(MP_CONN_KEY_t id)
{
    connectionID = id;
}

MP_CONN_KEY_t
CybertwinMpTagJoin::GetConnectionID()
{
    return connectionID;
}

//*********************************************************************
//*                   Cybertwin Multipath Tag -- Data                 *
//*********************************************************************
TypeId
CybertwinMpTagData::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinMpTagData")
                            .SetParent<Tag>()
                            .AddConstructor<CybertwinMpTagData>();

    return tid;
}

TypeId
CybertwinMpTagData::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinMpTagData::CybertwinMpTagData():
    senderKey(0),
    recverKey(0),
    connectionID(0)
{
}

uint32_t
CybertwinMpTagData::GetSerializedSize() const
{
    uint32_t size = 0;
    size = sizeof(CYBERTWINID_t)
        //+ sizeof(uint8_t)
        + sizeof(MP_CONN_KEY_t)
        + sizeof(MP_CONN_KEY_t)
        + sizeof(MP_CONN_ID_t);
    return size;
}

void
CybertwinMpTagData::Serialize(TagBuffer i) const
{
    i.WriteU64(cyberID);
    //i.WriteU8(CONN_DATA);
    i.WriteU32(senderKey);
    i.WriteU64(recverKey);
    i.WriteU64(connectionID);
}

void
CybertwinMpTagData::Deserialize(TagBuffer i)
{
    cyberID = i.ReadU64();
    //kind = i.ReadU8();
    senderKey = i.ReadU32();
    recverKey = i.ReadU32();
    connectionID = i.ReadU64();
}

void
CybertwinMpTagData::Print(std::ostream& os) const
{
    os << "{CyberID: "<< cyberID
        //<< ", kind = " <<(uint32_t)kind
        << ", senderKey = "<< senderKey
        << ", recverKey = "<< recverKey
        << "}";
}

uint8_t
CybertwinMpTagData::GetKind()
{
    return  CONN_JOIN;
}

void
CybertwinMpTagData::SetSenderKey(MP_CONN_KEY_t key)
{
    senderKey = key;
}

MP_CONN_KEY_t
CybertwinMpTagData::GetSenderKey()
{
    return senderKey;
}

void
CybertwinMpTagData::SetRecverKey(MP_CONN_KEY_t key)
{
    recverKey = key;
}

MP_CONN_KEY_t
CybertwinMpTagData::GetRecverKey()
{
    return recverKey;
}

void
CybertwinMpTagData::SetConnectionID(MP_CONN_KEY_t id)
{
    connectionID = id;
}

MP_CONN_KEY_t
CybertwinMpTagData::GetConnectionID()
{
    return connectionID;
}
