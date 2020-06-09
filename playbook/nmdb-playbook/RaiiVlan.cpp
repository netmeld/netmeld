// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#include "RaiiVlan.hpp"


namespace netmeld::playbook {

  RaiiVlan::RaiiVlan(std::string const& _ifaceName, uint16_t const _vlan) :
    ifaceName(_ifaceName),
    vlan(_vlan)
  {
    if (VlanId::NONE == vlan) { return; }

    if (VlanId::RESERVED <= vlan) {
      LOG_ERROR << "Invalid VLAN ID (VID)" << std::endl;
      // TODO 20NOV18 This originally did below which would abort entire scan
      //   throw std::invalid_argument("Invalid VLAN ID (VID).");
      //              We may want to alter this logic to just aborting stage
      std::exit(nmcu::Exit::FAILURE);
    }

    std::vector<std::string> commands;
    std::ostringstream oss;

    oss.str(std::string());
    oss << ifaceName << "." << vlan;
    vlanIfaceName = oss.str();

    oss.str(std::string());
    oss << "ip link add link " << ifaceName
        << " name " << vlanIfaceName
        << " type vlan id " << vlan;
    commands.emplace_back(oss.str());

    {
      std::lock_guard<std::mutex> coutLock(nmpb::coutMutex);
      for (const auto& command : commands) {
        cmdRunner.systemExec(command);
      }
      cmdRunner.scheduleSleep(250);
    }
  }


  RaiiVlan::~RaiiVlan()
  {
    if (vlan > VlanId::RESERVED) {
      return; // Do nothing as the constructor did nothing
    }

    std::vector<std::string> commands;
    std::ostringstream oss;

    oss.str(std::string());
    oss << "ip link del " << vlanIfaceName;
    commands.emplace_back(oss.str());

    {
      std::lock_guard<std::mutex> coutLock(nmpb::coutMutex);
      for (const auto& command : commands) {
        cmdRunner.systemExec(command);
      }
      cmdRunner.scheduleSleep(250);
    }
  }

  std::string
  RaiiVlan::getLinkName() const
  {
    std::ostringstream oss {ifaceName};
    if (VlanId::RESERVED > vlan) {
      oss.str(vlanIfaceName);
    }
    return oss.str();
  }
}
