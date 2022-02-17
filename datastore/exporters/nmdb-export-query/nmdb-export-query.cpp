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

#include <netmeld/datastore/tools/AbstractExportTool.hpp>
#include <boost/format.hpp>

#include "WriterContext.hpp"

namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;


// =============================================================================
// Export tool definition
// =============================================================================
class Tool : public nmdt::AbstractExportTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    float maxColumnWidth {1.0};
    std::vector<std::string> columnNames;

  protected: // Variables intended for internal/subclass API
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;

  public: // Variables should rarely appear at this scope

  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractExportTool
      ("ConTeXt formatted table of psql query",
       PROGRAM_NAME,
       PROGRAM_VERSION)
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractExportTool
    void
    addToolOptions() override
    {
      opts.addRequiredOption("query", std::make_tuple(
            "query,q",
            po::value<std::string>()->required(),
            "Query to convert to ConTeXt table")
          );

      opts.addOptionalOption("columnWidth", std::make_tuple(
            "columnWidth,w",
            po::value<std::vector<float>>()->multitoken()->composing(),
            "Specific column width(s) to use. Must equate to 1.0.")
          );
    }

    // Overriden from AbstractExportTool
    int
    runTool() override
    {
      pqxx::connection db       {getDbConnectString()};
      pqxx::read_transaction t  {db};
      pqxx::result records      {t.exec(opts.getValue("query"))};

      // Populate column width and sanity check
      std::vector<float> sizes;
      if (opts.exists("columnWidth")) {
        sizes = opts.getValueAs<std::vector<float>>("columnWidth");
        if (sizes.size() != records.columns()) {
          LOG_ERROR << "Column width and query count mismatch: "
                    << sizes.size() << " vs " << records.columns()
                    << std::endl;
          std::exit(nmcu::Exit::FAILURE);
        }
      } else {
        float size = (records.columns() != 0)
                   ? maxColumnWidth / static_cast<float>(records.columns())
                   : maxColumnWidth;
        size = std::round(size*100.0F) / 100.0F;
        for (size_t i {0}; i < records.columns(); i++) {
          sizes.push_back(size);
        }
      }

      float sizeTotal = 0.0;
      for (size_t i {0}; i < sizes.size(); i++) {
        sizeTotal += sizes[i];
        LOG_DEBUG << "Added "
                  << boost::format("%|.8f|") % sizes[i]
                  << " size to total, now "
                  << boost::format("%|.8f|") % sizeTotal
                  << std::endl;
      }

      if (maxColumnWidth < sizeTotal) {
        LOG_ERROR << "Column widths total more than "
                  << boost::format("%|.8f|") % maxColumnWidth
                  << ": "
                  << boost::format("%|.8f|") % sizeTotal
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      WriterContext wc;
      wc.addQueryInfo(opts.getCommandLine(), opts.getValue("query"));

      // Get column names
      for (const auto& record : records) {
        unsigned int i=0;
        for (const auto& field : record) {
          wc.addColumn(field.name(), sizes[i]);
          LOG_DEBUG << field.name() << " -- " << sizes[i] << std::endl;
          ++i;
        }
        break; // only iterate once as all columns are named the same
      }

      for (const auto& record : records) {
        std::vector<std::string> fields;
        for (const auto& field : record) {
          std::string fs = field.c_str();
          // todo sanitize in writer
          fields.push_back(fs);
          LOG_DEBUG << "\"" << field << "\"" << std::endl;
        }
        wc.addRow(fields);
      }

      LOG_INFO << wc.write();

      return nmcu::Exit::SUCCESS;
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractExportTool at this scope
      // virtual void printHelp() const;

  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool tool;
  return tool.start(argc, argv);
}
