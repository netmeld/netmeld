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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;


BOOST_AUTO_TEST_CASE(testWellFormed)
{
  {
    std::string mac {"00:11:22:33:44:55"};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>(mac);
    BOOST_TEST(ma == pma);
  }

  {
    std::vector<uint8_t> mac {0,17,34,51,68,85};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>
      ("00:11:22:33:44:55");
    BOOST_TEST(ma == pma);
  }

  {
    std::string mac {"00:11:22:33:44:55:66:77"};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>(mac);
    BOOST_TEST(ma == pma);
  }

  {
    std::vector<uint8_t> mac {0,17,34,51,68,85,102,119};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>
      ("00:11:22:33:44:55:66:77");
    BOOST_TEST(ma == pma);
  }

  {
    std::string mac {"00:00:00:00:00:00"};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>(mac);
    BOOST_TEST(ma == pma);
  }

  {
    std::vector<uint8_t> mac {0,0,0,0,0,0};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>
      ("00:00:00:00:00:00");
    BOOST_TEST(ma == pma);
  }

  {
    std::string mac {"FF:FF:FF:FF:FF:FF"};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>(mac);
    BOOST_TEST(ma == pma);
  }

  {
    std::vector<uint8_t> mac {255,255,255,255,255,255};
    nmdo::MacAddress ma {mac};
    auto pma = nmdp::fromString<nmdp::ParserMacAddress, nmdo::MacAddress>
      ("ff:FF:ff:FF:ff:FF");
    BOOST_TEST(ma == pma);
  }

  {
    std::vector<std::string> mas =
    {
       "00-11-22-33-44-55"
      ,"00-11-22-33-44-55-66-77"
      ,"0011.2233.4455"
      ,"0011.2233.4455.6677"
    };
    for (const auto& ma : mas) {
      nmdo::MacAddress result;
      std::istringstream dataStream {ma};
      dataStream.unsetf(std::ios::skipws);
      nmdp::IstreamIter i {dataStream}, e;
      bool const success = qi::parse(i, e, nmdp::ParserMacAddress(), result);

      BOOST_TEST(success);
      BOOST_TEST((i == e));
    }
  }
}

BOOST_AUTO_TEST_CASE(testMalformed)
{
  {
    std::vector<std::string> mas =
    {
       ""
    };
    for (const auto& ma : mas) {
      nmdo::MacAddress result;
      std::istringstream dataStream {ma};
      dataStream.unsetf(std::ios::skipws);
      nmdp::IstreamIter i {dataStream}, e;
      bool const success = qi::parse(i, e, nmdp::ParserMacAddress(), result);

      BOOST_TEST(!success);
      BOOST_TEST((i == e));
    }
  }

  {
    std::vector<std::string> mas =
    {
       "00"
      ,"00:11:22:33:44"
      ,"00:00:00:00:00:0G"
      ,"00-11-22-33-44"
      ,"00-00-00-00-00-0G"
      ,"0000.0000.000"
      ,"0000.0000.000G"
    };
    for (const auto& ma : mas) {
      nmdo::MacAddress result;
      std::istringstream dataStream {ma};
      dataStream.unsetf(std::ios::skipws);
      nmdp::IstreamIter i {dataStream}, e;
      bool const success = qi::parse(i, e, nmdp::ParserMacAddress(), result);

      BOOST_TEST(!success);
      BOOST_TEST((i != e));
    }
  }

  {
    std::vector<std::string> mas =
    {
       "00:11:22:33:44:55:66"
      ,"00:11:22:33:44:55:66:77:88"
      ,"00-11-22-33-44-55-66"
      ,"00-11-22-33-44-55-66-77-88"
      ,"0011.2233.4455.66"
      ,"0011.2233.4455.6677.88"
    };
    for (const auto& ma : mas) {
      nmdo::MacAddress result;
      std::istringstream dataStream {ma};
      dataStream.unsetf(std::ios::skipws);
      nmdp::IstreamIter i {dataStream}, e;
      bool const success = qi::parse(i, e, nmdp::ParserMacAddress(), result);

      BOOST_TEST(success);
      BOOST_TEST((i != e));
    }
  }
}
