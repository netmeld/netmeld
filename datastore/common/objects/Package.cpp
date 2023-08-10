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

#include <netmeld/datastore/objects/Package.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::datastore::objects
{
    Package::Package()
    {}

    void
    Package::setState(const std::string& _state)
    {
        state = _state;
    }

    void
    Package::setName(const std::string& _name)
    {
        name = _name;
    }

    void
    Package::setVersion(const std::string& _version)
    {
        version = _version;
    }

    void
    Package::setArchitecture(const std::string& _architecture)
    {
        architecture = _architecture;
    }

    void
    Package::setDescription(const std::string& _description)
    {
        description = _description;
    }

    std::string
    Package::getState() const
    {
        return state;
    }

    std::string
    Package::getName() const
    {
        return name;
    }

    std::string
    Package::getVersion() const
    {
        return version;
    }

    std::string
    Package::getArchitecture() const
    {
        return architecture;
    }

    std::string
    Package::getDescription() const
    {
        return description;
    }

    //toolOverrides
    bool
    Package::isValid() const
    {
        return !name.empty() && !version.empty() && !architecture.empty() && !description.empty();
    }

    void
    Package::save(pqxx::transaction_base& t, const nmco::Uuid& toolRunId, const std::string& deviceId)
    {
        if(!isValid() && !deviceId.empty()){
            LOG_DEBUG << "Package object is not saving: " << toDebugString()
                << std::endl;
        return;
        }

        t.exec_prepared("insert_raw_packages",
            toolRunId,
            state,
            name,
            version,
            architecture,
            description);
    }

    std::string
    Package::toDebugString() const
    {
        std::ostringstream oss;
        oss << "["; // opening bracket
        oss << "state: " << state << ", "
            << "name: " << name << ", "
            << "version: " << version << ", "
            << "architecture: " << architecture << ", "
            << "description: " << description;
        oss << "]"; // closing bracket
        return oss.str();
    }

  std::partial_ordering
  Package::operator<=>(const Package& rhs) const
  {
    return std::tie( state
                   , name
                   , version
                   , architecture
                   , description
                   )
       <=> std::tie( rhs.state
                   , rhs.name
                   , rhs.version
                   , rhs.architecture
                   , rhs.description
                   )
      ;
  }

  bool
  Package::operator==(const Package& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
