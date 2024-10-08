/*
 * Copyright (c) 2022 ZHIHENG DONG
 *
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
 *
 * Author: Zhiheng Dong <dzh2077@gmail.com>
 */

/**
 * This example shows how to generate neighbor caches on the scope of
 * a channel, a NetDeviceContainer, an InterfaceContainer and globally.
 * The example also shows a scenario that auto-generated neighbor caches
 * can avoid droping packets because of ARP message or Neighbor Discover
 * message.
 * If the user runs the example without any command line option, NeighborCacheHelper
 * will generate neighbor cache for all devices and the output is their
 * ARP table or NDISC table, the user can also generate neighbor cache on different
 * scope by adding command line option --useChannel (on specific channel),
 * --useNetDeviceContainer (on specific netDeviceContainer) and --useInterfaceContainer
 * (on specific netDeviceContainer), --noGenerate (don't generate neighbor cache).
 * The user can also enable --sendTraffic flag to send a data stream from n0 to n1,
 * the output will be the information of received packets, which shows the packets loss
 * are reduced after pre-generate neighbor caches.
 *
 * IPv4 Network Topology
 * \verbatim

    LAN 10.1.1.0/24
 n0 --------------- n1     n2     n3
                    |      |      |
                    ===============
                    LAN 10.1.2.0/24
  \endverbatim
 *
 *IPv6 Network Topology
 * \verbatim

    LAN 2001:1::/64
 n0 --------------- n1     n2     n3
                    |      |      |
                    ===============
                    LAN 2001:2::/64
  \endverbatim
 *
 * Expected Outputs:
 * IPv4 (default):
 * Generate ARP caches for all deveices (default) :
 * \verbatim
  ARP Cache of node 0 at time 0
  10.1.1.2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  ARP Cache of node 1 at time 0
  10.1.1.1 dev 0 lladdr 02-06-00:00:00:00:00:01 STATIC_AUTOGENERATED
  10.1.2.2 dev 1 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  10.1.2.3 dev 1 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  ARP Cache of node 2 at time 0
  10.1.2.1 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  10.1.2.3 dev 0 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  ARP Cache of node 3 at time 0
  10.1.2.1 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  10.1.2.2 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
   \endverbatim
 *
 * Generate ARP caches for the left channel (--useChannel):
 * \verbatim
  ARP Cache of node 0 at time 0
  10.1.1.2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  ARP Cache of node 1 at time 0
  10.1.1.1 dev 0 lladdr 02-06-00:00:00:00:00:01 STATIC_AUTOGENERATED
  ARP Cache of node 2 at time 0
  ARP Cache of node 3 at time 0
   \endverbatim
 *
 * Generate ARP caches for devices on the right side (--useNetDeviceContainer):
 * \verbatim
  ARP Cache of node 0 at time 0
  ARP Cache of node 1 at time 0
  10.1.2.2 dev 1 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  10.1.2.3 dev 1 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  ARP Cache of node 2 at time 0
  10.1.2.1 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  10.1.2.3 dev 0 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  ARP Cache of node 3 at time 0
  10.1.2.1 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  10.1.2.2 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
   \endverbatim
 *
 * Generate ARP caches for specific interfaces (--useInterfaceContainer):
 * \verbatim
  ARP Cache of node 0 at time 0
  10.1.1.2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  ARP Cache of node 1 at time 0
  ARP Cache of node 2 at time 0
  ARP Cache of node 3 at time 0
  10.1.2.1 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  10.1.2.2 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
   \endverbatim
 *
 * Do not generate neighbor cache (--noGenerate):
 * \verbatim
  ARP Cache of node 0 at time 0
  ARP Cache of node 1 at time 0
  ARP Cache of node 2 at time 0
  ARP Cache of node 3 at time 0
   \endverbatim
 *
 * sending packet with pre-generated neighbor caches(--sendTraffic)
 * \verbatim
  Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=0
 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND
 (seq=1 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512)
 AND (seq=2 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 ->
 (size=512) AND (seq=3 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to
 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=4 time=+1s) Rx pkt from
 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=5 time=+1s) Rx pkt
 from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=6 time=+1s) Rx
 pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=7
 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND
 (seq=8 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512)
 AND (seq=9 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 ->
 (size=512) AND (seq=10 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to
 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=11 time=+1s) Rx pkt from
 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=12 time=+1.00001s)
   \endverbatim
 *
 * sending packet without pre-generated neighbor caches(--sendTraffic --noGenerate)
 * \verbatim
  Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=0
 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND
 (seq=1 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512)
 AND (seq=2 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 ->
 (size=512) AND (seq=8 time=+1s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to
 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=9 time=+1s) Rx pkt from
 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=10 time=+1s) Rx pkt
 from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=11 time=+1s)
  Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512) AND (seq=12
 time=+1.00001s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 -> (size=512)
 AND (seq=13 time=+1.00001s) Rx pkt from 03-07-0a:01:01:01:01:c0:00 to 03-07-0a:01:01:02:09:00:00 ->
 (size=512) AND (seq=14 time=+1.00001s) \endverbatim
 *
 * IPv6 (--useIPv6):
 * Generate NDISC caches for all deveices (default) :
 * \verbatim
  NDISC Cache of node 0 at time +0s
  2001:1::200:ff:fe00:2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  NDISC Cache of node 1 at time +0s
  2001:1::200:ff:fe00:1 dev 0 lladdr 02-06-00:00:00:00:00:01 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:1 dev 0 lladdr 02-06-00:00:00:00:00:01 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:4 dev 1 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:5 dev 1 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:4 dev 1 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:5 dev 1 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  NDISC Cache of node 2 at time +0s
  2001:2::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:5 dev 0 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:5 dev 0 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  NDISC Cache of node 3 at time +0s
  2001:2::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:4 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:4 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
   \endverbatim
 *
 * Generate NDISC caches for the left channel (--useChannel):
 * \verbatim
  NDISC Cache of node 0 at time +0s
  2001:1::200:ff:fe00:2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  NDISC Cache of node 1 at time +0s
  2001:1::200:ff:fe00:1 dev 0 lladdr 02-06-00:00:00:00:00:01 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:1 dev 0 lladdr 02-06-00:00:00:00:00:01 STATIC_AUTOGENERATED
  NDISC Cache of node 2 at time +0s
  NDISC Cache of node 3 at time +0s
   \endverbatim
 *
 * Generate NDISC caches for devices on the right side (--useNetDeviceContainer):
 * \verbatim
  NDISC Cache of node 0 at time +0s
  NDISC Cache of node 1 at time +0s
  2001:2::200:ff:fe00:4 dev 1 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:5 dev 1 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:4 dev 1 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:5 dev 1 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  NDISC Cache of node 2 at time +0s
  2001:2::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:5 dev 0 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:5 dev 0 lladdr 02-06-00:00:00:00:00:05 STATIC_AUTOGENERATED
  NDISC Cache of node 3 at time +0s
  2001:2::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:4 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:4 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
   \endverbatim
 *
 * Generate NDISC caches for specific interfaces (--useInterfaceContainer):
 * \verbatim
  NDISC Cache of node 0 at time +0s
  2001:1::200:ff:fe00:2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:2 dev 0 lladdr 02-06-00:00:00:00:00:02 STATIC_AUTOGENERATED
  NDISC Cache of node 1 at time +0s
  NDISC Cache of node 2 at time +0s
  NDISC Cache of node 3 at time +0s
  2001:2::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  2001:2::200:ff:fe00:4 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:3 dev 0 lladdr 02-06-00:00:00:00:00:03 STATIC_AUTOGENERATED
  fe80::200:ff:fe00:4 dev 0 lladdr 02-06-00:00:00:00:00:04 STATIC_AUTOGENERATED
   \endverbatim
 *
 * Do not generate neighbor cache (--noGenerate):
 * \verbatim
  NDISC Cache of node 0 at time +0s
  NDISC Cache of node 1 at time +0s
  NDISC Cache of node 2 at time +0s
  NDISC Cache of node 3 at time +0s
   \endverbatim
 *
 * sending packet with pre-generated neighbor caches(--sendTraffic)
 * \verbatim
  Rx pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=0 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=1 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=2 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=3 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=4 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=5 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=6 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=7 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=8 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=9 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=10 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=11 time=+1s)
   \endverbatim
 *
 * sending packet without pre-generated neighbor caches(--sendTraffic --noGenerate)
 * \verbatim
  Rx pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=8 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=9 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=10 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=11 time=+1s) Rx
 pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=12
 time=+1.00001s) Rx pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=13
 time=+1.00001s) Rx pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=14
 time=+1.00001s) Rx pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=15
 time=+1.00001s) Rx pkt from 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:01:01:c0 to
 04-12-20:01:00:01:00:00:00:00:02:00:00:ff:fe:00:00:02:09:00 -> (size=512) AND (seq=16
 time=+1.00001s) \endverbatim
*/

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NeighborCacheExample");

