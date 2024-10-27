#ifndef CYBERTWIN_HEADER_H
#define CYBERTWIN_HEADER_H

#include "cybertwin-common.h"

#include "ns3/header.h"

namespace ns3
{

enum CybertwinCommand_t
{
    HOST_CONNECT,
    HOST_DISCONNECT,
    CYBERTWIN_CONNECT_SUCCESS,
    CYBERTWIN_CONNECT_ERROR,
    // Data commands
    CYBERTWIN_HEADER_DATA,
    CREATE_STREAM,
    ENDHOST_STOP_STREAM,
    ENDHOST_START_STREAM,

    // End Host Request Download
    ENDHOST_REQUEST_DOWNLOAD,
};

class CybertwinHeader : public Header
{
  public:
    CybertwinHeader();
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream&) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator) const override;
    uint32_t Deserialize(Buffer::Iterator) override;
    std::string ToString() const;

    bool isDataPacket() const;

    void SetCommand(uint8_t);
    uint8_t GetCommand() const;

    void SetSelfID(CYBERTWINID_t);
    CYBERTWINID_t GetSelfID() const;

    void SetPeerID(CYBERTWINID_t);
    CYBERTWINID_t GetPeerID() const;

    void SetSize(uint32_t);
    uint32_t GetSize() const;

    void SetCybertwinPort(uint16_t);
    uint16_t GetCybertwinPort() const;

    void SetRecvRate(uint8_t);
    uint8_t GetRecvRate() const;

  protected:
    uint8_t m_command;
    CYBERTWINID_t m_cybertwin;

    // data packet payload
    CYBERTWINID_t m_peer;
    uint32_t m_size;

    // control packet payload
    uint16_t m_cybertwinPort;

    // QoS
    uint8_t m_recvRate; //Mbps
};


//************************************************************************
//*                     End Host Header
//************************************************************************
typedef enum
{
    ENDHOST_HEARTBEAT = 0,
    DOWNLOAD_REQUEST,
}EndHostCommand_t;

class EndHostHeader : public Header
{
  public:
    EndHostHeader();
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream&) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator) const override;
    uint32_t Deserialize(Buffer::Iterator) override;

    void SetCommand(EndHostCommand_t command);
    EndHostCommand_t GetCommand() const;

    void SetTargetID(CYBERTWINID_t targetID);
    CYBERTWINID_t GetTargetID() const;

  private:
    uint8_t m_command;
    CYBERTWINID_t m_targetID;
};

//************************************************************************
//*               Cybertwin Controller Header                            *
//************************************************************************
enum CybertwinProxyCommand
{
    CYBERTWIN_REGISTRATION = 100,
    CYBERTWIN_REGISTRATION_ACK,
    CYBERTWIN_REGISTRATION_ERROR,

    CYBERTWIN_DESTRUCTION,
    CYBERTWIN_DESTRUCTION_ACK,
    CYBERTWIN_DESTRUCTION_ERROR,

    CYBERTWIN_RECONNECT,
    CYBERTWIN_RECONNECT_ACK,
    CYBERTWIN_RECONNECT_ERROR,
};

class CybertwinManagerHeader : public Header
{
  public:
    CybertwinManagerHeader();
    virtual ~CybertwinManagerHeader();

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;

    void SetCommand(uint8_t command);
    uint8_t GetCommand() const;

    void SetCName(const std::string& cname);
    std::string GetCName() const;

    void SetCUID(uint64_t cuid);
    uint64_t GetCUID() const;

    void SetPort(uint16_t port);
    uint16_t GetPort() const;

    virtual uint32_t GetSerializedSize() const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    virtual void Print(std::ostream& os) const;

  private:
    uint8_t m_command;
    std::string m_cname;
    uint64_t m_cuid;
    uint16_t m_port;
};

//************************************************************************
//*                        Multipath Header                              *
//************************************************************************

/**
 * Multipath Header
 *
 * This header is used to represent a multipath header in a packet.
 *
 * The header contains five private data fields:
 * - m_pathId: MP_PATH_ID_t
 * - m_cuid: CYBERTWINID_t
 * - m_senderKey: MP_CONN_KEY_t
 * - m_recverKey: MP_CONN_KEY_t
 * - m_connId: MP_CONN_ID_t
 */
class MultipathHeader : public Header
{
  public:
    MultipathHeader();
    virtual ~MultipathHeader();

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;

    virtual uint32_t GetSerializedSize() const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    virtual void Print(std::ostream& os) const;

    uint32_t GetSize();
    MP_PATH_ID_t GetPathId() const;
    void SetPathId(MP_PATH_ID_t pathId);
    CYBERTWINID_t GetCuid() const;
    void SetCuid(CYBERTWINID_t cuid);
    MP_CONN_KEY_t GetSenderKey() const;
    void SetSenderKey(MP_CONN_KEY_t senderKey);
    MP_CONN_KEY_t GetRecverKey() const;
    void SetRecverKey(MP_CONN_KEY_t recverKey);
    MP_CONN_ID_t GetConnId() const;
    void SetConnId(MP_CONN_ID_t connId);

  private:
    MP_PATH_ID_t m_pathId;
    CYBERTWINID_t m_cuid;
    MP_CONN_KEY_t m_senderKey;
    MP_CONN_KEY_t m_recverKey;
    MP_CONN_ID_t m_connId;
};

class MultipathHeaderDSN : public Header
{
  public:
    MultipathHeaderDSN();
    virtual ~MultipathHeaderDSN();

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;

    // Header size in bytes
    virtual uint32_t GetSerializedSize() const;

    // Serialize object to buffer
    virtual void Serialize(Buffer::Iterator start) const;

    // Deserialize object from buffer
    virtual uint32_t Deserialize(Buffer::Iterator start);

    // Print header data
    virtual void Print(std::ostream& os) const;

    // Getter and Setter methods for private variables
    void SetCuid(CYBERTWINID_t cuid);
    CYBERTWINID_t GetCuid() const;

    void SetDataSeqNum(MpDataSeqNum dataSeqNum);
    MpDataSeqNum GetDataSeqNum() const;

    void SetDataLen(uint32_t dataLen);
    uint32_t GetDataLen() const;

  private:
    CYBERTWINID_t m_cuid;
    MpDataSeqNum m_dataSeqNum;
    uint32_t m_dataLen;
};

//************************************************************************
//*                             CNRS Header                              *
//************************************************************************
class CNRSHeader : public Header
{
  public:
    CNRSHeader();

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;

    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Print(std::ostream& os) const;

    uint8_t GetMethod() const;
    void SetMethod(CNRS_METHOD method);

    QUERY_ID_t GetQueryId() const;
    void SetQueryId(QUERY_ID_t queryId);

    CYBERTWINID_t GetCuid() const;
    void SetCuid(CYBERTWINID_t cuid);

    uint8_t GetInterfaceNum() const;
    void SetInterfaceNum(uint8_t interfaceNum);

    CYBERTWIN_INTERFACE_LIST_t GetInterfaceList() const;
    void SetInterfaceList(CYBERTWIN_INTERFACE_LIST_t interfaceList);

  private:
    uint8_t m_method;
    QUERY_ID_t m_queryId;
    CYBERTWINID_t m_cuid;
    uint8_t m_interfaceNum;
    CYBERTWIN_INTERFACE_LIST_t m_interfaceList;
};

} // namespace ns3

#endif