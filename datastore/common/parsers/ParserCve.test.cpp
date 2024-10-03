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

#include <netmeld/datastore/objects/Cve.hpp>
#include <netmeld/datastore/parsers/ParserCve.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;


BOOST_AUTO_TEST_CASE(testWellFormed)
{
  {
    nmdo::Cve cve {0, 0};
    auto pcve = nmdp::fromString<nmdp::ParserCve, nmdo::Cve>("CVE-0-0");
    BOOST_TEST(cve == pcve);
  }

  {
    nmdo::Cve cve {2000, 2000};
    auto pcve = nmdp::fromString<nmdp::ParserCve, nmdo::Cve>("CVE-2000-2000");
    BOOST_TEST(cve == pcve);
  }

  {
    nmdo::Cve cve {"CVE-1234-4321"};
    auto pcve = nmdp::fromString<nmdp::ParserCve, nmdo::Cve>("CVE-1234-4321");
    BOOST_TEST(cve == pcve);
  }

  std::string ymin = std::to_string(SHRT_MIN);
  std::string ymax = std::to_string(SHRT_MAX);
  std::string nmin = std::to_string(INT_MIN);
  std::string nmax = std::to_string(INT_MAX);
  {
    nmdo::Cve cve {SHRT_MIN, INT_MIN};
    std::string cveStr {"CVE-" + ymin + "-" + nmin};
    auto pcve = nmdp::fromString<nmdp::ParserCve, nmdo::Cve>(cveStr);
    BOOST_TEST(cve == pcve);
  }
  {
    nmdo::Cve cve {SHRT_MAX, INT_MAX};
    std::string cveStr {"CVE-" + ymax + "-" + nmax};
    auto pcve = nmdp::fromString<nmdp::ParserCve, nmdo::Cve>(cveStr);
    BOOST_TEST(cve == pcve);
  }
}

BOOST_AUTO_TEST_CASE(testMalformed)
{
  {
    std::vector<std::string> cves =
    {
       ""
    };
    for (const auto& cve : cves) {
      nmdo::Cve result;
      std::istringstream dataStream {cve};
      dataStream.unsetf(std::ios::skipws);
      nmdp::IstreamIter i {dataStream}, e;
      bool const success = qi::parse(i, e, nmdp::ParserCve(), result);

      BOOST_TEST(!success);
      BOOST_TEST((i == e));
    }
  }

  {
    std::string ymin1 = std::to_string(((int)SHRT_MIN)-1);
    std::string ymax1 = std::to_string(((int)SHRT_MAX)+1);
    std::string nmin1 = std::to_string(((long)INT_MIN)-1);
    std::string nmax1 = std::to_string(((long)INT_MAX)+1);
    std::vector<std::string> cves =
    {
       "CVE--"
      ,"CVE-x-y"
      ,std::string("CVE-" + ymax1 + "-" + nmax1)
      ,std::string("CVE-" + ymin1 + "-" + nmin1)
    };
    for (const auto& cve : cves) {
      nmdo::Cve result;
      std::istringstream dataStream {cve};
      dataStream.unsetf(std::ios::skipws);
      nmdp::IstreamIter i {dataStream}, e;
      bool const success = qi::parse(i, e, nmdp::ParserCve(), result);

      BOOST_TEST(!success);
      BOOST_TEST((i != e));
    }
  }
}
