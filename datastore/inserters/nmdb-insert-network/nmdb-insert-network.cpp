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

#include <netmeld/datastore/objects/IpNetwork.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/datastore/tools/AbstractInsertTool.hpp>

namespace nmdt = netmeld::datastore::tools;
namespace nmdo = netmeld::datastore::objects;


class Tool : public nmdt::AbstractInsertTool
{
  public:
    Tool() : nmdt::AbstractInsertTool
      ("VLAN and/or IP network", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      opts.addOptionalOption("vlan", std::make_tuple(
          "vlan",
          po::value<uint16_t>(),
          "802.1Q VLAN ID of the network.")
        );
      opts.addOptionalOption("ip-net", std::make_tuple(
          "ip-net",
          po::value<std::string>(),
          "IP CIDR of the network.")
        );
      opts.addOptionalOption("description", std::make_tuple(
          "description",
          po::value<std::string>(),
          "Description of the network.")
        );
      opts.addOptionalOption("low-graph-priority", std::make_tuple(
          "low-graph-priority",
          NULL_SEMANTIC,
          "Network should be out-of-the-way (generally lower) in network graphs."
          " Use for management networks.")
        );
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {getToolRunId()};

      // Common logic
      std::string description;
      if (opts.exists("description")) {
        description = opts.getValue("description");
      }

      // IP net logic
      nmdo::IpNetwork ipNetwork;
      if (opts.exists("ip-net")) {
        nmdo::IpNetwork ipNet {opts.getValue("ip-net")};
        ipNet.setReason(description);
        if (opts.exists("low-graph-priority")) {
          ipNet.setExtraWeight(M_PI); // Not arbitrary, but probably high enough
        }

        ipNet.save(t, toolRunId, "");
        LOG_DEBUG << ipNet.toDebugString() << std::endl;

        ipNetwork = ipNet;
      }

      // VLAN logic
      if (opts.exists("vlan")) {
        nmdo::Vlan vlan;

        vlan.setId(opts.getValueAs<uint16_t>("vlan"));
        vlan.setDescription(description);
        vlan.setIpNet(ipNetwork); // Already set up with ip-net logic or invalid

        vlan.save(t, toolRunId, "");
        LOG_DEBUG << vlan.toDebugString() << std::endl;
      }
    }
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
