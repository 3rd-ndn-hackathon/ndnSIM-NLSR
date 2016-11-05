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

// ndn-nlsr-conf-helper.cpp

#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/face-uri.hpp>

// boost needs to be included after ndn-cxx, otherwise there will be conflict with _1, _2, ...
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/names.h"
#include "ns3/net-device-container.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-address.h"
#include "model/ndn-l3-protocol.hpp"
#include "ns3/random-variable-stream.h"
#include "ns3/error-model.h"
#include "ns3/double.h"
#include "ns3/constant-position-mobility-model.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <set>
#include <map>

#include "utility/name-helper.hpp"
#include "update/prefix-update-processor.hpp"

#include "adjacent.hpp"
#include "apps/ndn-app.hpp"
#include "apps/ndn-nlsr-app.hpp"

#include "nlsr-conf-reader.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.NlsrConfReader");

namespace ns3 {
namespace ndn {

using namespace std;
namespace pt = boost::property_tree;

template <class T>
class ConfigVariable
{
public:
  typedef ndn::function<void(T)> ConfParameterCallback;
  typedef boost::property_tree::ptree ConfigSection;

  ConfigVariable(const std::string& key, const ConfParameterCallback& setter)
    : m_key(key)
    , m_setterCallback(setter)
    , m_minValue(0)
    , m_maxValue(0)
    , m_shouldCheckRange(false)
    , m_isRequired(true)
  {
  }

  bool
  ParseFromConfigSection(const ConfigSection& section)
  {
    try {
      T value = section.get<T>(m_key);

      if (!isValidValue(value)) {
        return false;
      }

      m_setterCallback(value);
      return true;
    }
    catch (const std::exception& ex) {

      if (m_isRequired) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Missing required configuration variable" << std::endl;
        return false;
      }
      else {
        m_setterCallback(m_defaultValue);
        return true;
      }
    }

    return false;
  }

  void
  SetMinAndMaxValue(T min, T max)
  {
    m_minValue = min;
    m_maxValue = max;
    m_shouldCheckRange = true;
  }

  void
  SetOptional(T defaultValue)
  {
    m_isRequired = false;
    m_defaultValue = defaultValue;
  }

private:
  void
  PrintOutOfRangeError(T value)
  {
    std::cerr << "Invalid value for " << m_key << ": "
              << value << ". "
              << "Valid values: "
              << m_minValue << " - "
              << m_maxValue << std::endl;
  }

  bool
  IsValidValue(T value)
  {
    if (!m_shouldCheckRange) {
      return true;
    }
    else if (value < m_minValue || value > m_maxValue)
    {
      printOutOfRangeError(value);
      return false;
    }

    return true;
  }

private:
  const std::string m_key;
  const ConfParameterCallback m_setterCallback;
  T m_defaultValue;

  T m_minValue;
  T m_maxValue;

