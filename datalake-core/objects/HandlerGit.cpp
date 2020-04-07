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

#include <netmeld/core/utils/StringUtilities.hpp>
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
      + "; git init"
      + ";"
      ;
    cmdExec(cmd);
  }

  void
  HandlerGit::commit(DataEntry& _de)
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
        + "; git commit -m 'tool check-in: " + dstFilePath +"'"
        + ";"
        ;
    cmdExec(cmd);


    // Store import data
    //// git notes
    if (!(_de.getImportTool()).empty()) {
      std::ostringstream oss;
      oss << "import-tool:" << _de.getImportTool()
          << "\ntool-args:" << _de.getToolArgs()
          << "\n"
          ;
      cmd = "cd " + deviceDir
          + "; git notes add -f -m '" + oss.str() + '\''
          + " `git log -n 1 --pretty=format:\"%H\" -- " + dstFilePath + '`'
          + ";"
          ;
      cmdExec(cmd);
    }
  }

  std::vector<DataEntry>
  HandlerGit::getDataEntries(const nmco::Time& _dts)
  {
    std::vector<DataEntry> vde;

    if (!initCheck()) { return vde; }

    alignRepo(_dts);

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
      auto depth {i.depth()};

      if (0 > depth) {
        LOG_ERROR << "Exiting: Directory depth < 0, unknown issue\n";
        std::exit(nmcu::Exit::FAILURE);
      } else if (0 == depth) {
        deviceId = filename;
      } else {
        vde.push_back({});
        de = &vde.back();
        de->setDeviceId(deviceId);
        de->setDataPath(filePath);

        std::string cmd
          = "cd " + dataLakePath.string()
          + "; git notes show"
          + " `git log -n 1 --pretty=format:\"%H\" -- " + filePath + '`'
          + " 2> /dev/null"
          + ";"
          ;

        const auto& toolInfo {cmdExecOut(cmd)};
        std::string reg {"import-tool:(.*)\ntool-args:(.*)\n"};
        std::regex regex(reg, std::regex::extended);
        std::smatch m;
        if (std::regex_match(toolInfo, m, regex)) {
          de->setImportTool(m.str(1));
          de->setToolArgs(m.str(2));
        } else {
          LOG_DEBUG << "No import data found:\n" << filePath << '\n';
        }
      }
    }

    alignRepo();

    return vde;
  }

  void
  HandlerGit::alignRepo(const nmco::Time& _dts)
  {
    LOG_DEBUG << "Target time: " << _dts << '\n';

    std::ostringstream oss;
    oss << "cd " + dataLakeDir
        << "; echo -n `git rev-list -n 1 --first-parent"
          << " --before=\"" << _dts << "\" master`"
        << ";"
      ;

    auto result {cmdExecOut(oss.str())};
    result = nmcu::trim(result);
    LOG_DEBUG << "Target SHA: " << result << '\n';

    if (result.empty()) {
      LOG_ERROR << "Invalid repository date: " << _dts << '\n';
      std::ostringstream oss1;
      oss1 << "cd " + dataLakeDir
           << "; git log --reverse --date='format-local:%FT%T'"
            << " --format=\"format:%cd\""
          << ";"
        ;
      auto const& validDates {cmdExecOut(oss1.str())};
      LOG_ERROR << "Valid dates:"
                << '\n' << validDates
                << '\n'
                ;
      std::exit(1);
    } else {
      std::ostringstream oss1;
      oss1 << "cd " + dataLakeDir
           << "; git checkout -q "
           ;
      if ("infinity" == _dts.toString()) {
        oss1 << "master;" ;
      } else {
        oss1 << result << ";";
      }
      cmdExec(oss1.str());
    }
  }

  void
  HandlerGit::removeLast(const std::string& _deviceId,
                         const std::string& _dataPath)
  {
    if (!initCheck()) { return; }

    const sfs::path devicePath {dataLakePath/_deviceId};
    if (!sfs::exists(devicePath)) {
      LOG_WARN << "Device-id does not exists: "
               << _deviceId << '\n';
      return;
    }

    const sfs::path filePath {devicePath/_dataPath};
    if (!sfs::exists(filePath)) {
      LOG_WARN << "Device's data does not exists: "
               << _deviceId << "->" << _dataPath << '\n';
      return;
    }

    std::string cmd =
        "cd " + dataLakeDir
      + "; git rm --ignore-unmatch -r " + filePath.string()
      + "; git commit -m 'tool removal'"
      + ";"
      ;

    cmdExec(cmd);
  }

  void
  HandlerGit::removeAll(const std::string& _deviceId,
                        const std::string& _dataPath)
  {
    if (!initCheck()) { return; }

    // TODO determine if both test needed for remove functions
    const sfs::path devicePath {dataLakePath/_deviceId};
    if (!sfs::exists(devicePath)) {
      LOG_WARN << "Device-id does not exists: "
               << _deviceId << '\n';
      return;
    }

    // TODO standardize pathing re-alignment
    sfs::current_path(dataLakeDir);

    const sfs::path filePath {devicePath/_dataPath};
    //if (!sfs::exists(filePath)) {
    //  LOG_WARN << "Device's data does not exists: "
    //           << _deviceId << "->" << _dataPath << '\n';
    //  return;
    //}

    std::string cmd =
        "cd " + dataLakeDir
      + "; FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch --prune-empty --index-filter"
      + " 'git rm --cached --ignore-unmatch -fr " + sfs::relative(filePath).string() + "' HEAD"
      + "; git for-each-ref --format=\"%(refname)\" refs/original/ | xargs -n 1 git update-ref -d 2>/dev/null"
      + "; git reflog expire --expire=now --all && git gc --prune=now --aggressive"
      + ";"
      ;
    cmdExec(cmd);
  }
  // TODO what about restoring removed files (non-permanent)?

  // ===========================================================================
  // Friends
  // ===========================================================================
}
