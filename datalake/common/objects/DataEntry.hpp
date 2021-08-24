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

#ifndef DATA_ENTRY_HPP
#define DATA_ENTRY_HPP

#include <netmeld/core/objects/AbstractObject.hpp>

namespace nmco = netmeld::core::objects;


namespace netmeld::datalake::objects {

  class DataEntry : public nmco::AbstractObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      bool pipedData {false};

      std::string deviceId   {""};
      std::string dataPath   {""};
      std::string ingestTool {""};
      std::string toolArgs   {""};
      std::string newName    {""};
      std::string committer  {""};

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors and Destructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors and destructors part of public API
      virtual ~DataEntry() = default;
      DataEntry();

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      bool isPipedData() const;

      std::string getCommitter() const;
      std::string getDataPath() const;
      std::string getDeviceId() const;
      std::string getIngestCmd() const;
      std::string getIngestTool() const;
      std::string getNewName() const;
      std::string getSaveName() const;
      std::string getToolArgs() const;

      void setCommitter(const std::string&);
      void setDataPath(const std::string&);
      void setDeviceId(const std::string&);
      void setIngestTool(const std::string&);
      void setNewName(const std::string&);
      void setToolArgs(const std::string&);

      std::string toDebugString() const override;
  };
}
#endif // DATA_ENTRY_HPP
