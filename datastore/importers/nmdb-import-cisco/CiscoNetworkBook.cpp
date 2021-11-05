// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include "CiscoNetworkBook.hpp"

namespace nmdsic = netmeld::datastore::importers::cisco;

namespace netmeld::datastore::importers::cisco {
  // ===========================================================================
  // Parser logic
  // ===========================================================================
  CiscoNetworkBook::CiscoNetworkBook() : CiscoNetworkBook::base_type(start)
  {
    using nmdsic::token;
    using nmdsic::tokens;
    using nmdsic::indent;

    start =
      ciscoNetworkBook
       [(qi::_val = pnx::bind(&CiscoNetworkBook::getData, this))]
      ;

    ciscoNetworkBook =
      (  nameLine
       | objectNetwork
       | objectGroupNetwork
      ) [(pnx::bind(&CiscoNetworkBook::finalizeCurBook, this))]
      ;

    nameLine =
      qi::lit("name ") > dataIp > bookName > (description | qi::eol)
      ;

    objectNetwork =
      qi::lit("object network ") > bookName > qi::eol
      // cppcheck-suppress compareBoolExpressionWithInt
      > *(indent
          > (  objectNetworkHostLine
             | objectNetworkSubnetLine
             | objectNetworkRangeLine
             | objectNetworkNatLine
             | objectNetworkFqdnLine
             | description
             | (qi::eol) // space prefixed blank line
            )
         )
      ;
    objectNetworkHostLine =
      hostArgument > qi::eol
      ;
    objectNetworkSubnetLine =
      qi::lit("subnet ") > (dataIpMask | dataIpPrefix) > qi::eol
      ;
    objectNetworkRangeLine =
      qi::lit("range ") > dataIpRange > qi::eol
      ;
    objectNetworkNatLine =
      qi::lit("nat ") > tokens > qi::eol
      ;
    objectNetworkFqdnLine =
      qi::lit("fqdn ") > -(qi::lit("v4 ") | qi::lit("v6 "))
      > dataString > qi::eol
      ;

    objectGroupNetwork =
      qi::lit("object-group network ") > bookName > qi::eol
      // cppcheck-suppress compareBoolExpressionWithInt
      > *(indent
          > (  networkObjectLine
             | groupObjectLine
             | hostArgument
             | description
             | dataIpMask
             | (qi::eol) // space prefixed blank line
            )
         )
      ;
    networkObjectLine =
      qi::lit("network-object ")
      > (  objectArgument
         | hostArgument
         | dataNetworkObjectMask
         | (dataIpMask | dataIpPrefix)
      ) > qi::eol
      ;
    groupObjectLine =
      qi::lit("group-object ") > dataString > qi::eol
      ;


    //========
    // Helper piece-wise rules
    //========
    bookName =
      token [(pnx::bind([&](const std::string& _name)
                        {curBook.setName(_name);}, qi::_1))]
      ;

    description =
      qi::lit("description ") > tokens > qi::eol
      ;

    hostArgument =
      qi::lit("host ") > (dataIp | dataString)
      ;
    ipNoPrefix =
      (ipAddr.ipv4Addr.ipv4 | ipAddr.ipv6Addr.ipv6) >>
      !(qi::lit('/') >> ipAddr.ipv6Addr.prefix)
      ;
    dataIp =
      (&ipNoPrefix > ipAddr)
        [(pnx::bind(&CiscoNetworkBook::fromIp, this, qi::_1))]
      ;
    dataIpPrefix =
      (&((ipNoPrefix >> qi::eol) | (!ipNoPrefix)) >> ipAddr)
        [(pnx::bind(&CiscoNetworkBook::fromIp, this, qi::_1))]
      ;
    dataIpMask =
      (&ipNoPrefix >> ipAddr >> qi::omit[+qi::ascii::blank] >> &ipNoPrefix >> ipAddr)
        [(pnx::bind(&CiscoNetworkBook::fromIpMask, this, qi::_1, qi::_2))]
      ;
    dataIpRange =
      (ipAddr >> qi::omit[+qi::ascii::blank] >> ipAddr)
        [(pnx::bind(&CiscoNetworkBook::fromIpRange, this, qi::_1, qi::_2))]
      ;
    dataString =
      token [(pnx::bind(&CiscoNetworkBook::addData, this, qi::_1))]
      ;

    objectArgument =
      qi::lit("object ") > dataString
      ;
    dataNetworkObjectMask =
      (&(!ipAddr) >> token >> ipAddr)
        [(pnx::bind(&CiscoNetworkBook::fromNetworkObjectMask,
                    this, qi::_1, qi::_2))]
      ;



    BOOST_SPIRIT_DEBUG_NODES(
        //(start)
        (ciscoNetworkBook)
        (nameLine)
        (objectNetwork)
          (objectNetworkHostLine)
          (objectNetworkSubnetLine)
          (objectNetworkRangeLine)
          (objectNetworkNatLine)
        (objectGroupNetwork)
          (networkObjectLine)
          (groupObjectLine)
        (description)
        (objectArgument)
        (dataNetworkObjectMask)
        (bookName)
        (hostArgument)
        (dataIp)
        (dataIpPrefix)
        (dataIpMask)
        (dataIpRange)
        (dataString)
        (ipNoPrefix)
        //(token)(tokens)(indent)
        );
  }

