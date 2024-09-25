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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

class TestParser : public Parser {
    public:
    using Parser::fromJson;

    using Parser::processRoleDetails;
    using Parser::processGroupDetails;
    using Parser::processUserDetails;
    using Parser::processPolicy;

    using Parser::processManagedPolicy;
    using Parser::processPolicyList;
    using Parser::processDocument;
    using Parser::processStatement;
    using Parser::processPolicyVersionList;
    using Parser::processProfileList;
    using Parser::processPermissionsBoundary;
    using Parser::processRoleLastUsed;
    using Parser::processTags;
    using Parser::processGroupList;
};

BOOST_AUTO_TEST_CASE(testStatementDetails)
{
  { // TODO test multiple actions no actions, etc.
    TestParser tp;

    // RoleDetailList Statement
    json tv1 = json::parse(R"({
  "Sid": "Sid-0",
  "Effect": "Allow",
  "Principal": {
    "Service": "ec2.amazonaws.com"
  },
  "Action": "sts:AssumeRole"
})");

    json principal;
    principal["Service"] = "ec2.amazonaws.com";
    nmdoa::IamStatement statement(
      "Id1234", // attachmentId
      "2012-10-17", // documentVersion
      "Sid-0", // sid
      "Allow", // effects
      {"sts:AssumeRole"}, // actions
      std::vector<std::string>(), // resources
      principal, // principal
      json({}) // condition
    );
    tp.processStatement(tv1, std::string("Id1234"), std::string("2012-10-17"));

    BOOST_TEST(tp.d.statements.size() == 1);
    BOOST_TEST(statement == tp.d.statements.at(0));
  }
  {
    TestParser tp;

    json tv2 = json::parse(R"({
  "Effect": "Allow",
  "Action": [
      "iam:CreatePolicy",
      "iam:CreatePolicyVersion",
      "iam:DeletePolicy",
      "iam:DeletePolicyVersion",
      "iam:GetPolicy",
      "iam:GetPolicyVersion",
      "iam:ListPolicies",
      "iam:ListPolicyVersions",
      "iam:SetDefaultPolicyVersion"
  ],
  "Resource": "*"
})");

    nmdoa::IamStatement statement(
      "Id1234", // attachmentId
      "", // documentVersion
      "", // sid
      "Allow", // effects
      {
        "iam:CreatePolicy",
        "iam:CreatePolicyVersion",
        "iam:DeletePolicy",
        "iam:DeletePolicyVersion",
        "iam:GetPolicy",
        "iam:GetPolicyVersion",
        "iam:ListPolicies",
        "iam:ListPolicyVersions",
        "iam:SetDefaultPolicyVersion"
      }, // actions
      {"*"}, // resources
      json({}), // principal
      json({}) // condition
    );
    tp.processStatement(tv2, std::string("Id1234"), std::string());

    BOOST_TEST(tp.d.statements.size() == 1);
    BOOST_TEST(statement == tp.d.statements.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testDocumentDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
  "Version": "2012-10-17",
  "Statement": [
  ]
})");
    nmdoa::IamDocument document(
      "id1234", // attachmentId
      "id5678", // secondaryId
      "2012-10-17" // docVersion
    );
    tp.processDocument(tv1, std::string("id1234"), std::string("id5678"));

    BOOST_TEST(tp.d.documents.size() == 1);
    BOOST_TEST(document == tp.d.documents.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testAttachedManagedPolicyDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
  "PolicyName": "Random Policy Name",
  "PolicyArn": "arn:aws:iam::123456789012:user/Alice"
})");
    nmdoa::IamAttachedManagedPolicy amp(
      "attachment-id-0", // attachmentId
      "arn:aws:iam::123456789012:user/Alice", // policyArn
      "Random Policy Name"  // policyName
    );
    tp.processManagedPolicy(tv1, "attachment-id-0");

    BOOST_TEST(tp.d.amps.size() == 1);
    BOOST_TEST(amp == tp.d.amps.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testPolicyDocumentDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
  "PolicyName": "Random Name",
  "PolicyDocument": {
    "Version": "2012-10-17",
    "Statement": [
        {
            "Action": "aws-portal:*",
            "Sid": "Stmt1381777017000",
            "Resource": "*",
            "Effect": "Allow"
        }
    ]
  }
})");
    nmdoa::IamPolicyDocument pd(
      "attachment-id-0", // attachmentId
      "Random Name" // policyName
    );
    tp.processPolicyList(tv1, "attachment-id-0");

    BOOST_TEST(tp.d.policyDocuments.size() == 1);
    BOOST_TEST(pd == tp.d.policyDocuments.at(0));

    BOOST_TEST(tp.d.documents.size() == 1);
    // TODO Do we assume the document was parsed correctly?
  }
}
BOOST_AUTO_TEST_CASE(testPolicyVersionDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
  "CreateDate": "2015-02-06T19:58:34Z",
  "VersionId": "v1",
  "Document": {
    "Version": "2012-10-17",
    "Statement": {
        "Effect": "Allow",
        "Action": [
            "iam:CreatePolicy",
            "iam:CreatePolicyVersion",
            "iam:DeletePolicy",
            "iam:DeletePolicyVersion",
            "iam:GetPolicy",
            "iam:GetPolicyVersion",
            "iam:ListPolicies",
            "iam:ListPolicyVersions",
            "iam:SetDefaultPolicyVersion"
        ],
        "Resource": "*"
      }
    },
    "IsDefaultVersion": true
})");
    nmdoa::IamPolicyVersion pv(
      "policy-id-0", // policyId
      "v1", // versionId
      true, // isDefaultVersion
      "2015-02-06T19:58:34Z"  // createDate
    );
    tp.processPolicyVersionList(tv1, "policy-id-0");

    // TODO test Document as well
    BOOST_TEST(tp.d.policyVersions.size() == 1);
    BOOST_TEST(pv == tp.d.policyVersions.at(0));

    BOOST_TEST(tp.d.documents.size() == 1);
    BOOST_TEST(tp.d.statements.size() == 1);
  }
}
BOOST_AUTO_TEST_CASE(testRoleInstanceProfileDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
  "InstanceProfileId": "AIPA1234567890EXAMPLE",
  "Roles": [
      {
          "AssumeRolePolicyDocument": {
              "Version": "2012-10-17",
              "Statement": [
                  {
                      "Sid": "",
                      "Effect": "Allow",
                      "Principal": {
                          "Service": "ec2.amazonaws.com"
                      },
                      "Action": "sts:AssumeRole"
                  }
              ]
          },
          "RoleId": "AROA1234567890EXAMPLE",
          "CreateDate": "2014-07-30T17:09:20Z",
          "RoleName": "EC2role",
          "Path": "/",
          "Arn": "arn:aws:iam::123456789012:role/EC2role"
      }
  ],
  "CreateDate": "2014-07-30T17:09:20Z",
  "InstanceProfileName": "EC2role",
  "Path": "/",
  "Arn": "arn:aws:iam::123456789012:instance-profile/EC2role"
})");
    nmdoa::IamRoleInstanceProfile rip(
      "parent-role-id-0", // parentRoleId
      "AIPA1234567890EXAMPLE", // profileId
      "EC2role", // profileName
      "arn:aws:iam::123456789012:instance-profile/EC2role", // arn
      "2014-07-30T17:09:20Z", // createDate
      "/" // path
    );
    tp.processProfileList(tv1, "parent-role-id-0");

    BOOST_TEST(tp.d.roleProfiles.size() == 1);
    BOOST_TEST(rip == tp.d.roleProfiles.at(0));

    BOOST_TEST(tp.d.documents.size() == 1);
    // TODO do we assume the document parsed correctly?
  }
}
BOOST_AUTO_TEST_CASE(testRolePermissionBoundaryDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
  "PermissionsBoundaryType": "Policy",
  "PermissionsBoundaryArn": "arn:aws:iam:123456789012:user/Alice"
})");
    nmdoa::IamRolePermissionBoundary rpb(
      "role-id-0", // roleId
      "arn:aws:iam:123456789012:user/Alice", // boundaryArn
      "Policy"  // boundaryType
    );
    tp.processPermissionsBoundary(tv1, "role-id-0");

    BOOST_TEST(tp.d.roleBoundaries.size() == 1);
    BOOST_TEST(rpb == tp.d.roleBoundaries.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testUserDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
    "UserName": "Alice",
    "GroupList": [
        "Admins"
    ],
    "CreateDate": "2013-10-14T18:32:24Z",
    "UserId": "AIDA1234567890EXAMPLE",
    "UserPolicyList": [],
    "Path": "/",
    "AttachedManagedPolicies": [],
    "Arn": "arn:aws:iam::123456789012:user/Alice"
})");

    json tags = json::array();
    tags.push_back(json::object());
    nmdoa::IamUser user(
      "AIDA1234567890EXAMPLE",
      "arn:aws:iam::123456789012:user/Alice",
      "Alice",
      "2013-10-14T18:32:24Z",
      "/",
      tags
    );
    nmdoa::IamUserGroup userGroup(
      "AIDA1234567890EXAMPLE", // userId
      "Admins" // groupName
    );
    tp.processUserDetails(tv1);

    BOOST_TEST(tp.d.users.size() == 1);
    BOOST_TEST(user == tp.d.users.at(0));

    BOOST_TEST(tp.d.userGroups.size() == 1);
    BOOST_TEST(userGroup == tp.d.userGroups.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testRoleDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
    "AssumeRolePolicyDocument": {
        "Version": "2012-10-17",
        "Statement": [
            {
                "Sid": "",
                "Effect": "Allow",
                "Principal": {
                    "Service": "ec2.amazonaws.com"
                },
                "Action": "sts:AssumeRole"
            }
        ]
    },
    "RoleId": "AROA1234567890EXAMPLE",
    "CreateDate": "2014-07-30T17:09:20Z",
    "InstanceProfileList": [
        {
            "InstanceProfileId": "AIPA1234567890EXAMPLE",
            "Roles": [
                {
                    "AssumeRolePolicyDocument": {
                        "Version": "2012-10-17",
                        "Statement": [
                            {
                                "Sid": "",
                                "Effect": "Allow",
                                "Principal": {
                                    "Service": "ec2.amazonaws.com"
                                },
                                "Action": "sts:AssumeRole"
                            }
                        ]
                    },
                    "RoleId": "AROA1234567890EXAMPLE",
                    "CreateDate": "2014-07-30T17:09:20Z",
                    "RoleName": "EC2role",
                    "Path": "/",
                    "Arn": "arn:aws:iam::123456789012:role/EC2role"
                }
            ],
            "CreateDate": "2014-07-30T17:09:20Z",
            "InstanceProfileName": "EC2role",
            "Path": "/",
            "Arn": "arn:aws:iam::123456789012:instance-profile/EC2role"
        }
    ],
    "RoleName": "EC2role",
    "Path": "/",
    "AttachedManagedPolicies": [
        {
            "PolicyName": "AmazonS3FullAccess",
            "PolicyArn": "arn:aws:iam::aws:policy/AmazonS3FullAccess"
        },
        {
            "PolicyName": "AmazonDynamoDBFullAccess",
            "PolicyArn": "arn:aws:iam::aws:policy/AmazonDynamoDBFullAccess"
        }
    ],
    "RoleLastUsed": {
        "Region": "us-west-2",
        "LastUsedDate": "2019-11-13T17:30:00Z"
    },
    "RolePolicyList": [],
    "Arn": "arn:aws:iam::123456789012:role/EC2role"
})");

    json tags = json::array();
    tags.push_back(json::object());
    nmdoa::IamRole role(
      "AROA1234567890EXAMPLE",
      "arn:aws:iam::123456789012:role/EC2role",
      "EC2role",
      "2014-07-30T17:09:20Z",
      "2019-11-13T17:30:00Z",
      "/",
      0,
      tags
    );
    nmdoa::IamRole minorRole(
      "AROA1234567890EXAMPLE",
      "arn:aws:iam::123456789012:role/EC2role",
      "EC2role",
      "2014-07-30T17:09:20Z",
      "",
      "/",
      0,
      tags
    );
    tp.processRoleDetails(tv1);

    // There are two roles. The main one we are parsing is the last one added. The other one is InstanceProfileList[0].Roles[0]
    BOOST_TEST(tp.d.roles.size() == 2);
    BOOST_TEST(minorRole == tp.d.roles.at(0));
    BOOST_TEST(role == tp.d.roles.at(1));
  }
}
BOOST_AUTO_TEST_CASE(testGroupDetails)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
    "GroupId": "AIDA1234567890EXAMPLE",
    "AttachedManagedPolicies": [
        {
            "PolicyName": "AdministratorAccess",
            "PolicyArn": "arn:aws:iam::aws:policy/AdministratorAccess"
        }
    ],
    "GroupName": "Admins",
    "Path": "/",
    "Arn": "arn:aws:iam::123456789012:group/Admins",
    "CreateDate": "2013-10-14T18:32:24Z",
    "GroupPolicyList": []
})");

    json tags = json::array();
    tags.push_back(json::object());
    nmdoa::IamGroup group(
      "AIDA1234567890EXAMPLE",
      "arn:aws:iam::123456789012:group/Admins",
      "Admins",
      "2013-10-14T18:32:24Z",
      "/",
      tags
    );
    tp.processGroupDetails(tv1);
    BOOST_TEST(group == tp.d.groups.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testPolicies)
{
  {
    TestParser tp;

    json tv1 = json::parse(R"({
    "PolicyName": "create-update-delete-set-managed-policies",
    "CreateDate": "2015-02-06T19:58:34Z",
    "AttachmentCount": 1,
    "IsAttachable": true,
    "PolicyId": "ANPA1234567890EXAMPLE",
    "DefaultVersionId": "v1",
    "PolicyVersionList": [
        {
            "CreateDate": "2015-02-06T19:58:34Z",
            "VersionId": "v1",
            "Document": {
                "Version": "2012-10-17",
                "Statement": {
                    "Effect": "Allow",
                    "Action": [
                        "iam:CreatePolicy",
                        "iam:CreatePolicyVersion",
                        "iam:DeletePolicy",
                        "iam:DeletePolicyVersion",
                        "iam:GetPolicy",
                        "iam:GetPolicyVersion",
                        "iam:ListPolicies",
                        "iam:ListPolicyVersions",
                        "iam:SetDefaultPolicyVersion"
                    ],
                    "Resource": "*"
                }
            },
            "IsDefaultVersion": true
        }
    ],
    "Path": "/",
    "Arn": "arn:aws:iam::123456789012:policy/create-update-delete-set-managed-policies",
    "UpdateDate": "2015-02-06T19:58:34Z",
    "PermissionsBoundaryUsageCount": 2
})");

    json tags = json::array();
    tags.push_back(json::object());
    nmdoa::IamPolicy policy(
      "ANPA1234567890EXAMPLE", // id
      "arn:aws:iam::123456789012:policy/create-update-delete-set-managed-policies", // arn
      "create-update-delete-set-managed-policies", // name
      "2015-02-06T19:58:34Z", // createDate
      "/", // path
      tags, // tags
      "2015-02-06T19:58:34Z", // updateDate
      1, // attachmentCount
      "v1", // defaultVersionId
      true, // isAttachable
      2 // permissionsBoundaryUsageCount
    );
    tp.processPolicy(tv1);
    BOOST_TEST(policy == tp.d.policies.at(0));
  }
}
BOOST_AUTO_TEST_CASE(testWhole)
{
  TestParser tp;

  json tv1 = json::parse(R"(
  {
    "RoleDetailList": [
        {
            "AssumeRolePolicyDocument": {
                "Version": "2012-10-17",
                "Statement": [
                    {
                        "Sid": "",
                        "Effect": "Allow",
                        "Principal": {
                            "Service": "ec2.amazonaws.com"
                        },
                        "Action": "sts:AssumeRole"
                    }
                ]
            },
            "RoleId": "AROA1234567890EXAMPLE",
            "CreateDate": "2014-07-30T17:09:20Z",
            "InstanceProfileList": [
                {
                    "InstanceProfileId": "AIPA1234567890EXAMPLE",
                    "Roles": [
                        {
                            "AssumeRolePolicyDocument": {
                                "Version": "2012-10-17",
                                "Statement": [
                                    {
                                        "Sid": "",
                                        "Effect": "Allow",
                                        "Principal": {
                                            "Service": "ec2.amazonaws.com"
                                        },
                                        "Action": "sts:AssumeRole"
                                    }
                                ]
                            },
                            "RoleId": "AROA1234567890EXAMPLE",
                            "CreateDate": "2014-07-30T17:09:20Z",
                            "RoleName": "EC2role",
                            "Path": "/",
                            "Arn": "arn:aws:iam::123456789012:role/EC2role"
                        }
                    ],
                    "CreateDate": "2014-07-30T17:09:20Z",
                    "InstanceProfileName": "EC2role",
                    "Path": "/",
                    "Arn": "arn:aws:iam::123456789012:instance-profile/EC2role"
                }
            ],
            "RoleName": "EC2role",
            "Path": "/",
            "AttachedManagedPolicies": [
                {
                    "PolicyName": "AmazonS3FullAccess",
                    "PolicyArn": "arn:aws:iam::aws:policy/AmazonS3FullAccess"
                },
                {
                    "PolicyName": "AmazonDynamoDBFullAccess",
                    "PolicyArn": "arn:aws:iam::aws:policy/AmazonDynamoDBFullAccess"
                }
            ],
            "RoleLastUsed": {
                "Region": "us-west-2",
                "LastUsedDate": "2019-11-13T17:30:00Z"
            },
            "RolePolicyList": [],
            "Arn": "arn:aws:iam::123456789012:role/EC2role"
        }
    ],
    "GroupDetailList": [
        {
            "GroupId": "AIDA1234567890EXAMPLE",
            "AttachedManagedPolicies": [
                {
                    "PolicyName": "AdministratorAccess",
                    "PolicyArn": "arn:aws:iam::aws:policy/AdministratorAccess"
                }
            ],
            "GroupName": "Admins",
            "Path": "/",
            "Arn": "arn:aws:iam::123456789012:group/Admins",
            "CreateDate": "2013-10-14T18:32:24Z",
            "GroupPolicyList": []
        },
        {
            "GroupId": "AIDA1234567890EXAMPLE",
            "AttachedManagedPolicies": [
                {
                    "PolicyName": "PowerUserAccess",
                    "PolicyArn": "arn:aws:iam::aws:policy/PowerUserAccess"
                }
            ],
            "GroupName": "Dev",
            "Path": "/",
            "Arn": "arn:aws:iam::123456789012:group/Dev",
            "CreateDate": "2013-10-14T18:33:55Z",
            "GroupPolicyList": []
        },
        {
            "GroupId": "AIDA1234567890EXAMPLE",
            "AttachedManagedPolicies": [],
            "GroupName": "Finance",
            "Path": "/",
            "Arn": "arn:aws:iam::123456789012:group/Finance",
            "CreateDate": "2013-10-14T18:57:48Z",
            "GroupPolicyList": [
                {
                    "PolicyName": "policygen-201310141157",
                    "PolicyDocument": {
                        "Version": "2012-10-17",
                        "Statement": [
                            {
                                "Action": "aws-portal:*",
                                "Sid": "Stmt1381777017000",
                                "Resource": "*",
                                "Effect": "Allow"
                            }
                        ]
                    }
                }
            ]
        }
    ],
    "UserDetailList": [
        {
            "UserName": "Alice",
            "GroupList": [
                "Admins"
            ],
            "CreateDate": "2013-10-14T18:32:24Z",
            "UserId": "AIDA1234567890EXAMPLE",
            "UserPolicyList": [],
            "Path": "/",
            "AttachedManagedPolicies": [],
            "Arn": "arn:aws:iam::123456789012:user/Alice"
        },
        {
            "UserName": "Bob",
            "GroupList": [
                "Admins"
            ],
            "CreateDate": "2013-10-14T18:32:25Z",
            "UserId": "AIDA1234567890EXAMPLE",
            "UserPolicyList": [
                {
                    "PolicyName": "DenyBillingAndIAMPolicy",
                    "PolicyDocument": {
                        "Version": "2012-10-17",
                        "Statement": {
                            "Effect": "Deny",
                            "Action": [
                                "aws-portal:*",
                                "iam:*"
                            ],
                            "Resource": "*"
                        }
                    }
                }
            ],
            "Path": "/",
            "AttachedManagedPolicies": [],
            "Arn": "arn:aws:iam::123456789012:user/Bob"
        },
        {
            "UserName": "Charlie",
            "GroupList": [
                "Dev"
            ],
            "CreateDate": "2013-10-14T18:33:56Z",
            "UserId": "AIDA1234567890EXAMPLE",
            "UserPolicyList": [],
            "Path": "/",
            "AttachedManagedPolicies": [],
            "Arn": "arn:aws:iam::123456789012:user/Charlie"
        }
    ],
    "Policies": [
        {
            "PolicyName": "create-update-delete-set-managed-policies",
            "CreateDate": "2015-02-06T19:58:34Z",
            "AttachmentCount": 1,
            "IsAttachable": true,
            "PolicyId": "ANPA1234567890EXAMPLE",
            "DefaultVersionId": "v1",
            "PolicyVersionList": [
                {
                    "CreateDate": "2015-02-06T19:58:34Z",
                    "VersionId": "v1",
                    "Document": {
                        "Version": "2012-10-17",
                        "Statement": {
                            "Effect": "Allow",
                            "Action": [
                                "iam:CreatePolicy",
                                "iam:CreatePolicyVersion",
                                "iam:DeletePolicy",
                                "iam:DeletePolicyVersion",
                                "iam:GetPolicy",
                                "iam:GetPolicyVersion",
                                "iam:ListPolicies",
                                "iam:ListPolicyVersions",
                                "iam:SetDefaultPolicyVersion"
                            ],
                            "Resource": "*"
                        }
                    },
                    "IsDefaultVersion": true
                }
            ],
            "Path": "/",
            "Arn": "arn:aws:iam::123456789012:policy/create-update-delete-set-managed-policies",
            "UpdateDate": "2015-02-06T19:58:34Z"
        },
        {
            "PolicyName": "S3-read-only-specific-bucket",
            "CreateDate": "2015-01-21T21:39:41Z",
            "AttachmentCount": 1,
            "IsAttachable": true,
            "PolicyId": "ANPA1234567890EXAMPLE",
            "DefaultVersionId": "v1",
            "PolicyVersionList": [
                {
                    "CreateDate": "2015-01-21T21:39:41Z",
                    "VersionId": "v1",
                    "Document": {
                        "Version": "2012-10-17",
                        "Statement": [
                            {
                                "Effect": "Allow",
                                "Action": [
                                    "s3:Get*",
                                    "s3:List*"
                                ],
                                "Resource": [
                                    "arn:aws:s3:::example-bucket",
                                    "arn:aws:s3:::example-bucket/*"
                                ]
                            }
                        ]
                    },
                    "IsDefaultVersion": true
                }
            ],
            "Path": "/",
            "Arn": "arn:aws:iam::123456789012:policy/S3-read-only-specific-bucket",
            "UpdateDate": "2015-01-21T23:39:41Z"
        },
        {
            "PolicyName": "AmazonEC2FullAccess",
            "CreateDate": "2015-02-06T18:40:15Z",
            "AttachmentCount": 1,
            "IsAttachable": true,
            "PolicyId": "ANPA1234567890EXAMPLE",
            "DefaultVersionId": "v1",
            "PolicyVersionList": [
                {
                    "CreateDate": "2014-10-30T20:59:46Z",
                    "VersionId": "v1",
                    "Document": {
                        "Version": "2012-10-17",
                        "Statement": [
                            {
                                "Action": "ec2:*",
                                "Effect": "Allow",
                                "Resource": "*"
                            },
                            {
                                "Effect": "Allow",
                                "Action": "elasticloadbalancing:*",
                                "Resource": "*"
                            },
                            {
                                "Effect": "Allow",
                                "Action": "cloudwatch:*",
                                "Resource": "*"
                            },
                            {
                                "Effect": "Allow",
                                "Action": "autoscaling:*",
                                "Resource": "*"
                            }
                        ]
                    },
                    "IsDefaultVersion": true
                }
            ],
            "Path": "/",
            "Arn": "arn:aws:iam::aws:policy/AmazonEC2FullAccess",
            "UpdateDate": "2015-02-06T18:40:15Z"
        }
    ],
    "Marker": "EXAMPLEkakv9BCuUNFDtxWSyfzetYwEx2ADc8dnzfvERF5S6YMvXKx41t6gCl/eeaCX3Jo94/bKqezEAg8TEVS99EKFLxm3jtbpl25FDWEXAMPLE",
    "IsTruncated": true
}
)");
    //nmdoa::Vpc tobj, tev1;
    tp.fromJson(tv1);
    BOOST_TEST(true);
}
