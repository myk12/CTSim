#ifndef CYBERTWIN_COMMON_H
#define CYBERTWIN_COMMON_H

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/sequence-number.h"

#include "nlohmann/json.hpp"

#include <string>
#include <unordered_map>
#include <utility>
#include <functional>
#include <cstdint>

namespace ns3
{
//*****************************************************************
//*                 COMPILE OPTIONS                               *
//*****************************************************************
//1: enable MDTP, 0: disable MDTP
#define MDTP_ENABLED 0
#define STATISTIC_TIME_INTERVAL (10) // ms

#define MAX_SIM_SECONDS (100)
#define NORMAL_SIM_SECONDS (10)

#define CORE_CLOUD_REPLICATION_RATIO (100)

#define TX_MAX_NUM (128)
#define DEFAULT_PAYLOAD_LEN (128)
#define LOCAL_PORT_COUNTER_START (40000)
#define GLOBAL_PORT_COUNTER_START (50000)

#define NAME_RESOLUTION_SERVICE_PORT (5353)
#define CYBERTWIN_EDGESERVER_CONTROLLER_PORT (2323)         //Tranportation Layer cybertwin controller server port.

#define CYBERTWIN_MANAGER_PROXY_PORT (17)

#define APP_CONF_FILE_NAME ("apps.conf")

#define APPTYPE_DOWNLOAD_SERVER ("download-server")
#define APPTYPE_DOWNLOAD_CLIENT ("download-client")
#define APPTYPE_ENDHOST_INITD ("end-host-initd")
#define APPTYPE_ENDHOST_BULK_SEND ("end-host-bulk-send")

#define END_HOST_BULK_SEND_TEST_TIME (0.5) //seconds
#define STATISTIC_INTERVAL_MILLISECONDS (10) //milliseconds
#define TRAFFIC_POLICING_INTERVAL_MILLISECONDS (10) // milliseconds
#define TRAFFIC_POLICING_LIMIT_THROUGHPUT (120) // Mbps
#define TRAFFIC_SHAPING_LIMIT_THROUGHPUT (120) // Mbps
#define CYBERTWIN_COMM_MODEL_STAT_INTERVAL_MILLISECONDS (10) //milliseconds

#define SP_KEYS_TO_CONNEID(connid, key1, key2)\
do {\
    connid = key1;  \
    connid = connid << 32;   \
    connid += key2;  \
}while(0)

#define SYSTEM_PACKET_SIZE (536)
#define MAX_BUFFER_PKT_NUM (1024)

#define SECURITY_TEST_ENABLED (0)

#define CYBERTWIN_FORWARD_MAX_WAIT_NUM (10000)

#define GET_PEERID_FROM_STREAMID(streamid) (streamid & 0xFFFFFFFFFFFFFFFF)
#define GET_CYBERID_FROM_STREAMID(streamid) (streamid >> 64)

#define COMM_TEST_CYBERTWIN_ID (18888)

typedef uint64_t CYBERTWINID_t;
typedef uint64_t DEVNAME_t;
typedef uint16_t NETTYPE_t;
typedef uint64_t MP_CONN_ID_t;
typedef uint32_t MP_CONN_KEY_t;
typedef uint32_t MP_PATH_ID_t;
typedef uint32_t QUERY_ID_t; // query id for name resolution
typedef SequenceNumber<uint64_t, uint64_t> MpDataSeqNum;
typedef std::pair<ns3::Address, uint16_t> CybertwinInterface;
typedef std::pair<ns3::Ipv4Address, uint16_t> CYBERTWIN_INTERFACE_t;
typedef std::vector<CYBERTWIN_INTERFACE_t> CYBERTWIN_INTERFACE_LIST_t;
double TrustRateMapping(double trust);

//static std::unordered_map<CYBERTWINID_t, ns3::Address> GlobalRouteTable;

typedef struct
{
    CYBERTWINID_t local;
    CYBERTWINID_t peer;
    MP_CONN_ID_t connid;
}MpConnId_s;

typedef struct
{
    MpConnId_s conn;
    std::vector<std::pair<MP_PATH_ID_t, uint64_t>> pathTxBytes;
    uint64_t connTxBytes;
}MpSendReport_s;

typedef struct
{
    MpConnId_s conn;
    std::vector<std::pair<MP_PATH_ID_t, uint64_t>> pathRxBytes;
    uint64_t connRxBytes;
}MpRecvReport_s;

enum CNRS_METHOD
{
    CNRS_QUERY,
    CNRS_INSERT,
    CNRS_QUERY_OK,
    CNRS_QUERY_FAIL,
    CNRS_INSERT_OK,
    CNRS_INSERT_FAIL
};

struct AddressHash
{
    size_t operator()(const Address& x) const
    {
        if (InetSocketAddress::IsMatchingType(x))
        {
            InetSocketAddress a = InetSocketAddress::ConvertFrom(x);
            return Ipv4AddressHash()(a.GetIpv4());
        }
        else if (Inet6SocketAddress::IsMatchingType(x))
        {
            Inet6SocketAddress a = Inet6SocketAddress::ConvertFrom(x);
            return Ipv6AddressHash()(a.GetIpv6());
        }

        NS_ABORT_MSG("PacketSink: unexpected address type, neither IPv4 nor IPv6");
        return 0;
    }
};

void DoSocketMethod(int (Socket::*)(const Address&), Ptr<Socket>, const Address&, uint16_t);
uint16_t DoSocketBind(Ptr<Socket>, const Address&);
uint16_t GetBindPort(Ptr<Socket>);
void NotifyCybertwinConfiguration();

uint64_t StringToUint64(std::string);

} // namespace ns3

#endif