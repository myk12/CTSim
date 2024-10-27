#include "cybertwin-header.h"

#include "ns3/log.h"
#include "ns3/uinteger.h"

namespace ns3
{
//********************************************************************
//*             Cybertwin Header                                      *
//********************************************************************
NS_LOG_COMPONENT_DEFINE("CybertwinHeader");
NS_OBJECT_ENSURE_REGISTERED(CybertwinHeader);

TypeId
CybertwinHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinHeader")
                            .SetParent<Header>()
                            .SetGroupName("cybertwin")
                            .AddConstructor<CybertwinHeader>();
    return tid;
}

TypeId
CybertwinHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinHeader::CybertwinHeader()
    : m_command(0),
      m_cybertwin(0),
      m_peer(0),
      m_size(0),
      m_cybertwinPort(0),
      m_recvRate(0)
{
}

std::string
CybertwinHeader::ToString() const
{
    std::ostringstream oss;
    Print(oss);
    return oss.str();
}

uint32_t
CybertwinHeader::GetSerializedSize() const
{
    // TODO: the size of the header should be fixed
    return sizeof(m_command) + sizeof(m_cybertwin) + sizeof(m_peer) + sizeof(m_size) +
           sizeof(m_cybertwinPort) + sizeof(m_recvRate);
}

void
CybertwinHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    i.WriteU8(m_command);
    i.WriteHtonU64(m_cybertwin);
    i.WriteHtonU64(m_peer);
    i.WriteHtonU32(m_size);
    i.WriteHtonU16(m_cybertwinPort);
    i.WriteU8(m_recvRate);

    NS_LOG_DEBUG("Serialized command: " << static_cast<uint32_t>(m_command) << " cybertwin: "
                                        << m_cybertwin << " peer: " << m_peer << " size: " << m_size
                                        << " cybertwinPort: " << m_cybertwinPort
                                        << " recvRate: " << static_cast<uint32_t>(m_recvRate));
}

uint32_t
CybertwinHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_DEBUG("Deserializing CybertwinHeader");
    Buffer::Iterator i = start;
    m_command = i.ReadU8();
    m_cybertwin = i.ReadNtohU64();
    m_peer = i.ReadNtohU64();
    m_size = i.ReadNtohU32();
    m_cybertwinPort = i.ReadNtohU16();
    m_recvRate = i.ReadU8();

    NS_LOG_DEBUG("Deserialized command: " << static_cast<uint32_t>(m_command)
                                          << " cybertwin: " << m_cybertwin << " peer: " << m_peer
                                          << " size: " << m_size
                                          << " cybertwinPort: " << m_cybertwinPort
                                          << " recvRate: " << static_cast<uint32_t>(m_recvRate));

    return GetSerializedSize();
}

bool
CybertwinHeader::isDataPacket() const
{
    return m_command == CYBERTWIN_HEADER_DATA;
}

void
CybertwinHeader::SetCommand(uint8_t val)
{
    m_command = val;
}

uint8_t
CybertwinHeader::GetCommand() const
{
    return m_command;
}

void
CybertwinHeader::SetSelfID(CYBERTWINID_t val)
{
    m_cybertwin = val;
}

CYBERTWINID_t
CybertwinHeader::GetSelfID() const
{
    return m_cybertwin;
}

void
CybertwinHeader::SetPeerID(CYBERTWINID_t val)
{
    m_peer = val;
}

CYBERTWINID_t
CybertwinHeader::GetPeerID() const
{
    return m_peer;
}

void
CybertwinHeader::SetSize(uint32_t val)
{
    m_size = val;
}

uint32_t
CybertwinHeader::GetSize() const
{
    return m_size;
}

void
CybertwinHeader::SetCybertwinPort(uint16_t val)
{
    m_cybertwinPort = val;
}

uint16_t
CybertwinHeader::GetCybertwinPort() const
{
    return m_cybertwinPort;
}

void
CybertwinHeader::SetRecvRate(uint8_t val)
{
    m_recvRate = val;
}

uint8_t
CybertwinHeader::GetRecvRate() const
{
    return m_recvRate;
}

void
CybertwinHeader::Print(std::ostream& os) const
{
    os << "------- Cybertwin Header -------" << std::endl
       << "| Command: " << static_cast<uint32_t>(m_command) << std::endl
       << "| Cybertwin: " << m_cybertwin << std::endl
       << "| Peer: " << m_peer << std::endl
       << "| Size: " << m_size << std::endl
       << "| Port: " << m_cybertwinPort << std::endl
       << "| RecvRate: " << static_cast<uint32_t>(m_recvRate) << std::endl
       << "--------------------------------" << std::endl;
}

