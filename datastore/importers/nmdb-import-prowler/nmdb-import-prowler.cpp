// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/tools/AbstractImportTool.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp> // if parser not needed

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportTool<P,R>
      (
       "prowler -M json",  // command line tool imports data from
       PROGRAM_NAME,       // program name (set in CMakeLists.txt)
       PROGRAM_VERSION     // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportTool
    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addAdvancedOption("device-id", std::make_tuple(
            "device-id"
          , po::value<std::string>()
          , "(Not used) Name of device."
          )
        );

      this->opts.addRequiredOption("prowler-version", std::make_tuple(
            "prowler-version"
          , po::value<uint16_t>()->default_value(3)
          , "Which prowler version's JSON to process."
            " Known change between v2 and v3."
          )
        );

      this->opts.removeOptionalOption("device-type");
      this->opts.removeOptionalOption("device-color");
    }

    void
    parseData() override
    {
      std::ifstream f {this->getDataPath().string()};
      const auto version {this->opts.template getValueAs<uint16_t>("prowler-version")};

      this->executionStart = nmco::Time();
      try {
        Parser parser;

        if (2 == version) {
          parser.fromJsonV2(f);
        } else if (3 == version) {
          parser.fromJsonV3(f);
        } else {
          LOG_WARN << "No valid version given; aborting\n";
          std::exit(nmcu::Exit::FAILURE);
        }

        this->tResults = parser.getData();

      } catch (json::out_of_range& ex) {
        LOG_ERROR << "Parse error " << ex.what()
                  << std::endl
                  ;
        std::exit(nmcu::Exit::FAILURE);
      } catch (json::parse_error& ex) {
        LOG_ERROR << "Parse error at byte " << ex.byte
                  << " -- " << ex.what()
                  << std::endl
                  ;
        std::exit(nmcu::Exit::FAILURE);
      }
      this->executionStop = nmco::Time();
    }

    // Overriden from AbstractImportTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& results : this->tResults) {

        for (auto& entry : results.v2Data) {
          entry.save(t, toolRunId, deviceId);
          LOG_DEBUG << entry.toDebugString() << std::endl;
        }

        for (auto& entry : results.v3Data) {
          entry.save(t, toolRunId, deviceId);
          LOG_DEBUG << entry.toDebugString() << std::endl;
        }
      }
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool<nmdp::DummyParser, Result> tool; // custom parser
  return tool.start(argc, argv);
}
