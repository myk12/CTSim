#ifndef CYBERTWIN_APP_H
#define CYBERTWIN_APP_H

#include "ns3/cybertwin-common.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"

namespace ns3
{
class CybertwinApp: public Application
{
  public:
    CybertwinApp();
    ~CybertwinApp();

    static TypeId GetTypeId();

  private:
    virtual void StartApplication();
    virtual void StopApplication();

  protected:
    void OpenLogFile(std::string logDir, std::string logFile);
    void CloseLogFile();

    std::ofstream m_logStream;
};

} // namespace ns3

#endif // CYBERTWIN_APP_H