  bool m_shouldCheckRange;
  bool m_isRequired;
};

NlsrConfReader::NlsrConfReader (const std::string &path, double scale/*=1.0*/)
  : m_confFileName (path)
  , m_randX(CreateObject<UniformRandomVariable>())
  , m_randY(CreateObject<UniformRandomVariable>())
  , m_scale (scale)
  , m_requiredPartitions (1)
{
  m_path = m_confFileName;

  NS_LOG_FUNCTION (this);

  m_randX->SetAttribute("Min", DoubleValue(0));
  m_randX->SetAttribute("Max", DoubleValue(100.0));

  m_randY->SetAttribute("Min", DoubleValue(0));
  m_randY->SetAttribute("Max", DoubleValue(100.0));

  SetMobilityModel ("ns3::ConstantPositionMobilityModel");
}

NlsrConfReader::~NlsrConfReader ()
{
  NS_LOG_FUNCTION (this);
}

bool
NlsrConfReader::ProcessConfFile()
{
  bool ret = true;
  ifstream inputFile;
  inputFile.open(m_confFileName.c_str());
  if (!inputFile.is_open()) {
    string msg = "Failed to read configuration file: ";
    msg += m_confFileName;
    cerr << msg << endl;
    return false;
  }
  ret = Load(inputFile);
  inputFile.close();

  return ret;
}

bool
NlsrConfReader::Load(istream& input)
{
  ConfigSection pt;
  bool ret = true;
  try {
    boost::property_tree::read_info(input, pt);
  }
  catch (const boost::property_tree::info_parser_error& error) {
    stringstream msg;
    std::cerr << "Failed to parse configuration file " << std::endl;
    std::cerr << m_confFileName << std::endl;
    return false;
  }

  for (ConfigSection::const_iterator tn = pt.begin();
       tn != pt.end(); ++tn) {
    ret = ProcessSection(tn->first, tn->second);
    if (ret == false) {
      break;
    }
  }
  return ret;
}

bool
NlsrConfReader::ProcessSection(const std::string& sectionName, const ConfigSection& section)
{
  bool ret = true;
  if (sectionName == "ndn-node")
  {
    ret = ProcessNlsrConfig(section);
  }
  else if (sectionName == "general")
  {
    ret = ProcessNodeId(section);
  }
  else if (sectionName == "neighbors")
  {
    ret = ProcessNeighborNodes(section);
  }
  else if (sectionName == "hyperbolic")
  {
    // Do nothing.
  }
  else if (sectionName == "fib")
  {
    // Do nothing.
  }
  else if (sectionName == "advertising")
  {
    // Do nothing.
  }
  else if (sectionName == "security")
  {
    // Do nothing.
  }
  else
  {
    std::cerr << "Wrong configuration section: " << sectionName << " " << m_confFileName << std::endl;
  }
  return ret;
}

bool
NlsrConfReader::ProcessNlsrConfig(const ConfigSection& section)
{
  try {
    //std::string nodeId = section.get<string>("node-id");
    //if (nodeId.empty()) {
    //  cerr << " Node id can not be null or empty!" << endl;
    //  return false;
    //}

    std::string nlsrConfigFile = section.get<string>("nlsr-config");
    if (nlsrConfigFile.empty()) {
      cerr << "Config file path can not be null or empty!" << endl;
      return false;
    }

    //m_srcNodeId = nodeId;
    m_confFileName = nlsrConfigFile;
    this->ProcessConfFile();
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
    return false;
  }

  return true;
}

bool
NlsrConfReader::ProcessNodeId(const ConfigSection& section)
{
  std::string nodeId;
  std::string city;
  double latitude;
  double longitude;

  try {
    nodeId = section.get<string>("node-id");
    if (nodeId.empty()) {
      cerr << " Node id can not be null or empty!" << endl;
      return false;
    }

    city = section.get<string>("city");
    if (city.empty()) {
      cerr << " Node id can not be null or empty!" << endl;
      return false;
    }

    latitude = section.get<double>("latitude");
    longitude = section.get<double>("longitude");
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
    return false;
  }

  // Save the node configuration.
  m_srcNodeId = nodeId;
  NetworkNode node(m_confFileName, nodeId, city, latitude, longitude);
  m_node_map[nodeId] = node;

  return true;
}

bool
NlsrConfReader::ProcessNeighborNodes(const ConfigSection& section)
{
  ADJACENCY_LIST adjList;

  for (ConfigSection::const_iterator tn =
           section.begin(); tn != section.end(); ++tn) {

    if (tn->first == "neighbor")
    {
      try {
        ConfigSection CommandAttriTree = tn->second;
        std::string nodeId = CommandAttriTree.get<std::string>("node-id");
        std::string bandwidth = CommandAttriTree.get<std::string>("bandwidth");
        std::string metric = CommandAttriTree.get<std::string>("metric");
        std::string delay = CommandAttriTree.get<std::string>("delay");
        std::string queue = CommandAttriTree.get<std::string>("queue");
        std::string name = CommandAttriTree.get<std::string>("name");
        std::string faceUri = CommandAttriTree.get<std::string>("face-uri");
        std::string linkCost = CommandAttriTree.get<std::string>("link-cost");

        if (nodeId.empty()) {
          cerr << " Node id can not be null or empty!" << endl;
          return false;
        }
	
        ::ndn::util::FaceUri uri;
        if (!uri.parse(faceUri)) {
          std::cerr << "Malformed face-uri <" << faceUri << "> for " << name << std::endl;
          return false;
        }

        if (linkCost.empty()) {
          linkCost = std::to_string(nlsr::Adjacent::DEFAULT_LINK_COST);
        }

        ndn::Name neighborName(name);
        if (!neighborName.empty()) {
	  // Save the neighbor link attributes
	  NetworkLink adjLink(nodeId, bandwidth, metric, delay, queue, name, faceUri, linkCost);
	  adjList.push_back(adjLink);
        }
        else {
          std::cerr << " Wrong command format ! [name /nbr/name/ \n face-uri /uri\n]";
          std::cerr << " or bad URI format" << std::endl;
        }
      }
      catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return false;
      }
    }
  }

