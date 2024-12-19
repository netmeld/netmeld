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
                const std::string& _profileId,
                const std::string& _arn,
                const std::string& _name,
                const std::string& _createDate,
                const std::string& _lastUsed,
                const std::string& _path,
                const uint16_t _permissionsBoundaryUsageCount,
                const json& _tags) :
    IamBase(_id, _arn, _name, _createDate, _path, _tags),
    profileId(_profileId),
    permissionsBoundaryUsageCount(_permissionsBoundaryUsageCount),
    lastUsed(_lastUsed)
  {}

  IamRole::IamRole()
  {}

  void
  IamRole::setProfileId(const std::string& _profileId)
  {
      profileId = _profileId;
  }

  void
  IamRole::setLastUsed(const std::string& _lastUsed)
  {
      lastUsed = _lastUsed;
  }

  void
  IamRole::setPermissionsBoundaryUsageCount(const uint16_t _permissionsBoundaryUsageCount)
  {
      permissionsBoundaryUsageCount = _permissionsBoundaryUsageCount;
  }

  std::string
  IamRole::getProfileId() const
  {
      return profileId;
  }

  std::string
  IamRole::getLastUsed() const
  {
      return lastUsed;
  }

  uint16_t
  IamRole::getPermissionsBoundaryUsageCount() const
  {
      return permissionsBoundaryUsageCount;
  }

  bool
  IamRole::isValid() const {
    return !(
      id.empty() ||
      arn.empty() ||
      name.empty() ||
      createDate.empty() ||
      lastUsed.empty() ||
      path.empty() ||
      permissionsBoundaryUsageCount < 0
    );
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

    t.exec_prepared("insert_raw_aws_iam_role"
        , toolRunId
        , id
        , arn
        , name
        , createDate
        , path
        , tags.dump()
        , profileId
        , lastUsed
        , permissionsBoundaryUsageCount
    );
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
        << "profileId: " << profileId << ","
        << "permissionsBoundaryUsageCount: " << permissionsBoundaryUsageCount << ","
        << "tags: " << tags.dump();
    oss << "]";

    return oss.str();
  }

  auto
  IamRole::operator<=>(const IamRole& rhs) const
  {
    return std::tie( id
                   , arn
                   , name
                   , createDate
                   , path
                   , tags
                   , profileId
                   , permissionsBoundaryUsageCount
                   , lastUsed
                   )
       <=> std::tie( rhs.id
                   , rhs.arn
                   , rhs.name
                   , rhs.createDate
                   , rhs.path
                   , rhs.tags
                   , rhs.profileId
                   , rhs.permissionsBoundaryUsageCount
                   , rhs.lastUsed
                   )
      ;
  }

  bool
  IamRole::operator==(const IamRole& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
