// =============================================================================
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/AclRule.hpp>
#include <netmeld/datastore/objects/AclIpNetSet.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;
namespace nmdp = netmeld::datastore::parsers;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AclRule::AclRule() :
    priority{0}
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  AclRule::isValid() const
  {
    return !action.empty();
  }

  void
  AclRule::setPriority(size_t _priority)
  {
    priority = _priority;
  }

  void
  AclRule::setAction(const std::string& _action)
  {
    action = nmcu::toLower(_action);
  }

  void
  AclRule::setIncomingZoneId(const std::string& _incomingZoneId)
  {
    incomingZoneId = _incomingZoneId;
  }

  void
  AclRule::setOutgoingZoneId(const std::string& _outgoingZoneId)
  {
    outgoingZoneId = _outgoingZoneId;
  }

  void
  AclRule::setSrcIpNetSetId(const std::string& _srcIpNetSetId, const std::string& _srcIpNetSetNamespace)
  {
    srcIpNetSetNamespace = _srcIpNetSetNamespace;
    srcIpNetSetId = _srcIpNetSetId;
  }

  void
  AclRule::setDstIpNetSetId(const std::string& _dstIpNetSetId, const std::string& _dstIpNetSetNamespace)
  {
    dstIpNetSetNamespace = _dstIpNetSetNamespace;
    dstIpNetSetId = _dstIpNetSetId;
  }

  void
  AclRule::setDescription(const std::string& _description)
  {
    description = _description;
  }

  void
  AclRule::save(pqxx::transaction_base&, const nmco::Uuid&, const std::string&)
  {

  }

  std::string
  AclRule::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "action: " << action << ", "
        << "incomingZoneId: " << incomingZoneId << ", "
        << "outgoingZoneId: " << outgoingZoneId << ", "
        << "srcIpNetSetNamespace: " << srcIpNetSetNamespace << ", "
        << "srcIpNetSetId: " << srcIpNetSetId << ", "
        << "dstIpNetSetNamespace: " << dstIpNetSetNamespace << ", "
        << "dstIpNetSetId: " << dstIpNetSetId << ", "
        << "description: " << description
        << "]";

    return oss.str();
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
