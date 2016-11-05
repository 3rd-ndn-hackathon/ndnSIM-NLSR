/*{* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

// ndn-nlsr-tracer.hpp

#ifndef _NDN_NLSR_TRACER_H
#define _NDN_NLSR_TRACER_H

#include <iostream>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>

namespace ns3 {

using namespace std;

namespace ndn {

class NlsrTracer {

public:
  static NlsrTracer& Instance();

  virtual ~NlsrTracer();

  void InitializeTracer(std::string prefix);

  bool IsEnabled();

  void 
  HelloTrace(std::string agr1 = "-", std::string agr2 = "-", std::string agr3 = "-", std::string agr4 = "-", std::string agr5 = "-", std::string agr6 = "-");

  void 
  NameLsaTrace(std::string agr1 = "-", std::string agr2 = "-", std::string agr3 = "-", std::string agr4 = "-", std::string agr5 = "-", std::string agr6 = "-");

  void 
  LinkLsaTrace(std::string agr1 = "-", std::string agr2 = "-", std::string agr3 = "-", std::string agr4 = "-", std::string agr5 = "-", std::string agr6 = "-");

  void 
  NsyncTrace(std::string agr1 = "-", std::string agr2 = "-", std::string agr3 = "-", std::string agr4 = "-", std::string agr5 = "-", std::string agr6 = "-");

  void 
  FibTrace(std::string agr1 = "-", std::string agr2 = "-", std::string agr3 = "-", std::string agr4 = "-", std::string agr5 = "-", std::string agr6 = "-");

private:

  NlsrTracer();
  NlsrTracer(const NlsrTracer&);
  NlsrTracer& operator=(const NlsrTracer&);

  void WriteHeaders();
  void SetLogRollOverSize();

  std::string m_prefix;
  std::string m_currPath;
  std::string m_helloTracer;
  std::string m_nameLsaTracer;
  std::string m_linkLsaTracer;
  std::string m_nsyncTracer;
  std::string m_fibTracer;

  std::ofstream of_hello;
  std::ofstream of_nlsa;
  std::ofstream of_llsa;
  std::ofstream of_nsync;
  std::ofstream of_fib;

  static NlsrTracer* inst;
  static int m_HelloCount;
  static int m_NameLsaCount;
  static int m_LinkLsaCount;
  static int m_NsyncCount;
  static int m_FibCount;

  static int m_HelloFileCount;
  static int m_NameLsaFileCount;
  static int m_LinkLsaFileCount;
  static int m_NsyncFileCount;
  static int m_FibFileCount;

  static bool m_EnableTracer;
  int m_LogBlockSize;
};

} // namespace ndn
} // namespace ns3

#endif // _NDN_NLSR_TRACER_H