//********************************************************************
//*             End Host Header                                      *
//********************************************************************
NS_OBJECT_ENSURE_REGISTERED(EndHostHeader);

TypeId
EndHostHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::EndHostHeader")
                            .SetParent<Header>()
                            .AddConstructor<EndHostHeader>();
    return tid;
}

EndHostHeader::EndHostHeader()
    : m_command(ENDHOST_HEARTBEAT),
      m_targetID(0)
{
}

TypeId
EndHostHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
EndHostHeader::GetSerializedSize() const
{
    return sizeof(m_command) + sizeof(m_targetID);
}

void
EndHostHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(m_command);
    start.WriteHtonU64(m_targetID);
}

uint32_t
EndHostHeader::Deserialize(Buffer::Iterator start)
{
    m_command = start.ReadU8();
    m_targetID = start.ReadNtohU64();
    return GetSerializedSize();
}

void
EndHostHeader::Print(std::ostream& os) const
{
    os << "------- End Host Header -------" << std::endl
       << "| Command: " << static_cast<uint32_t>(m_command) << std::endl
       << "| Target ID: " << m_targetID << std::endl
       << "--------------------------------" << std::endl;
}

void
EndHostHeader::SetCommand(EndHostCommand_t command)
{
    m_command = command;
}

EndHostCommand_t
EndHostHeader::GetCommand() const
{
    return (EndHostCommand_t)m_command;
}

void
EndHostHeader::SetTargetID(CYBERTWINID_t targetID)
{
    m_targetID = targetID;
}

CYBERTWINID_t
EndHostHeader::GetTargetID() const
{
    return m_targetID;
}

//********************************************************************
//*             Cybertwin Controller Header                          *
//********************************************************************

NS_OBJECT_ENSURE_REGISTERED(CybertwinManagerHeader);

TypeId
CybertwinManagerHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinManagerHeader")
                            .SetParent<Header>()
                            .AddConstructor<CybertwinManagerHeader>();
    return tid;
}

TypeId
CybertwinManagerHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinManagerHeader::CybertwinManagerHeader()
    : m_command(0),
      m_cuid(0),
      m_port(0)
{
}

CybertwinManagerHeader::~CybertwinManagerHeader()
{
}

void
CybertwinManagerHeader::SetCommand(uint8_t command)
{
    m_command = command;
}

uint8_t
CybertwinManagerHeader::GetCommand() const
{
    return m_command;
}

void
CybertwinManagerHeader::SetCName(const std::string& cname)
{
    m_cname = cname;
}

std::string
CybertwinManagerHeader::GetCName() const
{
    return m_cname;
}

void
CybertwinManagerHeader::SetCUID(uint64_t cuid)
{
    m_cuid = cuid;
}

uint64_t
CybertwinManagerHeader::GetCUID() const
{
    return m_cuid;
}

void
CybertwinManagerHeader::SetPort(uint16_t port)
{
    m_port = port;
}

uint16_t
CybertwinManagerHeader::GetPort() const
{
    return m_port;
}

uint32_t
CybertwinManagerHeader::GetSerializedSize() const
{
    // Calculate the serialized size based on the size of each member variable
    uint32_t size =
        sizeof(m_command) + sizeof(uint16_t) + m_cname.size() + sizeof(m_cuid) + sizeof(m_port);
    return size;
}

void
CybertwinManagerHeader::Serialize(Buffer::Iterator it) const
{
    // Serialize each member variable to the buffer
    it.WriteU8(m_command);
    it.WriteHtonU16(static_cast<uint16_t>(m_cname.size()));
    it.Write(reinterpret_cast<const uint8_t*>(m_cname.c_str()), m_cname.size());
    it.WriteHtonU64(m_cuid);
    it.WriteHtonU16(m_port);
}

uint32_t
CybertwinManagerHeader::Deserialize(Buffer::Iterator it)
{
    // Deserialize each member variable from the buffer
    m_command = it.ReadU8();
    uint16_t cnameLength = it.ReadNtohU16();

    char* cnameBuffer = new char[cnameLength];
    it.Read(reinterpret_cast<uint8_t*>(cnameBuffer), cnameLength);
    m_cname.assign(cnameBuffer, cnameLength);
    delete[] cnameBuffer;

    m_cuid = it.ReadNtohU64();
    m_port = it.ReadNtohU16();

    return GetSerializedSize();
}

void
CybertwinManagerHeader::Print(std::ostream& os) const
{
    os << "------- Cybertwin Manager Header -------" << std::endl
       << "| Command: " << static_cast<uint32_t>(m_command) << std::endl
       << "| CName: " << m_cname << std::endl
       << "| CUID: " << m_cuid << std::endl
       << "| Port: " << m_port << std::endl
       << "----------------------------------------" << std::endl;
}