  m_adj_map[m_srcNodeId] = adjList;
  return true;
}

void
NlsrConfReader::PrintConfig()
{
  try {
    NODE_MAP::iterator nodeIt;
    for (nodeIt = m_node_map.begin(); nodeIt != m_node_map.end(); ++nodeIt) {
      std::string nodeId = nodeIt->first;

      ADJACENCY_LIST adjLinks = m_adj_map[nodeId];
      ADJACENCY_LIST::iterator nodeIt;
      for (nodeIt = adjLinks.begin(); nodeIt != adjLinks.end(); nodeIt++) {
	NetworkLink link = (*nodeIt);
      }
    }
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
  }
}

void
NlsrConfReader::InitializeNlsr()
{
  // Copy over the NLSR config object from node to the application.
  for (NodeContainer::Iterator it = m_nodes.Begin(); it != m_nodes.End(); ++it) {
    NetworkNode node = m_node_map[Names::FindName (*it)];
    std::string nodeConfig = node.GetConfigPath();
    NS_ASSERT (!nodeConfig.empty());

    Ptr<ndn::NlsrApp> nlsrApp = (*it)->GetApplication(0)->GetObject<ndn::NlsrApp> ();
    NS_ASSERT (nlsrApp != 0);

    // Set the node name Id mapping in NLSR.
    NODE_MAP::iterator nodeIt;
    for (nodeIt = m_node_map.begin(); nodeIt != m_node_map.end(); ++nodeIt) {
      std::string nodeId = nodeIt->first;
      NetworkNode& srcNode = nodeIt->second;
      nlsrApp->SetNodeNameToIdMapping(srcNode.GetNodeId(), srcNode.GetNdnNodeId());
    }

    nlsrApp->SetNodeName(node.GetNodeId());
    nlsrApp->Initialize(nodeConfig);
    nlsrApp->SetStartTime(Seconds (1.0));
  }
}

Ptr<Node>
NlsrConfReader::CreateNode (const std::string name, uint32_t systemId)
{
  NS_LOG_FUNCTION (this << name);
  m_requiredPartitions = std::max (m_requiredPartitions, systemId + 1);

  Ptr<Node> node = CreateObject<Node> (systemId);

  Names::Add (m_path, name, node);
  m_nodes.Add (node);

  return node;
}

Ptr<Node>
NlsrConfReader::CreateNode (const std::string name, double posX, double posY, uint32_t systemId)
{
  NS_LOG_FUNCTION (this << name << posX << posY);
  m_requiredPartitions = std::max (m_requiredPartitions, systemId + 1);

  Ptr<Node> node = CreateObject<Node> (systemId);
  Ptr<MobilityModel> loc = DynamicCast<MobilityModel> (m_mobilityFactory.Create ());

  node->AggregateObject (loc);

  loc->SetPosition (Vector (posX, posY, 0));

  Names::Add (m_path, name, node);
  m_nodes.Add (node);

  return node;
}

NodeContainer
NlsrConfReader::Read(void)
{
  ProcessConfFile();
  PrintConfig();
  BuildTopology();
  SetLinkMTUSize();

  //SaveGraphviz("nlsr_graph.dot");
  //WriteGraphviz("src/ndnSIM/examples/ndn-nlsr-conf/nlsr_topo.dot");
  return m_nodes;
}

