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


namespace netmeld::datalake::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  HandlerGit::HandlerGit()
  {
    LOG_DEBUG << "HandlerGit constructor\n";
  }

  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  HandlerGit::commit()
  {
    LOG_DEBUG << "Data lake save with git\n";
    LOG_DEBUG << "Vars:"
              << "\n  deviceId: " << this->deviceId
              << "\n  dataPath: " << this->dataPath
              << "\n  importTool: " << this->importTool
              << "\n  toolArgs: " << this->toolArgs
              << "\n  rename: " << this->newName
              << '\n';


    // Ensure data lake pathing exists
    // TODO formalize path init logic
    const sfs::path dlDir {nmfm.getSavePath()/"data-lake"};
    sfs::create_directories(dlDir);
   

    // Standard exec call
    // TODO formalize command execution logic
    std::string cmd;
    auto cmdExec = [](const std::string& _cmd)
    {
      LOG_DEBUG << _cmd << '\n';
      auto exitStatus {std::system(_cmd.c_str())};
      if (0 != exitStatus) { LOG_WARN << "Failure: " << _cmd << '\n'; }
    };


    // Initialize data lake
    // TODO formalize logic
    std::string saveDir {dlDir.string()};
    cmd = "cd " + saveDir
        + "; git init;"
        ;
    cmdExec(cmd);


    // Ensure device directory exists
    const sfs::path devDir {dlDir/this->deviceId};
    sfs::create_directories(devDir);
    saveDir = devDir.string();
    

    // Handle rename targeting
    const sfs::path tgtFile {this->dataPath};
    std::string tgtFilename {tgtFile.filename().string()};
    if (!(this->newName).empty()) {
      const sfs::path tmp {this->newName};
      tgtFilename = tmp.filename().string();
    }


    // Copy file to store proper named
    cmd = "cp " + tgtFile.string() + ' ' + saveDir + "/" + tgtFilename + ";";
    cmdExec(cmd);


    // Store in data lake, git add and commit
    std::string tgtFilePath {saveDir + '/' + tgtFilename};
    cmd = "cd " + saveDir
        + "; git add ."
        + "; git commit -m 'tool check-in: " + tgtFilePath +"';"
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
      note += " " + tgtFilePath;
      cmd = "cd " + saveDir +
          + "; git notes add -f -m '" + note + '\''
          + " `git log -n 1 --pretty=format:\"%H\" -- " + tgtFilePath + '`'
          ;
      cmdExec(cmd);
    }
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
