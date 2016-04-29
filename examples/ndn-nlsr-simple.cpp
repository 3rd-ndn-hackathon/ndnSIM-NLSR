/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-nlsr-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/log.h"

#include "utils/tracers/ndn-nlsr-tracer.hpp"

#include "utils/topology/nlsr-conf-reader.hpp"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NdnNlsrSimple");

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +---------+     1Mbps      +---------+
 *      | router1  | <------------> | router2 | <------------> | router3 |
 *      +----------+         10ms   +---------+          10ms  +---------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-nlsr-simple
 */

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating nodes
  NodeContainer nodes;

  // Build the NLSR network topology from nlsr.conf
  ndn::NlsrConfReader nlsrConfReader("src/ndnSIM/examples/ndn-nlsr-conf/nlsr_sim.conf", 25);
  nodes = nlsrConfReader.Read();

  ns3::ndn::NlsrTracer &tracer = ndn::NlsrTracer::Instance();
  std::string prefix = std::to_string(nodes.size());
  tracer.InitializeTracer(prefix);

  // Install NLSR app on all nodes.
  NS_LOG_INFO ("Installing NLSR application on " << nodes.size() << " nodes");
  ndn::AppHelper nlsrHelper ("ns3::ndn::NlsrApp");
  nlsrHelper.Install(nodes);

  // Install NDN stack on all nodes
  NS_LOG_INFO ("Installing NDN stack on " << nodes.size() << " nodes");
  ndn::StackHelper ndnHelper;
  ndnHelper.Install(nodes);

  ndn::StrategyChoiceHelper::InstallAll("/", "ndn:/localhost/nfd/strategy/best-route");

  // Initialize the NLSR app on nodes.
  nlsrConfReader.InitializeNlsr();

  Simulator::Stop (Seconds (150.0));

  //ndn::L3RateTracer::InstallAll ((prefix + "-nlsr-l3-rate-trace.txt"), Seconds (0.5));
  //L2RateTracer::InstallAll ((prefix + "-nlsr-l2-rate-trace.txt"));
  //ndn::AppDelayTracer::InstallAll((prefix + "-nlsr-app-delays-trace.txt"));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

