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

#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>

#include <netmeld/core/objects/Time.hpp>

namespace nmco = netmeld::core::objects;


class TestTime : public nmco::Time {
  public:
    TestTime() : Time() {};
    explicit TestTime(pt::ptime _time) : Time(_time) {};
    explicit TestTime(const std::string& _time) : Time(_time) {};

  public:
    pt::ptime getTime() const
    { return time; }
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    pt::ptime pt1 = pt::microsec_clock::universal_time();
    TestTime time;

    // add small ficticious delay so pt1 != pt2 as time_period() is [pt1, pt2)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    pt::ptime pt2 = pt::microsec_clock::universal_time();
    auto run = pt::time_period(pt1, pt2);

    BOOST_TEST(run.contains(time.getTime()));
  }

  {
    pt::ptime pt1 = pt::microsec_clock::universal_time();
    TestTime time {pt1};

    BOOST_TEST(pt1 == time.getTime());
  }

  {
    pt::ptime pt1 {pt::neg_infin};
    TestTime time {"-infinity"};

    BOOST_TEST(pt1 == time.getTime());
  }

  {
    pt::ptime pt1 {pt::pos_infin};
    TestTime time {"infinity"};

    BOOST_TEST(pt1 == time.getTime());
  }

  {
    std::string dts {"2001-01-01 00:00:00.001"};
    pt::ptime pt1 {pt::time_from_string(dts)};
    TestTime time {dts};

    BOOST_TEST(pt1 == time.getTime());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestTime time;

    time.readFormatted("Mon Jan 01 00:00:00 2001", "%a %b %d %H:%M:%S %Y");
    std::string dts {"2001-01-01 00:00:00"};
    pt::ptime pt1 {pt::time_from_string(dts)};

    BOOST_TEST(pt1 == time.getTime());
  }
  {
    TestTime tt1;
    // add small delay so tt1 != tt2
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    TestTime tt2;
    BOOST_TEST(tt1.getTime() != tt2.getTime());

    tt2 = tt1;
    // ensure empty is a no-op
    tt2.readFormatted("", "%a %b %d %H:%M:%S %Y");
    BOOST_TEST(tt1.getTime() == tt2.getTime());

    TestTime tt3 = tt1;
    BOOST_TEST(tt1.getTime() == tt3.getTime());
    BOOST_CHECK_THROW(tt3.readUnixTimestamp(""), std::exception);
    BOOST_TEST(tt1.getTime() == tt3.getTime());
  }

  {
    pt::ptime pt1 = pt::microsec_clock::universal_time();
    TestTime time1 {pt1}, time2;

    sfs::path temp {std::to_string(fileno(std::tmpfile()))};
    std::ofstream f {temp};
    f << time1.toString() << std::endl;
    f.close();
    time2.readTime(temp);
    sfs::remove(temp);
    BOOST_TEST(time1.getTime() == time2.getTime());
  }

  {
    {
      TestTime time {pt::not_a_date_time};
      BOOST_TEST(time.isNull());
    }
    {
      TestTime time;
      BOOST_TEST(!time.isNull());
    }
  }

  {
    TestTime time {"2001-01-01 00:00:00.001"};

    BOOST_TEST("2001-01-01T00:00:00.001000" == time.toString());
  }
}
