// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/AcNetworkBook.hpp>

#include <netmeld/datastore/objects/AclIpNetSet.hpp>
#include <netmeld/datastore/objects/IpNetwork.hpp>
#include <regex>


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AcNetworkBook::AcNetworkBook()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  AcNetworkBook::isValid() const
  {
    return !id.empty()
        && !name.empty();
  }

  void
  AcNetworkBook::save(pqxx::transaction_base& t,
                      const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AcNetworkBook object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    if (0 == data.size()) {
      t.exec_prepared("insert_raw_device_ac_net",
        toolRunId,
        deviceId,
        id,
        name,
        nullptr);
    } else {
      for (const auto& entry : data) {
        t.exec_prepared("insert_raw_device_ac_net",
          toolRunId,
          deviceId,
          id,
          name,
          entry);
      }
    }

    // START -- Temporary logic for AC to ACL duplication
    if (true) {
      LOG_DEBUG << "AcNetworkBook creating ACL object(s) to save\n";
      // create "any" nets in case not explicitly defined
      // - vendors typically have built-in defaults
      {
        AclIpNetSet ains;
        ains.setId("any", "global");
        ains.addIpNet(IpNetwork("0.0.0.0/0"));
        ains.addIpNet(IpNetwork("::/0"));
        ains.save(t, toolRunId, deviceId);
      }
      {
        AclIpNetSet ains;
        ains.setId("any4", "global"); // cisco
        ains.addIpNet(IpNetwork("0.0.0.0/0"));
        ains.save(t, toolRunId, deviceId);
        ains.setId("any-ipv4", "global"); // juniper
        ains.save(t, toolRunId, deviceId);
      }
      {
        AclIpNetSet ains;
        ains.setId("any6", "global"); //cisco
        ains.addIpNet(IpNetwork("::/0"));
        ains.save(t, toolRunId, deviceId);
        ains.setId("any-ipv6", "global"); // juniper
        ains.save(t, toolRunId, deviceId);
      }
      // -- save AclIpNetSet
      {
        AclIpNetSet ains;
        ains.setId(name, id);

        std::regex rIpNet {R"(^(([0-9.]+)|([0-9a-fA-F:]+))(/\d{1,3})?$)"};
        std::regex rAny   {R"(^any[46]?$)"};
        for (const auto& entry : data) {
          if (std::regex_match(entry, rIpNet)) {
            IpNetwork net {entry};
            ains.addIpNet(net);
          } else if (!std::regex_match(entry, rAny)) {
            ains.addHostname(entry);
          }
        }
        LOG_DEBUG << "AclIpNetSet to save: " << ains.toDebugString() << '\n';
        ains.save(t, toolRunId, deviceId);
      }
    }
    // END
  }
}
