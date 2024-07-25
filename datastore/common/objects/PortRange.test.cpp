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

#include <netmeld/datastore/objects/PortRange.hpp>

namespace nmdo = netmeld::datastore::objects;

class TestPortRange : public nmdo::PortRange
{
  public:
    using nmdo::PortRange::PortRange;
};


BOOST_AUTO_TEST_CASE(testConstructorsAndToStrings)
{
  // Constructors with numeric arguments:

  {
    TestPortRange tpr {0};  // Min port number

    BOOST_TEST(0 == tpr.first);
    BOOST_TEST(0 == tpr.last);

    BOOST_TEST("[0,0]" == tpr.toString());
  }

  {
    TestPortRange tpr {65535};  // Max port number

    BOOST_TEST(65535 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[65535,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {22};

    BOOST_TEST(22 == tpr.first);
    BOOST_TEST(22 == tpr.last);

    BOOST_TEST("[22,22]" == tpr.toString());
  }

  {
    TestPortRange tpr {"ssh"};

    BOOST_TEST(22 == tpr.first);
    BOOST_TEST(22 == tpr.last);

    BOOST_TEST("[22,22]" == tpr.toString());
  }

  {
    TestPortRange tpr {0, 65535};  // Full port range

    BOOST_TEST(    0 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[0,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {8000, 9999};  // Small port range

    BOOST_TEST(8000 == tpr.first);
    BOOST_TEST(9999 == tpr.last);

    BOOST_TEST("[8000,9999]" == tpr.toString());
  }

  // Constructors with string arguments (including variations in whitespace):

  {
    TestPortRange tpr {"[0,65535]"};

    BOOST_TEST(    0 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[0,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {"[0, 65535)"};

    BOOST_TEST(    0 == tpr.first);
    BOOST_TEST(65534 == tpr.last);

    BOOST_TEST("[0,65534]" == tpr.toString());
  }

  {
    TestPortRange tpr {"( 0,65535 ]"};

    BOOST_TEST(    1 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[1,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {"( 0, 65535 )"};

    BOOST_TEST(    1 == tpr.first);
    BOOST_TEST(65534 == tpr.last);

    BOOST_TEST("[1,65534]" == tpr.toString());
  }

  {
    TestPortRange tpr {"1024-65535"};

    BOOST_TEST( 1024 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[1024,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {"1024 -- 65535"};

    BOOST_TEST( 1024 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[1024,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {"443"};

    BOOST_TEST(443 == tpr.first);
    BOOST_TEST(443 == tpr.last);

    BOOST_TEST("[443,443]" == tpr.toString());
  }

  {
    TestPortRange tpr {"https"};

    BOOST_TEST(443 == tpr.first);
    BOOST_TEST(443 == tpr.last);

    BOOST_TEST("[443,443]" == tpr.toString());
  }

  {
    TestPortRange tpr {"http-https"};

    BOOST_TEST( 80 == tpr.first);
    BOOST_TEST(443 == tpr.last);

    BOOST_TEST("[80,443]" == tpr.toString());
  }

  {
    TestPortRange tpr {">32767"};

    BOOST_TEST(32768 == tpr.first);
    BOOST_TEST(65535 == tpr.last);

    BOOST_TEST("[32768,65535]" == tpr.toString());
  }

  {
    TestPortRange tpr {"<32767"};

    BOOST_TEST(    0 == tpr.first);
    BOOST_TEST(32766 == tpr.last);

    BOOST_TEST("[0,32766]" == tpr.toString());
  }
}

BOOST_AUTO_TEST_CASE(testAssignment)
{
  {
    TestPortRange tpr1 {123,456}
                , tpr2
                ;

    BOOST_TEST(tpr1 != tpr2);
    tpr2 = tpr1;
    BOOST_TEST(tpr1 == tpr2);
    BOOST_TEST(123 == tpr2.first);
    BOOST_TEST(456 == tpr2.last);
  }

  {
    TestPortRange tpr1 {"ssh-https"}
                , tpr2
                ;

    BOOST_TEST(tpr1 != tpr2);
    tpr2 = tpr1;
    BOOST_TEST(tpr1 == tpr2);
    BOOST_TEST( 22 == tpr2.first);
    BOOST_TEST(443 == tpr2.last);
  }
}
