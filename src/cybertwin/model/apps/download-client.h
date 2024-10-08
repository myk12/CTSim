#ifndef DOWNLOAD_CLIENT_H
#define DOWNLOAD_CLIENT_H

#include "ns3/cybertwin-common.h"
#include "ns3/cybertwin-name-resolution-service.h"
#include "ns3/cybertwin-node.h"
#include "ns3/cybertwin-header.h"
#include "ns3/cybertwin-app.h"

namespace ns3
{
class CybertwinEndHost;
typedef struct {
  uint32_t streamID;
  Time m_startTime;
  Time m_endTime;
  Ptr<Socket> m_socket;
  uint64_t m_totalBytes;
  uint64_t m_realBytes;
  std::ofstream m_logStream;
  Ipv4Address m_serverAddr;
  uint16_t m_serverPort;
  uint32_t m_offlineBytes;
} NaiveStreamInfo_s;

class DownloadStream: public Object
{
public:
  static TypeId GetTypeId();

  DownloadStream();
  ~DownloadStream();

  void SetStreamID(uint32_t streamID);
  void SetTargetID(CYBERTWINID_t targetID);
  void SetNode(Ptr<Node> node);
  void SetCUID(CYBERTWINID_t cuid);
  void SetCybertwin(Ipv4Address cybertwinAddress, uint16_t cybertwinPort);
  void SetLogDir(std::string logDir);
  void SetRate(uint8_t rate);
  void SetOfflineTime(uint8_t offlineTime);

  void Activate();
  //callbacks
  void ConnectionSucceeded(Ptr<Socket>);
  void ConnectionFailed(Ptr<Socket>);
  void ConnectionNormalClosed(Ptr<Socket>);
  void ConnectionErrorClosed(Ptr<Socket>);
  void RecvCallback(Ptr<Socket>);

  // stop and start stream
  void SendCreateRequest();
  void SendStopRequest();
  void SendStartRequest();

private:

  Ptr<Node> m_node;
  CYBERTWINID_t m_cuid;

  uint32_t m_streamID;
  CYBERTWINID_t m_targetID;
  Ptr<Socket> m_socket;

  Ipv4Address m_cybertwinAddress;
  uint16_t m_cybertwinPort;

  // statistics
  void DownloadThroughputStatistical();
  Time m_lastTime;
  EventId m_statisticalEvent;
  uint32_t m_intervalBytes;
  uint64_t m_totalBytes;

  std::string m_logDir;
  std::ofstream m_logStream;

  uint8_t m_rate;
  uint8_t m_offlineTime;
};

class DownloadClient : public CybertwinApp
{
public:
  static TypeId GetTypeId();

  DownloadClient();
  virtual ~DownloadClient();
  void AddTargetServer(CYBERTWINID_t cuid, uint8_t rate);

private:
  virtual void StartApplication();
  virtual void StopApplication();

  void StartDownloadStreams(uint8_t offlineTime);
  void StartNaiveStreams(uint8_t offlineTime, uint8_t streamID);

  void StartOnOffDownloadStreams();
  void StartNaiveDownloadStreams();

  void NaiveStreamRecvCallback(Ptr<Socket> socket);
  void NaiveStreamCloseCallback(Ptr<Socket> socket);
  void NaiveStreamReconnect(uint8_t streamID);
  void NaiveStreamClose(uint8_t streamID);

  Ptr<CybertwinEndHost> m_endHost;
  std::vector<std::pair<CYBERTWINID_t, uint8_t>> m_targetServers;
  std::vector<Ptr<DownloadStream>> m_streams;
  uint8_t m_maxOfflineTime;
  uint8_t m_streamID;

  std::ofstream m_logStream;
  std::vector<NaiveStreamInfo_s*> m_naiveStreams;
  std::unordered_map<Ptr<Socket>, NaiveStreamInfo_s*> m_socketToStream;
};

}

#endif