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

namespace nmct = netmeld::core::tools;


// =============================================================================
// Tool definition
// =============================================================================
class Tool : public nmct::AbstractTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:    // Variables intended for internal only use
    std::string encoded {""};

    const std::string ciscoType7Key {
        R"(dsfd;kfoA,.iyewrkldJKDHSUBsgvca69834ncxv9873254k;fg87)"
      };

  protected:  // Variables intended for internal/subclass API
    std::string decoded {""};

  public:     // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private:    // Constructors should rarely appear at this scope
  protected:  // Constructors intended for internal/subclass API
  public:     // Constructors should generally be public
    Tool() : nmct::AbstractTool
      ( "Cisco type 7 decoder"  // unused unless printHelp() is overridden
      , PROGRAM_NAME            // program name (set in CMakeLists.txt)
      , PROGRAM_VERSION         // program version (set in CMakeLists.txt)
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
            "password"
          , po::value<std::string>()->required()
          , "Type 7 encoded password."
          )
        );
      opts.addPositionalOption("password", -1);

      opts.addAdvancedOption("store-in-db", std::make_tuple(
            "store-in-db"
          , NULL_SEMANTIC
          , "Used to store results in the database."
            " Requires 'psql' binary."
          )
        );

      opts.addAdvancedOption("db-name", std::make_tuple(
            "db-name"
          , po::value<std::string>()->default_value("site")
          , "Database to connect to."
          )
        );

      opts.addAdvancedOption("db-args", std::make_tuple(
            "db-args"
          , po::value<std::string>()->default_value("")
          , "Additional database connection args."
            " Space separated `key=value` libpqxx connection string parameters."
          )
        );
    }

  protected: // Methods part of subclass API
    uint8_t
    fromByte(const std::string_view byte, const uint8_t radix) const
    {
      uint8_t val;
      auto [ptr, ec] = std::from_chars(byte.cbegin(), byte.cend(), val, radix);

      if (ec == std::errc()) {
        return val;
      } else {
        throw std::runtime_error(std::format(
                "Conversion of {} failed for base {}"
              , byte
              , radix
            )
          );
      }
    }

    std::vector<uint8_t>
    getBytes(const std::string& data) const
    {
      size_t numChars {data.size()};
      if (0 != (numChars % 2)) {
        throw std::runtime_error("Invalid encoded data length, uneven");
      }

      std::vector<uint8_t> bytes;
      for (size_t i {0}; i < numChars; i+=2) {
        std::string_view byte {std::string_view(data).substr(i,2)};
        if (i == 0) {
          LOG_DEBUG << "Adding to byte vector a dec: " << byte << '\n';
          bytes.emplace_back(fromByte(byte, 10));
        } else {
          LOG_DEBUG << "Adding to byte vector a hex: " << byte << '\n';
          bytes.emplace_back(fromByte(byte, 16));
        }
      }

      return bytes;
    }

    void
    decode(const std::string& data)
    {
      if (0 == data.size()) {
        return;
      }

      std::regex badChars("[^0-9a-fA-F]");
      encoded = std::regex_replace(data, badChars, "");

      std::vector<uint8_t> byteVec {getBytes(data)};

      std::vector<uint8_t>::const_iterator i {byteVec.cbegin()}
                                         , e {byteVec.cend()}
        ;
      size_t index = *i; // init as value of the decimal byte
      ++i;

      std::ostringstream oss;
      for (; i != e; ++i, ++index) {
        LOG_DEBUG << "i-dist: " << std::distance(byteVec.cbegin(), i)
                  << ", index: " << std::dec << index
                  << ", *i: " << std::dec << *i
                  << ", pow: " << ciscoType7Key[index % ciscoType7Key.size()]
                  << std::endl;
        oss << static_cast<char>(
                 *i ^ ciscoType7Key[index % ciscoType7Key.size()]
               );
      }
      decoded = oss.str();
    }

    void
    storeInDb() const
    {
      if (!nmcu::isCmdAvailable("psql")) {
        LOG_WARN << "Could not store, 'psql' was not found\n";
        return;
      }

      const std::string dbName {opts.getValue("db-name")};
      const std::string dbArgs {opts.getValue("db-args")};

      nmcu::cmdExecOrExit(
          std::format( R"(psql "{} dbname={}" -c)"
                       R"(  "INSERT INTO raw_tool_observations)"
                         "     (tool_run_id, category, observation)"
                         "   VALUES"
                         "     ('{}', '{}', 'Encoded: {}\nDecoded: {}')"
                       R"(   ON CONFLICT DO NOTHING")"
                     , dbArgs
                     , dbName
                     , "32b2fd62-08ff-4d44-8da7-6fbd581a90c6"
                     , "notable"
                     , encoded
                     , decoded
                     )
        );
    }

    int
    runTool() override
    {
      decode(opts.getValue("password"));
      LOG_INFO << encoded << ": " << decoded << '\n';

      if (opts.exists("store-in-db")) {
        storeInDb();
      }

      return nmcu::Exit::SUCCESS;
    }

  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
#ifndef UNIT_TESTING
int main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
#endif
