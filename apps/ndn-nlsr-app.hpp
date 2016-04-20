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

#ifndef NDN_NLSR_APP_H_
#define NDN_NLSR_APP_H_

#include "ndn-nlsr-exec.hpp"
#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/node-container.h"

namespace ns3 {
namespace ndn {

class NlsrApp: public Application {

public:
  static TypeId GetTypeId (void);

  NlsrApp();

  virtual
  ~NlsrApp();

  void
  Initialize(std::string& nodeConfig);

  nlsr::ConfParameter&
  GetConfParameter()
  {
    return m_instance->GetConfParameter();
  }

  nlsr::Nlsr&
  GetNlsr()
  {
    return m_instance->GetNlsr();
  }

  void
  SetNodeName(std::string name)
  {
    m_nodeName = name;
  }

  std::string
  GetNodeName()
  {
    return m_nodeName;
  }

  void
  SetNodeNameToIdMapping(std::string name, uint32_t id);

  Ptr<Node>
  GetNode(std::string name);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication ();    // Called at time specified by Start

  virtual void
  StopApplication ();     // Called at time specified by Stop

private:
  std::unique_ptr<ndn::NlsrExec> m_instance;
  std::string m_nodeConfigFile;
  std::string m_nodeName;
  NodeContainer *m_nodes;
  typedef std::map<std::string, uint32_t> NDN_NODE_MAP;
  NDN_NODE_MAP m_ndn_node_map;
};

} // namespace ndn
} // namespace ns3

#endif /* NDN_NLSR_APP_H_ */
