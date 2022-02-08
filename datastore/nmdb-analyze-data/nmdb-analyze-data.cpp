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

#include <regex>
#include <yaml-cpp/yaml.h>

#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>
#include <netmeld/core/utils/CmdExec.hpp>

//namespace nmcu = netmeld::core::utils;
namespace nmdt = netmeld::datastore::tools;

class Tool : public nmdt::AbstractDatastoreTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
    const std::string CMDS_ENTRY  {"procedures"};
    const std::string NAME        {"name"};
    const std::string CMDS        {"cmds"};

    nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

    std::vector<std::tuple<std::regex, std::string>> regexes;

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractDatastoreTool(
       "Executes commands to perform data analysis",  // help message
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    void
    addToolOptions() override
    {
      const auto& cmdsFile {nmfm.getConfPath()/"datastore/analyze-cmds.yaml"};
      opts.addRequiredOption("cmds-file", std::make_tuple(
          "cmds-file",
          po::value<std::string>()->required()->default_value(cmdsFile),
          "Command file to run."
          " Either --cmds-file param or implicit last argument.")
        );

      opts.addOptionalOption("example", std::make_tuple(
          "example",
          NULL_SEMANTIC,
          "Generates an example command file.")
        );

      opts.addPositionalOption("cmds-file", -1);
    }

    int
    runTool() override
    {
      const auto& cmdsFile {opts.getValue("cmds-file")};

      if (opts.exists("example")) {
        generateExample();
      } else {
        regexes.emplace_back(std::regex(R"(\{\{dbConnectString\}\})"),
                             getDbConnectString());
        //regexes.emplace_back({std::regex(r"(\{\{\}\})"), R"()"});

        actions(cmdsFile);
      }

      return nmcu::Exit::SUCCESS;
    }

    void
    generateExample()
    {
      YAML::Emitter out;

      auto lfempty = [&](){
        out << YAML::BeginMap
            << YAML::Key << NAME
            << YAML::Value << "" << YAML::Comment(R"(empty name; does nothing)")
            << YAML::Key << CMDS
            << YAML::Value << "" << YAML::Comment(R"(empty cmds; does nothing)")
            << YAML::EndMap
            ;
      };
      auto lfkv = [&](auto& _name, auto& _cmds){
        out << YAML::BeginMap
            << YAML::Key << NAME << YAML::Value << _name
            << YAML::Key << CMDS << YAML::Value << _cmds
            << YAML::EndMap
            ;
      };
      auto lfseq = [&](auto& _name, auto& _cmds){
        out << YAML::BeginMap
            << YAML::Key << NAME << YAML::Value << _name
            << YAML::Key << CMDS << YAML::Value << YAML::BeginSeq
            ;
        for (auto& _cmd : _cmds) {
          out << _cmd;
        }
        out << YAML::EndSeq << YAML::EndMap;
      };
      auto lflit = [&](auto& _name, auto& _cmds){
        out << YAML::BeginMap
            << YAML::Key << NAME << YAML::Value << _name
            << YAML::Key << CMDS << YAML::Value << YAML::BeginSeq
            << YAML::Literal << _cmds
            << YAML::EndSeq << YAML::EndMap;
      };

      out << YAML::BeginDoc
          << YAML::BeginMap
          << YAML::Key << CMDS_ENTRY << YAML::Value
          << YAML::BeginSeq
          ;

      lfempty();
      lfkv(R"(Simple, singular command)", R"(ls)");
      lfkv(R"(Simple, multiple commands)", R"(whoami; pwd;)");
      lfkv(R"(Error (unless you are root))", R"(ls /root)");

      std::vector<std::string> v;
      v.emplace_back(R"(whoami;)");
      v.emplace_back(R"(pwd;)");
      lfseq(R"(General, multiple commands)", v);

      v.clear();
      v.emplace_back(R"(psql "{{dbConnectString}}" -A -c ")");
      v.emplace_back(R"(SELECT * FROM ip_addrs LIMIT 5)");
      v.emplace_back(R"(")");
      lfseq(R"(Variable, `{{var}}`, substitution)", v);

      const auto s {
          R"(psql "{{dbConnectString}}" -A -c ")" "\n"
          R"(  SELECT * FROM ip_addrs LIMIT 5)" "\n"
          R"(")"
        };
      lflit(R"(Prior example as literal (i.e., no escaping))", s);

      out << YAML::EndSeq
          << YAML::EndMap
          << YAML::EndDoc
          ;

      LOG_INFO << out.c_str() << std::endl;
    }

    void
    actions(const std::string& cmdsFile)
    {
      YAML::Node yConfig {YAML::LoadFile(cmdsFile)};

      const auto& yProcedures {yConfig[CMDS_ENTRY]};

      size_t totalProcedures  {yProcedures.size()};
      size_t successes        {0};

      for (const auto& yProcedure : yProcedures) {
        // get procedure name
        const auto& name {yProcedure[NAME].as<std::string>()};

        // build command chain
        const auto& yCmds {yProcedure[CMDS]};
        std::ostringstream oss;
        if (yCmds.IsScalar()) {
          oss << yCmds.as<std::string>();
        } else if (yCmds.IsSequence()) {
          for (const auto& yCmd : yCmds) {
            oss << yCmd.as<std::string>();
          }
        } else {
          LOG_INFO << "# Skipping unknown `cmds` node type\n";
        }

        // do command chain variable substitution
        const auto cmds {regexReplace(oss.str())};
        LOG_INFO << "# Start -- " << name << '\n'
                 << "# Executing: `" << cmds << "`\n";

        // exec commands
        LOG_INFO << "# Results:" << std::endl;
        int success {nmcu::cmdExecOrExit(cmds)};
        if (0 == success) {
          ++successes;
        }
        LOG_INFO << "# End -- " << name << "\n\n";
      }
      LOG_INFO << "# Success counts: "
               << successes << '/' << totalProcedures
               << '\n';
    }

    std::string
    regexReplace(const std::string& _var) const
    {
      // Replace keywords with values
      auto out {_var};
      for (const auto& [k,v] : regexes) {
        out = std::regex_replace(out, k, v);
      }

      return out;
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
