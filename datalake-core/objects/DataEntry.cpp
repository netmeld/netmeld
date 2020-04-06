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

#include "DataEntry.hpp"

namespace sfs = std::filesystem;


namespace netmeld::datalake::core::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  DataEntry::DataEntry()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  std::string
  DataEntry::getDeviceId() const
  { return deviceId; }

  std::string
  DataEntry::getDataPath() const
  { return dataPath; }

  std::string
  DataEntry::getImportTool() const
  { return importTool; }

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
  DataEntry::getImportCmd(const std::string& _filePath) const
  {
    std::regex regexNetmeldImportTool("nmdb-import-[-a-z]+");
    std::string cmd {getImportTool()};

    if (std::regex_match(cmd, regexNetmeldImportTool)) {
      cmd += " --device-id " + getDeviceId();
    }
    if (!getToolArgs().empty()) {
      cmd += " " + getToolArgs();
    }
    cmd += " " + _filePath;

    return cmd;
  }

  void
  DataEntry::setToolAndArgsFromCmd(const std::string& _cmd)
  {
    std::regex regexNetmeldImportTool("^nmdb-import-[-a-z]+");
    std::smatch m;
    if (std::regex_search(_cmd, m, regexNetmeldImportTool)) {
      importTool = m.str(0);
      toolArgs = m.suffix().str();
    } else {
      size_t wsLoc {_cmd.find(' ')};
      if (std::string::npos == wsLoc) {
        importTool = _cmd;
        toolArgs   = "";
      } else {
        importTool = _cmd.substr(0, wsLoc);
        toolArgs   = _cmd.substr(wsLoc);
      }
    }

    if ('\n' == importTool.back()) {
      importTool.pop_back();
    }
    if ('\n' == toolArgs.back()) {
      toolArgs.pop_back();
    }
  }

  void
  DataEntry::setDeviceId(const std::string& _deviceId)
  {
    // TODO validate?
    deviceId = _deviceId;
  }

  void
  DataEntry::setDataPath(const std::string& _dataPath)
  {
    // TODO validate?
    dataPath = _dataPath;
  }

  void
  DataEntry::setImportTool(const std::string& _importTool)
  {
    // TODO validate?
    importTool = _importTool;
  }

  void
  DataEntry::setToolArgs(const std::string& _toolArgs)
  {
    // TODO validate?
    toolArgs = _toolArgs;
  }

  void
  DataEntry::setNewName(const std::string& _newName)
  {
    // TODO validate?
    newName = _newName;
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
  std::ostream& operator<<(std::ostream& os, const DataEntry& de)
  {
    return os
      << "deviceId: " << de.deviceId
      << "\n  dataPath: " << de.dataPath
      << "\n  importTool: " << de.importTool
      << "\n  toolArgs: " << de.toolArgs
      << "\n  rename: " << de.newName
      ;
  }
}
