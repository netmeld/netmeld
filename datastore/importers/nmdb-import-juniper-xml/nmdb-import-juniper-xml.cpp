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

#include <netmeld/datastore/tools/AbstractImportXmlTool.hpp>

#include "Parser.hpp"


namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;
namespace nmdu = netmeld::datastore::utils;


typedef std::vector<Data> Results;


template<typename P, typename R>
class Tool : public nmdt::AbstractImportXmlTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportXmlTool<P,R>
      ("Juniper XML output (.xml files)", PROGRAM_NAME, PROGRAM_VERSION)
    {
      this->devInfo.setVendor("Juniper");
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId{this->getToolRunId()};

      LOG_DEBUG << "Iterating over results:" << std::endl;
      for (auto& result : this->tResults) {
        nmdo::DeviceInformation deviceInfo{this->devInfo};
        const std::string baseDeviceId{deviceInfo.getDeviceId()};
        result.observations.save(t, toolRunId, baseDeviceId);

        LOG_DEBUG << "Iterating over logical-systems:" << std::endl;
        for (auto& [logicalSystemName, logicalSystem] : result.logicalSystems) {
          LOG_DEBUG << "logical-system: " << logicalSystemName << std::endl;
          std::string deviceId{baseDeviceId};
          if (!logicalSystemName.empty()) {
            deviceId += (":" + logicalSystemName);
          }
          deviceInfo.setDeviceId(deviceId);
          LOG_DEBUG << deviceInfo.toDebugString() << std::endl;
          deviceInfo.save(t, toolRunId);

          LOG_DEBUG << "Iterating over interfaces:" << std::endl;
          for (auto& [_, iface] : logicalSystem.ifaces) {
            LOG_DEBUG << iface.toDebugString() << std::endl;
            iface.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over interface hierarchies" << std::endl;
          for (auto& [underlyingIfaceName, virtualIfaceName] : logicalSystem.ifaceHierarchies) {
            t.exec_prepared("insert_raw_device_interface_hierarchy",
                toolRunId,
                deviceId,
                underlyingIfaceName,
                virtualIfaceName);
          }

          LOG_DEBUG << "Iterating over VRFs:" << std::endl;
          for (auto& [_, vrf] : logicalSystem.vrfs) {
            LOG_DEBUG << vrf.toDebugString() << std::endl;
            vrf.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over Services:" << std::endl;
          for (auto& service : logicalSystem.services) {
            LOG_DEBUG << service.toDebugString() << std::endl;
            service.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over DNS resolvers:" << std::endl;
          for (auto& dnsResolver : logicalSystem.dnsResolvers) {
            LOG_DEBUG << dnsResolver.toDebugString() << std::endl;
            dnsResolver.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over DNS search domains:" << std::endl;
          for (auto& dnsSearchDomain : logicalSystem.dnsSearchDomains) {
            LOG_DEBUG << dnsSearchDomain << std::endl;
            t.exec_prepared("insert_raw_device_dns_search_domain",
                toolRunId,
                deviceId,
                dnsSearchDomain);
          }

          LOG_DEBUG << "Iterating over ACL zones:" << std::endl;
          for (auto& [_, aclZone] : logicalSystem.aclZones) {
            LOG_DEBUG << aclZone.toDebugString() << std::endl;
            aclZone.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over ACL ipNets:" << std::endl;
          for (auto& [_, aclIpNetSetsValue] : logicalSystem.aclIpNetSets) {
            for (auto& [_, aclIpNetSet] : aclIpNetSetsValue) {
              LOG_DEBUG << aclIpNetSet.toDebugString() << std::endl;
              aclIpNetSet.save(t, toolRunId, deviceId);
            }
          }

          LOG_DEBUG << "Iterating over ACL services:" << std::endl;
          for (auto& aclService : logicalSystem.aclServices) {
            LOG_DEBUG << aclService.toDebugString() << std::endl;
            aclService.save(t, toolRunId, deviceId);
          }

          LOG_DEBUG << "Iterating over ACL rules:" << std::endl;
          for (auto& aclRule : logicalSystem.aclRules) {
            LOG_DEBUG << aclRule.toDebugString() << std::endl;
            aclRule.save(t, toolRunId, deviceId);
          }
        }
        // Insert virtualization relationship after all devices have been inserted.
        for (auto& [logicalSystemName, logicalSystem] : result.logicalSystems) {
          if (!logicalSystemName.empty()) {
            t.exec_prepared("insert_raw_device_virtualization",
                toolRunId,
                baseDeviceId,
                baseDeviceId + ":" + logicalSystemName);
          }
        }
      }
    }

};


int
main(int argc, char** argv)
{
  Tool<Parser, Results> tool;
  return tool.start(argc, argv);
}

