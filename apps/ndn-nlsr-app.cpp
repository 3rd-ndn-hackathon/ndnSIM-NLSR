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

#include "ndn-nlsr-app.hpp"

#include "ns3/log.h"
#include <ns3/node-list.h>
#include "ns3/simulator.h"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"

NS_LOG_COMPONENT_DEFINE ("NlsrApp");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (NlsrApp);

TypeId
NlsrApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::NlsrApp")
    .SetGroupName ("Ndn")
    .SetParent<Application> ()
    .AddConstructor<NlsrApp> ()
  ;
  return tid;
}

NlsrApp::NlsrApp() {
}

NlsrApp::~NlsrApp() {
}

void
NlsrApp::Initialize(std::string& nodeConfig) {
  m_nodeConfigFile = nodeConfig;
}

void
NlsrApp::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_instance.reset(new ndn::NlsrExec(ndn::StackHelper::getKeyChain(), m_nodeConfigFile));
  m_instance->run();
}

void
NlsrApp::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_instance.reset();
}

void
NlsrApp::SetNodeNameToIdMapping(std::string name, uint32_t id)
{
  m_ndn_node_map[name] = id;
}

Ptr<Node>
NlsrApp::GetNode (std::string nodeName)
{
  uint32_t id = m_ndn_node_map[nodeName];
  uint32_t size = ns3::NodeList::GetNNodes();
  for (uint32_t i = 0; i < size; i++) {
    ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(i);
    if (node->GetId() == id) {
      return node;
    }
  }
  return NULL;
}

} // namespace ndn
} // namespace ns3
