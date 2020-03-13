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

#include <netmeld/core/parsers/ParserHelper.hpp>
#include <netmeld/core/tools/AbstractTool.hpp>


namespace nmct = netmeld::core::tools;
namespace nmcp = netmeld::core::parsers;
namespace nmcu = netmeld::core::utils;

typedef std::vector<uint8_t>  Result;


class Parser :
  public qi::grammar<nmcp::IstreamIter, Result()>
{
  public:
    Parser() : Parser::base_type(start)
    {
      start =
        decByte >> +hexByte >> -qi::eol
        ;
    }
    
    qi::rule<nmcp::IstreamIter, Result()>
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
    addToolBaseOptions() override // Pre-subclass operations
    {
      opts.addRequiredOption("password", std::make_tuple(
            "password",
            po::value<std::string>()->required(),
            "type 7 encoded password")
          );

      opts.addPositionalOption("password", -1);

      opts.removeRequiredOption("db-name");
      opts.removeOptionalOption("pipe");
      opts.removeAdvancedOption("tool-run-metadata");
    }

    // Overriden from AbstractTool
    void
    modifyToolOptions() override { } // Private means no intention of allowing a subclass

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printHelp() const;
      // virtual void printVersion() const;
      // virtual int  runTool();

    int
    runTool() override
    {
      std::ostringstream oss;
      std::string encPass {opts.getValue("password")};

      Result r {nmcp::fromString<Parser,Result>(encPass)};

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

      LOG_INFO << std::regex_replace(encPass, badChars, "")
               << ": "
               << oss.str()
               << '\n'
               ;

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
