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

#include <format>
#include <regex>

#include <netmeld/core/tools/AbstractTool.hpp>
#include <netmeld/core/utils/CmdExec.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>


namespace nmct = netmeld::core::tools;
namespace nmdp = netmeld::datastore::parsers;

typedef std::vector<uint8_t>  Result;


class Parser :
  public qi::grammar<nmdp::IstreamIter, Result()>
{
  public:
    Parser() : Parser::base_type(start)
    {
      start =
        decByte >> +hexByte >> -qi::eol
        ;
    }

    qi::rule<nmdp::IstreamIter, Result()>
      start;

    qi::uint_parser<uint8_t, 10, 2, 2> decByte;
    qi::uint_parser<uint8_t, 16, 2, 2> hexByte;
};


class Tool : public nmct::AbstractTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    std::string const ciscoType7Key
      {"dsfd;kfoA,.iyewrkldJKDHSUBsgvca69834ncxv9873254k;fg87"};

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmct::AbstractTool
      (
       "Cisco type 7 decoder",  // unused unless printHelp() is overridden
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractTool
    void
    addToolOptions() override
    {
      opts.addRequiredOption("password", std::make_tuple(
            "password",
            po::value<std::string>()->required(),
            "type 7 encoded password")
          );

      opts.addPositionalOption("password", -1);

      opts.addAdvancedOption("store-in-db", std::make_tuple(
            "store-in-db",
            NULL_SEMANTIC,
            "Used to store results in the database. Requires Netmeld and PostgreSQL."
          ));

      opts.addAdvancedOption("db-name", std::make_tuple(
          "db-name",
          po::value<std::string>()->default_value("site"),
          "Database to connect to.")
          );

      opts.addAdvancedOption("db-args", std::make_tuple(
          "db-args",
          po::value<std::string>()->default_value(""),
          "Additional database connection args."
          " Space separated `key=value` libpqxx connection string parameters.")
          );
    }

  protected: // Methods part of subclass API
    int
    runTool() override
    {
      std::ostringstream oss;
      std::string encPass {opts.getValue("password")};

      Result r {nmdp::fromString<Parser,Result>(encPass)};

      std::vector<uint8_t>::const_iterator
        i = r.cbegin(),
        e = r.cend();

      size_t index = *i;
      ++i;

      for (; i != e; ++i, ++index) {
        oss << static_cast<char>(
                *i ^ ciscoType7Key[index % ciscoType7Key.size()]
               );
      }

      std::regex badChars("[^0-9a-fA-F]");
      auto encoded = std::regex_replace(encPass, badChars, "");
      auto decoded = oss.str();

      if (opts.exists("store-in-db")) {
        if (nmcu::isCmdAvailable("nmdb-initialize") && nmcu::isCmdAvailable("psql")) {
          std::string dbName {opts.getValue("db-name")};
          std::string dbArgs {opts.getValue("db-args")};
          nmcu::cmdExecOrExit(
              std::format(R"(psql {} --dbname={} -c "insert into raw_tool_observations (tool_run_id, category, observation) )"
                          R"(values ('{}', '{}', 'Encoded: {}\nDecoded: {}') on conflict do nothing")",
                          dbArgs, dbName, "32b2fd62-08ff-4d44-8da7-6fbd581a90c6", "notable", encoded, decoded));
        } else {
          LOG_INFO << "warning: could not save to psql because Netmeld or psql could not be found";
        }
      } else {
        LOG_INFO << encoded << ": " << decoded << '\n';
      }

      return nmcu::Exit::SUCCESS;
    }

  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
