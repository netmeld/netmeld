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

#include <netmeld/datastore/objects/Interface.hpp>
#include <netmeld/datastore/tools/AbstractInsertTool.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


class Tool : public nmdt::AbstractInsertTool
{
  public:
    Tool() : nmdt::AbstractInsertTool
      ("device", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      addRequiredDeviceId();

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
      opts.addOptionalOption("hostname", std::make_tuple(
          "hostname",
          po::value<std::string>(),
          "Hostname or FQDN associated with IP address")
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
      auto isResponding     {opts.getValueAs<bool>("responding")};

      if (opts.exists("vm-host-device-id")) {
        nmdo::DeviceInformation hostDevInfo;
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

      nmdo::IpAddress ipAddr;
      if (opts.exists("ip-addr")) {
        ipAddr = nmdo::IpAddress(opts.getValue("ip-addr"));
        ipAddr.setResponding(isResponding);
        if (opts.exists("hostname")) {
          ipAddr.addAlias(opts.getValue("hostname"), "insert-device");
        }

        ipAddr.save(t, toolRunId, deviceId);
        LOG_DEBUG << ipAddr.toDebugString() << std::endl;
      }

      nmdo::MacAddress macAddr;
      if (opts.exists("mac-addr")) {
        macAddr = nmdo::MacAddress(opts.getValue("mac-addr"));
        macAddr.addIpAddress(ipAddr);
        macAddr.setResponding(isResponding);

        macAddr.save(t, toolRunId, deviceId);
        LOG_DEBUG << macAddr.toDebugString() << std::endl;
      }

      if (opts.exists("interface")) {
        nmdo::Interface iface(opts.getValue("interface"));
        iface.setMediaType("ethernet");
        iface.setMacAddress(macAddr);
        iface.addIpAddress(ipAddr);

        if (isResponding) {
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
