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

#include <netmeld/core/utils/FileManager.hpp>

#include <iostream>


namespace netmeld::core::utils {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  FileManager::FileManager()
  {
    std::ostringstream oss;

    oss.str(std::string());
    oss << NETMELD_CONF_DIR;
    confPath = oss.str();

    oss.str(std::string());
    oss << std::getenv("HOME");
    if (!sfs::exists(oss.str())) {
      oss.str(std::string());
      oss << sfs::temp_directory_path().string();
    }
    oss << "/.netmeld";
    savePath = oss.str();

    sfs::create_directories(savePath);

    oss << "/.tmp";
    tempPath = oss.str();
    sfs::create_directories(tempPath);
  }

  FileManager::~FileManager()
  {
    sfs::remove_all(tempPath);
  }

  // ===========================================================================
  // Methods
  // ===========================================================================
  FileManager&
  FileManager::getInstance()
  {
    static FileManager instance;
    return instance;
  }

  const sfs::path&
  FileManager::getConfPath() const
  {
    return confPath;
  }

  const sfs::path&
  FileManager::getSavePath() const
  {
    return savePath;
  }

  const sfs::path&
  FileManager::getTempPath() const
  {
    return tempPath;
  }

  void
  FileManager::removeWrite(const sfs::path& path, bool recursive) const
  {
    if (!sfs::exists(path)) { return; }

    sfs::perms writeOnly {
          sfs::perms::owner_write
        | sfs::perms::group_write
        | sfs::perms::others_write
    };

    sfs::permissions(path, writeOnly, sfs::perm_options::remove);

    if (recursive) {
      for (auto& p : sfs::recursive_directory_iterator(path)) {
        sfs::permissions(p, writeOnly, sfs::perm_options::remove);
      }
    }
  }

  void
  FileManager::pipedInputFile(sfs::path const& inputFilePath) const
  {
    if (sfs::exists(inputFilePath)) {
      LOG_ERROR << "File already exists: " << inputFilePath.string() << '\n';
      std::exit(Exit::FAILURE);
    }

    std::ofstream f{inputFilePath.string()};
    for (std::string line; std::getline(std::cin, line); ) {
      f << line << std::endl;
    }
    f.close();
  }


  void
  FileManager::pipedInputFileOverwrite(const sfs::path& path) const
  {
    std::ofstream f {path.string(),
                     std::ios::out | std::ios::binary | std::ios::trunc};
    for (std::string line; std::getline(std::cin, line); ) {
      f << line << std::endl;
    }
    f.close();
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
