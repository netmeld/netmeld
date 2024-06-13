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

#include <netmeld/datastore/objects/aws/IamPolicyDocument.hpp>


namespace netmeld::datastore::objects::aws {

  IamPolicyDocument::IamPolicyDocument(const std::string& _attachmentId,
                const std::string& _policyName,
                const std::string& _stmtEffect,
                const std::string& _stmtAction,
                const std::string& _stmtResource,
                const json& _stmtCondition,
                const std::string& _stmtSid) :
    attachmentId(_attachmentId),
    policyName(_policyName),
    stmtEffect(_stmtEffect),
    stmtAction(_stmtAction),
    stmtResource(_stmtResource),
    stmtCondition(_stmtCondition),
    stmtSid(_stmtSid)
  {}

  IamPolicyDocument::IamPolicyDocument()
  {}

  void
  IamPolicyDocument::setAttachmentId(const std::string& _attachmentId) {
    attachmentId = _attachmentId;
  }

  void
  IamPolicyDocument::setPolicyName(const std::string& _policyName) {
    policyName = _policyName;
  }

  void
  IamPolicyDocument::setStmtEffect(const std::string& _stmtEffect) {
    stmtEffect = _stmtEffect;
  }

  void
  IamPolicyDocument::setStmtAction(const std::string& _stmtAction) {
    stmtAction = _stmtAction;
  }

  void
  IamPolicyDocument::setStmtResource(const std::string& _stmtResource) {
    stmtResource = _stmtResource;
  }

  void
  IamPolicyDocument::setStmtCondition(const json& _stmtCondition) {
    stmtCondition = _stmtCondition;
  }

  void
  IamPolicyDocument::setStmtSid(const std::string& _stmtSid) {
    stmtSid = _stmtSid;
  }

  std::string
  IamPolicyDocument::getAttachmentId() const {
    return attachmentId;
  }

  std::string
  IamPolicyDocument::getPolicyName() const {
    return policyName;
  }

  std::string
  IamPolicyDocument::getStmtEffect() const {
    return stmtEffect;
  }

  std::string
  IamPolicyDocument::getStmtAction() const {
    return stmtAction;
  }

  std::string
  IamPolicyDocument::getStmtResource() const {
    return stmtResource;
  }

  json
  IamPolicyDocument::getStmtCondition() const {
    return stmtCondition;
  }

  std::string
  IamPolicyDocument::getStmtSid() const {
    return stmtSid;
  }

  bool
  IamPolicyDocument::isValid() const {
      return false;
  }

  std::string
  IamPolicyDocument::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "attachmentId: " << attachmentId << ", "
        << "policyName: " << policyName << ", "
        << "stmtEffect: " << stmtEffect << ", "
        << "stmtAction: " << stmtAction << ", "
        << "stmtResource: " << stmtResource << ", "
        << "stmtCondition: " << stmtCondition.dump() << ", "
        << "stmtSid: " << stmtSid;
    oss << "]";

    return oss.str();
  }


  auto
  IamPolicyDocument::operator<=>(const IamPolicyDocument& rhs) const {
    return std::tie(attachmentId
                      , policyName
                      , stmtEffect
                      , stmtAction
                      , stmtResource
                      , stmtCondition
                      , stmtSid
                      )
           <=> std::tie(rhs.attachmentId
                          , rhs.policyName
                          , rhs.stmtEffect
                          , rhs.stmtAction
                          , rhs.stmtResource
                          , rhs.stmtCondition
                          , rhs.stmtSid
                          )
    ;
}


  void
  IamPolicyDocument::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamPolicyDocument object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
