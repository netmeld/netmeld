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
  std::cout << "fromJson\n";
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
Parser::processRoleDetails(const json& _roleDetail) { // Object
  std::cout << "processRoleDetails\n";
  std::cout << "\tPermissionsBoundaryUsageCount: " << _roleDetail.value("PermissionsBoundaryUsageCount", "") << '\n'; // number
  role.setId(_roleDetail.value("RoleId", ""));
  role.setArn(_roleDetail.value("Arn", ""));
  role.setName(_roleDetail.value("RoleName", ""));
  role.setCreateDate(_roleDetail.value("CreateDate", ""));
  role.setLastUsed(_roleDetail.value("RoleLastUsed", json({})).value("LastUsedDate", "")); // TODO clean this up
  role.setPath(_roleDetail.value("Path", ""));
  role.setTags(_roleDetail.value("Tags", json ({})));
  std::cout << "\t" << role.toDebugString() << "\n";

  processDocument(_roleDetail.at("AssumeRolePolicyDocument")); // Object
  if(_roleDetail.contains("AttachedManagedPolicies")) {
    for (const auto& policy : _roleDetail.at("AttachedManagedPolicies")) { // List
        processManagedPolicy(policy);
    }
  }
  if(_roleDetail.contains("InstanceProfileList")) {
    for (const auto& profile : _roleDetail.at("InstanceProfileList")) { // List
        processProfileList(profile);
    }
  }
  if(_roleDetail.contains("PermissionsBoundary")) {
    processPermissionsBoundary(_roleDetail["PermissionsBoundary"]); // Object
  }
  if(_roleDetail.contains("RolePolicyList")) {
    for (const auto& rolePolicy : _roleDetail.at("RolePolicyList")) { // List
        processPolicyList(rolePolicy);
    }
  }
  std::cout << "\tTags: " << _roleDetail.value("Tags", json ({})) << '\n'; // List
}

void
Parser::processGroupDetails(const json& _groupDetail) {
  std::cout << "processGroupDetails\n";
  group.setId(_groupDetail.value("GroupId", ""));
  group.setArn(_groupDetail.value("Arn", ""));
  group.setName(_groupDetail.value("GroupName", ""));
  group.setCreateDate(_groupDetail.value("CreateDate", ""));
  group.setPath(_groupDetail.value("Path", ""));
  group.setTags(_groupDetail.value("Tags", json({})));
  std::cout << "\t" << group.toDebugString() << "\n";

  if(_groupDetail.contains("AttachedManagedPolicies")) {
    for (const auto& policy : _groupDetail.at("AttachedManagedPolicies")) { // List
        processManagedPolicy(policy);
    }
  }
  if(_groupDetail.contains("GroupPolicyList")) {
    for (const auto& policy : _groupDetail.at("GroupPolicyList")) { // List
        processPolicyList(policy);
    }
  }
}

void
Parser::processUserDetails(const json& _userDetail) {
  std::cout << "processUserDetails\n";
  user.setId(_userDetail.value("UserId", ""));
  user.setArn(_userDetail.value("Arn", ""));
  user.setName(_userDetail.value("UserName", ""));
  user.setCreateDate(_userDetail.value("CreateDate", ""));
  user.setPath(_userDetail.value("Path", ""));
  user.setTags(_userDetail.value("Tags", json ({})));
  std::cout << "\t" << user.toDebugString() << "\n";

  if(_userDetail.contains("AttachedManagedPolicy")) {
    for (const auto& managedPolicy : _userDetail.at("AttachedManagedPolicy")) { // List
        processManagedPolicy(managedPolicy);
    }
  }
  if(_userDetail.contains("GroupList")) {
    for (const auto& group : _userDetail.at("GroupList")) { // List
        // Just a list of strings. Save them all?
    }
  }
  if(_userDetail.contains("Tags")) {
    for (const auto& tag : _userDetail.at("Tags")) { // List
        // We just want to save this json object
    }
  }
  if(_userDetail.contains("UserPolicyList")) {
    for (const auto& userPolicy : _userDetail.at("UserPolicyList")) { // List
        processPolicyList(userPolicy);
    }
  }
}

