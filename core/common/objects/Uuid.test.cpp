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

#include <netmeld/core/objects/Uuid.hpp>

namespace nmco = netmeld::core::objects;


class TestUuid : public nmco::Uuid {
  public:
    using Uuid::Uuid;

    using Uuid::uuid;
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    nmco::Uuid u1;

    BOOST_TEST(!u1.isNull());
    BOOST_TEST(!u1.toString().empty());
  }

  {
    nmco::Uuid u1;
    nmco::Uuid u2 {u1};

    BOOST_TEST(u1 == u2);
  }

  {
    uuids::uuid u1 {uuids::random_generator()()};
    TestUuid tu {u1};

    BOOST_TEST(u1 == tu.uuid);
  }

  {
    uuids::uuid u1 {
      uuids::string_generator()("12345678-1234-1234-1234-123456789012")
    };
    TestUuid tu {"12345678-1234-1234-1234-123456789012"};

    BOOST_TEST(u1 == tu.uuid);
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestUuid tu1, tu2;

    sfs::path temp {std::to_string(fileno(std::tmpfile()))};
    std::ofstream f {temp.string()};
    f << tu1.toString() << std::endl;
    f.close();
    tu2.readUuid(temp);
    sfs::remove(temp);
    BOOST_TEST(tu1 == tu2);
  }

  {
    {
      TestUuid tu {uuids::nil_generator()()};
      BOOST_TEST(tu.isNull());
    }
    {
      TestUuid tu;
      BOOST_TEST(!tu.isNull());
    }
  }

  {
    std::string u1 {"12345678-1234-1234-1234-123456789012"};
    TestUuid tu {u1};

    BOOST_TEST(u1 == tu.toString());
  }
}
