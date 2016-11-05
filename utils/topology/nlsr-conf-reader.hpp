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

// ndn-nlsr-conf-helper.hpp

#ifndef NLSR_CONF_READER_HPP
#define NLSR_CONF_READER_HPP

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/cstdint.hpp>
#include <string>
#include <iostream>
#include <map>
#include <stdint.h>
#include <sstream>

#include "ns3/topology-reader.h"
#include "ns3/node-container.h"
#include "ns3/random-variable-stream.h"
#include "ns3/object-factory.h"
#include "ns3/ptr.h"
#include "ns3/log.h"

#include "adjacency-list.hpp"

namespace ns3 {
namespace ndn {

class NlsrConfReader : public TopologyReader
{
public:

  // Network node. 
  class NetworkNode 
  {
  public:
    NetworkNode()
    {
    }

    NetworkNode(std::string& path, std::string& nodeId, std::string& city,
	  double& latitude, double& longitude) 
      : m_configPath(path)
      , m_nodeId(nodeId)
      , m_city(city)
      , m_latitude(latitude)
      , m_longitude(longitude)
    {
    }
  
    ~NetworkNode() 
    {
    }

    std::string
    GetConfigPath () const
    {
      return m_configPath;
    }
  
    std::string
    GetNodeId () const
    {
      return m_nodeId;
    }
  
    std::string
    GetCity () const
    {
      return m_city;
    }

    double
    GetLatitude () const
    {
      return m_latitude;
    }

    double
    GetLongitude () const
    {
      return m_longitude;
    }
    
    void
    SetNdnNodeId (uint32_t id)
    {
      m_ndnNodeId = id;
    }
    
    uint32_t
    GetNdnNodeId () const
    {
      return m_ndnNodeId;
    }

    void
    PrintNode()
    {
      std::cout << "Node Id: " << m_nodeId << ", " << "City: " << m_city << ", " 
	        << "Latitude: " << m_latitude << ", " << "Longitude: " << m_longitude << ", "
		<< "Config path: " << m_configPath << "NDN Node Id: " << m_ndnNodeId << std::endl;
    }
    
  private:
    std::string m_configPath;
    std::string m_nodeId;
    std::string m_city;
    double m_latitude;
    double m_longitude;
    uint32_t m_ndnNodeId;
  };

  // Network link
  class NetworkLink 
  {
  public:
    NetworkLink()
    {
    }

    NetworkLink(std::string& nodeId, std::string& bandwidth,
	  std::string& metric, std::string& delay, std::string& queue,
	  std::string& name, std::string& faceUri, std::string& linkCost) 
      : m_nodeId(nodeId)
      , m_bandwidth(bandwidth)
      , m_metric(metric)
      , m_delay(delay)
      , m_queue(queue)
      , m_name(name)
      , m_faceUri(faceUri)
      , m_linkCost(linkCost)
    {
    }
  
    ~NetworkLink() 
    {
    }

    std::string
    GetNodeId () const
    {
      return m_nodeId;
    }
  
    std::string
    GetBandwidth () const
    {
      return m_bandwidth;
    }

    std::string
    GetMetric () const
    {
      return m_metric;
    }

    std::string
    GetDelay () const
    {
      return m_delay;
    }
    
    std::string
    GetQueue () const
    {
      return m_queue;
    }
  
    std::string
    GetName () const
    {
      return m_name;
    }
  
    std::string
    GetFaceUri () const
    {
      return m_faceUri;
    }
  
    std::string
    GetLinkCost () const
    {
      return m_linkCost;
    }
  
    void
    PrintLink()
    {
      std::cout << "NodeId: " << m_nodeId << ", " << "Bandwidth: " << m_bandwidth
                << "Metric: " << m_metric << ", " << "Delay: " << m_delay
                << "Queue: " << m_queue << ", " << "Name: " << m_name << ", "  
                << "FaceUri: " << m_faceUri << ", " << "Link cost: " << m_linkCost << std::endl;
    }
    
  private:
    std::string m_nodeId;
    std::string m_bandwidth;
    std::string m_metric;
    std::string m_delay;
    std::string m_queue;
    std::string m_name;
    std::string m_faceUri;
    std::string m_linkCost;
  };

  NlsrConfReader(const std::string &path="", double scale=1.0);

  virtual ~NlsrConfReader();

  bool
  ProcessConfFile();

  void
  PrintConfig();

  void
  InitializeNlsr();

  virtual NodeContainer
  Read ();

  virtual NodeContainer
  GetNodes () const;
    
  virtual const std::list<Link>&
  GetLinks () const;
  
  virtual void
  AssignIpv4Addresses (Ipv4Address base);

  virtual void
  SetBoundingBox (double ulx, double uly, double lrx, double lry);

  virtual void
  SetMobilityModel (const std::string &model);

  virtual void
  ApplyOspfMetric ();

  virtual void
  SaveTopology (const std::string &file);

  virtual void
  SaveGraphviz (const std::string &file);
  
  virtual void
  WriteGraphviz (const std::string &file);
  
protected:
  Ptr<Node>
  CreateNode (const std::string name, uint32_t systemId);

  Ptr<Node>
  CreateNode (const std::string name, double posX, double posY, uint32_t systemId);
  
  void ApplySettings ();

  NodeContainer BuildTopology ();

  void SetLinkMTUSize();

protected:
  std::string m_path;
  NodeContainer m_nodes;

private:
  typedef boost::property_tree::ptree ConfigSection;
  typedef std::map<std::string, NetworkNode> NODE_MAP;
  typedef std::vector<NetworkLink> ADJACENCY_LIST; 
  typedef std::map<std::string, ADJACENCY_LIST> ADJACENCY_MAP;

  bool
  Load(std::istream& input);

  bool
  ProcessSection(const std::string& sectionName, const ConfigSection& section);

  bool
  ProcessNlsrConfig(const ConfigSection& section);

  bool
  ProcessNodeId(const ConfigSection& section);

  bool
  ProcessNeighborNodes(const ConfigSection& section);

private:
  std::string m_confFileName;
  std::string m_srcNodeId;
  NODE_MAP m_node_map;
  ADJACENCY_MAP m_adj_map;

  NlsrConfReader (const NlsrConfReader&);
  NlsrConfReader& operator= (const NlsrConfReader&);

  Ptr<UniformRandomVariable> m_randX;
  Ptr<UniformRandomVariable> m_randY;

  ObjectFactory m_mobilityFactory;
  double m_scale;
  uint32_t m_requiredPartitions;
};

} // namespace ndn
} // namespace ns3

#endif // NLSR_CONF_READER_HPP
