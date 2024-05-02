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

#include <netmeld/datastore/objects/AclRuleService.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include <netmeld/datastore/objects/AcRule.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AclRuleService::AclRuleService()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  AclRuleService::setServiceId(const std::string& _serviceId)
  {
    serviceId = _serviceId;
  }

  void
  AclRuleService::save( pqxx::transaction_base& t
                      , const nmco::Uuid& toolRunId
                      , const std::string& deviceId
                      )
  {
    //AclRule::save(t, toolRunId, deviceId);
    t.exec_prepared( "insert_raw_device_acl_rule_service"
                   , toolRunId
                   , deviceId
                   , priority
                   , action
                   , incomingZoneId
                   , outgoingZoneId
                   , srcIpNetSetNamespace
                   , srcIpNetSetId
                   , dstIpNetSetNamespace
                   , dstIpNetSetId
                   , serviceId
                   , description
                   );

    if (addAcObjects) { // START -- Temporary logic for ACL to AC duplication
      LOG_DEBUG << "AclRule object creating AC object(s) to save\n"
                << "AclRuleService to save: " << toDebugString()
                << std::endl;

      { // -- save AcRule
        auto zoneLambda = [&](const std::string& zoneId)
          {
            LOG_DEBUG << "Getting interfaces (zone_id): "
                      << zoneId
                      << std::endl;
            pqxx::result ifaceRows {
                t.exec_prepared("select_raw_device_acl_zone_interfaces"
                                , toolRunId
                                , deviceId
                                , zoneId
                                )
              };
            return ifaceRows;
          };

        AcRule rule;
        rule.addAclObjects = false; // don't create ACL objects (infinite loop)

        rule.setRuleId(priority);
        rule.setRuleDescription(description);

        rule.setSrcId(incomingZoneId);
        rule.addSrc(srcIpNetSetId);
        for (const auto& iface : zoneLambda(incomingZoneId)) {
          rule.addSrcIface(iface[0].c_str());
        }

        rule.setDstId(outgoingZoneId);
        rule.addDst(dstIpNetSetId);
        for (const auto& iface : zoneLambda(outgoingZoneId)) {
          rule.addDstIface(iface[0].c_str());
        }

        rule.addAction(action);
        rule.addService(serviceId);
        rule.enable(); // all AclRule objects are assumed enabled

        LOG_DEBUG << "AcRule to save: " << rule.toDebugString() << '\n';
        rule.save(t, toolRunId, deviceId);
      }
    } // END
  }

  std::string
  AclRuleService::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "aclRule: " << AclRule::toDebugString()
        << ", serviceId: " << serviceId
        << "]";

    return oss.str();
  }

  std::strong_ordering
  AclRuleService::operator<=>(const AclRuleService& rhs) const
  {
    if (auto cmp = AclRule::operator<=>(rhs); 0 != cmp) {
      return cmp;
    }

    return std::tie( serviceId
                   )
       <=> std::tie( rhs.serviceId
                   )
      ;
  }

  bool
  AclRuleService::operator==(const AclRuleService& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