/**
 * Neigbor Cache example class.
 *
 * It handles the creation and run of an example.
 */
class NeighborCacheExample
{
  public:
    NeighborCacheExample();

    /**
     * Run the example.
     */
    void Run();

    /**
     * \brief description the command-line parameters.
     * \param argc The argument count.
     * \param argv The argument vector.
     */
    void CommandSetup(int argc, char** argv);

  private:
    /**
     * \brief Print the information of receive data.
     * \param pkt The received packet.
     * \param from The sender.
     * \param dst The receiver.
     * \param header The header with a sequence, a timestamp, and a "size" attribute
     */
    void ReceivePacket(Ptr<const Packet> pkt,
                       const Address& from,
                       const Address& dst,
                       const SeqTsSizeHeader& header);

    bool m_useIpv6{false};    //!< Use IPv6 instead of IPv4
    bool m_enableLog{false};  //!< Enable ArpL3Protocol and Icmpv6L4Protocol logging
    bool m_useChannel{false}; //!< Generate neighbor cache for specific Channel
    bool m_useNetDeviceContainer{
        false}; //!< Generate neighbor cache for specific netDeviceContainer
    bool m_useInterfaceContainer{
        false};                //!< Generate neighbor cache for specific interfaceContainer
    bool m_noGenerate{false};  //!< do not generate neighbor cache automatically
    bool m_sendTraffic{false}; //!< send data stream from n0 to n1
};

