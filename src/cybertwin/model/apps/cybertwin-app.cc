#include "cybertwin-app.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CybertwinApp");
NS_OBJECT_ENSURE_REGISTERED(CybertwinApp);

CybertwinApp::CybertwinApp()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinApp] create CybertwinApp.");
}

CybertwinApp::~CybertwinApp()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[CybertwinApp] destroy CybertwinApp.");
}

TypeId
CybertwinApp::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinApp")
            .SetParent<Application>()
            .SetGroupName("Cybertwin")
            .AddConstructor<CybertwinApp>();
    return tid;
}

void
CybertwinApp::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("[App][CybertwinApp] Starting CybertwinApp.");
}

void
CybertwinApp::StopApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Stopping CybertwinApp.");
}

void
CybertwinApp::OpenLogFile(std::string logDir, std::string logFile)
{
    NS_LOG_FUNCTION(this);
    std::string logPath = logDir + "/" + logFile;
    //create file if not exist
    std::ofstream file(logPath);
    m_logStream.open(logPath, std::ios::app);
    if (!m_logStream.is_open())
    {
        NS_LOG_ERROR("Failed to open log file: " << logPath);
        exit(1);
    }
    m_logStream << "Time\t"
                << "Message" << std::endl;
}

void
CybertwinApp::CloseLogFile()
{
    NS_LOG_FUNCTION(this);
    if (m_logStream.is_open())
    {
        m_logStream.close();
    }
}

} // namespace ns3
