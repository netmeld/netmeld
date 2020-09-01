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

#include <filesystem>
#include <regex>

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/core/utils/LoggerSingleton.hpp>

#include <netmeld/datalake/objects/DataEntry.hpp>

namespace sfs  = std::filesystem;
namespace nmcu = netmeld::core::utils;


namespace netmeld::datalake::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  DataEntry::DataEntry()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  DataEntry::isPipedData() const
  { return pipedData; }

  std::string
  DataEntry::getDataPath() const
  { return dataPath; }

  std::string
  DataEntry::getDeviceId() const
  { return deviceId; }

  std::string
  DataEntry::getIngestTool() const
  { return ingestTool; }

  std::string
  DataEntry::getToolArgs() const
  { return toolArgs; }

  std::string
  DataEntry::getNewName() const
  { return newName; }

  std::string
  DataEntry::getSaveName() const
  {
    sfs::path dp {getDataPath()};
    std::string dstFilename {dp.filename().string()};
    if (!(getNewName()).empty()) {
      const sfs::path tmp {getNewName()};
      dstFilename = tmp.filename().string();
    }

    return dstFilename;
  }

  std::string
  DataEntry::getIngestCmd() const
  {
    std::regex regexNetmeldImportTool("nmdb-import-[-a-z]+");
    std::ostringstream oss;
    oss << getIngestTool();

    const auto& toolVal {oss.str()};
    if (!toolVal.empty()) {
      if (std::regex_match(toolVal, regexNetmeldImportTool)) {
        oss << " --db-name \"${DB_NAME}\" --db-args \"${DB_ARGS}\""
            << " --device-id " << getDeviceId();
      }
      if (!getToolArgs().empty()) {
        oss << " " << getToolArgs();
      }
      if (!getDataPath().empty()) {
        oss << " " << getDataPath();
      }
    }

    return nmcu::trim(oss.str());
  }

  void
  DataEntry::setDataPath(const std::string& _dataPath)
  {
    if (_dataPath.empty()) {
      pipedData = true;
    } else {
      dataPath = sfs::absolute(_dataPath);
    }
  }

  void
  DataEntry::setDeviceId(const std::string& _deviceId)
  {
    deviceId = _deviceId;
  }

  void
  DataEntry::setIngestTool(const std::string& _ingestTool)
  {
    ingestTool = _ingestTool;
  }

  void
  DataEntry::setNewName(const std::string& _newName)
  {
    newName = _newName;
  }

  void
  DataEntry::setToolArgs(const std::string& _toolArgs)
  {
    toolArgs = _toolArgs;
  }

  std::string
  DataEntry::toDebugString() const
  {
    std::ostringstream oss;
    oss << "deviceId: " << getDeviceId()
        << "\n  dataPath: " << getDataPath()
        << "\n  ingestTool: " << getIngestTool()
        << "\n  toolArgs: " << getToolArgs()
        << "\n  rename: " << getNewName()
        ;
    return oss.str();
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
