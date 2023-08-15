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

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/IpNetwork.hpp>

#include <netmeld/datastore/objects/aws/CidrBlock.hpp>


namespace netmeld::datastore::objects::aws {

  CidrBlock::CidrBlock()
  {}

  CidrBlock::CidrBlock(const std::string& _cidr)
  {
    setCidrBlock(_cidr);
  }

  void
  CidrBlock::setCidrBlock(const std::string& _cidr)
  {
    cidrBlock = _cidr;
  }
  void
  CidrBlock::setState(const std::string& _state)
  {
    state = _state;
  }
  void
  CidrBlock::setDescription(const std::string& _desc)
  {
    description = _desc;
  }
  void
  CidrBlock::addAlias(const std::string& _alias)
  {
    if (_alias.empty()) { return; } // Don't add empties
    aliases.insert(_alias);
  }

  std::string
  CidrBlock::getCidrBlock() const
  {
    return cidrBlock;
  }

  bool
  CidrBlock::isValid() const
  {
    return !(cidrBlock.empty())
        ;
  }

  void
  CidrBlock::save(pqxx::transaction_base& t,
                  const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS CidrBlock object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_cidr_block"
        , toolRunId
        , cidrBlock
      );

    nmdo::IpAddress ipa {cidrBlock};
    if (ipa.isValid()) {
      for (const auto& alias : aliases) {
        ipa.addAlias(alias, "AWS CidrBlock");
      }
      ipa.save(t, toolRunId, "");
    } else {
      nmdo::IpNetwork ipn {cidrBlock};
      ipn.save(t, toolRunId, "");
    }

    bool hasDetails {
        !(state.empty())
      };
    if (hasDetails) {
      t.exec_prepared("insert_raw_aws_cidr_block_detail"
          , toolRunId
          , cidrBlock
          , state
          , description
        );
    }

    for (const auto& alias : aliases) {
      t.exec_prepared("insert_raw_aws_cidr_block_fqdn"
          , toolRunId
          , cidrBlock
          , alias
        );
    }
  }

  std::string
  CidrBlock::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "cidrBlock: " << cidrBlock << ", "
        << "state: " << state << ", "
        << "description: " << description << ", "
        << "aliases: " << aliases
        << ']'
        ;

    return oss.str();
  }

  std::string
  CidrBlock::toString() const
  {
    return cidrBlock;
  }

  std::strong_ordering
  CidrBlock::operator<=>(const CidrBlock& rhs) const
  {
    return std::tie( cidrBlock
                   , state
                   , description
                   , aliases
                   )
       <=> std::tie( rhs.cidrBlock
                   , rhs.state
                   , rhs.description
                   , rhs.aliases
                   )
      ;
  }

  bool
  CidrBlock::operator==(const CidrBlock& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
