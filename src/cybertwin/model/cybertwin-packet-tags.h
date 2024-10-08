#ifndef CYBERTWIN_PACKET_TAGS_H
#define CYBERTWIN_PACKET_TAGS_H

#include "cybertwin-common.h"
#include "ns3/tag.h"
#include "ns3/type-id.h"
#include "ns3/packet.h"

using namespace ns3;

//*********************************************************************
//*                   Cybertwin Multipath Tag                         *
//*********************************************************************

class CybertwinMpTag: public Tag
{
public:
    enum TAG_KIND_E
    {
        CONN_INIT, //create new connection
        CONN_JOIN, //join in an existing connection
        CONN_DATA, //data transfer
    };
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

    void SetCybertwinID(CYBERTWINID_t id);
    CYBERTWINID_t GetCybertwinID();
protected:
    uint8_t kind;
    CYBERTWINID_t cyberID;
};


//*********************************************************************
//*                   Cybertwin Multipath Tag -- Join                 *
//*********************************************************************
class CybertwinMpTagJoin: public CybertwinMpTag
{
public:
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    CybertwinMpTagJoin();

    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

    void SetCybertwinID(CYBERTWINID_t id);
    CYBERTWINID_t GetCybertwinID();
    uint8_t GetKind();
    void SetSenderKey(MP_CONN_KEY_t key);
    MP_CONN_KEY_t GetSenderKey();
    void SetRecverKey(MP_CONN_KEY_t key);
    MP_CONN_KEY_t GetRecverKey();
    void SetConnectionID(MP_CONN_KEY_t id);
    MP_CONN_KEY_t GetConnectionID();

private:
    CYBERTWINID_t cyberID;
    uint8_t kind;
    MP_CONN_KEY_t senderKey;
    MP_CONN_KEY_t recverKey;
    MP_CONN_ID_t connectionID;
};

//*********************************************************************
//*                   Cybertwin Multipath Tag -- Data                 *
//*********************************************************************
class CybertwinMpTagData: public CybertwinMpTag
{
public:
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    CybertwinMpTagData();

    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

    uint8_t GetKind();
    void SetSenderKey(MP_CONN_KEY_t key);
    MP_CONN_KEY_t GetSenderKey();
    void SetRecverKey(MP_CONN_KEY_t key);
    MP_CONN_KEY_t GetRecverKey();
    void SetConnectionID(MP_CONN_KEY_t id);
    MP_CONN_KEY_t GetConnectionID();

private:
    MP_CONN_KEY_t senderKey;
    MP_CONN_KEY_t recverKey;
    MP_CONN_ID_t connectionID;
};

#endif