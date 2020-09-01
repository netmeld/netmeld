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

#include <netmeld/core/utils/CmdExec.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include <netmeld/datalake/handlers/Git.hpp>

namespace nmcu  = netmeld::core::utils;


namespace netmeld::datalake::handlers {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  Git::Git(const std::string& _path) :
    AbstractHandler(_path)
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  Git::changeDirToRepo()
  {
    const auto& tgtPath {this->dataLakePath};

    if (!sfs::exists(tgtPath)) {
      LOG_ERROR << "Storage not initialized, use nmdl-initialize\n";
      return false;
    }

    sfs::current_path(tgtPath);
    return true;
  }

  bool
  Git::alignRepo(const nmco::Time& _dts)
  {
    LOG_DEBUG << "Target time: " << _dts << '\n';

    std::ostringstream oss;
    oss << "echo -n `git rev-list -n 1 --first-parent"
        << " --before=\"" << _dts << "\" master`" ;
    auto result {nmcu::trim(nmcu::cmdExecOut(oss.str()))};
    LOG_DEBUG << "Target SHA: " << result << '\n';

    oss.str("");
    if (result.empty()) {
      oss << "git log --reverse --date='format-local:%FT%T'"
            << " --format=\"format:%cd\""
          ;
      auto const& validDates {nmcu::cmdExecOut(oss.str())};
      LOG_ERROR << "Invalid repository date: " << _dts << '\n'
                << "Valid dates:"
                << '\n' << validDates
                << '\n';
      return false;
    } else {
      oss << "git checkout -q "
          << ("infinity" == _dts.toString() ? "master" : result);
      nmcu::cmdExec(oss.str());
    }

    return true;
  }

  void
  Git::setIngestToolData(nmdlo::DataEntry& _de, const std::string& _path)
  {
    std::ostringstream oss;
    oss << "git log -n 1 --pretty=format:\"%B\" -- " << _path;

    std::istringstream iss(nmcu::cmdExecOut(oss.str()));

    std::regex toolRegex('^' + INGEST_TOOL_PREFIX + "(.*)$");
    std::regex argsRegex('^' + TOOL_ARGS_PREFIX + "(.*)$");
    std::smatch m;

    for (std::string line; std::getline(iss, line);) {
      if (std::regex_search(line, m, toolRegex)) {
        _de.setIngestTool(m.str(1));
      }
      if (std::regex_search(line, m, argsRegex)) {
        _de.setToolArgs(m.str(1));
      }
    }
  }

  void
  Git::initialize()
  {
    const auto& tgtPath {this->dataLakePath};

    if (sfs::exists(tgtPath)) {
      LOG_INFO << "Datalake " << tgtPath << " already exists.\n"
               << "Re-initialzie the datalake [y/n]?";
      char response;
      std::cin >> response;
      if ('y' != response && 'Y' != response) {
        LOG_INFO << "Datalake NOT re-initialized by user\n";
        std::exit(nmcu::Exit::USER_ABORTED);
      }
    }

    LOG_DEBUG << "Removing existing: " << tgtPath << '\n';
    sfs::remove_all(tgtPath);
    LOG_DEBUG << "Creating new: " << tgtPath << '\n';
    sfs::create_directories(tgtPath);

    sfs::current_path(tgtPath);
    std::ostringstream oss;
    oss << "git init";
    nmcu::cmdExec(oss.str());
  }

  void
  Git::commit(nmdlo::DataEntry& _de)
  {
    if (!changeDirToRepo()) { return; }

    // Ensure device directory exists
    const sfs::path devicePath {this->dataLakePath/_de.getDeviceId()};
    sfs::create_directories(devicePath);

    // Copy file to store, properly named
    const sfs::path dstPath {devicePath/_de.getSaveName()};
    const std::string dstRelPath {sfs::relative(dstPath)};
    if (_de.isPipedData()) {
      nmfm.pipedInputFileOverwrite(dstRelPath);
    } else {
      const sfs::path srcPath {_de.getDataPath()};
      const std::string srcRelPath {sfs::relative(srcPath)};
      sfs::copy(srcRelPath, dstRelPath, sfs::copy_options::overwrite_existing);
    }

    // Store data
    std::ostringstream oss;
    oss << "git add ."
        << " && git commit -m '"
          << CHECK_IN_PREFIX << dstRelPath
          << '\n' << INGEST_TOOL_PREFIX << _de.getIngestTool()
          << '\n' << TOOL_ARGS_PREFIX << _de.getToolArgs()
        << "\n'";
    nmcu::cmdExec(oss.str());
  }

  std::vector<nmdlo::DataEntry>
  Git::getDataEntries(const nmco::Time& _dts)
  {
    std::vector<nmdlo::DataEntry> vde;
    if (!(changeDirToRepo() && alignRepo(_dts))) { return vde; }

    std::string deviceId;
    for (auto i = sfs::recursive_directory_iterator(this->dataLakePath);
         i != sfs::recursive_directory_iterator();
         ++i)
    {
      // skip control directory
      if (".git" == i->path().filename()) {
        i.disable_recursion_pending();
        continue;
      }

      auto depth {i.depth()};
      if (0 > depth) {
        LOG_ERROR << "Exiting: Directory depth < 0\n";
        std::exit(nmcu::Exit::FAILURE);
      } else if (0 == depth) {
        deviceId = i->path().filename().string();
      } else {
        const auto& filePath {i->path().string()};
        nmdlo::DataEntry data;

        data.setDeviceId(deviceId);
        data.setDataPath(filePath);
        setIngestToolData(data, filePath);

        vde.push_back(data);
      }
    }

    alignRepo();
    return vde;
  }


  void
  Git::removeLast(const std::string& _deviceId,
                         const std::string& _dataPath)
  {
    if (!changeDirToRepo()) { return; }

    const sfs::path tgtPath       {this->dataLakePath/_deviceId/_dataPath};
    const std::string tgtRelPath  {sfs::relative(tgtPath).string()};
    if (!sfs::exists(tgtPath)) {
      LOG_WARN << "Target does not exists: " << tgtPath << '\n';
    }

    std::ostringstream oss;
    oss << "git rm --ignore-unmatch -r " << tgtRelPath
        << " &&  git commit -m 'nmdl-remove: " << tgtRelPath << "'";

    nmcu::cmdExec(oss.str());
  }

  void
  Git::removeAll(const std::string& _deviceId,
                        const std::string& _dataPath)
  {
    if (!changeDirToRepo()) { return; }

    const sfs::path tgtPath       {this->dataLakePath/_deviceId/_dataPath};
    const std::string tgtRelPath  {sfs::relative(tgtPath).string()};
    if (!sfs::exists(tgtPath)) {
      LOG_WARN << "Target does not exists: " << tgtPath << '\n';
    }

    std::ostringstream oss;
    oss << "FILTER_BRANCH_SQUELCH_WARNING=1"
          << " git filter-branch --prune-empty --index-filter"
          << " 'git rm --cached --ignore-unmatch -fr " << tgtRelPath << "' HEAD"
        << " && git for-each-ref --format=\"%(refname)\" refs/original/"
          << " | xargs -n 1 git update-ref -d 2>/dev/null"
        << " && git reflog expire --expire=now --all"
        << "&& git gc --prune=now --aggressive";
    nmcu::cmdExec(oss.str());
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
