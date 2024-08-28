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

#include <netmeld/datastore/objects/aws/IamPolicy.hpp>


namespace netmeld::datastore::objects::aws {

  IamPolicy::IamPolicy(const std::string& _id,
                const std::string& _arn,
                const std::string& _name,
                const std::string& _createDate,
                const std::string& _path,
                const json& _tags,
                const std::string& _updateDate,
                const int _attachmentCnt,
                const std::string& _defaultVersionId,
                const bool _isAttachable,
                const int _permissionsBoundaryUsageCount) :
    IamBase(_id, _arn, _name, _createDate, _path, _tags),
    updateDate(_updateDate),
    attachmentCnt(_attachmentCnt),
    defaultVersionId(_defaultVersionId),
    isAttachable(_isAttachable),
    permissionsBoundaryUsageCount(_permissionsBoundaryUsageCount)
  {}

  IamPolicy::IamPolicy()
  {}

  void
  IamPolicy::setUpdateDate(const std::string& _updateDate)
  {
    updateDate = _updateDate;
  }

  void
  IamPolicy::setAttachmentCount(const int _attachmentCnt)
  {
    attachmentCnt = _attachmentCnt;
  }

  void
  IamPolicy::setDefaultVersionId(const std::string& _defaultVersionId)
  {
    defaultVersionId = _defaultVersionId;
  }

  void
  IamPolicy::setIsAttachable(const bool _isAttachable)
  {
    isAttachable = _isAttachable;
  }

  void
  IamPolicy::setPermissionsBoundaryUsageCount(const int _permissionsBoundaryUsageCount)
  {
    permissionsBoundaryUsageCount = _permissionsBoundaryUsageCount;
  }

  std::string
  IamPolicy::getUpdateDate() const
  {
    return updateDate;
  }

  int
  IamPolicy::getAttachmentCount() const
  {
    return attachmentCnt;
  }

  std::string
  IamPolicy::getDefaultVersionId() const
  {
    return defaultVersionId;
  }

  bool
  IamPolicy::getIsAttachable() const
  {
    return isAttachable;
  }

  int
  IamPolicy::getPermissionsBoundaryUsageCount() const
  {
    return permissionsBoundaryUsageCount;
  }

  void
  IamPolicy::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "IamPolicy object is not saving: " << toDebugString()
                << std::endl;
      return;
    }
  }
}
