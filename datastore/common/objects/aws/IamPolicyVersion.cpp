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

#include <netmeld/datastore/objects/aws/IamPolicyVersion.hpp>


namespace netmeld::datastore::objects::aws {

  IamPolicyVersion::IamPolicyVersion(const std::string& _policyId,
                const std::string& _versionId,
                const bool _isDefaultVersion,
                const std::string& _createDate) :
    policyId(_policyId),
    versionId(_versionId),
    isDefaultVersion(_isDefaultVersion),
    createDate(_createDate)
  {}

  IamPolicyVersion::IamPolicyVersion()
  {}

  void
  IamPolicyVersion::setPolicyId(const std::string& _policyId) {
    policyId = _policyId;
  }

  void
  IamPolicyVersion::setVersionId(const std::string& _versionId) {
    versionId = _versionId;
  }

  void
  IamPolicyVersion::setIsDefaultVersion(const bool _isDefaultVersion) {
    isDefaultVersion = _isDefaultVersion;
  }

  void
  IamPolicyVersion::setCreateDate(const std::string& _createDate) {
    createDate = _createDate;
  }

  std::string
  IamPolicyVersion::getPolicyId() const {
    return policyId;
  }

  std::string
  IamPolicyVersion::getVersionId() const {
    return versionId;
  }

  bool
  IamPolicyVersion::getIsDefaultVersion() const {
    return isDefaultVersion;
  }

  std::string
  IamPolicyVersion::getCreateDate() const {
    return createDate;
  }

  bool
  IamPolicyVersion::isValid() const {
      return false;
  }

  std::string
  IamPolicyVersion::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "policyId: " << policyId << ", "
        << "versionId: " << versionId << ", "
        << "isDefaultVersion: " << isDefaultVersion << ", "
        << "createDate: " << createDate;
    oss << "]";

    return oss.str();
  }

  auto
  IamPolicyVersion::operator<=>(const IamPolicyVersion& rhs) const {
    return std::tie(policyId
                      , versionId
                      , isDefaultVersion
                      , createDate
                      )
           <=> std::tie(rhs.policyId
                          , rhs.versionId
                          , rhs.isDefaultVersion
                          , rhs.createDate
                          )
    ;
  }

  bool
  IamPolicyVersion::operator==(const IamPolicyVersion& rhs) const {
      return 0 == operator<=>(rhs);
  }

  void
  IamPolicyVersion::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamPolicyVersion object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
