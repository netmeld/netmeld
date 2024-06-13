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

#include <netmeld/datastore/objects/aws/IamUserGroup.hpp>


namespace netmeld::datastore::objects::aws {

  IamUserGroup::IamUserGroup(const std::string& _userId,
                const std::string& _groupName) :
    userId(_userId),
    groupName(_groupName)
  {}

  IamUserGroup::IamUserGroup()
  {}

  void
  IamUserGroup::setUserId(const std::string& _userId) {
    userId = _userId;
  }

  void
  IamUserGroup::setGroupName(const std::string& _groupName) {
    groupName = _groupName;
  }

  std::string
  IamUserGroup::getUserId() const {
    return userId;
  }

  std::string
  IamUserGroup::getGroupName() const {
    return groupName;
  }

  std::string
  IamUserGroup::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "userId: " << userId << ", "
        << "groupName: " << groupName;
    oss << "]";

    return oss.str();
  }

  bool
  IamUserGroup::isValid() const {
      return false;
  }

  auto
  IamUserGroup::operator<=>(const IamUserGroup& rhs) const {
    return std::tie(userId
                        , groupName
                        )
        <=> std::tie(rhs.userId
                        , rhs.groupName
                        )
    ;
}

  void
  IamUserGroup::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamUserGroup object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
