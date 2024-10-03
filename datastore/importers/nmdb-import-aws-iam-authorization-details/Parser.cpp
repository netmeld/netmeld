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

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data) // Object
{
  for (const auto& roleDetail : _data.at("RoleDetailList")) { // List
    processRoleDetails(roleDetail);
  }
  for (const auto& groupDetail : _data.at("GroupDetailList")) { // List
    processGroupDetails(groupDetail);
  }
  for (const auto& userDetail : _data.at("UserDetailList")) { // List
    processUserDetails(userDetail);
  }
  for (const auto& policy : _data.at("Policies")) { // List
    processPolicy(policy);
  }
}

void
Parser::processRoleDetails(const json& _roleDetail,
                                        const std::string& _profileId) { // Object
  LOG_DEBUG << "processRoleDetails:\n";

  nmdoa::IamRole role;
  role.setProfileId(_profileId);
  role.setId(_roleDetail.value("RoleId", ""));
  role.setArn(_roleDetail.value("Arn", ""));
  role.setName(_roleDetail.value("RoleName", ""));
  role.setCreateDate(_roleDetail.value("CreateDate", ""));

  // Default: -1 or 0?
  role.setPermissionsBoundaryUsageCount(
    _roleDetail.value("PermissionsBoundaryUsageCount", 0));

  const json rlu = _roleDetail.value("RoleLastUsed", json({}));
  role.setLastUsed(rlu.value("LastUsedDate", ""));

  role.setPath(_roleDetail.value("Path", ""));
  json defaultTags = json::array();
  defaultTags.push_back(json::object());
  role.setTags(_roleDetail.value("Tags", defaultTags));

  // AssumeRolePolicyDocument does not have a version associated with it
  processDocument(
    _roleDetail.at("AssumeRolePolicyDocument"), // Object
    role.getId(), std::string());

  if(_roleDetail.contains("AttachedManagedPolicies")) {
    const json amp = _roleDetail.at("AttachedManagedPolicies");
    if(amp.is_array()) {
      for (const auto& policy : amp) { // List
          processManagedPolicy(policy, role.getId());
      }
    } else {
      processManagedPolicy(amp, role.getId());
    }
  }
  if(_roleDetail.contains("InstanceProfileList")) {
    for (const auto& profile : _roleDetail.at("InstanceProfileList")) { // List
        processProfileList(profile, role.getId());
    }
  }
  if(_roleDetail.contains("PermissionsBoundary")) {
    // Object
    processPermissionsBoundary(_roleDetail["PermissionsBoundary"], role.getId());
  }
  if(_roleDetail.contains("RolePolicyList")) {
    for (const auto& rolePolicy : _roleDetail.at("RolePolicyList")) { // List
        processPolicyList(rolePolicy, role.getId());
    }
  }

  d.roles.push_back(role);
}

void
Parser::processGroupDetails(const json& _groupDetail) {
  LOG_DEBUG << "processGroupDetails:\n";

  nmdoa::IamGroup group;
  group.setId(_groupDetail.value("GroupId", ""));
  group.setArn(_groupDetail.value("Arn", ""));
  group.setName(_groupDetail.value("GroupName", ""));
  group.setCreateDate(_groupDetail.value("CreateDate", ""));
  group.setPath(_groupDetail.value("Path", ""));
  json defaultTags = json::array();
  defaultTags.push_back(json::object());
  group.setTags(_groupDetail.value("Tags", defaultTags));

  if(_groupDetail.contains("AttachedManagedPolicies")) {
    const json amp = _groupDetail.at("AttachedManagedPolicies");
    if(amp.is_array()) {
      for (const auto& policy : amp) { // List
          processManagedPolicy(policy, group.getId());
      }
    } else {
      processManagedPolicy(amp, group.getId());
    }
  }
  if(_groupDetail.contains("GroupPolicyList")) {
    for (const auto& policy : _groupDetail.at("GroupPolicyList")) { // List
        processPolicyList(policy, group.getId());
    }
  }

  d.groups.push_back(group);
}

