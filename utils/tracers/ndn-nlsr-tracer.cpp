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

#include "ns3/log.h"
#include "ndn-nlsr-tracer.hpp"

#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/node-list.h>
#include <ns3/names.h>

#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("NlsrTracer");

std::string helloTraceFile = "-node-nlsr-hello-trace-";
std::string syncTraceFile = "-node-nlsr-sync-trace-";
std::string nlsaTraceFile = "-node-nlsr-nlsa-trace-";
std::string llsaTraceFile = "-node-nlsr-llsa-trace-";
std::string fibTraceFile = "-node-nlsr-fib-trace-";

namespace ns3 {

namespace ndn {

NlsrTracer* NlsrTracer::inst = 0;
int NlsrTracer::m_HelloCount = 0;
int NlsrTracer::m_NameLsaCount = 0;
int NlsrTracer::m_LinkLsaCount = 0;
int NlsrTracer::m_NsyncCount = 0;
int NlsrTracer::m_FibCount = 0;

int NlsrTracer::m_HelloFileCount = 0;
int NlsrTracer::m_NameLsaFileCount = 0;
int NlsrTracer::m_LinkLsaFileCount = 0;
int NlsrTracer::m_NsyncFileCount = 0;
int NlsrTracer::m_FibFileCount = 0;

bool NlsrTracer::m_EnableTracer = true;

NlsrTracer& NlsrTracer::Instance() {
  if (!inst) 
    inst = new NlsrTracer();
  return *inst;
}

NlsrTracer::NlsrTracer() {
  m_LogBlockSize = 4000;

  char* str = getenv("ENABLE_TRACER");
  if (str != NULL) {
    if (strcmp(str, "TRUE") == 0) {
      m_EnableTracer = true;
      NS_LOG_INFO ("Nlsr tracer is enabled");
    } else if(strcmp(str, "FALSE") == 0) {
      m_EnableTracer = false;
      NS_LOG_INFO ("Nlsr tracer is disabled");
    }
  } else {
    m_EnableTracer = true;
    NS_LOG_INFO ("Nlsr tracer is enabled");
  }
}

NlsrTracer::~NlsrTracer() {
  // Close of streams
  of_hello.close();
  of_nlsa.close();
  of_llsa.close();
  of_nsync.close();
  of_fib.close();
}

// LOG_FILE_SIZE can be configured from env variable.
void
NlsrTracer::SetLogRollOverSize() {
  char* str = getenv("LOG_ROLL_OVER");
  if (str != NULL) {
    try {
      m_LogBlockSize = std::stoi(str);
      NS_LOG_INFO ("Log roll-over size set to: " << m_LogBlockSize);
    } catch (const std::invalid_argument& ia) {
      NS_LOG_ERROR ("Invalid LOG_ROLL_OVER size configured, using default: " << m_LogBlockSize);
    }
  } else {
    NS_LOG_INFO ("No log roll over specified, using default: 4000");
  }
}

bool
NlsrTracer::IsEnabled() {
  return m_EnableTracer;
}

void 
NlsrTracer::InitializeTracer(std::string prefix) {

  if (!m_EnableTracer) {
    return;
  }

  m_prefix = prefix;
  boost::filesystem::path full_path(boost::filesystem::current_path());
  m_helloTracer = full_path.string() + "/" + m_prefix + helloTraceFile + std::to_string(m_HelloFileCount++) + ".txt";
  of_hello.open(m_helloTracer.c_str());

  m_nameLsaTracer = full_path.string() + "/" + m_prefix + nlsaTraceFile + std::to_string(m_NameLsaFileCount++) + ".txt";
  of_nlsa.open(m_nameLsaTracer.c_str()); 

  m_linkLsaTracer = full_path.string() + "/" + m_prefix + llsaTraceFile + std::to_string(m_LinkLsaFileCount++) + ".txt";
  of_llsa.open(m_linkLsaTracer.c_str()); 

  m_nsyncTracer = full_path.string() + "/" + m_prefix + syncTraceFile + std::to_string(m_NsyncFileCount++) + ".txt";
  of_nsync.open(m_nsyncTracer.c_str()); 

  m_fibTracer = full_path.string() + "/" + m_prefix + fibTraceFile + std::to_string(m_FibFileCount++) + ".txt";
  of_fib.open(m_fibTracer.c_str()); 

  WriteHeaders();
  SetLogRollOverSize();
}

void
NlsrTracer::WriteHeaders() {
  of_llsa << "Time" << "\tNode" << "\tFaceId" << "\tFaceDescr" << "\tType" << "\tPackets" << "\tKBytes" << "\tPacketRaw" << endl;
  of_nlsa << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  of_hello << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  of_nsync << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  of_fib << "Time" << "\tNode" << "\tName" << "\tType" << "\tAttempt" << "\t-" << "\t-" << "\t-" << endl;
}

void
NlsrTracer::HelloTrace(std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6) {

  if (!m_EnableTracer) {
    return;
  }

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_hello << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << arg1 << "\t" << arg2 << "\t" << arg3 << "\t" << arg4 << "\t" << arg5 << "\t" << arg6 << endl; 
  if (++m_HelloCount == m_LogBlockSize) {
    // Close and create a new log file
    of_hello.flush();
    m_HelloCount = 0;
    of_hello.close();

    boost::filesystem::path full_path(boost::filesystem::current_path());
    m_helloTracer = full_path.string() + "/" + m_prefix + helloTraceFile + std::to_string(m_HelloFileCount++) + ".txt";
    of_hello.open(m_helloTracer.c_str()); 
    of_hello << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  }
}

void 
NlsrTracer::NameLsaTrace(std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6) {

  if (!m_EnableTracer) {
    return;
  }

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_nlsa << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << arg1 << "\t" << arg2 << "\t" << arg3 << "\t" << arg4 << "\t" << arg5 << "\t" << arg6 << endl; 
  if (++m_NameLsaCount == m_LogBlockSize) {
    // Close and create a new log file
    of_nlsa.flush();
    m_NameLsaCount = 0;
    of_nlsa.close();

    boost::filesystem::path full_path(boost::filesystem::current_path());
    m_nameLsaTracer = full_path.string() + "/" + m_prefix + nlsaTraceFile + std::to_string(m_NameLsaFileCount++) + ".txt";
    of_nlsa.open(m_nameLsaTracer.c_str()); 
    of_nlsa << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  }
}

void 
NlsrTracer::LinkLsaTrace(std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6) {

  if (!m_EnableTracer) {
    return;
  }

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_llsa << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << arg1 << "\t" << arg2 << "\t" << arg3 << "\t" << arg4 << "\t" << arg5 << "\t" << arg6 << endl; 
  if (++m_LinkLsaCount == m_LogBlockSize) {
    of_llsa.flush();
    m_LinkLsaCount = 0;
    of_llsa.close();

    boost::filesystem::path full_path(boost::filesystem::current_path());
    m_linkLsaTracer = full_path.string() + "/" + m_prefix + llsaTraceFile + std::to_string(m_LinkLsaFileCount++) + ".txt";
    of_llsa.open(m_linkLsaTracer.c_str()); 
    of_llsa << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  }
}

void 
NlsrTracer::NsyncTrace(std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6) {

  if (!m_EnableTracer) {
    return;
  }

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_nsync << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << arg1 << "\t" << arg2 << "\t" << arg3 << "\t" << arg4 << "\t" << arg5 << "\t" << arg6 << endl; 
  if (++m_NsyncCount == m_LogBlockSize) {
    // Close and create a new log file
    of_nsync.flush();
    m_NsyncCount = 0;
    of_nsync.close();

    boost::filesystem::path full_path(boost::filesystem::current_path());
    m_nsyncTracer = full_path.string() + "/" + m_prefix + syncTraceFile + std::to_string(m_NsyncFileCount++) + ".txt";
    of_nsync.open(m_nsyncTracer.c_str()); 
    of_nsync << "Time" << "\tNode" << "\tName" << "\tType" << "\tPackets" << "\tKBytes" << "\t-" << "\t-" << endl;
  }
}

void 
NlsrTracer::FibTrace(std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6) {

  if (!m_EnableTracer) {
    return;
  }

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  std::string nodeName = Names::FindName(node);

  of_fib << Simulator::Now().ToDouble(Time::S) << "\t" << nodeName << "\t" << arg1 << "\t" << arg2 << "\t" << arg3 << "\t" << arg4 << "\t" << arg5 << "\t" << arg6 << endl; 
  if (++m_FibCount == m_LogBlockSize) {
    // Close and create a new log file
    of_fib.flush();
    m_FibCount = 0;
    of_fib.close();

    boost::filesystem::path full_path(boost::filesystem::current_path());
    m_fibTracer = full_path.string() + "/" + m_prefix + fibTraceFile + std::to_string(m_FibFileCount++) + ".txt";
    of_fib.open(m_fibTracer.c_str()); 
    of_fib << "Time" << "\tNode" << "\tName" << "\tType" << "\tAttempt" << "\t-" << "\t-" << "\t-" << endl;
  }
}

} // namespace ndn
} // namespace ns3
