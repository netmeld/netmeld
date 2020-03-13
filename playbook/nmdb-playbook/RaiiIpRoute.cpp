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

#include "RaiiIpRoute.hpp"

namespace netmeld { namespace playbook {

  RaiiIpRoute::RaiiIpRoute(std::string const& _ifaceName,
                           std::string const& _ipAddr) :
    ifaceName(_ifaceName),
    ipAddr(_ipAddr)
  {
    std::vector<std::string> commands;
    std::ostringstream oss;

    // This first command is normally not required (but doesn't hurt);
    // however, it is required when dealing with various point-to-point links.
    oss.str(std::string());
    oss << "ip route add " << ipAddr << " dev " << ifaceName;
    commands.emplace_back(oss.str());

    oss.str(std::string());
    oss << "ip route add default via " << ipAddr;
    commands.emplace_back(oss.str());

    oss.str(std::string());
    oss << "ip route flush cache";
    commands.emplace_back(oss.str());

    {
      std::lock_guard<std::mutex> coutLock(nmpb::coutMutex);
      for (const auto& command : commands) {
        cmdRunner.systemExec(command);
      }
      cmdRunner.scheduleSleep(500);
    }
  }


  RaiiIpRoute::~RaiiIpRoute()
  {
    std::vector<std::string> commands;
    std::ostringstream oss;

    oss.str(std::string());
    oss << "ip route del default via " << ipAddr;
    commands.emplace_back(oss.str());

    oss.str(std::string());
    oss << "ip route del " << ipAddr << " dev " << ifaceName;
    commands.emplace_back(oss.str());

    oss.str(std::string());
    oss << "ip route flush cache";
    commands.emplace_back(oss.str());

    {
      std::lock_guard<std::mutex> coutLock(nmpb::coutMutex);
      for (const auto& command : commands) {
        cmdRunner.systemExec(command);
      }
      cmdRunner.scheduleSleep(500);
    }
  }
}}