NeighborCacheExample::NeighborCacheExample()
{
    NS_LOG_FUNCTION(this);
}

void
NeighborCacheExample::ReceivePacket(Ptr<const Packet> pkt,
                                    const Address& from,
                                    const Address& dst,
                                    const SeqTsSizeHeader& header)
{
    std::cout << "Rx pkt from " << from << " to " << dst << " -> " << header << std::endl;
}

void
NeighborCacheExample::CommandSetup(int argc, char** argv)
{
    CommandLine cmd(__FILE__);
    cmd.AddValue("useIPv6", "Use IPv6 instead of IPv4", m_useIpv6);
    cmd.AddValue("enableLog", "Enable ArpL3Protocol and Icmpv6L4Protocol logging", m_enableLog);
    cmd.AddValue("useChannel", "Generate neighbor cache for specific Channel", m_useChannel);
    cmd.AddValue("useNetDeviceContainer",
                 "Generate neighbor cache for specific netDeviceContainer",
                 m_useNetDeviceContainer);
    cmd.AddValue("useInterfaceContainer",
                 "Generate neighbor cache for specific interfaceContainer",
                 m_useInterfaceContainer);
    cmd.AddValue("noGenerate", "do not generate neighbor cache automatically", m_noGenerate);
    cmd.AddValue("sendTraffic", "send data stream from n0 to n1", m_sendTraffic);

    cmd.Parse(argc, argv);
}

int
main(int argc, char* argv[])
{
    NeighborCacheExample example;
    example.CommandSetup(argc, argv);
    example.Run();
    return 0;
}

