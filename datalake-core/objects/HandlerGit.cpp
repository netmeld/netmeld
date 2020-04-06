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

#include <netmeld/datalake/core/objects/DataEntry.hpp>

#include "HandlerGit.hpp"


namespace netmeld::datalake::core::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  HandlerGit::HandlerGit()
  {
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

  std::string
  HandlerGit::cmdExecOut(const std::string& _cmd)
  {
    // TODO formalize command execution logic
    LOG_DEBUG << _cmd << '\n';

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(_cmd.c_str(), "r"), pclose);
    if (!pipe) {
      LOG_ERROR << "Failure: " << _cmd << '\n';
      return "";
    }

    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }

    return result;
  }

  bool
  HandlerGit::initCheck()
  {
    // Ensure data lake pathing exists
    if (!sfs::exists(dataLakePath)) {
      LOG_ERROR << "Data lake not initialized, doing nothing\n";
      return false;
    }

    return true;
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
  HandlerGit::commit(const DataEntry& _de)
  {
    if (!initCheck()) { return; }

    std::string cmd;


    // Ensure device directory exists
    const sfs::path devicePath {dataLakePath/_de.getDeviceId()};
    sfs::create_directories(devicePath);
    deviceDir = devicePath.string();
    
    const sfs::path srcFilePath {_de.getDataPath()};
    std::string dstFilename {_de.getSaveName()};


    // Store in data lake
    //// Copy file to store proper named
    cmd = "cp " + srcFilePath.string()
        + ' ' + deviceDir + "/" + dstFilename + ";";
    cmdExec(cmd);
    //// git add and commit
    std::string dstFilePath {deviceDir + '/' + dstFilename};
    cmd = "cd " + deviceDir
        + "; git add ."
        + "; git commit -m 'tool check-in: " + dstFilePath +"';"
        ;
    cmdExec(cmd);


    // Store import data
    //// git notes
    if (!(_de.getImportTool()).empty()) {
      std::string importCmd {_de.getImportCmd(dstFilePath)};
      cmd = "cd " + deviceDir
          + "; git notes add -f -m '" + importCmd + '\''
          + " `git log -n 1 --pretty=format:\"%H\" -- " + dstFilePath + '`'
          ;
      cmdExec(cmd);
    }
  }

  std::vector<DataEntry>
  HandlerGit::getDataEntries()
  {
    std::vector<DataEntry> vde;

    if (!initCheck()) { return vde; }

    DataEntry* de;
    std::string deviceId;
    for (auto i = sfs::recursive_directory_iterator(dataLakePath);
         i != sfs::recursive_directory_iterator();
         ++i)
    {
      // skip control directory
      if (".git" == i->path().filename()) {
        i.disable_recursion_pending();
        continue;
      }

      const auto& filePath  {i->path().string()};
      const auto& filename  {i->path().filename().string()};
      LOG_DEBUG << std::string(i.depth(), ' ')
                << filename
                << '\n'
                ;

      if (0 == i.depth()) {
        deviceId = filename;
      } else {
        vde.push_back({});
        de = &vde.back();
        de->setDeviceId(deviceId);
        de->setDataPath(filename);

        std::string cmd
          = "cd " + dataLakePath.string()
          + "; git notes show"
          + " `git log -n 1 --pretty=format:\"%H\" -- " + filePath + '`'
          + " 2> /dev/null"
          ;

        const auto& toolInfo {cmdExecOut(cmd)};
        de->setToolAndArgsFromCmd(toolInfo);
      }
    }

    return vde;
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
