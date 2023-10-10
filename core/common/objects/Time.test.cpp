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
    pt::ptime t1 = pt::microsec_clock::universal_time();
    TestTime time;

    // add small ficticious delay so t1 != t2 as time_period() is [t1, t2)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    pt::ptime t2 = pt::microsec_clock::universal_time();
    auto run = pt::time_period(t1, t2);

    BOOST_CHECK(run.contains(time.getTime()));
  }

  {
    pt::ptime t1 = pt::microsec_clock::universal_time();
    TestTime time {t1};

    BOOST_CHECK_EQUAL(t1, time.getTime());
  }

  {
    pt::ptime t1 {pt::neg_infin};
    TestTime time {"-infinity"};

    BOOST_CHECK_EQUAL(t1, time.getTime());
  }

  {
    pt::ptime t1 {pt::pos_infin};
    TestTime time {"infinity"};

    BOOST_CHECK_EQUAL(t1, time.getTime());
  }

  {
    std::string dts {"2001-01-01 00:00:00.001"};
    pt::ptime t1 {pt::time_from_string(dts)};
    TestTime time {dts};

    BOOST_CHECK_EQUAL(t1, time.getTime());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestTime time;

    time.readFormatted("Mon Jan 01 00:00:00 2001", "%a %b %d %H:%M:%S %Y");
    std::string dts {"2001-01-01 00:00:00"};
    pt::ptime t1 {pt::time_from_string(dts)};

    BOOST_CHECK_EQUAL(t1, time.getTime());
  }

  {
    pt::ptime t1 = pt::microsec_clock::universal_time();
    TestTime time1 {t1}, time2;

    sfs::path temp {std::to_string(fileno(std::tmpfile()))};
    std::ofstream f {temp};
    f << time1.toString() << std::endl;
    f.close();
    time2.readTime(temp);
    sfs::remove(temp);
    BOOST_CHECK_EQUAL(time1.getTime(), time2.getTime());
  }

  {
    {
      TestTime time {pt::not_a_date_time};
      BOOST_CHECK(time.isNull());
    }
    {
      TestTime time;
      BOOST_CHECK(!time.isNull());
    }
  }

  {
    TestTime time {"2001-01-01 00:00:00.001"};

    BOOST_CHECK_EQUAL("2001-01-01T00:00:00.001000", time.toString());
  }
}
