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

#include <netmeld/datastore/objects/aws/Vpc.hpp>


namespace netmeld::datastore::objects::aws {

  Vpc::Vpc()
  {}

  void
  Vpc::setId(const std::string& _id)
  {
    vpcId = _id;
  }
  void
  Vpc::setOwnerId(const std::string& _id)
  {
    ownerId = _id;
  }
  void
  Vpc::setState(const std::string& _state)
  {
    state = _state;
  }
  void
  Vpc::addCidrBlock(const CidrBlock& _cidr)
  {
    CidrBlock t;
    if (t == _cidr) { return; } // Don't add empties
    cidrBlocks.insert(_cidr);
  }

  std::string
  Vpc::getId() const
  {
    return vpcId;
  }
  std::string
  Vpc::getOwnerId() const
  {
    return ownerId;
  }


  bool
  Vpc::isValid() const
  {
    return !(vpcId.empty() || ownerId.empty());
  }

  void
  Vpc::save(pqxx::transaction_base& t,
            const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS Vpc object is not saving: "
                << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_vpc"
        , toolRunId
        , vpcId
      );

    t.exec_prepared("insert_raw_aws_vpc_owner"
        , toolRunId
        , vpcId
        , ownerId
      );

    if (!state.empty()) {
      t.exec_prepared("insert_raw_aws_vpc_detail"
          , toolRunId
          , vpcId
          , state
        );
    }

    for (auto cidr : cidrBlocks) {
      cidr.save(t, toolRunId, vpcId);

      t.exec_prepared("insert_raw_aws_vpc_cidr_block"
          , toolRunId
          , vpcId
          , cidr.toString()
          , state
        );
    }
  }

  std::string
  Vpc::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "vpcId: " << vpcId
        << ", ownerId: " << ownerId
        << ", state: " << state
        << ", cidrBlocks: " << cidrBlocks
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  Vpc::operator<=>(const Vpc& rhs) const
  {
    return std::tie( vpcId
                   , ownerId
                   , state
                   , cidrBlocks
                   )
       <=> std::tie( rhs.vpcId
                   , rhs.ownerId
                   , rhs.state
                   , rhs.cidrBlocks
                   )
      ;
  }

  bool
  Vpc::operator==(const Vpc& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
