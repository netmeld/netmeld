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
// ======

#include <netmeld/datastore/objects/Package.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects 
{
    
    Package::Package()
    {}
    // string constructor
    Package::Package(const std::string& _status) :
        packageStatus(nmcu::toLower(_status))
    {}

    void
    Package::setStatus(const std::string& _status)
    {
        packageStatus = _status;
    }
    void
    Package::setName(const std::string& _name)
    {
    packageName = _name;
    }
    void
    Package::setVersion(const std::string& _version)
    {
    packageVersion = _version;
    }
    void
    Package::setArch(const std::string& _arch)
    {
    packageArch = _arch;
    }
    void
    Package::setDesc(const std::string& _desc)
    {
    packageDesc = _desc;
    }

    //getters
    std::string
    Package::getStatus() const
    {
        return packageStatus;
    }
    std::string
    Package::getName() const
    {
        return packageName;
    }
    std::string
    Package::getVersion() const
    {
        return packageVersion;
    }
    std::string
    Package::getArch() const 
    {
        return packageArch;
    }
    std::string
    Package::getDesc() const
    {
        return packageDesc;
    }

    //toolOverrides
    bool 
    Package::isValid() const
    {
        return !packageName.empty() && !packageVersion.empty();
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
            packageStatus,
            packageName,
            packageVersion,
            packageArch,
            packageDesc);
        
    }

    //to string
    std::string
    Package::toDebugString() const
    {
        std::ostringstream oss;

        oss << "["; // opening bracket
        oss << packageStatus << ", "
            << packageName << ", "
            << packageVersion << ", "
            << packageArch << ", "
            << packageDesc;
        oss << "]"; // closing bracket

        return oss.str();
    }
}