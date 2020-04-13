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

#include <netmeld/datalake/objects/DataEntry.hpp>

namespace nmdlo = netmeld::datalake::objects;


class TestDataEntry : public nmdlo::DataEntry {
  public:
    TestDataEntry() : DataEntry() {};
};

BOOST_AUTO_TEST_CASE(testConstructors)
{
  {
    TestDataEntry tde;
    BOOST_CHECK(tde.getDeviceId().empty());
    BOOST_CHECK(tde.getDataPath().empty());
    BOOST_CHECK(tde.getIngestTool().empty());
    BOOST_CHECK(tde.getToolArgs().empty());
    BOOST_CHECK(tde.getNewName().empty());
    BOOST_CHECK(tde.getSaveName().empty());
    BOOST_CHECK(tde.getIngestCmd().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestDataEntry tde;
    const std::string path {"../maybe_some/non-existant/path"};
    tde.setDataPath(path);

    BOOST_CHECK_EQUAL(path, tde.getDataPath());
    BOOST_CHECK_EQUAL("path", tde.getSaveName());

    BOOST_CHECK(tde.getDeviceId().empty());
    BOOST_CHECK(tde.getIngestTool().empty());
    BOOST_CHECK(tde.getToolArgs().empty());
    BOOST_CHECK(tde.getNewName().empty());
    BOOST_CHECK(tde.getIngestCmd().empty());
  }

  {
    TestDataEntry tde;
    const std::string devId {"test"};
    tde.setDeviceId(devId);

    BOOST_CHECK_EQUAL(devId, tde.getDeviceId());

    BOOST_CHECK(tde.getDataPath().empty());
    BOOST_CHECK(tde.getIngestTool().empty());
    BOOST_CHECK(tde.getToolArgs().empty());
    BOOST_CHECK(tde.getNewName().empty());
    BOOST_CHECK(tde.getSaveName().empty());
    BOOST_CHECK(tde.getIngestCmd().empty());
  }

  {
    std::string toolName;
    TestDataEntry tde;

    toolName = "test";
    tde.setIngestTool(toolName);

    BOOST_CHECK_EQUAL(toolName, tde.getIngestTool());
    BOOST_CHECK_EQUAL(toolName, tde.getIngestCmd());

    toolName = "nmdb-import-x";
    const std::string args {" --db-name \"${DB_NAME}\" --db-args \"${DB_ARGS}\" --device-id"};
    tde.setIngestTool(toolName);

    BOOST_CHECK_EQUAL(toolName, tde.getIngestTool());
    BOOST_CHECK_EQUAL((toolName+args), tde.getIngestCmd());

    BOOST_CHECK(tde.getDataPath().empty());
    BOOST_CHECK(tde.getDeviceId().empty());
    BOOST_CHECK(tde.getToolArgs().empty());
    BOOST_CHECK(tde.getNewName().empty());
    BOOST_CHECK(tde.getSaveName().empty());
  }

  {
    TestDataEntry tde;
    const std::string name {"something"};
    tde.setNewName(name);

    BOOST_CHECK_EQUAL(name, tde.getNewName());
    BOOST_CHECK_EQUAL(name, tde.getSaveName());

    BOOST_CHECK(tde.getDataPath().empty());
    BOOST_CHECK(tde.getDeviceId().empty());
    BOOST_CHECK(tde.getIngestTool().empty());
    BOOST_CHECK(tde.getToolArgs().empty());
    BOOST_CHECK(tde.getIngestCmd().empty());
  }
  {
    TestDataEntry tde;
    const std::string path {"../maybe_some/non-existant/path"};
    tde.setDataPath(path);
    const std::string name {"something"};
    tde.setNewName(name);

    BOOST_CHECK_EQUAL(path, tde.getDataPath());
    BOOST_CHECK_EQUAL(name, tde.getNewName());
    BOOST_CHECK_EQUAL(name, tde.getSaveName());

    BOOST_CHECK(tde.getDeviceId().empty());
    BOOST_CHECK(tde.getIngestTool().empty());
    BOOST_CHECK(tde.getToolArgs().empty());
    BOOST_CHECK(tde.getIngestCmd().empty());
  }

  {
    TestDataEntry tde;
    const std::string args {"some -rando --args"};
    tde.setToolArgs(args);

    BOOST_CHECK_EQUAL(args, tde.getToolArgs());

    BOOST_CHECK(tde.getDeviceId().empty());
    BOOST_CHECK(tde.getDataPath().empty());
    BOOST_CHECK(tde.getIngestTool().empty());
    BOOST_CHECK(tde.getNewName().empty());
    BOOST_CHECK(tde.getSaveName().empty());
    BOOST_CHECK(tde.getIngestCmd().empty());
  }
}
