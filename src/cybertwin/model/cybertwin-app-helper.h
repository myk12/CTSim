#ifndef _CYBERTWIN_APP_HELPER_H_
#define _CYBERTWIN_APP_HELPER_H_

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include <vector>
#include <string>

#include "yaml-cpp/yaml.h"


#define APPTYPE_DOWNLOAD_SERVER ("download-server")
#define APPTYPE_DOWNLOAD_CLIENT ("download-client")
#define APPTYPE_ENDHOST_INITD ("end-host-initd")
#define APPTYPE_ENDHOST_BULK_SEND ("end-host-bulk-send")

namespace ns3
{
    class CybertwinAppHelper
    {
    public:

        CybertwinAppHelper();
        ~CybertwinAppHelper();

        void InstallApplications(std::string name, YAML::Node parameters, NodeContainer nodes);
        void InstallApplications(std::string name, YAML::Node parameters, Ptr<Node> node);

        void InstallDownloadClient(YAML::Node parameters, Ptr<Node> node);
        void InstallDownloadServer(YAML::Node parameters, Ptr<Node> node);

    private:
        //void InstallApp(Ptr<Node> node);
        
    };
} // namespace ns3

#endif // _CYBERTWIN_APP_HELPER_H_