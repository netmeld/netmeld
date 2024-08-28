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
                const std::string& _createDate,
                const std::string& _versionId) :
    attachmentId(_attachmentId),
    createDate(_createDate),
    versionId(_versionId)
  {}

  IamPolicyDocument::IamPolicyDocument()
  {}

  void
  IamPolicyDocument::setAttachmentId(const std::string& _attachmentId) {
    attachmentId = _attachmentId;
  }

  void
  IamPolicyDocument::setCreateDate(const std::string& _createDate) {
    createDate = _createDate;
  }

  void
  IamPolicyDocument::setVersionId(const std::string& _versionId) {
    versionId = _versionId;
  }

  std::string
  IamPolicyDocument::getAttachmentId() const {
    return attachmentId;
  }

  std::string
  IamPolicyDocument::getCreateDate() const {
    return createDate;
  }

  std::string
  IamPolicyDocument::getVersionId() const {
    return versionId;
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
        << "createDate: " << createDate << ", "
        << "versionId: " << versionId;
    oss << "]";

    return oss.str();
  }


  auto
  IamPolicyDocument::operator<=>(const IamPolicyDocument& rhs) const {
    return std::tie(attachmentId
                      , createDate
                      , versionId
                      )
           <=> std::tie(rhs.attachmentId
                          , rhs.createDate
                          , rhs.versionId
                          )
    ;
}

  bool
  IamPolicyDocument::operator==(const IamPolicyDocument& rhs) const
  {
    return 0 == operator<=>(rhs);
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