//********************************************************************
//*             CyberTwin Multipath Header                           *
//********************************************************************
NS_OBJECT_ENSURE_REGISTERED(MultipathHeader);

MultipathHeader::MultipathHeader()
    : m_pathId(0),
      m_cuid(0),
      m_senderKey(0),
      m_recverKey(0),
      m_connId(0)
{
}

MultipathHeader::~MultipathHeader()
{
}

TypeId
MultipathHeader::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::MultipathHeader").SetParent<Header>().AddConstructor<MultipathHeader>();
    return tid;
}

TypeId
MultipathHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
MultipathHeader::GetSize()
{
    return sizeof(m_pathId) + sizeof(m_cuid) + sizeof(m_senderKey) + sizeof(m_recverKey) +
           sizeof(m_connId);
}

uint32_t
MultipathHeader::GetSerializedSize() const
{
    return sizeof(m_pathId) + sizeof(m_cuid) + sizeof(m_senderKey) + sizeof(m_recverKey) +
           sizeof(m_connId);
}

void
MultipathHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_pathId);
    start.WriteHtonU64(m_cuid);
    start.WriteHtonU32(m_senderKey);
    start.WriteHtonU32(m_recverKey);
    start.WriteHtonU64(m_connId);
}

uint32_t
MultipathHeader::Deserialize(Buffer::Iterator start)
{
    m_pathId = start.ReadNtohU32();
    m_cuid = start.ReadNtohU64();
    m_senderKey = start.ReadNtohU32();
    m_recverKey = start.ReadNtohU32();
    m_connId = start.ReadNtohU64();

    return GetSerializedSize();
}

void
MultipathHeader::Print(std::ostream& os) const
{
    os << "------- Multipath Header -------" << std::endl
       << "| Path ID: " << m_pathId << std::endl
       << "| CyberTwin ID: " << m_cuid << std::endl
       << "| Sender Key: " << m_senderKey << std::endl
       << "| Receiver Key: " << m_recverKey << std::endl
       << "| Connection ID: " << m_connId << std::endl
       << "--------------------------------" << std::endl;
}

MP_PATH_ID_t
MultipathHeader::GetPathId() const
{
    return m_pathId;
}

void
MultipathHeader::SetPathId(MP_PATH_ID_t pathId)
{
    m_pathId = pathId;
}

CYBERTWINID_t
MultipathHeader::GetCuid() const
{
    return m_cuid;
}

void
MultipathHeader::SetCuid(CYBERTWINID_t cuid)
{
    m_cuid = cuid;
}

MP_CONN_KEY_t
MultipathHeader::GetSenderKey() const
{
    return m_senderKey;
}

void
MultipathHeader::SetSenderKey(MP_CONN_KEY_t senderKey)
{
    m_senderKey = senderKey;
}

MP_CONN_KEY_t
MultipathHeader::GetRecverKey() const
{
    return m_recverKey;
}

void
MultipathHeader::SetRecverKey(MP_CONN_KEY_t recverKey)
{
    m_recverKey = recverKey;
}

MP_CONN_ID_t
MultipathHeader::GetConnId() const
{
    return m_connId;
}

void
MultipathHeader::SetConnId(MP_CONN_ID_t connId)
{
    m_connId = connId;
}

//********************************************************************
//*             CyberTwin Multipath Header DSN                       *
//********************************************************************

MultipathHeaderDSN::MultipathHeaderDSN()
    : m_cuid(0),
      m_dataSeqNum(0),
      m_dataLen(0)
{
}

MultipathHeaderDSN::~MultipathHeaderDSN()
{
}

TypeId
MultipathHeaderDSN::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::MultipathHeaderDSN").SetParent<Header>().AddConstructor<MultipathHeaderDSN>();
    return tid;
}

TypeId
MultipathHeaderDSN::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
MultipathHeaderDSN::GetSerializedSize() const
{
    return sizeof(m_cuid) + sizeof(m_dataSeqNum) + sizeof(m_dataLen);
}

void
MultipathHeaderDSN::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU64(m_cuid);
    start.WriteHtonU64(m_dataSeqNum.GetValue());
    start.WriteHtonU32(m_dataLen);
}

uint32_t
MultipathHeaderDSN::Deserialize(Buffer::Iterator start)
{
    m_cuid = start.ReadNtohU64();
    m_dataSeqNum = start.ReadNtohU64();
    m_dataLen = start.ReadNtohU32();

    return GetSerializedSize();
}

