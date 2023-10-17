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
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <regex>

namespace nmdp = netmeld::datastore::parsers;

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

    if (addAclObjects) { // START -- Temporary logic for AC to ACL duplication
      LOG_DEBUG << "AcNetworkBook creating ACL object(s) to save\n"
                << "AcNetworkBook to save: " << toDebugString()
                << std::endl;

      // "any" nets which are not always explicitly defined, but still used
      //  - vendors typically have built-in defaults
      { // -- save AclIpNetSet "any"
        AclIpNetSet ains;
        ains.addAcObjects = false; // don't create AC objects (infinite loop)
        ains.setId("any", "global");
        ains.addIpNet(IpNetwork("0.0.0.0/0"));
        ains.addIpNet(IpNetwork("::/0"));
        LOG_DEBUG << "AclIpNetSet to save: " << ains.toDebugString() << '\n';
        ains.save(t, toolRunId, deviceId);
      }
      { // -- save AclIpNetSet "any4"
        AclIpNetSet ains;
        ains.addAcObjects = false; // don't create AC objects (infinite loop)
        ains.setId("any4", "global"); // cisco
        ains.addIpNet(IpNetwork("0.0.0.0/0"));
        ains.save(t, toolRunId, deviceId);
        ains.setId("any-ipv4", "global"); // juniper
        LOG_DEBUG << "AclIpNetSet to save: " << ains.toDebugString() << '\n';
        ains.save(t, toolRunId, deviceId);
      }
      { // -- save AclIpNetSet "any6"
        AclIpNetSet ains;
        ains.addAcObjects = false; // don't create AC objects (infinite loop)
        ains.setId("any6", "global"); //cisco
        ains.addIpNet(IpNetwork("::/0"));
        ains.save(t, toolRunId, deviceId);
        ains.setId("any-ipv6", "global"); // juniper
        LOG_DEBUG << "AclIpNetSet to save: " << ains.toDebugString() << '\n';
        ains.save(t, toolRunId, deviceId);
      }

      { // -- save AclIpNetSet
        AclIpNetSet ains;
        ains.addAcObjects = false; // don't create AC objects (infinite loop)
        ains.setId(name, id);

        std::regex rAny   {R"(^any[46]?$)"};
        for (const auto& entry : data) {
          bool isIpNet {
              nmdp::matchString<nmdp::ParserIpAddress, IpAddress>(entry)
            };

          if (isIpNet) {
            IpNetwork net {entry};
            ains.addIpNet(net);
          } else if (!std::regex_match(entry, rAny)) {
            ains.addHostname(entry);
          }
        }

        LOG_DEBUG << "AclIpNetSet to save: " << ains.toDebugString() << '\n';
        ains.save(t, toolRunId, deviceId);
      }
    } // END
  }
}
