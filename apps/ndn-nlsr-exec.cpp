/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <boost/filesystem.hpp>

#include "ndn-nlsr-exec.hpp"

#include "conf-file-processor.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"

#ifdef NS3_NLSR_SIM
#include "nlsr-logger.hpp"
#else
#include "logger.hpp"
#endif

namespace ns3 {

namespace ndn {

INIT_LOGGER("NlsrExec");

NlsrExec::NlsrExec(::ndn::KeyChain& keyChain, std::string& nlsrConf)
  : m_ioService(m_face.getIoService())
  , m_scheduler(m_ioService)
  , m_keyChain(keyChain)
  , m_nlsr(m_ioService, m_scheduler, m_face, m_keyChain)
{
  m_nlsr.setConfFileName(nlsrConf);
  nlsr::ConfFileProcessor configProcessor(m_nlsr, m_nlsr.getConfFileName());
  if (!configProcessor.processConfFile()) {
    throw Error("Error in configuration file processing! Exiting from NLSR");
  }
}

void
NlsrExec::run()
{
  m_nlsr.initialize();

  try {
    m_nlsr.startEventLoop();
  }
  catch (std::exception& e) {
    _LOG_FATAL("ERROR: " << e.what());
    std::cerr << "ERROR: " << e.what() << std::endl;

    m_nlsr.getFib().clean();
    m_nlsr.destroyFaces();
  }
}

} // namespace ndn
} // namespace ns3
