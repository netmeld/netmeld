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

#include <netmeld/datastore/objects/aws/IamStatement.hpp>


namespace netmeld::datastore::objects::aws {

  IamStatement::IamStatement(const std::string& _attachmentId,
                const std::string& _documentVersion,
                const std::string& _sid,
                const std::string& _effects,
                const bool& _isAction,
                const std::vector<std::string>& _actions,
                const std::vector<std::string>& _resources,
                const json& _principal,
                const json& _condition) :
    attachmentId(_attachmentId),
    documentVersion(_documentVersion),
    sid(_sid),
    effect(_effects),
    isAction(_isAction),
    actions(_actions),
    resources(_resources),
    principal(_principal),
    condition(_condition)
  {}

  IamStatement::IamStatement()
  {}

  void
  IamStatement::setAttachmentId(const std::string& _attachmentId) {
      attachmentId = _attachmentId;
  }

  void
  IamStatement::setDocumentVersion(const std::string& _documentVersion) {
      documentVersion = _documentVersion;
  }

  void
  IamStatement::setSid(const std::string& _sid) {
      sid = _sid;
  }

  void
  IamStatement::setEffect(const std::string& _effect) {
    effect = _effect;
  }

  void
  IamStatement::setIsAction(const bool& _isAction) {
    isAction = _isAction;
  }

  void
  IamStatement::addAction(const std::string& _action) {
    actions.push_back(_action);
  }

  void
  IamStatement::addResource(const std::string& _resource) {
    resources.push_back(_resource);
  }

  void
  IamStatement::setPrincipal(const json& _principal) {
      principal = _principal;
  }

  void
  IamStatement::setCondition(const json& _condition) {
    condition = _condition;
  }

  std::string
  IamStatement::getAttachmentId() const {
      return attachmentId;
  }

  std::string
  IamStatement::getDocumentVersion() const {
      return documentVersion;
  }

  std::string
  IamStatement::getSid() const {
      return sid;
  }

  std::string
  IamStatement::getEffect() const {
    return effect;
  }

  bool
  IamStatement::getIsAction() const {
    return isAction;
  }

  std::vector<std::string>
  IamStatement::getActions() const {
    return actions;
  }

  std::vector<std::string>
  IamStatement::getResources() const {
    return resources;
  }

  json
  IamStatement::getPrincipal() const {
    return principal;
  }

  json
  IamStatement::getCondition() const {
    return condition;
  }

  bool
  IamStatement::isValid() const {
    return !(
      attachmentId.empty() ||
      documentVersion.empty() ||
      sid.empty() ||
      effect.empty() ||
      actions.empty() ||
      resources.empty()
    );
  }

  std::string
  IamStatement::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "attachmentId: " << attachmentId << ","
        << "documentVersion: " << documentVersion << ","
        << "sid: " << sid << ","
        << "effect: " << effect << ", "
        << "isAction: " << isAction << ","
        << "actions: " << actions << ", "
        << "resources: " << resources << ","
        << "principal: " << principal.dump() << ","
        << "condition: " << condition.dump();
    oss << "]";

    return oss.str();
  }

  auto
  IamStatement::operator<=>(const IamStatement& rhs) const
  {
    return std::tie(attachmentId
                      , documentVersion
                      , sid
                      , effect
                      , isAction
                      , actions
                      , resources
                      , principal
                      , condition
                      )
           <=> std::tie(rhs.attachmentId
                          , rhs.documentVersion
                          , rhs.sid
                          , rhs.effect
                          , rhs.isAction
                          , rhs.actions
                          , rhs.resources
                          , rhs.principal
                          , rhs.condition
                          )
    ;
  }

  bool
  IamStatement::operator==(const IamStatement& rhs) const
  {
    return 0 == operator<=>(rhs);
  }

  void
  IamStatement::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamStatement object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_iam_statement"
        , toolRunId
        , attachmentId
        , documentVersion
        , sid
        , effect
        , actions
        , resources
        , principal.dump()
        , condition.dump()
    );
  }
}
