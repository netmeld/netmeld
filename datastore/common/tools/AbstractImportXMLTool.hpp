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

#ifndef ABSTRACT_IMPORT_JSON_TOOL_HPP
#define ABSTRACT_IMPORT_JSON_TOOL_HPP

#include <netmeld/core/objects/Time.hpp>
#include <netmeld/core/objects/Uuid.hpp>
#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <pugixml.hpp>

namespace nmco = netmeld::core::objects;
namespace nmdo = netmeld::datastore::objects;


namespace netmeld::datastore::tools {

  template<typename TParser, typename TResults>
  class AbstractImportXMLTool : public AbstractDatastoreTool
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
      sfs::path   dataPath;

    protected:
      TResults    tResults;

      nmco::Uuid  toolRunId;
      nmco::Time  executionStart;
      nmco::Time  executionStop;

      nmdo::DeviceInformation devInfo;


    // =========================================================================
    // Constructors and Destructors
    // =========================================================================
    protected:
      // Default constructor, provided only for convienence
      AbstractImportXMLTool();
      // Standard constructor, should be primary
      AbstractImportXMLTool(const char*, const char*, const char*);

    public:
      virtual ~AbstractImportXMLTool() = default;

    // =========================================================================
    // Methods
    // =========================================================================
    private:
      // Performs default inserts into the DB
      void generalInserts(pqxx::transaction_base&, const std::string&);
      void addModuleOptions() override;

    protected:
      const sfs::path   getDataPath() const;
      const std::string getDeviceId() const;
      const nmco::Uuid  getToolRunId() const;
      virtual void addToolOptions() override;
      virtual void parseData();
      // Tool specific behavior entry point
      virtual void specificInserts(pqxx::transaction_base&);
      // Tool run metadata behavior entry point
      virtual void toolRunMetadataInserts(pqxx::transaction_base&);
  };
}
#include "AbstractImportXMLTool.ipp"
#endif // ABSTRACT_IMPORT_JSON_TOOL_HPP
