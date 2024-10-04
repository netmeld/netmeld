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

#include <netmeld/datastore/objects/aws/IamRolePermissionBoundary.hpp>


namespace netmeld::datastore::objects::aws {

  IamRolePermissionBoundary::IamRolePermissionBoundary(const std::string& _roleId,
                const std::string& _boundaryArn,
                const std::string& _boundaryType) :
    roleId(_roleId),
    boundaryArn(_boundaryArn),
    boundaryType(_boundaryType)
  {}

  IamRolePermissionBoundary::IamRolePermissionBoundary()
  {}

  void
  IamRolePermissionBoundary::setRoleId(const std::string& _roleId) {
    roleId = _roleId;
  }

  void
  IamRolePermissionBoundary::setBoundaryArn(const std::string& _boundaryArn) {
    boundaryArn = _boundaryArn;
  }

  void
  IamRolePermissionBoundary::setBoundaryType(const std::string& _boundaryType) {
    boundaryType = _boundaryType;
  }

  std::string
  IamRolePermissionBoundary::getRoleId() const {
    return roleId;
  }

  std::string
  IamRolePermissionBoundary::getBoundaryArn() const {
    return boundaryArn;
  }

  std::string
  IamRolePermissionBoundary::getBoundaryType() const {
    return boundaryType;
  }

  bool
  IamRolePermissionBoundary::isValid() const {
      return false;
  }

  std::string
  IamRolePermissionBoundary::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "roleId: " << roleId << ", "
        << "boundaryArn: " << boundaryArn << ", "
        << "boundaryType: " << boundaryType;
    oss << "]";

    return oss.str();
  }

  auto
  IamRolePermissionBoundary::operator<=>(const IamRolePermissionBoundary& rhs) const {
    return std::tie(roleId
                      , boundaryArn
                      , boundaryType)
              <=> std::tie(rhs.roleId
                            , rhs.boundaryArn
                            , rhs.boundaryType
                            )
    ;
  }

  bool
  IamRolePermissionBoundary::operator==(const IamRolePermissionBoundary& rhs) const {
      return 0 == operator<=>(rhs);
  }

  void
  IamRolePermissionBoundary::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamRolePermissionBoundary object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