void
Parser::processPolicy(const json& _policy) {
  std::cout << "processPolicy\n";
  std::cout << "\tArn: " << _policy.value("Arn", "") << '\n'; // string
  std::cout << "\tAttachmentCount: " << _policy.value("AttachmentCount", 0) << '\n'; // number
  std::cout << "\tCreateDate: " << _policy.value("CreateDate", "") << '\n'; // string
  std::cout << "\tDefaultVersionId: " << _policy.value("DefaultVersionId", "") << '\n'; // string
  std::cout << "\tIsAttachable: " << _policy.value("IsAttachable", false) << '\n'; // boolean
  std::cout << "\tPath: " << _policy.value("Path", "") << '\n'; // string
  std::cout << "\tPermissionsBoundaryUsageCount: " << _policy.value("PermissionsBoundaryUsageCount", "") << '\n'; // number
  std::cout << "\tPolicyId: " << _policy.value("PolicyId", "") << '\n'; // string
  std::cout << "\tPolicyName: " << _policy.value("PolicyName", "") << '\n'; // string
  std::cout << "\tUpdateDate: " << _policy.value("UpdateDate", "") << '\n'; // string

  if(_policy.contains("PolicyVersionList")) {
    for (const auto& policyVersion : _policy.at("PolicyVersionList")) { // List
      processPolicyVersionList(policyVersion);
    }
  }
}

// GroupDetail, RoleDetail, UserDetail
void
Parser::processManagedPolicy(const json& _managedPolicy) {
  std::cout << "processManagedPolicy\n";
  std::cout << "\tmanagedPolicy = " << _managedPolicy << '\n';
  std::cout << "\tPolicyArn: " << _managedPolicy.value("PolicyArn", "") << '\n'; // string
  std::cout << "\tPolicyName: " << _managedPolicy.value("PolicyName", "") << '\n'; // string
}

// GroupDetail, RoleDetail, UserDetail
void
Parser::processPolicyList(const json& _policyList) {
  std::cout << "processPolicyList\n";
  std::cout << "\tPolicyName: " << _policyList.value("PolicyName", "") << '\n'; // string

  processDocument(_policyList.at("PolicyDocument"));
}

void
Parser::processDocument(const json& _document) {
  std::cout << "processDocument\n";
  std::cout << "\tVersion: " << _document.value("Version", "") << '\n'; // string
  auto& statements = _document.at("Statement");
  std::cout << "\tStatements: " << statements << '\n';
  if(statements.type() == json::value_t::array) {
    std::cout << "Statement type: array\n";
    for (const auto& statement : statements) { // List
        processStatement(statement);
    }
  } else if(statements.type() == json::value_t::object) {
    std::cout << "Statement type: object\n";
    processStatement(statements); // Object
  }
}

void
Parser::processStatement(const json& _statement) {
  std::cout << "processStatement\n";
  auto& action = _statement.at("Action");
  if(action.type() == json::value_t::array) {
    std::cout << "\tAction (List): " << action << '\n'; // List of strings
  } else {
    std::cout << "\tAction (String): " << action << '\n'; // string
  }
  std::cout << "\tEffect: " << _statement.value("Effect", "") << '\n'; // string
  if(_statement.contains("Resource")) {
    auto& resource = _statement.at("Resource");
    if(resource.type() == json::value_t::array) {
      std::cout << "\tResource (List): " << resource << '\n'; // List of strings
    } else {
      std::cout << "\tResource (String): " << resource << '\n'; // string
    }
  }
  std::cout << "\tSid: " << _statement.value("Sid", "") << '\n'; // string
  if(_statement.contains("Condition")) {
    for (const auto& condition : _statement.at("Condition")) { // Object
    }
  }
}

// Policies
void
Parser::processPolicyVersionList(const json& _policyVersion) {
  std::cout << "processPolicyVersionList\n";
  std::cout << "\tCreateDate: " << _policyVersion.value("CreateDate", "") << '\n'; // string
  std::cout << "\tIsDefaultVersion: " << _policyVersion.value("IsDefaultVersion", false) << '\n'; // boolean
  std::cout << "\tVersionId: " << _policyVersion.value("VersionId", "") << '\n'; // string
  processDocument(_policyVersion.at("Document"));
}

// RoleDetail
void
Parser::processProfileList(const json& _profileList) {
  std::cout << "processProfileList\n";
  std::cout << "\tArn: " << _profileList.value("Arn", "") << '\n'; // string
  std::cout << "\tCreateDate: " << _profileList.value("CreateDate", "") << '\n'; // string
  std::cout << "\tInstanceProfileId: " << _profileList.value("InstanceProfileId", "") << '\n'; // string
  std::cout << "\tInstanceProfileName: " << _profileList.value("InstanceProfileName", "") << '\n'; // string
  std::cout << "\tPath: " << _profileList.value("Path", "") << '\n'; // string

  for (const auto& role : _profileList.at("Roles")) { // List
      processRoleDetails(role); // Maybe? TODO
  }
}

// RoleDetail
void
Parser::processPermissionsBoundary(const json& _permissionsBoundary) {
  std::cout << "processPermissionsBoundary\n";
  std::cout << "\tPermissionsBoundaryArn: " << _permissionsBoundary.value("PermissionsBoundaryArn", "") << '\n'; // string
  std::cout << "\tPermissionsBoundaryType: " << _permissionsBoundary.value("PermissionsBoundaryType", "") << '\n'; // string
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