/*
NodeContainer
NlsrConfReader::Read(void)
{
  ifstream topgen;
  topgen.open (GetFileName ().c_str ());

  if ( !topgen.is_open () || !topgen.good () )
    {
      NS_FATAL_ERROR ("Cannot open file " << GetFileName () << " for reading");
      return m_nodes;
    }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);

      if (line == "router") break;
    }

  if (topgen.eof ())
    {
      NS_FATAL_ERROR ("Topology file " << GetFileName () << " does not have \"router\" section");
      return m_nodes;
    }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen,line);
      if (line[0] == '#') continue; // comments
      if (line=="link") break; // stop reading nodes

      istringstream lineBuffer (line);
      std::string name, city;
      double latitude = 0, longitude = 0;
      uint32_t systemId = 0;

      lineBuffer >> name >> city >> latitude >> longitude >> systemId;
      if (name.empty ()) 
        continue;

      Ptr<Node> node;


      if (abs(latitude) > 0.001 && abs(latitude) > 0.001) {
        node = CreateNode (name, m_scale * longitude, -m_scale * latitude, systemId);
      }
      else
        {
	  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
	  var->SetAttribute("Min", DoubleValue(0));
          var->SetAttribute("Max", DoubleValue(200.0));
          node = CreateNode (name, var->GetValue(), var->GetValue(), systemId);
          // node = CreateNode (name, systemId);
        }
    }

  map<string, set<string> > processedLinks; // to eliminate duplications

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName () << " does not have \"link\" section");
      return m_nodes;
    }

  // SeekToSection ("link");
  while (!topgen.eof ())
    {
      string line;
      getline (topgen,line);
      if (line == "") continue;
      if (line[0] == '#') continue; // comments

      NS_LOG_DEBUG ("Input: [" << line << "]");

      istringstream lineBuffer (line);
      std::string from, to, capacity, metric, delay, maxPackets, lossRate;

      lineBuffer >> from >> to >> capacity >> metric >> delay >> maxPackets >> lossRate;

      if (processedLinks[to].size () != 0 &&
          processedLinks[to].find (from) != processedLinks[to].end ()) {
          continue; // duplicated link
      }

      processedLinks[from].insert (to);

      Ptr<Node> fromNode = Names::Find<Node> (m_path, from);
      NS_ASSERT_MSG (fromNode != 0, from << " node not found");
      Ptr<Node> toNode   = Names::Find<Node> (m_path, to);
      NS_ASSERT_MSG (toNode != 0, to << " node not found");

      Link link (fromNode, from, toNode, to);

      link.SetAttribute ("DataRate", capacity);
      link.SetAttribute ("OSPF", metric);

      if (!delay.empty ())
          link.SetAttribute ("Delay", delay);
      if (!maxPackets.empty ())
        link.SetAttribute ("MaxPackets", maxPackets);

      // Saran Added lossRate
      if (!lossRate.empty ())
        link.SetAttribute ("LossRate", lossRate);

      AddLink (link);
      NS_LOG_DEBUG ("New link " << from << " <==> " << to << " / " << capacity << " with " << metric << " metric (" << delay << ", " << maxPackets << ", " << lossRate << ")");
    }

  NS_LOG_INFO ("Nlsr topology created with " << m_nodes.GetN () << " nodes and " << LinksSize () << " links");
  topgen.close ();

  ApplySettings ();

  return m_nodes;
}
*/

NodeContainer
NlsrConfReader::GetNodes () const
{
  return m_nodes;
}

const std::list<TopologyReader::Link>&
NlsrConfReader::GetLinks () const
{
  return m_linksList;
}

void
NlsrConfReader::SetBoundingBox (double ulx, double uly, double lrx, double lry)
{
  NS_LOG_FUNCTION (this << ulx << uly << lrx << lry);

  m_randX->SetAttribute("Min", DoubleValue(ulx));
  m_randX->SetAttribute("Max", DoubleValue(lrx));

  m_randY->SetAttribute("Min", DoubleValue(uly));
  m_randY->SetAttribute("Max", DoubleValue(lry));
}

void
NlsrConfReader::SetMobilityModel (const std::string &model)
{
  NS_LOG_FUNCTION (this << model);
  m_mobilityFactory.SetTypeId (model);
}

void
NlsrConfReader::AssignIpv4Addresses (Ipv4Address base)
{
  Ipv4AddressHelper address (base, Ipv4Mask ("/24"));

  BOOST_FOREACH (const Link &link, m_linksList)
    {
      address.Assign (NetDeviceContainer (link.GetFromNetDevice (),
                                          link.GetToNetDevice ()));

      base = Ipv4Address (base.Get () + 256);
      address.SetBase (base, Ipv4Mask ("/24"));
    }
}

