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

#include <netmeld/datastore/objects/AcServiceBook.hpp>

#include <ranges>
#include <string_view>
#include <netmeld/datastore/objects/AclService.hpp>
#include <netmeld/datastore/objects/PortRange.hpp>

namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AcServiceBook::AcServiceBook()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  AcServiceBook::isValid() const
  {
    return !name.empty();
  }

  void
  AcServiceBook::save(pqxx::transaction_base& t,
               const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AcServiceBook object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    if (0 == data.size()) {
      t.exec_prepared("insert_raw_device_ac_service",
        toolRunId,
        deviceId,
        name,
        nullptr);
    } else {
      for (const auto& entry : data) {
        t.exec_prepared("insert_raw_device_ac_service",
          toolRunId,
          deviceId,
          name,
          entry);
      }
    }

    // START -- Temporary logic for AC to ACL duplication
    if (true) {
      LOG_DEBUG << "AcServiceBook creating ACL object(s) to save\n";
      // -- save AclService
      for (const auto& entry : data) {
        std::vector<std::string> serviceParts;
        for (const auto word : std::views::split(entry, ':')) {
          std::string token {std::string_view(word)};
          auto n {token.find("--")};
          if (n != std::string::npos) {
            token.erase(n);
          }
          serviceParts.push_back(token);
        }

        AclService as;
        as.setId(name);
        as.setProtocol(serviceParts[0]);

        PortRange any {0, 65535};
        if (1 < serviceParts.size() && !serviceParts[1].empty()) {
          for (const auto word : std::views::split(serviceParts[1], ',')) {
            as.addSrcPortRange(PortRange(std::string(std::string_view(word))));
          }
        } else {
          as.addSrcPortRange(any);
        }
        if (2 < serviceParts.size() && !serviceParts[2].empty()) {
          for (const auto word : std::views::split(serviceParts[2], ',')) {
            std::string token {std::string_view(word)};
            as.addDstPortRange(PortRange(std::string(std::string_view(word))));
          }
        } else {
          as.addDstPortRange(any);
        }

        LOG_DEBUG << "AclService to save: " << as.toDebugString() << std::endl;
        as.save(t, toolRunId, deviceId);
      }
    }
    // END
  }
}