void
NeighborCacheExample::Run()
{
    if (m_enableLog)
    {
        LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
        LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_LOGIC);
    }
    uint32_t nCsmaLeft = 2;
    uint32_t nCsmaRight = 2;

    NodeContainer csmaNodesLeft;
    csmaNodesLeft.Create(nCsmaLeft);
    NodeContainer csmaNodesRight;
    csmaNodesRight.Add(csmaNodesLeft.Get(1));
    csmaNodesRight.Create(nCsmaRight);

    CsmaHelper csmaLeft;
    csmaLeft.SetChannelAttribute("DataRate", StringValue("20Gbps"));
    // The 1 microSeconds delay is only for showing packets dropped effect without generating
    // neighbor caches.
    csmaLeft.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));
    CsmaHelper csmaRight;
    csmaRight.SetChannelAttribute("DataRate", StringValue("20Gbps"));
    csmaRight.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));

    NetDeviceContainer csmaDevicesLeft;
    csmaDevicesLeft = csmaLeft.Install(csmaNodesLeft);
    NetDeviceContainer csmaDevicesRight;
    csmaDevicesRight = csmaRight.Install(csmaNodesRight);

    InternetStackHelper stack;
    // disabled Ipv4ArpJitter and Ipv6NsRsJitter to avoid the influence on packet dropped
    stack.SetIpv4ArpJitter(false);
    stack.SetIpv6NsRsJitter(false);
    stack.Install(csmaNodesLeft.Get(0));
    stack.Install(csmaNodesRight);

    if (!m_useIpv6)
    {
        Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfacesLeft;
        csmaInterfacesLeft = address.Assign(csmaDevicesLeft);
        address.SetBase("10.1.2.0", "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfacesRight;
        csmaInterfacesRight = address.Assign(csmaDevicesRight);

        // Populate ARP caches
        NeighborCacheHelper neighborCache;
        if (m_useChannel)
        {
            // Populate ARP caches for given channel
            Ptr<Channel> csmaChannel = csmaDevicesLeft.Get(0)->GetChannel();
            neighborCache.PopulateNeighborCache(csmaChannel);
        }
        else if (m_useNetDeviceContainer)
        {
            // Populate ARP caches for given netDeviceContainer
            neighborCache.PopulateNeighborCache(csmaDevicesRight);
        }
        else if (m_useInterfaceContainer)
        {
            std::pair<Ptr<Ipv4>, uint32_t> txInterface = csmaInterfacesLeft.Get(0);
            Ptr<Ipv4> ipv41 = txInterface.first;
            uint32_t index1 = txInterface.second;
            std::pair<Ptr<Ipv4>, uint32_t> rxInterface = csmaInterfacesRight.Get(nCsmaRight);
            Ptr<Ipv4> ipv42 = rxInterface.first;
            uint32_t index2 = rxInterface.second;

            // Populate ARP caches for given interfaceContainer
            Ipv4InterfaceContainer interfaces;
            interfaces.Add(ipv41, index1);
            interfaces.Add(ipv42, index2);
            neighborCache.PopulateNeighborCache(interfaces);
        }
        else if (!m_noGenerate)
        {
            // Populate ARP caches for all devices
            neighborCache.PopulateNeighborCache();
        }

        if (m_sendTraffic)
        {
            // send Packet from n0 to n1
            uint16_t port = 9; // Discard port (RFC 863)
            OnOffHelper onoff("ns3::UdpSocketFactory",
                              Address(InetSocketAddress(csmaInterfacesLeft.GetAddress(1), port)));
            onoff.SetConstantRate(DataRate("10Gbps"));
            onoff.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
            ApplicationContainer apps = onoff.Install(csmaNodesLeft.Get(0));
            apps.Start(Seconds(1.0));

            // Create a packet sink to receive these packets
            PacketSinkHelper sink("ns3::UdpSocketFactory",
                                  Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
            sink.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
            apps = sink.Install(csmaNodesLeft);
            apps.Get(1)->TraceConnectWithoutContext(
                "RxWithSeqTsSize",
                MakeCallback(&NeighborCacheExample::ReceivePacket, this));
            AsciiTraceHelper ascii;
            Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("neighbor-cache-example.tr");
            csmaLeft.EnableAsciiAll(stream);
            csmaLeft.EnablePcapAll("neighbor-cache-example");
        }
        else
        {
            Ptr<OutputStreamWrapper> outputStream = Create<OutputStreamWrapper>(&std::cout);
            Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), outputStream);
        }
    }
    else
    {
        Ipv6AddressHelper address;
        address.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
        Ipv6InterfaceContainer csmaInterfacesLeft;
        csmaInterfacesLeft = address.Assign(csmaDevicesLeft);
        csmaInterfacesLeft.SetForwarding(1, true);
        csmaInterfacesLeft.SetDefaultRouteInAllNodes(1);

        address.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
        Ipv6InterfaceContainer csmaInterfacesRight;
        csmaInterfacesRight = address.Assign(csmaDevicesRight);
        csmaInterfacesRight.SetForwarding(0, true);
        csmaInterfacesRight.SetDefaultRouteInAllNodes(0);

        // Populate neighbor NDISC caches
        NeighborCacheHelper neighborCache;
        if (m_useChannel)
        {
            // Populate NDISC caches for given channel
            Ptr<Channel> csmaChannel = csmaDevicesLeft.Get(0)->GetChannel();
            neighborCache.PopulateNeighborCache(csmaChannel);
        }
        else if (m_useNetDeviceContainer)
        {
            // Populate NDISC caches for given netDeviceContainer
            neighborCache.PopulateNeighborCache(csmaDevicesRight);
        }
        else if (m_useInterfaceContainer)
        {
            std::pair<Ptr<Ipv6>, uint32_t> txInterface = csmaInterfacesLeft.Get(0);
            Ptr<Ipv6> ipv61 = txInterface.first;
            uint32_t index1 = txInterface.second;
            std::pair<Ptr<Ipv6>, uint32_t> rxInterface = csmaInterfacesRight.Get(nCsmaRight);
            Ptr<Ipv6> ipv62 = rxInterface.first;
            uint32_t index2 = rxInterface.second;

            // Populate NDISC caches for given interfaceContainer
            Ipv6InterfaceContainer interfaces;
            interfaces.Add(ipv61, index1);
            interfaces.Add(ipv62, index2);
            neighborCache.PopulateNeighborCache(interfaces);
        }
        else if (!m_noGenerate)
        {
            // Populate NDISC caches for all devices
            neighborCache.PopulateNeighborCache();
        }

        if (m_sendTraffic)
        {
            // send Packet from n0 to n1
            uint16_t port = 9; // Discard port (RFC 863)
            OnOffHelper onoff(
                "ns3::UdpSocketFactory",
                Address(Inet6SocketAddress(csmaInterfacesLeft.GetAddress(1, 1), port)));
            onoff.SetConstantRate(DataRate("10Gbps"));
            onoff.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
            ApplicationContainer apps = onoff.Install(csmaNodesLeft.Get(0));
            apps.Start(Seconds(1.0));

            // Create a packet sink to receive these packets
            PacketSinkHelper sink("ns3::UdpSocketFactory",
                                  Address(Inet6SocketAddress(Ipv6Address::GetAny(), port)));
            sink.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
            apps = sink.Install(csmaNodesLeft);
            apps.Get(1)->TraceConnectWithoutContext(
                "RxWithSeqTsSize",
                MakeCallback(&NeighborCacheExample::ReceivePacket, this));
            AsciiTraceHelper ascii;
            Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("neighbor-cache-example.tr");
            csmaLeft.EnableAsciiAll(stream);
            csmaLeft.EnablePcapAll("neighbor-cache-example");
        }
        else
        {
            Ptr<OutputStreamWrapper> outputStream = Create<OutputStreamWrapper>(&std::cout);
            Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), outputStream);
        }
    }
    Simulator::Stop(Seconds(1.00002));
    Simulator::Run();
    Simulator::Destroy();
}
