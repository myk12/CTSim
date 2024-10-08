#ifndef END_HOST_BULK_SEND_H
#define END_HOST_BULK_SEND_H

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/cybertwin-app.h"

namespace ns3
{

class EndHostBulkSend : public CybertwinApp
{
  enum TrafficPattern
  {
    TRAFFIC_PATTERN_PARETO,
    TRAFFIC_PATTERN_POISSON,
    TRAFFIC_PATTERN_UNIFORM,
    TRAFFIC_PATTERN_NORMAL,
    TRAFFIC_PATTERN_EXPONENTIAL,
    TRAFFIC_PATTERN_CONSTANT,
    TRAFFIC_PATTERN_MAX
  };

  public:
    EndHostBulkSend();
    ~EndHostBulkSend();

    static TypeId GetTypeId();

  private:
    void StartApplication();
    void StopApplication();

    void InitRandomVariableStream();
    void ConnectCybertwin();

    // callbacks
    void ConnectionSucceeded(Ptr<Socket>);
    void ConnectionFailed(Ptr<Socket>);
    void ConnectionNormalClosed(Ptr<Socket>);
    void ConnectionErrorClosed(Ptr<Socket>);

    void RecvData(Ptr<Socket>);
    void SendData();
    void EndSend();
    void ThroughputLogger(double interval);

  private:
    //private member variables
    Ipv4Address m_cybertwinAddr;
    uint16_t m_cybertwinPort;

    Ptr<Socket> m_socket;
    double m_averageSendRate;
    bool m_endSend;

    uint32_t m_maxBytes;
    uint32_t m_totalSendBytes;
    Time m_startTime;
    Time m_lastTime;
    EventId m_loggerEvent;

    // throughput logger
    uint64_t m_sendBytes1msLogger;
    uint64_t m_sendBytes10msLogger;
    uint64_t m_sendBytes100msLogger;

    TrafficPattern m_trafficPattern;
    Ptr<RandomVariableStream> m_randomVariableStream;

};
    
} // namespace ns

#endif // END_HOST_BULK_SEND_H