void
Parser::processUserDetails(const json& _userDetail) {
  LOG_DEBUG << "processUserDetails:\n";

  nmdoa::IamUser user;
  user.setId(_userDetail.value("UserId", ""));
  user.setArn(_userDetail.value("Arn", ""));
  user.setName(_userDetail.value("UserName", ""));
  user.setCreateDate(_userDetail.value("CreateDate", ""));
  user.setPath(_userDetail.value("Path", ""));
  json defaultTags = json::array();
  defaultTags.push_back(json::object());
  user.setTags(_userDetail.value("Tags", defaultTags));

  if(_userDetail.contains("AttachedManagedPolicy")) {
    const json amp = _userDetail.at("AttachedManagedPolicy");
    if(amp.is_array()) {
      for (const auto& managedPolicy : amp) { // List
          processManagedPolicy(managedPolicy, user.getId());
      }
    } else {
      processManagedPolicy(amp, user.getId());
    }
  }
  if(_userDetail.contains("GroupList")) {
    for (const auto& group : _userDetail.at("GroupList")) { // List
        // Just a list of strings. Save them all?
        nmdoa::IamUserGroup userGroup(user.getId(), group);
        d.userGroups.push_back(userGroup);
    }
  }
  if(_userDetail.contains("UserPolicyList")) {
    for (const auto& userPolicy : _userDetail.at("UserPolicyList")) { // List
        processPolicyList(userPolicy, user.getId());
    }
  }

  d.users.push_back(user);
}

void
Parser::processPolicy(const json& _policy) {
  LOG_DEBUG << "processPolicy:\n";

  nmdoa::IamPolicy policy;
  policy.setId(_policy.value("PolicyId", ""));
  policy.setArn(_policy.value("Arn", ""));
  policy.setName(_policy.value("PolicyName", ""));
  policy.setCreateDate(_policy.value("CreateDate", ""));
  policy.setPath(_policy.value("Path", ""));
  json defaultTags = json::array();
  defaultTags.push_back(json::object());
  policy.setTags(_policy.value("Tags", defaultTags));
  policy.setUpdateDate(_policy.value("UpdateDate", ""));
  policy.setAttachmentCount(_policy.value("AttachmentCount", 0));
  policy.setDefaultVersionId(_policy.value("DefaultVersionId", ""));
  policy.setIsAttachable(_policy.value("IsAttachable", false));
  policy.setPermissionsBoundaryUsageCount(
    _policy.value("PermissionsBoundaryUsageCount", 0));

  if(_policy.contains("PolicyVersionList")) {
    for (const auto& policyVersion : _policy.at("PolicyVersionList")) { // List
      processPolicyVersionList(policyVersion, policy.getId());
    }
  }

  d.policies.push_back(policy);
}

// GroupDetail, RoleDetail, UserDetail
void
Parser::processManagedPolicy(const json& _managedPolicy,
                                             const std::string& _attachmentId) {
  LOG_DEBUG << "processManagedPolicy:\n";

  nmdoa::IamAttachedManagedPolicy amp;
  amp.setAttachmentId(_attachmentId);
  amp.setPolicyArn(_managedPolicy.value("PolicyArn", ""));
  amp.setPolicyName(_managedPolicy.value("PolicyName", ""));
  d.amps.push_back(amp);
}

// GroupDetail, RoleDetail, UserDetail
void
Parser::processPolicyList(const json& _policyList, const std::string& _attachmentId) {
  LOG_DEBUG << "processPolicyList:\n";

  nmdoa::IamPolicyDocument policyDocument;
  policyDocument.setAttachmentId(_attachmentId);
  policyDocument.setPolicyName(_policyList.value("PolicyName", ""));
  d.policyDocuments.push_back(policyDocument);
// Since we're passing the name to this Document,
//   we may not need the IamPolicyDocument at all
//   TODO revist this
  processDocument(_policyList.at("PolicyDocument"),
                             _attachmentId, policyDocument.getPolicyName());
}

