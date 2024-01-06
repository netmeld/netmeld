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
#include <numeric>

#include <netmeld/core/tools/AbstractTool.hpp>
#include <netmeld/core/utils/CmdExec.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>


namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;
namespace nmdp = netmeld::datastore::parsers;


typedef std::string  Result;

class Parser :
  public qi::grammar<nmdp::IstreamIter, Result()>
{
  public:
    Parser() : Parser::base_type(start)
    {
      // cppcheck-suppress useInitializationList
      start =
        qi::lit("$9$") >> +qi::ascii::graph >> -qi::eol
        ;
    }

    qi::rule<nmdp::IstreamIter, Result()>
      start;
};


class Tool : public nmct::AbstractTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    // Used used for lookup table generation
    const std::vector<std::string> keyFamilies
      {"QzF3n6/9CAtpu0O", "B1IREhcSyrleKvMW8LXx", "7N-dVbwsY2g4oaJZGUDj", "iHkq.mPf5T"};
    const std::string key
      {"QzF3n6/9CAtpu0OB1IREhcSyrleKvMW8LXx7N-dVbwsY2g4oaJZGUDjiHkq.mPf5T"};


    // Used for decoding
    const uint8_t KEY_SIZE {static_cast<uint8_t>(key.size())};

    const std::vector<std::vector<uint8_t>> encoding
      {{1,4,32},{1,16,32},{1,8,32},{1,64},{1,32},{1,4,16,128},{1,32,64}};

    const std::map<char, uint8_t> alphaNum
      { {'-', 37}, {'.', 59}, {'/', 6},  {'0', 13}, {'1', 16}, {'2', 44},
        {'3', 3},  {'4', 46}, {'5', 63}, {'6', 5},  {'7', 35}, {'8', 31},
        {'9', 7},  {'A', 9},  {'B', 15}, {'C', 8},  {'D', 53}, {'E', 19},
        {'F', 2},  {'G', 51}, {'H', 56}, {'I', 17}, {'J', 49}, {'K', 27},
        {'L', 32}, {'M', 29}, {'N', 36}, {'O', 14}, {'P', 61}, {'Q', 0},
        {'R', 18}, {'S', 22}, {'T', 64}, {'U', 52}, {'V', 39}, {'W', 30},
        {'X', 33}, {'Y', 43}, {'Z', 50}, {'a', 48}, {'b', 40}, {'c', 21},
        {'d', 38}, {'e', 26}, {'f', 62}, {'g', 45}, {'h', 20}, {'i', 55},
        {'j', 54}, {'k', 57}, {'l', 25}, {'m', 60}, {'n', 4},  {'o', 47},
        {'p', 11}, {'q', 58}, {'r', 24}, {'s', 42}, {'t', 10}, {'u', 12},
        {'v', 28}, {'w', 41}, {'x', 34}, {'y', 23}, {'z', 1}
      };

    const std::map<char, uint8_t> extra
      { {'-', 1}, {'.', 0}, {'/', 3}, {'0', 3}, {'1', 2}, {'2', 1}, {'3', 3},
        {'4', 1}, {'5', 0}, {'6', 3}, {'7', 1}, {'8', 2}, {'9', 3}, {'A', 3},
        {'B', 2}, {'C', 3}, {'D', 1}, {'E', 2}, {'F', 3}, {'G', 1}, {'H', 0},
        {'I', 2}, {'J', 1}, {'K', 2}, {'L', 2}, {'M', 2}, {'N', 1}, {'O', 3},
        {'P', 0}, {'Q', 3}, {'R', 2}, {'S', 2}, {'T', 0}, {'U', 1}, {'V', 1},
        {'W', 2}, {'X', 2}, {'Y', 1}, {'Z', 1}, {'a', 1}, {'b', 1}, {'c', 2},
        {'d', 1}, {'e', 2}, {'f', 0}, {'g', 1}, {'h', 2}, {'i', 0}, {'j', 1},
        {'k', 0}, {'l', 2}, {'m', 0}, {'n', 3}, {'o', 1}, {'p', 3}, {'q', 0},
        {'r', 2}, {'s', 1}, {'t', 3}, {'u', 3}, {'v', 2}, {'w', 1}, {'x', 2},
        {'y', 2}, {'z', 3}
      };

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
    Tool() : nmct::AbstractTool
      (
       "Junos type 9 decoder",  // unused unless printHelp() is overridden
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    { }


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
            po::value<std::string>()->required(),//->default_value(std::string()),
            "Type 9 encoded password")
          );
      opts.addPositionalOption("password", -1);

      opts.addAdvancedOption("gen-lookup", std::make_tuple(
            "gen-lookup",
            NULL_SEMANTIC,
            "Generate lookup table instead of decode")
          );

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

    void
    genLookup()
    {
      LOG_INFO << "Alpha-Size Mapping: ";
      auto keyIndexMax {keyFamilies.size()-1};
      for (size_t i {0}; i <= keyIndexMax; ++i) {
        for (char c : keyFamilies.at(i)) {
          LOG_INFO << '{' << c << ',' << (keyIndexMax-i) << '}';
        }
      }
      LOG_INFO << '\n';

      LOG_INFO << "Alpha-Numeric Mapping: ";
      for (size_t i {0}; i < KEY_SIZE; ++i) {
        LOG_INFO << '{' << key.at(i) << ',' << i << '}';
      }
      LOG_INFO << '\n';
    }

    std::pair<std::string, std::string>
    getNext(const std::string& str, const size_t len)
    {
      if (str.empty() || str.size() < len) {
        LOG_ERROR << "Expected " << len << " characters, but only "
                  << str.size() << " left\n";
        std::exit(nmcu::Exit::FAILURE);
      }

      return {str.substr(0, len), str.substr(len)};
    }

    char
    gapDecode(const std::vector<uint8_t>& gaps,
              const std::vector<uint8_t>& moduli)
    {
      if (gaps.size() != moduli.size()) {
        LOG_ERROR << "Gaps and moduli size mismatch";
        std::exit(nmcu::Exit::FAILURE);
      }

      uint8_t num {0};
      for (size_t x {0}; x < gaps.size(); ++x) {
        num = static_cast<uint8_t>(num + (gaps.at(x) * moduli.at(x)));
      }
      return static_cast<char>(num);
    }

    std::string
    decode(const std::string& encoded)
    {
      auto [next, rest]  {getNext(encoded, 1)};
      auto prev          {next.front()};

      std::string decoded;
      std::tie(next, rest) = getNext(rest, extra.at(prev));
      while (!rest.empty()) {
        auto decodeModuli {encoding[decoded.size() % encoding.size()]};

        std::vector<uint8_t> gaps;
        std::tie(next, rest) = getNext(rest, decodeModuli.size());
        for (auto c : next) {
          auto diff {KEY_SIZE + alphaNum.at(c) - alphaNum.at(prev)};
          gaps.push_back(static_cast<uint8_t>((diff % KEY_SIZE) - 1));
          prev = c;
        }
        decoded.append(1, gapDecode(gaps, decodeModuli));
      }
      return decoded;
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printHelp() const;
      // virtual void printVersion() const;

    int
    runTool() override
    {
      if (opts.exists("gen-lookup")) {
        genLookup();
      } else {
        Result encoded {
          nmdp::fromString<Parser, Result>(opts.getValue("password"))
        };
        auto decoded {decode(encoded)};

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
      }
      return nmcu::Exit::SUCCESS;
    }

  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