void
NlsrConfReader::ApplyOspfMetric ()
{
  BOOST_FOREACH (const Link &link, m_linksList)
    {
      NS_LOG_DEBUG ("OSPF: " << link.GetAttribute ("OSPF"));
      uint16_t metric = boost::lexical_cast<uint16_t> (link.GetAttribute ("OSPF"));

      {
        Ptr<Ipv4> ipv4 = link.GetFromNode ()->GetObject<Ipv4> ();
        if (ipv4 != 0)
          {
            int32_t interfaceId = ipv4->GetInterfaceForDevice (link.GetFromNetDevice ());
            NS_ASSERT (interfaceId >= 0);

            ipv4->SetMetric (interfaceId,metric);
          }

        Ptr<ndn::L3Protocol> ndn = link.GetFromNode ()->GetObject<ndn::L3Protocol> ();
        if (ndn != 0)
          {
            shared_ptr<ndn::Face> face = ndn->getFaceByNetDevice (link.GetFromNetDevice ());
            NS_ASSERT (face != 0);

            face->setMetric (metric);
          }
      }

      {
        Ptr<Ipv4> ipv4 = link.GetToNode ()->GetObject<Ipv4> ();
        if (ipv4 != 0)
          {
            int32_t interfaceId = ipv4->GetInterfaceForDevice (link.GetToNetDevice ());
            NS_ASSERT (interfaceId >= 0);

            ipv4->SetMetric (interfaceId,metric);
          }

        Ptr<ndn::L3Protocol> ndn = link.GetToNode ()->GetObject<ndn::L3Protocol> ();
        if (ndn != 0)
          {
            shared_ptr<ndn::Face> face = ndn->getFaceByNetDevice (link.GetToNetDevice ());
            NS_ASSERT (face != 0);

            face->setMetric (metric);
          }
      }
    }
}

void
NlsrConfReader::ApplySettings ()
{
#ifdef NS3_MPI
  if (MpiInterface::IsEnabled () &&
      MpiInterface::GetSize () != m_requiredPartitions)
    {
      std::cerr << "MPI interface is enabled, but number of partitions (" << MpiInterface::GetSize ()
                << ") is not equal to number of partitions in the topology (" << m_requiredPartitions << ")";
      exit (-1);
    }
#endif

  PointToPointHelper p2p;

  BOOST_FOREACH (Link &link, m_linksList)
    {
      // cout << "Link: " << Findlink.GetFromNode () << ", " << link.GetToNode () << endl;
      string tmp;

      ////////////////////////////////////////////////
      if (link.GetAttributeFailSafe ("MaxPackets", tmp))
        {
          NS_LOG_INFO ("MaxPackets = " + link.GetAttribute ("MaxPackets"));

          try
            {
              uint32_t maxPackets = boost::lexical_cast<uint32_t> (link.GetAttribute ("MaxPackets"));

              // compatibility mode. Only DropTailQueue is supported
              p2p.SetQueue ("ns3::DropTailQueue",
                            "MaxPackets", UintegerValue (maxPackets));
            }
          catch (...)
            {
              typedef boost::tokenizer<boost::escaped_list_separator<char> > tokenizer;
              tokenizer tok (link.GetAttribute ("MaxPackets"));

              tokenizer::iterator token = tok.begin ();
              p2p.SetQueue (*token);

              for (token ++; token != tok.end (); token ++)
                {
                  boost::escaped_list_separator<char> separator ('\\', '=', '\"');
                  tokenizer attributeTok (*token, separator);

                  tokenizer::iterator attributeToken = attributeTok.begin ();

                  string attribute = *attributeToken;
                  attributeToken++;

                  if (attributeToken == attributeTok.end ())
                    {
                      NS_LOG_ERROR ("Queue attribute [" << *token << "] should be in form <Attribute>=<Value>");
                      continue;
                    }

                  string value = *attributeToken;

                  p2p.SetQueueAttribute (attribute, StringValue (value));
                }
            }
        }
      
      if (link.GetAttributeFailSafe ("DataRate", tmp))
        {
          NS_LOG_INFO ("DataRate = " + link.GetAttribute("DataRate"));
          p2p.SetDeviceAttribute ("DataRate", StringValue (link.GetAttribute ("DataRate")));
        }

      if (link.GetAttributeFailSafe ("Delay", tmp))
        {
          NS_LOG_INFO ("Delay = " + link.GetAttribute("Delay"));
          p2p.SetChannelAttribute ("Delay", StringValue (link.GetAttribute ("Delay")));
        }

      NetDeviceContainer nd = p2p.Install(link.GetFromNode (), link.GetToNode ());
      link.SetNetDevices (nd.Get (0), nd.Get (1));

      ////////////////////////////////////////////////
      if (link.GetAttributeFailSafe ("LossRate", tmp))
        {
          NS_LOG_INFO ("LinkError = " + link.GetAttribute("LossRate"));

          typedef boost::tokenizer<boost::escaped_list_separator<char> > tokenizer;
          tokenizer tok (link.GetAttribute("LossRate"));

          tokenizer::iterator token = tok.begin ();
          ObjectFactory factory (*token);

          for (token ++; token != tok.end (); token ++)
            {
              boost::escaped_list_separator<char> separator ('\\', '=', '\"');
              tokenizer attributeTok (*token, separator);

              tokenizer::iterator attributeToken = attributeTok.begin ();

              string attribute = *attributeToken;
              attributeToken++;

              if (attributeToken == attributeTok.end ())
                {
                  NS_LOG_ERROR ("ErrorModel attribute [" << *token << "] should be in form <Attribute>=<Value>");
                  continue;
                }

              string value = *attributeToken;

              factory.Set (attribute, StringValue (value));
            }

          nd.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (factory.Create<ErrorModel> ()));
          nd.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (factory.Create<ErrorModel> ()));
        }
    }
}

