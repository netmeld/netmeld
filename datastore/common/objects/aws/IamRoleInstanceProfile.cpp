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

#include <netmeld/datastore/objects/aws/IamRoleInstanceProfile.hpp>


namespace netmeld::datastore::objects::aws {

  IamRoleInstanceProfile::IamRoleInstanceProfile(const std::string& _parentRoleId,
                const std::string& _profileId,
                const std::string& _profileName,
                const std::string& _arn,
                const std::string& _createDate,
                const std::string& _path) :
    parentRoleId(_parentRoleId),
    profileId(_profileId),
    profileName(_profileName),
    arn(_arn),
    createDate(_createDate),
    path(_path)
  {}

  IamRoleInstanceProfile::IamRoleInstanceProfile()
  {}

  void
  IamRoleInstanceProfile::setParentRoleId(const std::string& _parentRoleId) {
    parentRoleId = _parentRoleId;
  }

  void
  IamRoleInstanceProfile::setProfileId(const std::string& _profileId) {
    profileId = _profileId;
  }

  void
  IamRoleInstanceProfile::setProfileName(const std::string& _profileName) {
    profileName = _profileName;
  }

  void
  IamRoleInstanceProfile::setArn(const std::string& _arn) {
    arn = _arn;
  }

  void
  IamRoleInstanceProfile::setCreateDate(const std::string& _createDate) {
    createDate = _createDate;
  }

  void
  IamRoleInstanceProfile::setPath(const std::string& _path) {
    path = _path;
  }

  std::string
  IamRoleInstanceProfile::getParentRoleId() const {
    return parentRoleId;
  }

  std::string
  IamRoleInstanceProfile::getProfileId() const {
    return profileId;
  }

  std::string
  IamRoleInstanceProfile::getProfileName() const {
    return profileName;
  }

  std::string
  IamRoleInstanceProfile::getArn() const {
    return arn;
  }

  std::string
  IamRoleInstanceProfile::getCreateDate() const {
    return createDate;
  }

  std::string
  IamRoleInstanceProfile::getPath() const {
    return path;
  }

  bool
  IamRoleInstanceProfile::isValid() const {
      return false;
  }

  std::string
  IamRoleInstanceProfile::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "parentRoleId: " << parentRoleId << ", "
        << "profileId: " << profileId << ", "
        << "profileName: " << profileName << ", "
        << "arn: " << arn << ", "
        << "createDate: " << createDate << ", "
        << "path: " << path << ", ";
    oss << "]";

    return oss.str();
  }

  auto
  IamRoleInstanceProfile::operator<=>(const IamRoleInstanceProfile& rhs) const {
    return std::tie(parentRoleId
                      , profileId
                      , profileName
                      , arn
                      , createDate
                      , path
                      )
           <=> std::tie(rhs.parentRoleId
                          , rhs.profileId
                          , rhs.profileName
                          , rhs.arn
                          , rhs.createDate
                          , rhs.path
                          )
    ;
  }

  bool
  IamRoleInstanceProfile::operator==(const IamRoleInstanceProfile& rhs) const {
      return 0 == operator<=>(rhs);
  }

  void
  IamRoleInstanceProfile::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamRoleInstanceProfile object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