// processPolicyList : "PolicyDocument"
// processPolicyVersionList : "Document"
// processRoleDetails : "AssumeRolePolicyDocument"
void
Parser::processDocument(const json& _document,
                                      const std::string& _attachmentId,
                                      const std::string& _secondaryId) {
  LOG_DEBUG << "processDocument:\n";

  nmdoa::IamDocument document;
  document.setAttachmentId(_attachmentId);
  document.setSecondaryId(_secondaryId);
  document.setDocVersion(_document.value("Version", ""));

  auto& statements = _document.at("Statement");
  if(statements.is_array()) {
    for (const auto& statement : statements) {
        processStatement(statement,
                                   document.getAttachmentId(),
                                   document.getDocVersion());
    }
  } else if(statements.is_object()) {
    processStatement(statements, document.getAttachmentId(),
                               document.getDocVersion());
  }

  d.documents.push_back(document);
}

void
Parser::processStatement(const json& _statement,
                                      const std::string&_attachmentId,
                                      const std::string& _documentVersion) {
  LOG_DEBUG << "processStatement:\n";

  nmdoa::IamStatement statement;
  statement.setAttachmentId(_attachmentId);
  statement.setDocumentVersion(_documentVersion);
  statement.setEffect(_statement.value("Effect", ""));
  statement.setSid(_statement.value("Sid", ""));
  statement.setCondition(_statement.value("Condition", json({})));
  statement.setPrincipal(_statement.value("Principal", json({})));

  auto& action = _statement.at("Action");
  if(action.is_array()) {
      for(const auto& a : action) {
          statement.addAction(a);
      }
  } else {
      statement.addAction(action);
  }

  if(_statement.contains("Resource")) {
    auto& resource = _statement.at("Resource");
    if(resource.is_array()) {
        for(const auto& r : resource) {
            statement.addResource(r);
        }
    } else {
      statement.addResource(resource);
    }
  }

  d.statements.push_back(statement);
}

// Policies
void
Parser::processPolicyVersionList(const json& _policyVersion,
                                                const std::string& _policyId) {
  LOG_DEBUG << "processPolicyVersionList:\n";

  nmdoa::IamPolicyVersion version;
  version.setPolicyId(_policyId);
  version.setCreateDate(_policyVersion.value("CreateDate", ""));
  version.setIsDefaultVersion(_policyVersion.value("IsDefaultVersion", false));
  version.setVersionId(_policyVersion.value("VersionId", ""));
  d.policyVersions.push_back(version);

  processDocument(_policyVersion.at("Document"), _policyId, version.getVersionId());
}

// RoleDetail
void
Parser::processProfileList(const json& _profileList,
                                      const std::string& _parentRoleId) {
  LOG_DEBUG << "processProfileList:\n";

  nmdoa::IamRoleInstanceProfile profile;
  profile.setParentRoleId(_parentRoleId);
  profile.setArn(_profileList.value("Arn", ""));
  profile.setCreateDate(_profileList.value("CreateDate", ""));
  profile.setProfileId(_profileList.value("InstanceProfileId", ""));
  profile.setProfileName(_profileList.value("InstanceProfileName", ""));
  profile.setPath(_profileList.value("Path", ""));
  d.roleProfiles.push_back(profile);

  for (const auto& role : _profileList.at("Roles")) { // List
      processRoleDetails(role, profile.getProfileId());
  }
}

// RoleDetail
void
Parser::processPermissionsBoundary(const json& _permissionsBoundary,
                                                       const std::string& _roleId) {
  LOG_DEBUG << "processPermissionsBoundary:\n";

  nmdoa::IamRolePermissionBoundary boundary;
  boundary.setRoleId(_roleId);
  boundary.setBoundaryArn(
                  _permissionsBoundary.value("PermissionsBoundaryArn", ""));
  boundary.setBoundaryType(
                  _permissionsBoundary.value("PermissionsBoundaryType", ""));
  d.roleBoundaries.push_back(boundary);
}

// =============================================================================
// Parser helper methods
// =============================================================================

Result
Parser::getData()
{
  Result r;

  if (d != Data()) {
    r.emplace_back(d);
  }

  return r;
}