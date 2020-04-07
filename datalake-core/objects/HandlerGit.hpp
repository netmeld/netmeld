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

#ifndef HANDLER_GIT_HPP
#define HANDLER_GIT_HPP

#include <netmeld/core/utils/FileManager.hpp>

#include "DataLake.hpp"

namespace nmcu = netmeld::core::utils;


namespace netmeld::datalake::core::objects {

  class HandlerGit : public DataLake {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
      const std::string  CHECK_IN_PREFIX     {"check-in:"};
      const std::string  IMPORT_TOOL_PREFIX  {"import-tool:"};
      const std::string  TOOL_ARGS_PREFIX    {"tool-args:"};

      sfs::path    dataLakePath;
      std::string  dataLakeDir;
      std::string  deviceDir;


    protected: // Variables intended for internal/subclass API
      nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      HandlerGit();

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
      void setImportToolData(DataEntry&, const std::string&);
      
      void cmdExec(const std::string&);
      std::string cmdExecOut(const std::string&);

      bool alignRepo(const nmco::Time& = nmco::Time("infinity"));
      bool changeDirToRepo();

    protected: // Methods part of subclass API
    public: // Methods part of public API
      void commit(DataEntry&) override;
      void initialize() override;
      void removeAll(const std::string&, const std::string&) override;
      void removeLast(const std::string&, const std::string&) override;

      std::vector<DataEntry> getDataEntries(const nmco::Time& = {}) override;
  };
}
#endif // HANDLER_GIT_HPP
