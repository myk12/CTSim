/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CybertwinSim.h"


// Network Topology, Cybertwin: v1.0
//
//
#define TOPOLOGY_GRAPH                                                                             \
    ("                             - Cybertwin Network Simulation: version 1 -                   " \
     "    \n"                                                                                      \
     "                                                                                           " \
     "    \n"                                                                                      \
     "                            [core1]                                [core2]                 " \
     "    \n"                                                                                      \
     "                               |                                      |                    " \
     "    \n"                                                                                      \
     "                          [20.1.0.0]                             [20.2.0.0]                " \
     " #point to point#   \n"                                                                      \
     "                    ___________|______________________________________|___________         " \
     "    \n"                                                                                      \
     "                     |                   |                   |                  |          " \
     "    \n"                                                                                      \
     "                     |                   |                   |                  |          " \
     "    \n"                                                                                      \
     "                  [edge1]             [edge2]             [edge3]            [edge4]       " \
     "    \n"                                                                                      \
     "                     |                   |                   |                  |          " \
     "    \n"                                                                                      \
     "             ___[10.1.0.0]___    ___[10.2.0.0]___    ___[10.3.0.0]___   ___[10.4.0.0]___   " \
     " #CSMA#   \n"                                                                                \
     "              |            |      |            |      |            |     |            |    " \
     "    \n"                                                                                      \
     "              |            |      |            |      |            |     |            |    " \
     "    \n"                                                                                      \
     "             [h1]         [h2]   [h3]         [h4]   [h5]         [h6]  [h7]         [h8]  " \
     "    \n")
//
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CybertwinV1");

int
main(int argc, char* argv[])
{
    Packet::EnablePrinting();
    // Time::SetResolution(Time::NS);

    LogComponentEnable("CybertwinV1", LOG_LEVEL_INFO);
    LogComponentEnable("Cybertwin", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinSim", LOG_LEVEL_INFO);
    LogComponentEnable("NameResolutionService", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinMultipathTransfer", LOG_LEVEL_DEBUG);
    LogComponentEnable("EndHostInitd", LOG_LEVEL_DEBUG);
    LogComponentEnable("CybertwinManager", LOG_LEVEL_DEBUG);
    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
    LogComponentEnable("CybertwinNode", LOG_LEVEL_INFO);
    LogComponentEnable("EndHostBulkSend", LOG_LEVEL_DEBUG);
    LogComponentEnable("DownloadServer", LOG_LEVEL_DEBUG);
    LogComponentEnable("DownloadClient", LOG_LEVEL_DEBUG);
    //LogComponentEnable("Socket", LOG_LEVEL_INFO);
    LogComponentEnableAll(LOG_LEVEL_DEBUG);
    

    CybertwinSim cybertwinSim;
    cybertwinSim.Compiler();
    cybertwinSim.Run();

    return 0;
}