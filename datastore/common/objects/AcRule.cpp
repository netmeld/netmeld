// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/core/utils/ContainerUtilities.hpp>
#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AcRule::AcRule()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  AcRule::setRuleId(const size_t _value)
  {
    id = _value;
  }

  void
  AcRule::setRuleDescription(const std::string& _value)
  {
    description = _value;
  }

  void
  AcRule::setSrcId(const std::string& _value)
  {
    srcId = _value;
  }

  void
  AcRule::addSrc(const std::string& _value)
  {
    nmcu::addIfUnique(&srcs, _value);
  }

  void
  AcRule::addSrcIface(const std::string& _value)
  {
    nmcu::addIfUnique(&srcIfaces, _value);
  }

  void
  AcRule::setDstId(const std::string& _value)
  {
    dstId = _value;
  }

  void
  AcRule::addDst(const std::string& _value)
  {
    nmcu::addIfUnique(&dsts, _value);
  }

  void
  AcRule::addDstIface(const std::string& _value)
  {
    nmcu::addIfUnique(&dstIfaces, _value);
  }

  void
  AcRule::addAction(const std::string& _value)
  {
    nmcu::addIfUnique(&actions, _value);
  }

  void
  AcRule::addService(const std::string& _value)
  {
    nmcu::addIfUnique(&services, _value);
  }

  void
  AcRule::enable()
  {
    enabled = true;
  }

  void
  AcRule::disable()
  {
    enabled = false;
  }

  const std::string&
  AcRule::getSrcId() const
  {
    return srcId;
  }

  const std::vector<std::string>&
  AcRule::getSrcs() const
  {
    return srcs;
  }

  const std::vector<std::string>&
  AcRule::getSrcIfaces() const
  {
    return srcIfaces;
  }

  const std::string&
  AcRule::getDstId() const
  {
    return dstId;
  }

  const std::vector<std::string>&
  AcRule::getDsts() const
  {
    return dsts;
  }

  const std::vector<std::string>&
  AcRule::getDstIfaces() const
  {
    return dstIfaces;
  }

  const std::vector<std::string>&
  AcRule::getServices() const
  {
    return services;
  }

  const std::vector<std::string>&
  AcRule::getActions() const
  {
    return actions;
  }

  bool
  AcRule::isValid() const
  {
    return !srcId.empty()
        && !srcs.empty()
//        && !srcIfaces.empty()
//        && !dstId.empty()
//        && !dsts.empty()
//        && !dstIfaces.empty()
//        && !services.empty()
        && !actions.empty()
        ;
  }

  void
  AcRule::save(pqxx::transaction_base& t,
               const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AcRule object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    std::string actionStr;
    if (actions.size() > 0) {
      actionStr = nmcu::toString(actions, ',');
    }

    if (srcIfaces.empty() || dstIfaces.empty()) {
      ToolObservations to;
      to.addNotable(
          "AcRule (" + description +
          ") defined but may not be applied to an interface.");
      to.saveQuiet(t, toolRunId, deviceId);
    }

    if (srcIfaces.empty()) { addSrcIface(""); }
    if (dstIfaces.empty()) { addDstIface(""); }
    if (dsts.empty())      { addDst("");      }
    if (services.empty())  { addService("");  }

    for (const auto& src : srcs) {
      for (const auto& srcIface : srcIfaces) {
        for (const auto& dst : dsts) {
          for (const auto& dstIface : dstIfaces) {
            for (const auto& service : services) {
              t.exec_prepared("insert_raw_device_ac_rule",
                toolRunId,
                deviceId,
                enabled,
                id,
                srcId,
                src,
                srcIface,
                dstId,
                dst,
                dstIface,
                service,
                actionStr,
                description
                );
            }
          }
        }
      }
    }
  }

  // Utilized for full object data dump, for debug purposes
  std::string
  AcRule::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[" // opening bracket
        << "enabled: " << std::boolalpha << enabled << ", "
        << "id: " << id << ", "
        << "srcId: " << srcId << ", "
        << "srcs: " << srcs << ", "
        << "srcIfaces: " << srcIfaces << ", "
        << "dstId: " << dstId << ", "
        << "dsts: " << dsts << ", "
        << "dstIfaces: " << dstIfaces << ", "
        << "services: " << services << ", "
        << "actions: " << actions << ", "
        << "description: " << description
        << "]"; // closing bracket

    return oss.str();
  }

  std::strong_ordering
  AcRule::operator<=>(const AcRule& rhs) const
  {
    return std::tie( id
                   , srcId
                   , srcs
                   , srcIfaces
                   , dstId
                   , dsts
                   , dstIfaces
                   , services
                   , actions
                   , description
                   , enabled
                   )
       <=> std::tie( rhs.id
                   , rhs.srcId
                   , rhs.srcs
                   , rhs.srcIfaces
                   , rhs.dstId
                   , rhs.dsts
                   , rhs.dstIfaces
                   , rhs.services
                   , rhs.actions
                   , rhs.description
                   , rhs.enabled
                   )
      ;
  }

  bool
  AcRule::operator==(const AcRule& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
