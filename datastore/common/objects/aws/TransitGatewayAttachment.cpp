// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/TransitGatewayAttachment.hpp>


namespace netmeld::datastore::objects::aws {

  TransitGatewayAttachment::TransitGatewayAttachment()
  {}

  void
  TransitGatewayAttachment::setTgwId(const std::string& _id)
  {
    tgwId = _id;
  }
  void
  TransitGatewayAttachment::setTgwOwnerId(const std::string& _id)
  {
    tgwOwnerId = _id;
  }
  void
  TransitGatewayAttachment::setTgwAttachmentId(const std::string& _id)
  {
    tgwAttachmentId = _id;
  }
  void
  TransitGatewayAttachment::setResourceId(const std::string& _id)
  {
    resourceId = _id;
  }
  void
  TransitGatewayAttachment::setResourceOwnerId(const std::string& _id)
  {
    resourceOwnerId = _id;
  }
  void
  TransitGatewayAttachment::setResourceType(const std::string& _type)
  {
    resourceType = _type;
  }
  void
  TransitGatewayAttachment::setState(const std::string& _state)
  {
    state = _state;
  }
  void
  TransitGatewayAttachment::setAssociationState(const std::string& _state)
  {
    associationState = _state;
  }

  bool
  TransitGatewayAttachment::isValid() const
  {
    return !(  tgwAttachmentId.empty()
            || tgwId.empty()
            || state.empty()
            );
  }

  void
  TransitGatewayAttachment::save(pqxx::transaction_base& t,
                   const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS TransitGatewayAttachment object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_transit_gateway"
        , toolRunId
        , tgwId
      );

    t.exec_prepared("insert_raw_aws_transit_gateway_attachment"
        , toolRunId
        , tgwId
        , tgwAttachmentId
        , state
      );

    if (!tgwOwnerId.empty()) {
      t.exec_prepared("insert_raw_aws_transit_gateway_owner"
          , toolRunId
          , tgwId
          , tgwOwnerId
        );
    }

    bool hasDetails {
        !(  resourceType.empty()
         || resourceId.empty()
         || resourceOwnerId.empty()
         || associationState.empty()
         )
      };
    if (hasDetails) {
      t.exec_prepared("insert_raw_aws_transit_gateway_attachment_detail"
          , toolRunId
          , tgwId
          , tgwAttachmentId
          , resourceType
          , resourceId
          , resourceOwnerId
          , associationState
        );
    }
  }

  std::string
  TransitGatewayAttachment::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "tgwAttachmentId: " << tgwAttachmentId << ", "
        << "tgwId: " << tgwId << ", "
        << "tgwOwnerId: " << tgwOwnerId << ", "
        << "resourceId: " << resourceId << ", "
        << "resourceOwnerId: " << resourceOwnerId << ", "
        << "resourceType: " << resourceType << ", "
        << "state: " << state << ", "
        << "associationState: " << associationState
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  TransitGatewayAttachment::operator<=>(const TransitGatewayAttachment& rhs) const
  {
    return std::tie( tgwAttachmentId
                   , tgwId
                   , tgwOwnerId
                   , resourceId
                   , resourceOwnerId
                   , resourceType
                   , state
                   , associationState
                   )
       <=> std::tie( rhs.tgwAttachmentId
                   , rhs.tgwId
                   , rhs.tgwOwnerId
                   , rhs.resourceId
                   , rhs.resourceOwnerId
                   , rhs.resourceType
                   , rhs.state
                   , rhs.associationState
                   )
      ;
  }

  bool
  TransitGatewayAttachment::operator==(const TransitGatewayAttachment& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
