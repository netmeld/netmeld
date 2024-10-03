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

#include <filesystem>

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
    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDataPath().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getNewName().empty());
    BOOST_TEST(tde.getSaveName().empty());
    BOOST_TEST(tde.getToolArgs().empty());
  }
}

BOOST_AUTO_TEST_CASE(testSetters)
{
  {
    TestDataEntry tde;

    std::vector<std::string> testsOk {
      "Some One",
      "Some Other <email>",
      "Some Other1<email@other.new>",
    };

    for (const auto& committer : testsOk) {
      tde.setCommitter(committer);
      BOOST_TEST(committer == tde.getCommitter());
    }

    std::vector<std::string> testsBad {
      " ", ".", ",", ":", ";", "<", ">", "\"", "'", "\\",
      " .,:;<>\"'\\",
    };
    const std::string empty {""};
    tde.setCommitter(empty); // reset
    for (const auto& committer : testsBad) {
      tde.setCommitter(committer);
      BOOST_TEST(empty == tde.getCommitter());
    }
    int i {0};
    for (char c {'\0'}; i <= 20; ++i, ++c) {
      std::string s(1,c);
      tde.setCommitter(s);
      BOOST_TEST(empty == tde.getCommitter());
    }

    BOOST_TEST(tde.getDataPath().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getNewName().empty());
    BOOST_TEST(tde.getSaveName().empty());
    BOOST_TEST(tde.getToolArgs().empty());
  }
  {
    TestDataEntry tde;
    const std::string path {"../maybe_some/non-existant/path"};
    tde.setDataPath(path);

    BOOST_TEST(std::filesystem::absolute(path) == tde.getDataPath());
    BOOST_TEST("path" == tde.getSaveName());

    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getNewName().empty());
    BOOST_TEST(tde.getToolArgs().empty());
  }

  {
    TestDataEntry tde;
    const std::string devId {"test"};
    tde.setDeviceId(devId);

    BOOST_TEST(devId == tde.getDeviceId());

    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDataPath().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getToolArgs().empty());
    BOOST_TEST(tde.getNewName().empty());
    BOOST_TEST(tde.getSaveName().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
  }

  {
    std::string toolName;
    TestDataEntry tde;

    toolName = "test";
    tde.setIngestTool(toolName);

    BOOST_TEST(toolName == tde.getIngestTool());
    BOOST_TEST(toolName == tde.getIngestCmd());

    toolName = "nmdb-import-x";
    const std::string args {" --db-name \"${DB_NAME}\" --db-args \"${DB_ARGS}\" --device-id"};
    tde.setIngestTool(toolName);

    BOOST_TEST(toolName == tde.getIngestTool());
    BOOST_TEST((toolName+args) == tde.getIngestCmd());

    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDataPath().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getToolArgs().empty());
    BOOST_TEST(tde.getNewName().empty());
    BOOST_TEST(tde.getSaveName().empty());
  }

  {
    TestDataEntry tde;
    const std::string name {"something"};
    tde.setNewName(name);

    BOOST_TEST(name == tde.getNewName());
    BOOST_TEST(name == tde.getSaveName());

    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDataPath().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getToolArgs().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
  }
  {
    TestDataEntry tde;
    const std::string path {"../maybe_some/non-existant/path"};
    tde.setDataPath(path);
    const std::string name {"something"};
    tde.setNewName(name);

    BOOST_TEST(std::filesystem::absolute(path) == tde.getDataPath());
    BOOST_TEST(name == tde.getNewName());
    BOOST_TEST(name == tde.getSaveName());

    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getToolArgs().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
  }

  {
    TestDataEntry tde;
    const std::string args {"some -rando --args"};
    tde.setToolArgs(args);

    BOOST_TEST(args == tde.getToolArgs());

    BOOST_TEST(tde.getCommitter().empty());
    BOOST_TEST(tde.getDeviceId().empty());
    BOOST_TEST(tde.getDataPath().empty());
    BOOST_TEST(tde.getIngestTool().empty());
    BOOST_TEST(tde.getNewName().empty());
    BOOST_TEST(tde.getSaveName().empty());
    BOOST_TEST(tde.getIngestCmd().empty());
  }
}