void
NlsrConfReader::SaveTopology (const std::string &file)
{
  ofstream os (file.c_str (), ios::trunc);
  os << "# any empty lines and lines starting with '#' symbol is ignored\n"
     << "\n"
     << "# The file should contain exactly two sections: router and link, each starting with the corresponding keyword\n"
     << "\n"
     << "# router section defines topology nodes and their relative positions (e.g., to use in visualizer)\n"
     << "router\n"
     << "\n"
     << "# each line in this section represents one router and should have the following data\n"
     << "# node  comment     yPos    xPos\n";

  for (NodeContainer::Iterator node = m_nodes.Begin ();
       node != m_nodes.End ();
       node++)
    {
      std::string name = Names::FindName (*node);
      Ptr<MobilityModel> mobility = (*node)->GetObject<MobilityModel> ();
      Vector position = mobility->GetPosition ();

      os << name << "\t" << "NA" << "\t" << -position.y << "\t" << position.x << "\n";
    }

  os << "# link section defines point-to-point links between nodes and characteristics of these links\n"
     << "\n"
     << "link\n"
     << "\n"
     << "# Each line should be in the following format (only first two are required, the rest can be omitted)\n"
     << "# srcNode   dstNode     bandwidth   metric  delay   queue\n"
     << "# bandwidth: link bandwidth\n"
     << "# metric: routing metric\n"
     << "# delay:  link delay\n"
     << "# queue:  MaxPackets for transmission queue on the link (both directions)\n"
     << "# error:  comma-separated list, specifying class for ErrorModel and necessary attributes\n";

  for (std::list<Link>::const_iterator link = m_linksList.begin ();
       link != m_linksList.end ();
       link ++)
    {
      os << Names::FindName (link->GetFromNode ()) << "\t";
      os << Names::FindName (link->GetToNode ()) << "\t";

      string tmp;
      if (link->GetAttributeFailSafe ("DataRate", tmp))
        os << link->GetAttribute("DataRate") << "\t";
      else
        NS_FATAL_ERROR ("DataRate must be specified for the link");

      if (link->GetAttributeFailSafe ("OSPF", tmp))
        os << link->GetAttribute("OSPF") << "\t";
      else
        os << "1\t";

      if (link->GetAttributeFailSafe ("Delay", tmp))
        {
          os << link->GetAttribute("Delay") << "\t";

          if (link->GetAttributeFailSafe ("MaxPackets", tmp))
            {
              os << link->GetAttribute("MaxPackets") << "\t";

              if (link->GetAttributeFailSafe ("LossRate", tmp))
                {
                  os << link->GetAttribute ("LossRate") << "\t";
                }
            }
        }
      os << "\n";
    }
}

template <class Names>
class name_writer {
public:
  name_writer(Names _names) : names(_names) {}

