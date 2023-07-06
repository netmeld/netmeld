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

#include <netmeld/datastore/objects/aws/VpcPeeringConnection.hpp>


namespace netmeld::datastore::objects::aws {

  VpcPeeringConnection::VpcPeeringConnection()
  {}

  void
  VpcPeeringConnection::setId(const std::string& _id)
  {
    pcxId = _id;
  }
  void
  VpcPeeringConnection::setStatus( const std::string& _code
                                 , const std::string& _message
                                 )
  {
    statusCode = _code;
    statusMessage = _message;
  }
  void
  VpcPeeringConnection::setAccepter(const Vpc& _vpc)
  {
    accepter = _vpc;
  }
  void
  VpcPeeringConnection::setRequester(const Vpc& _vpc)
  {
    requester = _vpc;
  }


  bool
  VpcPeeringConnection::isValid() const
  {
    return !(pcxId.empty())
           && (accepter.isValid() && requester.isValid())
           ;
  }

  void
  VpcPeeringConnection::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS VpcPeeringConnection object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_vpc_peering_connection"
        , toolRunId
        , pcxId
      );

    t.exec_prepared("insert_raw_aws_vpc_peering_connection_peer"
        , toolRunId
        , pcxId
        , accepter.getId()
        , accepter.getOwnerId()
        , requester.getId()
        , requester.getOwnerId()
      );

    if (!statusCode.empty()) {
      t.exec_prepared("insert_raw_aws_vpc_peering_connection_status"
          , toolRunId
          , pcxId
          , statusCode
          , statusMessage
        );
    }

    accepter.save(t, toolRunId);
    requester.save(t, toolRunId);
  }

  std::string
  VpcPeeringConnection::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "pcxId: " << pcxId << ", "
        << "statusCode: " << statusCode << ", "
        << "statusMessage: " << statusMessage << ", "
        << "accepter: " << accepter << ", "
        << "requester: " << requester
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  VpcPeeringConnection::operator<=>(const VpcPeeringConnection& rhs) const
  {
    return std::tie( pcxId
                   , statusCode
                   , statusMessage
                   , accepter
                   , requester
                   )
       <=> std::tie( rhs.pcxId
                   , rhs.statusCode
                   , rhs.statusMessage
                   , rhs.accepter
                   , rhs.requester
                   )
      ;
  }

  bool
  VpcPeeringConnection::operator==(const VpcPeeringConnection& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
