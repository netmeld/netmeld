// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/IamRole.hpp>


namespace netmeld::datastore::objects::aws {

  IamRole::IamRole(const std::string& _id,
                const std::string& _arn,
                const std::string& _name,
                const std::string& _createDate,
                const std::string& _lastUsed,
                const std::string& _path,
                const json& _tags) :
    IamBase(_id, _arn, _name, _createDate, _path, _tags),
    lastUsed(_lastUsed)
  {}

  IamRole::IamRole()
  {}

  void
  IamRole::setLastUsed(const std::string& _lastUsed)
  {
      lastUsed = _lastUsed;
  }

  std::string
  IamRole::getLastUsed() const
  {
      return lastUsed;
  }

  void
  IamRole::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamRole object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }

  std::string
  IamRole::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";
    oss << "id: " << id << ", "
        << "arn: " << arn << ", "
        << "name: " << name << ", "
        << "createDate: " << createDate << ","
        << "lastUsed: " << lastUsed << ","
        << "path: " << path << ","
        << "tags: " << tags.dump();
    oss << "]";

    return oss.str();
  }
}
