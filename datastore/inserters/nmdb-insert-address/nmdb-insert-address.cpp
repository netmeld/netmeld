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

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/tools/AbstractInsertTool.hpp>

namespace nmdt = netmeld::datastore::tools;
namespace nmdo = netmeld::datastore::objects;


class Tool : public nmdt::AbstractInsertTool
{
  public:
    Tool() : nmdt::AbstractInsertTool
      ("ip and/or mac address", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      //opts.addRequiredDeviceId();
      opts.addOptionalOption("device-id", std::make_tuple(
          "device-id",
          po::value<std::string>(),
          "Name of device.")
        );
      opts.addOptionalOption("device-color", std::make_tuple(
          "device-color",
          po::value<std::string>(),
          "Graph color of device.")
        );
      opts.addOptionalOption("device-type", std::make_tuple(
          "device-type",
          po::value<std::string>(),
          "Type of device, determines graph icon.")
        );

      opts.addOptionalOption("mac-addr", std::make_tuple(
          "mac-addr",
          po::value<std::string>(),
          "MAC address")
        );
      opts.addOptionalOption("ip-addr", std::make_tuple(
          "ip-addr",
          po::value<std::string>(),
          "IP address")
        );
      opts.addOptionalOption("hostname", std::make_tuple(
          "hostname",
          po::value<std::string>(),
          "Hostname or FQDN associated with IP address")
        );

      opts.addOptionalOption("responding", std::make_tuple(
          "responding",
          po::value<bool>()->default_value(true),
          "Set address to responding or not")
        );
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {getToolRunId()};
      const auto& deviceId  {getDeviceId()};
      auto isResponding     {opts.getValueAs<bool>("responding")};

      nmdo::MacAddress macAddr;
      if (opts.exists("mac-addr")) {
        macAddr = nmdo::MacAddress(opts.getValue("mac-addr"));
      }

      nmdo::IpAddress ipAddr;
      if (opts.exists("ip-addr")) {
        ipAddr = nmdo::IpAddress(opts.getValue("ip-addr"));
        if (opts.exists("hostname")) {
          ipAddr.addAlias(opts.getValue("hostname"), "insert-address");
        }
      }

      macAddr.setResponding(isResponding);
      ipAddr.setResponding(isResponding);

      macAddr.addIpAddress(ipAddr);

      macAddr.save(t, toolRunId, deviceId);
      LOG_DEBUG << macAddr.toDebugString() << std::endl;
    }
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
