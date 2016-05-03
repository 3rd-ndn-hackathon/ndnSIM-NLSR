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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace ns3 {

using namespace std;
namespace pt = boost::property_tree;
typedef boost::property_tree::ptree ConfigSection;

NS_LOG_COMPONENT_DEFINE ("NdnNlsrConfigGen");

// #################### Bulk Config Reader ####################

struct nlsr_common_config {
  std::string lsa_refresh_time;
  std::string router_dead_interval;
  std::string lsa_interest_lifetime;
  std::string hello_retries;
  std::string hello_timeout;
  std::string hello_interval;
  std::string adj_lsa_build_interval;
  std::string first_hello_interval;
  std::string link_cost;
  std::string bandwidth;
  std::string metric;
  std::string delay;
  std::string queue;
  std::string max_faces_per_prefix;
  std::string routing_calc_interval;
} nlsrConf;

bool ProcessCommonConfig() {

  const char *homeDir = NULL;
  if ((homeDir = getenv("HOME")) == NULL) {
      homeDir = getpwuid(getuid())->pw_dir;
  }

  // Get the static config for all nodes.
  std::string common_conf = std::string(homeDir) + "/sandbox/ndnSIM/ns-3/src/ndnSIM/examples/ndn-nlsr-conf/static_nlsr.conf";
  bool ret = true;

  ifstream inputFile;
  inputFile.open(common_conf.c_str());
  if (inputFile.is_open()) {
    ConfigSection pt;
    try {
      boost::property_tree::read_info(inputFile, pt);
    } catch (const boost::property_tree::info_parser_error& error) {
      stringstream msg;
      std::cerr << "Failed to parse common config file " << std::endl;
      std::cerr << common_conf << std::endl;
      return -1;
    }

    for (ConfigSection::const_iterator tn = pt.begin();
         tn != pt.end(); ++tn) {
      std::string sectionName = tn->first;
      const ConfigSection& section = tn->second;

      if (sectionName == "node_config") {
        try {
          nlsrConf.lsa_refresh_time = section.get<string>("lsa-refresh-time");
          nlsrConf.router_dead_interval = section.get<string>("router-dead-interval");
          nlsrConf.lsa_interest_lifetime = section.get<string>("lsa-interest-lifetime");
        } catch (const std::exception& ex) {
          cerr << ex.what() << endl;
          ret = false;
        }
      } else if (sectionName == "adjacency_config") {
        try {
          nlsrConf.hello_retries = section.get<string>("hello-retries");
          nlsrConf.hello_timeout = section.get<string>("hello-timeout");
          nlsrConf.hello_interval = section.get<string>("hello-interval");
          nlsrConf.adj_lsa_build_interval = section.get<string>("adj-lsa-build-interval");
          nlsrConf.first_hello_interval = section.get<string>("first-hello-interval");
          nlsrConf.link_cost = section.get<string>("link-cost");
          nlsrConf.bandwidth = section.get<string>("bandwidth");
          nlsrConf.metric = section.get<string>("metric");
          nlsrConf.delay = section.get<string>("delay");
          nlsrConf.queue = section.get<string>("queue");
        } catch (const std::exception& ex) {
          cerr << ex.what() << endl;
          ret = false;
        }
      } else if (sectionName == "fib_config") {
        try {
          nlsrConf.max_faces_per_prefix = section.get<string>("max-faces-per-prefix");
          nlsrConf.routing_calc_interval = section.get<string>("routing-calc-interval");
        } catch (const std::exception& ex) {
          cerr << ex.what() << endl;
          ret = false;
        }
      }
    } // end of for loop

    {
      cout << "lsa-refresh-time: " << nlsrConf.lsa_refresh_time << std::endl;
      cout << "router-dead-interval: " << nlsrConf.router_dead_interval << std::endl;
      cout << "lsa-interest-lifetime: " << nlsrConf.lsa_interest_lifetime << std::endl;
      cout << "hello-retries: " << nlsrConf.hello_retries << std::endl;
      cout << "hello-timeout: " << nlsrConf.hello_timeout << std::endl;
      cout << "hello-interval: " << nlsrConf.hello_interval << std::endl;
      cout << "adj-lsa-build-interval: " << nlsrConf.adj_lsa_build_interval << std::endl;
      cout << "first-hello-interval: " << nlsrConf.first_hello_interval << std::endl;
      cout << "max-faces-per-prefix: " << nlsrConf.max_faces_per_prefix << std::endl;
      cout << "routing-calc-interval: " << nlsrConf.routing_calc_interval << std::endl;
      cout << "link-cost: " << nlsrConf.link_cost << std::endl;
      cout << "bandwidth: " << nlsrConf.bandwidth << std::endl;
      cout << "metric: " << nlsrConf.metric << std::endl;
      cout << "delay: " << nlsrConf.delay << std::endl;
      cout << "queue: " << nlsrConf.queue << std::endl;
    }
  } else {
    cout << "Could not open common config file: " << common_conf << std::endl;
    return false;
  }

  return ret;
}

bool
ProcessRouterList(const std::string &file)
{
  std::string bulk_config = "src/ndnSIM/examples/ndn-nlsr-conf/router_list.conf";
  ifstream topgen;
  topgen.open (bulk_config.c_str ());
  typedef vector<std::string> nodeList;
  nodeList rtrList;
  std::string rtrId;
  const char *homeDir = NULL;

  if ((homeDir = getenv("HOME")) == NULL) {
      homeDir = getpwuid(getuid())->pw_dir;
  }
  NS_LOG_INFO ("Home dir is: " << homeDir);

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
    std::string logDir = std::string(homeDir) + "/log/" + rtrName + "/nlsr";
    std::string seqDir = std::string(homeDir) + "/log/" + rtrName + "/nlsr";
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
ProcessDotTopology(std::string confFile)
{
  std::string dot_topo_file = confFile;
  ifstream topgen;
  topgen.open (dot_topo_file.c_str ());
  typedef map<std::string, pt::ptree> NODE_CONFIG_MAP;
  typedef std::list<std::string> NODE_LIST;
  typedef map<std::string, NODE_LIST> NODE_ADJ_MAP;
  NODE_CONFIG_MAP nodeMap;
  NODE_ADJ_MAP nodeAdjMap;
  const char *homeDir = NULL;
  std::string nodeLabel;
  std::string nlsr_topo;
  std::string srcNodeId, dir, dstNodeId;
  //ofstream graphvizfile;

  if ((homeDir = getenv("HOME")) == NULL) {
      homeDir = getpwuid(getuid())->pw_dir;
  }
  NS_LOG_INFO ("Home dir is: " << homeDir);

  if ( !topgen.is_open () || !topgen.good () ) {
    NS_FATAL_ERROR ("Cannot open file " << dot_topo_file << " for reading");
    return false;
  }

  while (!topgen.eof ()) {
    string line;
    getline (topgen, line);
    NS_LOG_INFO ("Dot config line: " << line);

    if (line.compare(0, 6, "strict") == 0)  {
      std::size_t f1 = line.find_first_of("(", 0);
      std::size_t f2 = line.find_first_of(",", f1+1);
      nodeLabel = line.substr(f1+1, f2-f1-1);
      NS_LOG_INFO ("Node Label: " << nodeLabel);
      break;
    }
  }

  nlsr_topo = nodeLabel + "-nlsr-topology.dot";
  //graphvizfile.open (nlsr_topo.c_str());
  //if ( !graphvizfile.is_open () || !graphvizfile.good () ) {
  //  NS_FATAL_ERROR ("Cannot open file " << nlsr_topo << " for writing");
  //  return false;
  //}

  if (topgen.eof ()) {
    NS_FATAL_ERROR ("DOT topology config file " << dot_topo_file << " does not have \"Nodes\" section");
    return false;
  }

  // Write Graphviz file header
  //graphvizfile << "graph G {" << endl;
  //graphvizfile << "label=\"" << nodeLabel << " Node Network Topology\";" << endl;
  //graphvizfile << "rankdir=LR;" << endl;

  // Read network 'Node' configuration.
  while (!topgen.eof ()) {
    string line;
    getline (topgen,line);

    std::size_t found = line.find('}');
    if (found != std::string::npos)
      break;

    istringstream linebuffer(line);
    linebuffer >> srcNodeId >> dir >> dstNodeId;
    dstNodeId.erase(dstNodeId.length() - 1, 1);
    //NS_LOG_INFO ("Src Node Id: " << srcNodeId);
    //NS_LOG_INFO ("Dst Node Id: " << dstNodeId.erase(dstNodeId.length() - 1, 1));

    // Populate node array and their adjacency. 
    NODE_ADJ_MAP::iterator it;
    it = nodeAdjMap.find(srcNodeId);
    if (it != nodeAdjMap.end()) {
      nodeAdjMap[srcNodeId].push_back(dstNodeId);
    } else {
      NODE_LIST ndList;
      ndList.push_back(dstNodeId);
      nodeAdjMap[srcNodeId] = ndList;
    }

    it = nodeAdjMap.find(dstNodeId);
    if (it != nodeAdjMap.end()) {
      nodeAdjMap[dstNodeId].push_back(srcNodeId);
    } else {
      NODE_LIST ndList;
      ndList.push_back(srcNodeId);
      nodeAdjMap[dstNodeId] = ndList;
    }

    // Add Nodesto Graphviz (DOT) file.
    //graphvizfile << nodeId << "[width=0.1, label=\"" << nodeId <<  "\", style=filled, fillcolor=\"green\"]" << endl;

    linebuffer.str("");
  }

  NS_LOG_INFO ("Map size: " << nodeAdjMap.size());
  NODE_ADJ_MAP::iterator mi = nodeAdjMap.begin();
  while (mi != nodeAdjMap.end()) {

    NS_LOG_INFO ("Source Node Id: " << mi->first);
    std::string id = mi->first;
    srcNodeId = "N" + id;
    std::string nodeId = srcNodeId;
    std::string city = "NA";
    std::string latitude = "3";
    std::string longitude = "1";
    std::string network = "/n";
    std::string site = "/e";
    std::string router = "/%C1r" + id;
    std::string lsaRefreshTime = nlsrConf.lsa_refresh_time;
    std::string routerDeadInterval = nlsrConf.router_dead_interval;
    std::string lsaInterestLifetime = nlsrConf.lsa_interest_lifetime;
    std::string logLevel = "INFO";
    std::string logDir = std::string(homeDir) + "/log/" + nodeId + "/nlsr";
    std::string seqDir = std::string(homeDir) + "/log/" + nodeId + "/nlsr";
    std::string helloRetries = nlsrConf.hello_retries;
    std::string helloTimeout = nlsrConf.hello_timeout;
    std::string helloInterval = nlsrConf.hello_interval;
    std::string adjLsaBuildInterval = nlsrConf.adj_lsa_build_interval;
    std::string firstHelloInterval = nlsrConf.first_hello_interval;
    std::string state = "off";
    std::string radius = "123.456";
    std::string angle = "1.45";
    std::string maxFacesPerPrefix = nlsrConf.max_faces_per_prefix;
    std::string routingCalcInterval = nlsrConf.routing_calc_interval;
    std::string prefix1 = "/n/e/" + nodeId + "/p1";
    std::string prefix2 = "/n/e/" + nodeId + "/p2";

    //NS_LOG_DEBUG ("Router: " << nodeId << " " << city << " " << latitude << " " << longitude << " " << network << " " << site << " " << router << " " << lsaRefreshTime << " " << routerDeadInterval << " " << lsaInterestLifetime << " " << logLevel << " " << logDir << " " << seqDir << " " << helloRetries << " " << helloTimeout << " " << helloInterval << " " << adjLsaBuildInterval << " " << firstHelloInterval << " " << state << " " << radius << " " << angle << " " << maxFacesPerPrefix << " " << routingCalcInterval << " " << prefix1 << " " << prefix2);

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

    NODE_LIST list = mi->second;
    NODE_LIST::iterator li = list.begin();
    while (li != list.end()) {

      NS_LOG_INFO ("Adj Node Id: " << *li);
      std::string srcId = id;
      std::string dstId = *li;
      dstNodeId = dstId;
      dstNodeId = "N" + dstNodeId;
      std::string name = "/n/e/%C1r" + dstId;
      std::string faceUri = "tcp4://10.0.0." + dstId + ":6363";
      std::string linkCost = nlsrConf.link_cost;
      std::string bandwidth = nlsrConf.bandwidth;
      std::string metric = nlsrConf.metric;
      std::string delay = nlsrConf.delay;
      std::string queue = nlsrConf.queue;

      //NS_LOG_DEBUG ("Adjacent router: " << srcNodeId << " " << dstNodeId << " " << name << " " << faceUri << " " << linkCost << " " << bandwidth << " " << metric << " " << delay << " " << queue);
  
      // Establish full-duplex link between adjacent nodes.
      NODE_CONFIG_MAP::iterator it = nodeMap.find(srcNodeId);
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

/*
      it = nodeMap.find(dstNodeId);
      if (it != nodeMap.end()) {

        // Add neighbor
        auto& nbr = (it->second).get_child("neighbors");
        name = "/n/e/%C1r" + srcId;
        faceUri = "tcp4://10.0.0." + srcId + ":6363";

        pt::ptree nb;
        nb.add("node-id", srcNodeId);
        nb.add("name", name);
        nb.add("face-uri", faceUri);
        nb.add("link-cost", linkCost);
        nb.add("bandwidth", bandwidth);
        nb.add("metric", metric);
        nb.add("delay", delay);
        nb.add("queue", queue);
        nbr.add_child("neighbor", nb);
      }
*/

      // Add Edges to Graphviz (DOT) file.
      //graphvizfile << srcNodeId << "--" << dstNodeId << "[label=\"" << linkCost << "\"]" << endl;

      li++;
    }
    NS_LOG_DEBUG ("\n");
    srcNodeId.clear();
    mi++;
  }

  // Generate the NLSR configuration file
  for (NODE_CONFIG_MAP::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
    std::string filename = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_node_" + it->first + ".conf";
    pt::write_info(filename, it->second); 
  }

  // Generate topology config file.
  std::string sim_config = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_sim.conf";
  pt::ptree simTree;
  for (NODE_CONFIG_MAP::iterator itr = nodeMap.begin(); itr != nodeMap.end(); ++itr) {
    std::string filename = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_node_" + itr->first + ".conf";
    pt::ptree nodeTree;
    nodeTree.add("node-id", itr->first);
    nodeTree.add("nlsr-config", filename);
    simTree.add_child("ndn-node", nodeTree);
  }

  pt::write_info(sim_config, simTree); 

  //graphvizfile << "}" << endl; 
  //graphvizfile.close();

  return true;
}

bool
ProcessBriteTopology(std::string confFile)
{
  std::string brite_topo_file = confFile;
  ifstream topgen;
  topgen.open (brite_topo_file.c_str ());
  typedef map<std::string, pt::ptree> nodeConfigMap;
  nodeConfigMap nodeMap;
  const char *homeDir = NULL;
  std::string nodeLabel;
  std::string nlsr_topo;
  ofstream graphvizfile;

  if ((homeDir = getenv("HOME")) == NULL) {
      homeDir = getpwuid(getuid())->pw_dir;
  }
  NS_LOG_INFO ("Home dir is: " << homeDir);

  if ( !topgen.is_open () || !topgen.good () ) {
    NS_FATAL_ERROR ("Cannot open file " << brite_topo_file << " for reading");
    return false;
  }

  while (!topgen.eof ()) {
    string line;
    getline (topgen, line);
    NS_LOG_INFO ("Brite config line: " << line);

    if (line.compare(0, 9, "Topology:") == 0)  {
      std::size_t f1 = line.find_first_of("(", 0);
      std::size_t f2 = line.find_first_of(" ", f1+2);
      nodeLabel = line.substr(f1+2, f2-f1-2);
    }
 
    if (line.compare(0, 6, "Nodes:") == 0) 
      break;
  }

  nlsr_topo = nodeLabel + "-nlsr-topology.dot";
  graphvizfile.open (nlsr_topo.c_str());
  if ( !graphvizfile.is_open () || !graphvizfile.good () ) {
    NS_FATAL_ERROR ("Cannot open file " << nlsr_topo << " for writing");
    return false;
  }

  if (topgen.eof ()) {
    NS_FATAL_ERROR ("BRITE topology config file " << brite_topo_file << " does not have \"Nodes\" section");
    return false;
  }

  // Write Graphviz file header
  graphvizfile << "graph G {" << endl;
  graphvizfile << "label=\"" << nodeLabel << " Node Network Topology\";" << endl;
  graphvizfile << "rankdir=LR;" << endl;

  // Read network 'Node' configuration.
  while (!topgen.eof ()) {
    string line;
    getline (topgen,line);

    if (line[0] == ';') 
      continue;
    if (line.compare(0, 6, "Edges:") == 0) 
      break;

    istringstream linebuffer(line);
    std::string nodeId, xpos, ypos, indegree, outdegree, asid, type;
    linebuffer >> nodeId >> xpos >> ypos >> indegree >> outdegree >> asid >> type;

    if (nodeId.empty ()) 
      continue;

    std::string id = nodeId;
    nodeId = "N" + nodeId;
    std::string city = "NA";
    std::string latitude = "3";
    std::string longitude = "1";
    std::string network = "/n";
    std::string site = "/e";
    std::string router = "/%C1r" + id;
    std::string lsaRefreshTime = "1800";
    std::string routerDeadInterval = "3600";
    std::string lsaInterestLifetime = "4";
    std::string logLevel = "INFO";
    std::string logDir = std::string(homeDir) + "/log/" + nodeId + "/nlsr";
    std::string seqDir = std::string(homeDir) + "/log/" + nodeId + "/nlsr";
    std::string helloRetries = "2";
    std::string helloTimeout = "5";
    std::string helloInterval = "60";
    std::string adjLsaBuildInterval = "5";
    std::string firstHelloInterval = "10";
    std::string state = "off";
    std::string radius = "123.456";
    std::string angle = "1.45";
    std::string maxFacesPerPrefix = "3";
    std::string routingCalcInterval = "15";
    std::string prefix1 = "/n/e/" + nodeId + "/p1";
    std::string prefix2 = "/n/e/" + nodeId + "/p2";

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
      NS_FATAL_ERROR ("BRITE topology config file " << brite_topo_file << " does not have \"Edge\" section");
      return false;
    }

    // Add Nodesto Graphviz (DOT) file.
    graphvizfile << nodeId << "[width=0.1, label=\"" << nodeId <<  "\", style=filled, fillcolor=\"green\"]" << endl;

    linebuffer.str("");
  }

  // Read network 'Link' configuration.
  while (!topgen.eof()) {
    string line;
    getline (topgen,line);

    if (line[0] == ';') 
      continue;

    istringstream linebuffer(line);
    std::string edgeId, srcNodeId, dstNodeId, length, delay, bandwidth, asfrom, asto, type, unknown, srcId, dstId;
    linebuffer >> edgeId >> srcNodeId >> dstNodeId >> length >> delay >> bandwidth >> asfrom >> asto >> type >> unknown;

    if (edgeId.empty ()) 
      continue;

    srcId = srcNodeId;
    dstId = dstNodeId;
    srcNodeId = "N" + srcNodeId;
    dstNodeId = "N" + dstNodeId;
    std::string name = "/n/e/%C1r" + dstId;
    std::string faceUri = "tcp4://10.0.0." + dstId + ":6363";
    std::string linkCost = "25";
    //std::string length = length.substr(0, length.find('.', 0) + 3);
    bandwidth = "100"; //bandwidth.substr(0, bandwidth.find('.', 0) + 3);
    std::string metric = "1";
    delay = "0"; //delay.substr(0, delay.find('.', 0) + 3);
    std::string queue = "1000";

    NS_LOG_DEBUG ("Adjacent router: " << srcNodeId << " " << dstNodeId << " " << name << " " << faceUri << " " << linkCost << " " << bandwidth << " " << metric << " " << delay << " " << queue);

    // Establish full-duplex link between adjacent nodes.
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

    it = nodeMap.find(dstNodeId);
    if (it != nodeMap.end()) {

      // Add neighbor
      auto& nbr = (it->second).get_child("neighbors");
      name = "/n/e/%C1r" + srcId;
      faceUri = "tcp4://10.0.0." + srcId + ":6363";

      pt::ptree nb;
      nb.add("node-id", srcNodeId);
      nb.add("name", name);
      nb.add("face-uri", faceUri);
      nb.add("link-cost", linkCost);
      nb.add("bandwidth", bandwidth);
      nb.add("metric", metric);
      nb.add("delay", delay);
      nb.add("queue", queue);
      nbr.add_child("neighbor", nb);
    }

    // Add Edges to Graphviz (DOT) file.
    graphvizfile << srcNodeId << "--" << dstNodeId << "[label=\"" << linkCost << "\"]" << endl;

    srcNodeId.clear();
    linebuffer.str("");
  }

  graphvizfile << "}" << endl; 
  graphvizfile.close();

  // Generate the NLSR configuration file
  for (nodeConfigMap::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
    std::string filename = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_node_" + it->first + ".conf";
    pt::write_info(filename, it->second); 
  }

  // Generate topology config file.
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

  const char *homeDir = NULL;

  if ((homeDir = getenv("HOME")) == NULL) {
      homeDir = getpwuid(getuid())->pw_dir;
  }

  //std::string topo_file = "src/ndnSIM/examples/ndn-nlsr-conf/nlsr_router_topo.brite";
  std::string topo_file = std::string(homeDir) + "/sandbox/creepyCode/networkx/scalefree_topo.dot";

#if 0
  std::cout << "No of arguments are: " << argc << endl;
  if (argc < 3) {
    std::cout << "Not enough or invalid arguments, usage is.\n";
    std::cout << "Usage is -t <topo file>\n";
    std::cin.get();
    exit(0);
  } else {
    std::cout << argv[0];
    for (int i = 1; i < argc; i++) {
      if (i + 1 != argc) {
        if (strcmp (argv[i], "-t") == 0) {
          brite_topo = argv[i + 1];
          std::cout << "Topology file is: " << brite_topo;
        } else {
          std::cout << "Not enough or invalid arguments, please try again.\n";
          sleep(2000); 
          exit(0);
        }
      }
    }
    std::cin.get();
    return 0;
  }
#endif

  if(!ProcessCommonConfig())
    return -1;

  //ProcessBriteTopology(topo_file);
  ProcessDotTopology(topo_file);
  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

