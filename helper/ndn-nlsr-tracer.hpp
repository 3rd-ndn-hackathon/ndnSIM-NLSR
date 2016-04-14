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
  static
  NlsrTracer& Instance() {
    if (!inst) 
      inst = new NlsrTracer();
    return *inst;
  }

  virtual
  ~NlsrTracer() {
    // Close of streams
    of_hello.close();
    of_nlsa.close();
    of_llsa.close();
    of_nsync.close();
    of_fib.close();
  }

  void 
  InitializeTracer(std::string prefix) {
    boost::filesystem::path full_path(boost::filesystem::current_path());
    m_helloTracer += full_path.string() + "/" + prefix + "-nlsr-hello-trace.txt";
    of_hello.open(m_helloTracer.c_str());

    m_nameLsaTracer += full_path.string() + "/" + prefix + "-nlsr-name-lsa-trace.txt";
    of_nlsa.open(m_nameLsaTracer.c_str()); 

    m_linkLsaTracer += full_path.string() + "/" + prefix + "-nlsr-link-lsa-trace.txt";
    of_llsa.open(m_linkLsaTracer.c_str()); 

    m_nsyncTracer += full_path.string() + "/" + prefix + "-nlsr-nsync-trace.txt";
    of_nsync.open(m_nsyncTracer.c_str()); 

    m_fibTracer += full_path.string() + "/" + prefix + "-nlsr-fib-trace.txt";
    of_fib.open(m_fibTracer.c_str()); 

    WriteHeaders();
  }

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
  NlsrTracer() {
  }

  void WriteHeaders();

  NlsrTracer(const NlsrTracer&);
  NlsrTracer& operator=(const NlsrTracer&);

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
};

} // namespace ndn
} // namespace ns3

#endif // _NDN_NLSR_TRACER_H
