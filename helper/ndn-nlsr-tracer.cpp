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

// ndn-nlsr-tracer.cpp

#include <fstream>

#include "ndn-nlsr-tracer.hpp"

#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/node-list.h>
#include <ns3/names.h>

#include "ns3/simulator.h"

namespace ns3 {

namespace ndn {

NlsrTracer* NlsrTracer::inst = 0;

void
NlsrTracer::WriteHeaders() {
  of_hello << "Time" << "\tNode" << "\tFaceId" << "\tFaceDescr" << "\tType" << "\tPackets" << "\tKBytes" << "\tPacketRaw" << endl;
  of_nlsa << "Time" << "\tNode" << "\tFaceId" << "\tFaceDescr" << "\tType" << "\tPackets" << "\tKBytes" << "\tPacketRaw" << endl;
  of_llsa << "Time" << "\tNode" << "\tFaceId" << "\tFaceDescr" << "\tType" << "\tPackets" << "\tKBytes" << "\tPacketRaw" << endl;
  of_nsync << "Time" << "\tNode" << "\tFaceId" << "\tFaceDescr" << "\tType" << "\tPackets" << "\tKBytes" << "\tPacketRaw" << endl;
  of_fib << "Time" << "\tNode" << "\tFaceId" << "\tFaceDescr" << "\tType" << "\tPackets" << "\tKBytes" << "\tPacketRaw" << endl;
}

void
NlsrTracer::HelloTrace(const char*, const char*, const char*, const char*, const char*, const char*) {
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_hello << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << " 3 " << "\t" << " 4 " << "\t" << " 5 " << "\t" << " 6 " << endl; 
}

void 
NlsrTracer::NameLsaTrace(const char*, const char*, const char*, const char*, const char*, const char*) {
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_nlsa << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << " 3 " << "\t" << " 4 " << "\t" << " 5 " << "\t" << " 6 " << endl; 
}

void 
NlsrTracer::LinkLsaTrace(const char*, const char*, const char*, const char*, const char*, const char*) {
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_llsa << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << " 3 " << "\t" << " 4 " << "\t" << " 5 " << "\t" << " 6 " << endl; 
}

void 
NlsrTracer::NsyncTrace(const char*, const char*, const char*, const char*, const char*, const char*) {
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_nsync << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << " 3 " << "\t" << " 4 " << "\t" << " 5 " << "\t" << " 6 " << endl; 
}

void 
NlsrTracer::FibTrace(const char*, const char*, const char*, const char*, const char*, const char*) {
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_fib << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << " 3 " << "\t" << " 4 " << "\t" << " 5 " << "\t" << " 6 " << endl; 
}

} // namespace ndn
} // namespace ns3