  template <class VertexOrEdge>
  void operator()(std::ostream& out, const VertexOrEdge& v) const {
    // out << "[label=\"" << names[v] << "\",style=filled,fillcolor=\"" << colors[v] << "\"]";
    out << "[shape=\"circle\",width=0.1,label=\"\",style=filled,fillcolor=\"green\"]";
  }
private:
  Names names;
};

template <class Names>
inline name_writer<Names>
make_name_writer(Names n) {
  return name_writer<Names>(n);
}

void
NlsrConfReader::SaveGraphviz (const std::string &file)
{
  typedef boost::adjacency_list_traits<boost::setS, boost::setS, boost::undirectedS> Traits;

  typedef boost::property< boost::vertex_name_t, std::string, boost::property
                           < boost::vertex_index_t, uint32_t > > nodeProperty;

  typedef boost::no_property edgeProperty;

  typedef boost::adjacency_list< boost::setS, boost::setS, boost::undirectedS,
                                 nodeProperty, edgeProperty > Graph;

  typedef map<string, Traits::vertex_descriptor> node_map_t;
  node_map_t graphNodes;
  Graph      graph;

  for (NodeContainer::Iterator node = m_nodes.Begin ();
       node != m_nodes.End ();
       node++)
    {
       std::pair<node_map_t::iterator, bool>
         retval = graphNodes.insert (make_pair (Names::FindName (*node),
                                                add_vertex (nodeProperty (Names::FindName (*node)), graph)));
       // NS_ASSERT (ok == true);

       put (boost::vertex_index, graph, retval.first->second, (*node)->GetId ());
    }

  for (std::list<Link>::const_iterator link = m_linksList.begin ();
       link != m_linksList.end ();
       link ++)
    {
      node_map_t::iterator from = graphNodes.find (Names::FindName (link->GetFromNode ()));
      node_map_t::iterator to   = graphNodes.find (Names::FindName (link->GetToNode ()));

      // add_edge (node->second, otherNode->second, m_graph);
      boost::add_edge (from->second, to->second, graph);
    }

  ofstream of (file.c_str ());
  boost::property_map<Graph, boost::vertex_name_t>::type names = get (boost::vertex_name, graph);
  write_graphviz (of, graph, make_name_writer (names));
}

void
NlsrConfReader::WriteGraphviz (const std::string &file)
{
  ofstream of (file.c_str ());

  of << "graph G {" << endl;
  of << "rankdir=LR;" << endl;
  for (NodeContainer::Iterator node = m_nodes.Begin ();
       node != m_nodes.End ();
       node++) {
    of << ns3::Names::FindName((*node)) << "[width=0.1, label=\"" << 
	  ns3::Names::FindName((*node)) << 
	  "\", style=filled, fillcolor=\"green\"]" << endl;
  }

  for (std::list<Link>::const_iterator link = m_linksList.begin ();
       link != m_linksList.end ();
       link ++) {
    of << ns3::Names::FindName((link->GetFromNode())) << "--"
       << ns3::Names::FindName((link->GetToNode()))  
       << "[label=\"" << link->GetAttribute("LinkCost") << "\"]" << endl;
  }
  of << "}" << endl;

  of.close();
}

