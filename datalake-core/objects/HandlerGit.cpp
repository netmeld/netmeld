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

#include <regex>

#include "HandlerGit.hpp"


namespace netmeld::datalake::core::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  HandlerGit::HandlerGit()
  {
    LOG_DEBUG << "Data lake with git, vars:"
              << "\n  deviceId: " << this->deviceId
              << "\n  dataPath: " << this->dataPath
              << "\n  importTool: " << this->importTool
              << "\n  toolArgs: " << this->toolArgs
              << "\n  rename: " << this->newName
              << '\n';

    dataLakePath = {nmfm.getSavePath()/"datalake"};
    dataLakeDir = dataLakePath.string();
  }

  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  HandlerGit::cmdExec(const std::string& _cmd)
  {
    // TODO formalize command execution logic
    LOG_DEBUG << _cmd << '\n';
    auto exitStatus {std::system(_cmd.c_str())};
    if (-1 == exitStatus) { LOG_ERROR << "Failure: " << _cmd << '\n'; }
    if (0 != exitStatus) { LOG_WARN << "Non-Zero: " << _cmd << '\n'; }
  }

  void
  HandlerGit::initialize()
  {
    LOG_DEBUG << "Removing existing: " << dataLakePath << '\n';
    sfs::remove_all(dataLakePath);
    LOG_DEBUG << "Creating new: " << dataLakePath << '\n';
    sfs::create_directories(dataLakePath);


    // Initialize data lake
    // TODO formalize logic
    std::string cmd =
        "cd " + dataLakeDir
      + "; git init;"
      ;
    cmdExec(cmd);
  }

  void
  HandlerGit::commit()
  {

    // Ensure data lake pathing exists
    if (!std::filesystem::exists(dataLakePath)) {
      LOG_ERROR << "Data lake not initialized, doing nothing\n";
      return;
    }


    std::string cmd;


    // Ensure device directory exists
    const sfs::path devicePath {dataLakePath/this->deviceId};
    sfs::create_directories(devicePath);
    deviceDir = devicePath.string();
    

    // Handle rename targeting
    const sfs::path srcFilePath {this->dataPath};
    std::string dstFilename {srcFilePath.filename().string()};
    if (!(this->newName).empty()) {
      const sfs::path tmp {this->newName};
      dstFilename = tmp.filename().string();
    }


    // Copy file to store proper named
    cmd = "cp " + srcFilePath.string()
        + ' ' + deviceDir + "/" + dstFilename + ";";
    cmdExec(cmd);


    // Store in data lake, git add and commit
    std::string dstFilePath {deviceDir + '/' + dstFilename};
    cmd = "cd " + deviceDir
        + "; git add ."
        + "; git commit -m 'tool check-in: " + dstFilePath +"';"
        ;
    cmdExec(cmd);


    // Store import data, git notes
    if (!(this->importTool).empty()) {
      std::string note {this->importTool};
      std::regex regexImportTool("nmdb-import-[-a-z]+");
      if (std::regex_match(note, regexImportTool)) {
        note += " --device-id " + this->deviceId;
      }
      if (!(this->toolArgs).empty()) {
        note += " " + this->toolArgs;
      }
      note += " " + dstFilePath;
      cmd = "cd " + deviceDir +
          + "; git notes add -f -m '" + note + '\''
          + " `git log -n 1 --pretty=format:\"%H\" -- " + dstFilePath + '`'
          ;
      cmdExec(cmd);
    }
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
