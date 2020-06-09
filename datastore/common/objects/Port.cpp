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

#include <netmeld/datastore/objects/Port.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::datastore::objects {

  Port::Port()
  {}

  Port::Port(const IpAddress& _ipAddr) :
    ipAddr(_ipAddr)
  {}

  void
  Port::setProtocol(const std::string& _protocol)
  {
    protocol = nmcu::toLower(_protocol);
  }

  void
  Port::setPort(const int _port)
  {
    port = _port;
  }

  void
  Port::setState(const std::string& _state)
  {
    state = nmcu::toLower(_state);
  }

  void
  Port::setReason(const std::string& _reason)
  {
    reason = nmcu::toLower(_reason);
  }

  std::string
  Port::getIpAddr() const
  {
    return ipAddr.toString();
  }

  std::string
  Port::getProtocol() const
  {
    return protocol;
  }

  int
  Port::getPort() const
  {
    return port;
  }

  bool
  Port::isValid() const
  {
    bool portIsValid = (port == -1 || (port >= 0 && port <= 65535));
    return ipAddr.isValid()
        && !protocol.empty()
        && portIsValid
        ;
  }

  void
  Port::save(pqxx::transaction_base& t,
             const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "Port object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    ipAddr.save(t, toolRunId, deviceId);

    t.exec_prepared("insert_raw_port",
        toolRunId,
        ipAddr.toString(),
        protocol,
        port,
        state,
        reason);
  }

  std::string
  Port::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["; // opening bracket
    oss << port << ", "
        << protocol << ", "
        << ipAddr.toDebugString() << ", "
        << state << ", "
        << reason;
    oss << "]"; // closing bracket

    return oss.str();
  }

  bool
  operator==(const Port& first, const Port& second)
  {
    return first.port == second.port
        && first.protocol == second.protocol
        && first.ipAddr == second.ipAddr
        && first.state == second.state
        && first.reason == second.reason
        ;
  }
}
