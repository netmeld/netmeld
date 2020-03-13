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

#include <netmeld/core/objects/Interface.hpp>
#include <netmeld/core/tools/AbstractInsertTool.hpp>

namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;
namespace nmco = netmeld::core::objects;


class Tool : public nmct::AbstractInsertTool
{
  public:
    Tool() : nmct::AbstractInsertTool
      ("device", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    modifyToolOptions() override
    {
      opts.addRequiredDeviceId();

      opts.addOptionalOption("vm-host-device-id", std::make_tuple(
            "vm-host-device-id",
            po::value<std::string>(),
            "Name of VM host device.")
          );
      opts.addOptionalOption("interface", std::make_tuple(
            "interface",
            po::value<std::string>(),
            "Name of network interface.")
          );
      opts.addOptionalOption("mediaType", std::make_tuple(
            "mediaType",
            po::value<std::string>()->default_value("ethernet"),
            "Interface media type.")
          );
      opts.addOptionalOption("mac-addr", std::make_tuple(
            "mac-addr",
            po::value<std::string>(),
            "MAC address of network interface.")
          );
      opts.addOptionalOption("ip-addr", std::make_tuple(
            "ip-addr",
            po::value<std::string>(),
            "IP address of network interface.")
          );
      opts.addOptionalOption("responding", std::make_tuple(
            "responding",
            po::value<bool>()->default_value(true),
            "Flag device as responding or not")
          );
      opts.addOptionalOption("low-graph-priority", std::make_tuple(
            "low-graph-priority",
            NULL_SEMANTIC,
            "Device should be out-of-the-way (e.g. lower) in network graphs.")
          );
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {getToolRunId()};
      const auto& deviceId  {getDeviceId()};

      if (opts.exists("vm-host-device-id")) {
        nmco::DeviceInformation hostDevInfo;
        hostDevInfo.setDeviceId(opts.getValue("vm-host-device-id"));
        hostDevInfo.save(t, toolRunId);

        const auto& hostDevId {hostDevInfo.getDeviceId()};
        t.exec_prepared("insert_raw_device_virtualization",
            toolRunId,
            hostDevId,
            deviceId);
      }

      if (opts.exists("low-graph-priority")) {
        t.exec_prepared("insert_device_extra_weight",
            deviceId,
            M_PI); // Not arbitrary, but probably high enough
      }

      nmco::IpAddress ipAddr;
      if (opts.exists("ip-addr")) {
        ipAddr = nmco::IpAddress(opts.getValue("ip-addr"));
        ipAddr.setResponding(opts.exists("responding"));

        ipAddr.save(t, toolRunId, deviceId);
        LOG_DEBUG << ipAddr.toDebugString() << std::endl;
      }

      nmco::MacAddress macAddr;
      if (opts.exists("mac-addr")) {
        macAddr = nmco::MacAddress(opts.getValue("mac-addr"));
        macAddr.addIp(ipAddr);
        macAddr.setResponding(opts.exists("responding"));

        macAddr.save(t, toolRunId, deviceId);
        LOG_DEBUG << macAddr.toDebugString() << std::endl;
      }

      if (opts.exists("interface")) {
        nmco::Interface iface(opts.getValue("interface"));
        iface.setMediaType("ethernet");
        iface.setMacAddress(macAddr);
        iface.addIpAddress(ipAddr);

        if (opts.exists("responding")) {
          iface.setUp();
        } else {
          iface.setDown();
        }

        iface.save(t, toolRunId, deviceId);
        LOG_DEBUG << iface.toDebugString() << std::endl;
      }
    }
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
