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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/core/objects/Uuid.hpp>

namespace nmco = netmeld::core::objects;


class TestUuid : public nmco::Uuid {
  public:
    TestUuid() : Uuid() {};
    explicit TestUuid(Uuid const& _id) : Uuid(_id) {};
    explicit TestUuid(uuids::uuid _id) : Uuid(_id) {};
    explicit TestUuid(const std::string& _id) : Uuid(_id) {};

    auto operator<=>(const TestUuid&) const = default;

  public:
    uuids::uuid getUuid() const
    { return uuid; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestUuid uuid;

    BOOST_CHECK(!uuid.isNull());
    BOOST_CHECK(!uuid.toString().empty());
  }

  {
    nmco::Uuid u1;
    TestUuid uuid {u1};

    BOOST_CHECK_EQUAL(u1, uuid);
  }

  {
    uuids::uuid u1 {uuids::random_generator()()};
    TestUuid uuid {u1};

    BOOST_CHECK_EQUAL(u1, uuid.getUuid());
  }

  {
    uuids::uuid u1 {
      uuids::string_generator()("12345678-1234-1234-1234-123456789012")
    };
    TestUuid uuid {"12345678-1234-1234-1234-123456789012"};

    BOOST_CHECK_EQUAL(u1, uuid.getUuid());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestUuid uuid1, uuid2;

    sfs::path temp {std::to_string(fileno(std::tmpfile()))};
    std::ofstream f {temp.string()};
    f << uuid1.toString() << std::endl;
    f.close();
    uuid2.readUuid(temp);
    sfs::remove(temp);
    BOOST_CHECK_EQUAL(uuid1, uuid2);
  }

  {
    {
      TestUuid uuid {uuids::nil_generator()()};
      BOOST_CHECK(uuid.isNull());
    }
    {
      TestUuid uuid;
      BOOST_CHECK(!uuid.isNull());
    }
  }

  {
    std::string u1 {"12345678-1234-1234-1234-123456789012"};
    TestUuid uuid {u1};

    BOOST_CHECK_EQUAL(u1, uuid.toString());
  }
}
