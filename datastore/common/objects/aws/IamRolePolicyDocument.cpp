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

#include <netmeld/datastore/objects/aws/IamRolePolicyDocument.hpp>


namespace netmeld::datastore::objects::aws {

  IamRolePolicyDocument::IamRolePolicyDocument(const std::string& _roleId,
                const std::string& _docVersion,
                const std::string& _policyName,
                const std::string& _stmtEffect,
                const std::string& _stmtAction,
                const json& _stmtPrincipal,
                const json& _stmtCondition,
                const std::string& _stmtSid) :
    roleId(_roleId),
    docVersion(_docVersion),
    policyName(_policyName),
    stmtEffect(_stmtEffect),
    stmtAction(_stmtAction),
    stmtPrincipal(_stmtPrincipal),
    stmtCondition(_stmtCondition),
    stmtSid(_stmtSid)
  {}

  IamRolePolicyDocument::IamRolePolicyDocument()
  {}

  void
  IamRolePolicyDocument::setRoleId(const std::string& _roleId) {
    roleId = _roleId;
  }

  void
  IamRolePolicyDocument::setDocVersion(const std::string& _docVersion) {
    docVersion = _docVersion;
  }

  void
  IamRolePolicyDocument::setPolicyName(const std::string& _policyName) {
    policyName = _policyName;
  }

  void
  IamRolePolicyDocument::setStmtEffect(const std::string& _stmtEffect) {
    stmtEffect = _stmtEffect;
  }

  void
  IamRolePolicyDocument::setStmtAction(const std::string& _stmtAction) {
    stmtAction = _stmtAction;
  }

  void
  IamRolePolicyDocument::setStmtPrincipal(const json& _stmtPrincipal) {
    stmtPrincipal = _stmtPrincipal;
  }

  void
  IamRolePolicyDocument::setStmtCondition(const json& _stmtCondition) {
    stmtCondition = _stmtCondition;
  }

  void
  IamRolePolicyDocument::setStmtSid(const std::string& _stmtSid) {
    stmtSid = _stmtSid;
  }

  std::string
  IamRolePolicyDocument::getRoleId() const {
    return roleId;
  }

  std::string
  IamRolePolicyDocument::getDocVersion() const {
    return docVersion;
  }

  std::string
  IamRolePolicyDocument::getPolicyName() const {
    return policyName;
  }

  std::string
  IamRolePolicyDocument::getStmtEffect() const {
    return stmtEffect;
  }

  std::string
  IamRolePolicyDocument::getStmtAction() const {
    return stmtAction;
  }

  json
  IamRolePolicyDocument::getStmtPrincipal() const {
    return stmtPrincipal;
  }

  json
  IamRolePolicyDocument::getStmtCondition() const {
    return stmtCondition;
  }

  std::string
  IamRolePolicyDocument::getStmtSid() const {
    return stmtSid;
  }

  bool
  IamRolePolicyDocument::isValid() const {
      return false;
  }

  std::string
  IamRolePolicyDocument::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "roleId: " << roleId << ", "
        << "docVersion: " << docVersion << ", "
        << "policyName: " << policyName << ", "
        << "stmtEffect: " << stmtEffect << ", "
        << "stmtAction: " << stmtAction << ", "
        << "stmtPrincipal: " << stmtPrincipal.dump() << ", "
        << "stmtCondition: " << stmtCondition.dump() << ", "
        << "stmtSid: " << stmtSid;
    oss << "]";

    return oss.str();
  }

  auto
  IamRolePolicyDocument::operator<=>(const IamRolePolicyDocument& rhs) const
  {
    return std::tie(roleId
                        , docVersion
                        , policyName
                        , stmtEffect
                        , stmtAction
                        , stmtPrincipal
                        , stmtCondition
                        , stmtSid
                        )
              <=> std::tie(rhs.roleId
                              , rhs.docVersion
                              , rhs.policyName
                              , rhs.stmtEffect
                              , rhs.stmtAction
                              , rhs.stmtPrincipal
                              , rhs.stmtCondition
                              , rhs.stmtSid
                              )
    ;
  }

  void
  IamRolePolicyDocument::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamRolePolicyDocument object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