NodeContainer
NlsrConfReader::BuildTopology()
{
  // Create nodes.
  try {
    NODE_MAP::iterator nodeIt;

    for (nodeIt = m_node_map.begin(); nodeIt != m_node_map.end(); ++nodeIt) {
      std::string nodeId = nodeIt->first;
      NetworkNode& srcNode = nodeIt->second;

      // Create a Node
      double longitude = srcNode.GetLongitude();
      double latitude = srcNode.GetLatitude();
      uint32_t systemId = 0;
      Ptr<Node> node;

      if (abs(longitude) > 0.001 && abs(latitude) > 0.001) {
        node = CreateNode (srcNode.GetNodeId(), m_scale * longitude, -m_scale * latitude, systemId);
      }
      else
      {
        Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
	var->SetAttribute("Min", DoubleValue(0));
        var->SetAttribute("Max", DoubleValue(200.0));
        node = CreateNode (srcNode.GetNodeId(), var->GetValue(), var->GetValue(), systemId);
      }

      // Map the NDN Node object to its name
      srcNode.SetNdnNodeId(node->GetId());
      NS_LOG_INFO ("Node name is: " + srcNode.GetNodeId());
      //Names::Add(srcNode.GetNodeId(), node);
    }
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
  }

  // Create links.
  map<string, set<string> > processedLinks; // to eliminate duplications
  try {
    NODE_MAP::iterator nodeIt;

    for (nodeIt = m_node_map.begin(); nodeIt != m_node_map.end(); ++nodeIt) {
      std::string nodeId = nodeIt->first;
      NetworkNode& srcNode = nodeIt->second;

      // Pass through each of the adjacent nodes
      ADJACENCY_LIST adjLinks = m_adj_map[nodeId];
      ADJACENCY_LIST::iterator nodeIt;
      for (nodeIt = adjLinks.begin(); nodeIt != adjLinks.end(); nodeIt++) {

	NetworkLink dstLink = *nodeIt;
        if (processedLinks[dstLink.GetNodeId()].size () != 0 &&
          processedLinks[dstLink.GetNodeId()].find (srcNode.GetNodeId()) != 
	  processedLinks[dstLink.GetNodeId()].end ()) {
          continue; // duplicated link
        }
        processedLinks[srcNode.GetNodeId()].insert (dstLink.GetNodeId());

        Ptr<Node> fromNode = Names::Find<Node> (m_path, srcNode.GetNodeId());
        NS_ASSERT_MSG (fromNode != 0, srcNode.GetNodeId() << " node not found");
        Ptr<Node> toNode   = Names::Find<Node> (m_path, dstLink.GetNodeId());
        NS_ASSERT_MSG (toNode != 0, dstLink.GetNodeId() << " node not found");

        Link link (fromNode, srcNode.GetNodeId(), toNode, dstLink.GetNodeId());
        link.SetAttribute ("LinkCost", dstLink.GetLinkCost());
        //link.SetAttribute ("DataRate", dstLink.GetBandwidth());
        //link.SetAttribute ("OSPF", dstLink.GetMetric());

        //if (!dstLink.GetDelay().empty())
        //  link.SetAttribute ("Delay", dstLink.GetDelay());
        //if (!dstLink.GetQueue().empty ())
        //  link.SetAttribute ("MaxPackets", dstLink.GetQueue());

        //if (!lossRate.empty())
        //  link.SetAttribute ("LossRate", lossRate);

	// Add the link.
        AddLink (link);
        NS_LOG_DEBUG ("New link " << srcNode.GetNodeId() << " <==> " << dstLink.GetNodeId() << ", Bandwidth: " << dstLink.GetBandwidth() << ", OSPF metric " << dstLink.GetMetric() << ", Delay: " << dstLink.GetDelay() << ", Queue size: " << dstLink.GetQueue());
      }
    }
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
  }

  NS_LOG_INFO ("Nlsr topology created with " << m_nodes.GetN () << " nodes and " << LinksSize () << " links");
  ApplySettings ();

  return m_nodes;
}

void
NlsrConfReader::SetLinkMTUSize()
{
  ns3::PointToPointNetDevice *netDevice = 0;
  int mtuSize = 8192;

  // MTU can be configured from env variable.
  char* str = getenv("MTU_SIZE");
  if (str != NULL) {
    try {
      mtuSize = std::stoi(str);
      NS_LOG_INFO ("MTU size set to: " << mtuSize);
    } catch (const std::invalid_argument& ia) {
      NS_LOG_ERROR ("Invalid MTU size configured, using default: 8192");
    }
  } else {
    NS_LOG_INFO ("No MTU specified, using default: 8192");
  }

  for (NodeContainer::Iterator it = m_nodes.Begin(); it != m_nodes.End(); ++it) {
    ns3::Ptr<ns3::Node> node = (*it);

    std::string nodeName = ns3::Names::FindName(node);
    uint32_t numDevices = node->GetNDevices();
    NS_LOG_INFO ("Node name: " << nodeName << " devices: " << numDevices);
    for (uint32_t deviceId = 0; deviceId < numDevices; deviceId++) {
      netDevice = dynamic_cast<ns3::PointToPointNetDevice*>(&(*(node->GetDevice(deviceId))));
      if (netDevice == NULL)
	continue;

      netDevice->SetMtu(mtuSize);
    }
  }
}

} // namespace ndn
} // namespace ns3
