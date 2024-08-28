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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <nlohmann/json.hpp>

#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/aws/IamAttachedManagedPolicy.hpp>   // processManagedPolicy
#include <netmeld/datastore/objects/aws/IamDocument.hpp>                       // processDocument
#include <netmeld/datastore/objects/aws/IamGroup.hpp>                             // processGroupDetails
#include <netmeld/datastore/objects/aws/IamPolicyDocument.hpp>               // processPolicyList
#include <netmeld/datastore/objects/aws/IamPolicy.hpp>                             // processPolicy
#include <netmeld/datastore/objects/aws/IamPolicyVersion.hpp>                  // processPolicyVersionList
#include <netmeld/datastore/objects/aws/IamRole.hpp>                               // processRoleDetails
#include <netmeld/datastore/objects/aws/IamRoleInstanceProfile.hpp>          // processProfileList
#include <netmeld/datastore/objects/aws/IamRolePermissionBoundary.hpp>  // processPermissionsBoundary
#include <netmeld/datastore/objects/aws/IamStatement.hpp>                       // processStatement
#include <netmeld/datastore/objects/aws/IamUserGroup.hpp>                      // processUserDetails
#include <netmeld/datastore/objects/aws/IamUser.hpp>                               // processUserDetails

namespace nmdo = netmeld::datastore::objects;
namespace nmdoa = netmeld::datastore::objects::aws;

using json = nlohmann::json;


// =============================================================================
// Data containers
// =============================================================================
struct Data {
  std::vector<nmdoa::IamUser> users;
  std::vector<nmdoa::IamGroup> groups;
  std::vector<nmdoa::IamRole> roles;
  std::vector<nmdoa::IamPolicy> policies;
  std::vector<nmdoa::IamDocument> documents;
  std::vector<nmdoa::IamStatement> statements;
  std::vector<nmdoa::IamPolicyDocument> policyDocuments;
  std::vector<nmdoa::IamAttachedManagedPolicy> amps;
  std::vector<nmdoa::IamUserGroup> userGroups;

  auto operator<=>(const Data&) const = default;
  bool operator==(const Data&) const = default;
};
typedef std::vector<Data>    Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables are always private
  public:
    Data d;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
  protected:
    void processRoleDetails(const json&);
    void processGroupDetails(const json&);
    void processUserDetails(const json&);
    void processPolicy(const json&);

    void processManagedPolicy(const json&);
    void processPolicyList(const json&, const std::string&);
    void processDocument(const json&, const std::string&, const std::string&);
    void processStatement(const json&, const std::string&, const std::string&);
    void processPolicyVersionList(const json&, const std::string&);
    void processProfileList(const json&);
    void processPermissionsBoundary(const json&);
    void processRoleLastUsed(const json&);
    void processTags(const json&);
    void processGroupList(const json&);

  public:
    void fromJson(const json&);
    Result getData();
};
#endif // PARSER_HPP