  // ===========================================================================
  // Parser helper methods
  // ===========================================================================

  void
  CiscoNetworkBook::addData(const std::string& _data)
  {
    const auto& data {nmcu::trim(_data)};
    curBook.addData(data);
  }

  void
  CiscoNetworkBook::fromIp(const nmdo::IpAddress& _ip)
  {
    addData(_ip.toString());
  }
  void
  CiscoNetworkBook::fromIpMask(const nmdo::IpAddress& _ip,
                               const nmdo::IpAddress& _mask)
  {
    auto temp {_ip};
    auto maskStr {_mask.toString()};
    if ("0.0.0.0/32" == maskStr || "255.255.255.255/32" == maskStr) {
      temp.setNetmask(_mask);
      addData(temp.toString());
    } else {
      temp.setMask(_mask);
      addData(temp.toString());
    }
  }
  void
  CiscoNetworkBook::fromIpRange(const nmdo::IpAddress& _start,
                                const nmdo::IpAddress& _end)
  {
    std::ostringstream oss;
    oss << _start.toString() << "-" << _end.toString();
    addData(oss.str());
  }

  void
  CiscoNetworkBook::fromNetworkObjectMask(const std::string& _otherBook,
                                          const nmdo::IpAddress& _mask)
  {
    if (0 == networkBooks.count(_otherBook)) {
      LOG_WARN << "CiscoNetworkBook:"
               << " Cannot apply mask (" << _mask << ")"
               << " to undefined book (" << _otherBook << ")"
               << '\n';
      return;
    }
    for (const auto& ip : networkBooks[_otherBook].getData()) {
      if (std::string::npos != ip.find('-')) {
        LOG_WARN << "CiscoNetworkBook:"
                 << " Cannot apply mask (" << _mask << ")"
                 << " to network range (" << ip << ")"
                 << '\n';
        continue;
      }
      nmdo::IpAddress temp {ip};
      temp.setMask(_mask);
      addData(temp.toString());
    }
  }

  void
  CiscoNetworkBook::finalizeCurBook()
  {
    const auto& tgtBook {curBook.getName()};
    if (0 == networkBooks.count(tgtBook)) {
      networkBooks.emplace(curBook.getName(), curBook);
    } else {
      auto& existingBook {networkBooks.at(tgtBook)};
      for (auto& data : curBook.getData()) {
        existingBook.addData(data);
      }
    }
    curBook = nmdo::AcNetworkBook();
  }


  // Object return
  NetworkBooks
  CiscoNetworkBook::getFinalVersion()
  {
    NetworkBooks zoneBooks;
    zoneBooks.emplace(ZONE, networkBooks);
    for (const auto& [zone, books] : zoneBooks) {
      for (const auto& [name, book] : books) {
        nmdu::expanded(zoneBooks, zone, name, ZONE);
      }
    }

    return zoneBooks;
  }

  NetworkBooks
  CiscoNetworkBook::getData()
  {
    return NetworkBooks();
  }
} // end of namespace
