// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/Attachment.hpp>


namespace netmeld::datastore::objects::aws {

  Attachment::Attachment()
  {}

  void
  Attachment::setId(const std::string& _id)
  {
    attachmentId = _id;
  }
  void
  Attachment::setStatus(const std::string& _status)
  {
    status = _status;
  }
  void
  Attachment::enableDeleteOnTermination()
  {
    deleteOnTermination = true;
  }
  void
  Attachment::disableDeleteOnTermination()
  {
    deleteOnTermination = false;
  }

  bool
  Attachment::isValid() const
  {
    return !(attachmentId.empty() || status.empty());
  }

  void
  Attachment::save(pqxx::transaction_base& t,
                   const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS Attachment object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_network_interface_attachment"
        , toolRunId
        , deviceId
        , attachmentId
        , status
        , deleteOnTermination
      );
  }

  std::string
  Attachment::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "attachmentId: " << attachmentId << ", " 
        << "status: " << status << ", " 
        << "deleteOnTermination: " << deleteOnTermination
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  Attachment::operator<=>(const Attachment& rhs) const
  {
    return std::tie( attachmentId
                   , status
                   , deleteOnTermination
                   )
       <=> std::tie( rhs.attachmentId
                   , rhs.status
                   , rhs.deleteOnTermination
                   )
      ;
  }

  bool
  Attachment::operator==(const Attachment& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
