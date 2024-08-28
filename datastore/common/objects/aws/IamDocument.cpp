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

#include <netmeld/datastore/objects/aws/IamDocument.hpp>


namespace netmeld::datastore::objects::aws {

  IamDocument::IamDocument(const std::string& _attachmentId,
                const std::string& _secondaryId,
                const std::string& _docVersion) :
    attachmentId(_attachmentId),
    secondaryId(_secondaryId),
    docVersion(_docVersion)
  {}

  IamDocument::IamDocument()
  {}

  void
  IamDocument::setAttachmentId(const std::string& _attachmentId) {
      attachmentId = _attachmentId;
  }

  void
  IamDocument::setSecondaryId(const std::string& _secondaryId) {
      secondaryId = _secondaryId;
  }

  void
  IamDocument::setDocVersion(const std::string& _docVersion) {
    docVersion = _docVersion;
  }

  std::string
  IamDocument::getAttachmentId() const {
      return attachmentId;
  }

  std::string
  IamDocument::getSecondaryId() const {
      return secondaryId;
  }

  std::string
  IamDocument::getDocVersion() const {
    return docVersion;
  }

  bool
  IamDocument::isValid() const {
      return false;
  }

  std::string
  IamDocument::toDebugString() const {
    std::ostringstream oss;

    oss << "[";
    oss << "attachmentId: " << attachmentId << ", "
        << "docVersion: " << docVersion << ", ";
    oss << "]";

    return oss.str();
  }

  auto
  IamDocument::operator<=>(const IamDocument& rhs) const
  {
    return std::tie(attachmentId
                        , docVersion
                        )
              <=> std::tie(rhs.attachmentId
                              , rhs.docVersion
                              )
    ;
  }

  void
  IamDocument::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamDocument object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
