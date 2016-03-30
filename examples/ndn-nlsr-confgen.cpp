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

// ndn-nlsr-conf-gen.cpp

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/names.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <set>
#include <map>
#include <vector>

namespace ns3 {

using namespace std;
namespace pt = boost::property_tree;

NS_LOG_COMPONENT_DEFINE ("NdnNlsrConfigGen");

// #################### Bulk Config Reader ####################
bool
ProcessRouterList(const std::string &file)
{
  std::string bulk_config = "src/ndnSIM/examples/ndn-nlsr-conf/router_list.conf";
  ifstream topgen;
  topgen.open (bulk_config.c_str ());
  typedef vector<std::string> nodeList;
  nodeList rtrList;
  std::string rtrId;

  if ( !topgen.is_open () || !topgen.good () ) {
    NS_FATAL_ERROR ("Cannot open file " << bulk_config << " for reading");
    return false;
  }

  while (!topgen.eof ()) {
    string line;
    getline (topgen, line);

    if (line == "router-list") 
      break;
  }

  if (topgen.eof ()) {
    NS_FATAL_ERROR ("Bulk config file " << bulk_config << " does not have \"router-list\" section");
    return false;
  }

  while (!topgen.eof ()) {
    std::string line;
    getline (topgen,line);

    if (line[0] == ';' || line.empty()) 
      continue;

    NS_LOG_INFO ("Router list: " << line);
    char* str = strtok ((char*)line.c_str(), " ");
    while (str != NULL) {
      rtrList.push_back(std::string(str));
      str = strtok (NULL, " ");
    }
  }

  // Print all Ids.
  ofstream of (file.c_str ());
  int size = rtrList.size();

  of << ";" << endl;
  of << ";This is a bulk configuration file. Each of the router configuration is provided" << endl;
  of << ";under 'ndn-router' tag. Each line contains a unique router config." << endl;
  of << ";" << endl;
  of << "ndn-routers" << endl;
  of << ";node-id city latitude longitude network site router lsa-refresh-time router-dead-interval lsa-interest-lifetime log-level log-dir seq-dir hello-retries hello-timeout hello-interval adj-lsa-build-interval first-hello-interval state radius angle max-faces-per-prefix routing-calc-interval prefix1 prefix2" << endl;

  for (int i = 0; i < size; ++i) {
    std::string srcRtr = rtrList[i];
    std::string rtrName = rtrList[i];
    std::transform(rtrName.begin(), rtrName.end(), rtrName.begin(), ::tolower);
    std::string logDir = "/home/anilj1/log/" + rtrName + "/nlsr";
    std::string seqDir = "/home/anilj1/log/" + rtrName + "/nlsr";
    std::string prfx1 = "/ndn/edu/" + rtrName + "/cs/netlab";
    std::string prfx2 = "/ndn/edu/" + rtrName + "/cs/sensorlab";

    of << srcRtr << " " << "NA" << " " << "3" << " " << "1" << " " << "/ndn" << " " << "/edu/" << rtrName << " " << "/%C1.Router/cs/" << rtrName << "rtr" << " " << "1800 3600 4 INFO" << " " << logDir << " " << seqDir << " " << "2 5 60 5 10 off 123.456 1.45 3 15 " << " " << prfx1 << " " << prfx2 << endl;
  }

  // Build the adjacency matrix. This is a MESH topology (NxN).
  of << endl;
  of << ";Each of the link adjacency configuration is provided under 'adjacency-matric' tag." << endl;
  of << ";Each of the line contains a link specification connecting two router nodes." << endl;
  of << ";" << endl;
  of << "adjacency-matrix" << endl;
  of << ";src-node-id dst-node-id name face-uri link-cost bandwidth metric delay queue" << endl;
  for (int i = 0; i < size; ++i) {
    std::string srcRtr = rtrList[i];
      for (int j = 0; j < size; ++j) {
        std::string dstRtr = rtrList[j];
        std::string dstRtrName = rtrList[j];
        std::transform(dstRtrName.begin(), dstRtrName.end(), dstRtrName.begin(), ::tolower);
	if (srcRtr.compare(dstRtr) != 0) {
          of << srcRtr << " " << dstRtr << " " << "/ndn/edu/" << dstRtrName << "/%C1.Router/cs/" << dstRtrName << "rtr" << " " << "tcp4://10.0.0." << j << ":6363" << " " << "25 100 1 0 1000"<< endl;
	}
      }
  }

  of.flush();
  of.close();
  return true;
}

bool
ProcessBulkConfig(std::string confFile)
{
  std::string bulk_config = confFile;
  ifstream topgen;
  topgen.open (bulk_config.c_str ());
  typedef map<std::string, pt::ptree> nodeConfigMap;
  nodeConfigMap nodeMap;

  if ( !topgen.is_open () || !topgen.good () ) {
    NS_FATAL_ERROR ("Cannot open file " << bulk_config << " for reading");
    return false;
  }

  while (!topgen.eof ()) {
    string line;
    getline (topgen, line);

    if (line == "ndn-routers") 
      break;
  }

  if (topgen.eof ()) {
    NS_FATAL_ERROR ("Bulk config file " << bulk_config << " does not have \"ndn-routers\" section");
    return false;
  }

  while (!topgen.eof ()) {
    string line;
    getline (topgen,line);

    if (line[0] == ';') 
      continue;
    if (line == "adjacency-matrix") 
      break;

    istringstream linebuffer(line);
    std::string nodeId, city, latitude, longitude, network, site, router, lsaRefreshTime, routerDeadInterval, lsaInterestLifetime, logLevel, logDir, seqDir, helloRetries, helloTimeout, helloInterval, adjLsaBuildInterval, firstHelloInterval, state, radius, angle, maxFacesPerPrefix, routingCalcInterval, prefix1, prefix2;

    linebuffer >> nodeId >> city >> latitude >> longitude >> network >> site >> router >> lsaRefreshTime >> routerDeadInterval >> lsaInterestLifetime >> logLevel >> logDir >> seqDir >> helloRetries >> helloTimeout >> helloInterval >> adjLsaBuildInterval >> firstHelloInterval >> state >> radius >> angle >> maxFacesPerPrefix >> routingCalcInterval >> prefix1 >> prefix2;

    if (nodeId.empty ()) 
      continue;

    NS_LOG_DEBUG ("Router: " << nodeId << " " << city << " " << latitude << " " << longitude << " " << network << " " << site << " " << router << " " << lsaRefreshTime << " " << routerDeadInterval << " " << lsaInterestLifetime << " " << logLevel << " " << logDir << " " << seqDir << " " << helloRetries << " " << helloTimeout << " " << helloInterval << " " << adjLsaBuildInterval << " " << firstHelloInterval << " " << state << " " << radius << " " << angle << " " << maxFacesPerPrefix << " " << routingCalcInterval << " " << prefix1 << " " << prefix2);

    // Add General config
    pt::ptree nt;
    nt.put("general.node-id", nodeId);
    nt.put("general.city", city);
    nt.put("general.latitude", latitude);
    nt.put("general.longitude", longitude);
    nt.put("general.network", network);
    nt.put("general.site", site);
    nt.put("general.router", router);
    nt.put("general.lsa-refresh-time", lsaRefreshTime);
    nt.put("general.router-dead-interval", routerDeadInterval);
    nt.put("general.lsa-interest-lifetime", lsaInterestLifetime);
    nt.put("general.log-level", logLevel);
    nt.put("general.log-dir", logDir);
    nt.put("general.seq-dir", seqDir);

    // Add Neighbors config
    nt.put("neighbors.hello-retries", helloRetries);
    nt.put("neighbors.hello-timeout", helloTimeout);
    nt.put("neighbors.hello-interval", helloInterval);
    nt.put("neighbors.adj-lsa-build-interval", adjLsaBuildInterval);
    nt.put("neighbors.first-hello-interval", firstHelloInterval);

    // Add Hyperbolic config
    nt.put("hyperbolic.state", state);
    nt.put("hyperbolic.radius", radius);
    nt.put("hyperbolic.angle", angle);

    // Add Fib config
    nt.put("fib.max-faces-per-prefix", maxFacesPerPrefix);
    nt.put("fib.routing-calc-interval", routingCalcInterval);

    // Add Advertising config
    nt.add("advertising.prefix", prefix1);
    nt.add("advertising.prefix", prefix2);

    // Add Security config
    nt.put("security.validator.trust-anchor.type", "any");
    nt.put("security.prefix-update-validator.trust-anchor.type", "any");

    // Save the tree object for later on.
    nodeMap[nodeId] = nt;

    if (topgen.eof ()) {
      NS_LOG_ERROR ("Bulk config file " << bulk_config << " does not have \"adjacency matrix\" section");
      return false;
    }

    linebuffer.str("");
  }

  while (!topgen.eof()) {
    string line;
    getline (topgen,line);

    if (line[0] == ';') 
      continue;

    istringstream linebuffer(line);
    std::string srcNodeId, dstNodeId, name, faceUri, linkCost, bandwidth, metric, delay, queue;
    linebuffer >> srcNodeId >> dstNodeId >> name >> faceUri >> linkCost >> bandwidth >> metric >> delay >> queue;

    if (srcNodeId.empty ()) 
      continue;

    NS_LOG_DEBUG ("Adjacent router: " << srcNodeId << " " << dstNodeId << " " << name << " " << faceUri << " " << linkCost << " " << bandwidth << " " << metric << " " << delay << " " << queue);

    nodeConfigMap::iterator it = nodeMap.find(srcNodeId);
    if (it != nodeMap.end()) {
      auto& nbr = (it->second).get_child("neighbors");

      // Add neighbor
      pt::ptree nb;
      nb.add("node-id", dstNodeId);
      nb.add("name", name);
      nb.add("face-uri", faceUri);
      nb.add("link-cost", linkCost);
      nb.add("bandwidth", bandwidth);
      nb.add("metric", metric);
      nb.add("delay", delay);
      nb.add("queue", queue);
      nbr.add_child("neighbor", nb);
    }
    srcNodeId.clear();
    linebuffer.str("");
  }

  // Generate the NLSR configuration file
  for (nodeConfigMap::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
    std::string filename = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_node_" + it->first + ".conf";
    pt::write_info(filename, it->second); 
  }

  // Generate topology config file.
  //std::string sim_config = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_sim.conf";
  std::string sim_config = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_sim.conf";
  pt::ptree simTree;
  for (nodeConfigMap::iterator itr = nodeMap.begin(); itr != nodeMap.end(); ++itr) {
    std::string filename = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_node_" + itr->first + ".conf";
    pt::ptree nodeTree;
    nodeTree.add("node-id", itr->first);
    nodeTree.add("nlsr-config", filename);
    simTree.add_child("ndn-node", nodeTree);
  }

  pt::write_info(sim_config, simTree); 

  return true;
}

int
main (int argc, char *argv[])
{
  std::string bulk_config = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_bulk.conf";

  ProcessRouterList(bulk_config);
  ProcessBulkConfig(bulk_config);

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