void
MultipathHeaderDSN::Print(std::ostream& os) const
{
    os << "CUID: " << m_cuid << std::endl;
    os << "DataSeqNum: " << m_dataSeqNum << std::endl;
    os << "DataLen: " << m_dataLen << std::endl;
}

void
MultipathHeaderDSN::SetCuid(CYBERTWINID_t cuid)
{
    m_cuid = cuid;
}

CYBERTWINID_t
MultipathHeaderDSN::GetCuid() const
{
    return m_cuid;
}

void
MultipathHeaderDSN::SetDataSeqNum(MpDataSeqNum dataSeqNum)
{
    m_dataSeqNum = dataSeqNum;
}

MpDataSeqNum
MultipathHeaderDSN::GetDataSeqNum() const
{
    return m_dataSeqNum;
}

void
MultipathHeaderDSN::SetDataLen(uint32_t dataLen)
{
    m_dataLen = dataLen;
}

uint32_t
MultipathHeaderDSN::GetDataLen() const
{
    return m_dataLen;
}

//************************************************************************
//*                             CNRS Header                              *
//************************************************************************
NS_OBJECT_ENSURE_REGISTERED(CNRSHeader);

CNRSHeader::CNRSHeader()
    : m_method(0),
      m_queryId(0),
      m_cuid(0),
      m_interfaceNum(0)
{
}

TypeId
CNRSHeader::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::CNRSHeader").SetParent<Header>().AddConstructor<CNRSHeader>();
    return tid;
}

TypeId
CNRSHeader::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t
CNRSHeader::GetSerializedSize(void) const
{
    uint32_t size = sizeof(uint8_t) + sizeof(QUERY_ID_t) + sizeof(CYBERTWINID_t) + sizeof(uint8_t) +
                    m_interfaceNum * (sizeof(uint32_t) + sizeof(uint16_t));

    return size;
}

void
CNRSHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(m_method);
    start.WriteHtonU32(m_queryId);
    start.WriteHtonU64(m_cuid);
    start.WriteU8(m_interfaceNum);
    if (m_method == CNRS_INSERT || m_method == CNRS_QUERY_OK)
    {
        for (auto& interface : m_interfaceList)
        {
            start.WriteHtonU32(interface.first.Get());
            start.WriteHtonU16(interface.second);
        }
    }
}

uint32_t
CNRSHeader::Deserialize(Buffer::Iterator start)
{
    m_method = start.ReadU8();
    m_queryId = start.ReadNtohU32();
    m_cuid = start.ReadNtohU64();
    m_interfaceNum = start.ReadU8();
    if (m_method == CNRS_INSERT || m_method == CNRS_QUERY_OK)
    {
        for (uint8_t i = 0; i < m_interfaceNum; i++)
        {
            ns3::Ipv4Address address(start.ReadNtohU32());
            uint16_t port = start.ReadNtohU16();
            m_interfaceList.push_back({address, port});
        }
    }
    return GetSerializedSize();
}

void
CNRSHeader::Print(std::ostream& os) const
{
    os << "CNRS Header [Method: " << static_cast<uint32_t>(m_method) << ", Query ID: " << m_queryId
       << ", CUID: " << m_cuid;
    if (m_method == CNRS_INSERT || m_method == CNRS_QUERY_OK)
    {
        os << ", Interface Num: " << static_cast<uint32_t>(m_interfaceNum);
        for (auto& interface : m_interfaceList)
        {
            os << ", Address: " << interface.first << ", Port: " << interface.second;
        }
    }
    os << "]";
}

void
CNRSHeader::SetMethod(CNRS_METHOD method)
{
    m_method = method;
}

uint8_t
CNRSHeader::GetMethod() const
{
    return m_method;
}

void
CNRSHeader::SetQueryId(QUERY_ID_t queryId)
{
    m_queryId = queryId;
}

QUERY_ID_t
CNRSHeader::GetQueryId() const
{
    return m_queryId;
}

void
CNRSHeader::SetCuid(CYBERTWINID_t cuid)
{
    m_cuid = cuid;
}

CYBERTWINID_t
CNRSHeader::GetCuid() const
{
    return m_cuid;
}

void
CNRSHeader::SetInterfaceList(CYBERTWIN_INTERFACE_LIST_t interfaceList)
{
    m_interfaceList = interfaceList;
    m_interfaceNum = m_interfaceList.size();
}

CYBERTWIN_INTERFACE_LIST_t
CNRSHeader::GetInterfaceList() const
{
    return m_interfaceList;
}

uint8_t
CNRSHeader::GetInterfaceNum() const
{
    return m_interfaceNum;
}

void
CNRSHeader::SetInterfaceNum(uint8_t interfaceNum)
{
    m_interfaceNum = interfaceNum;
}

} // namespace ns3