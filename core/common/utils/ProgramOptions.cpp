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

#include <boost/algorithm/string/join.hpp>

#include <netmeld/core/utils/ProgramOptions.hpp>


namespace netmeld::core::utils {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  ProgramOptions::ProgramOptions()
  {
    optionsMaps.insert({REQUIRED, {}});
    optionsMaps.insert({OPTIONAL, {}});
    optionsMaps.insert({ADVANCED, {}});
    optionsMaps.insert({CONFIG, {}});
  }


  // ===========================================================================
  // Friend Functions
  // ===========================================================================
  std::ostream&
  operator<<(std::ostream& os, const ProgramOptions& npo)
  {
    return (os << npo.cmdLineOptions);
  }


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================

  void
  ProgramOptions::createCustomOptionsMap(const std::string& map)
  {
    if (!optionsMaps.count(map)) {
      optionsMaps.insert({map, {}});
    } else {
      LOG_WARN << "ProgramOptions::createCustomOptionsMap failed to insert "
               << map
               << std::endl;
    }
  }

  void
  ProgramOptions::addOption(const std::string& map, const std::string& key,
                            const OptionsValue& value)
  {
    if (optionsMaps.count(map)) {
      optionsMaps[map].insert({key, value});
    } else {
      LOG_WARN << "ProgramOptions::addOption failed to add value "
               << std::get<0>(value) << " at key " << key << " on map " << map
               << std::endl;
    }
  }

  void
  ProgramOptions::addConfFileOption(const std::string& key,
                                    const OptionsValue& value)
  {
    addOption(CONFIG, key, value);
  }

  void
  ProgramOptions::addRequiredOption(const std::string& key,
                                    const OptionsValue& value)
  {
    addOption(REQUIRED, key, value);
  }

  void
  ProgramOptions::addRequiredDeviceId()
  {
    addRequiredOption("device-id", std::make_tuple(
          "device-id",
          po::value<std::string>()->required(),
          "Name of device.")
        );
    addOptionalOption("device-type", std::make_tuple(
          "device-type",
          po::value<std::string>(),
          "Type of device, determines graph icon.")
        );
    addOptionalOption("device-color", std::make_tuple(
          "device-color",
          po::value<std::string>(),
          "Graph color of device.")
        );
  }

  void
  ProgramOptions::addOptionalOption(const std::string& key,
                                    const OptionsValue& value)
  {
    addOption(OPTIONAL, key, value);
  }

  void
  ProgramOptions::addAdvancedOption(const std::string& key,
                                    const OptionsValue& value)
  {
    addOption(ADVANCED, key, value);
  }

  void
  ProgramOptions::addPositionalOption(const std::string& key, int loc)
  {
    positionalOptionsLocation.add(key.c_str(), loc);
  }

  void
  ProgramOptions::setConfFile(const std::string& _confFile)
  {
    confFile = _confFile;
  }

  bool
  ProgramOptions::exists(const std::string& key) const
  {
    try {
      return (varMap.count(key)) ? true : false;
    } catch (std::exception& e) {
      LOG_ERROR << "ProgramOptions::exists(): "
                << e.what()
                << std::endl;
      std::exit(Exit::FAILURE);
    }
  }

  po::options_description
  ProgramOptions::getMappedOptions(const std::string& name,
                                   const OptionsMap& options) const
  {
    po::options_description newOptions(name);

    for (auto pair : options) {
      auto option = pair.second;

      if (nullptr == std::get<1>(option)) {
        newOptions.add_options()
          (std::get<0>(option),
           std::get<2>(option));
      } else {
        newOptions.add_options()
          (std::get<0>(option),
           std::get<1>(option),
           std::get<2>(option));
      }
    }

    return newOptions;
  }

  std::string
  ProgramOptions::getCommandLine() const
  {
    return commandLine;
  }

  std::string
  ProgramOptions::getValue(const std::string& key) const
  {
    try {
      return varMap.at(key).as<std::string>();
    } catch (std::exception& e) {
      LOG_ERROR << "ProgramOptions::getValue(): "
                << e.what()
                << std::endl;
      std::exit(Exit::FAILURE);
    }
  }

  std::vector<std::string>
  ProgramOptions::getValues(const std::string& key) const
  {
    try {
      if (varMap.count(key)) {
        return varMap.at(key).as<std::vector<std::string>>();
      } else {
        return std::vector<std::string>();
      }
    } catch (std::exception& e) {
      LOG_ERROR << "ProgramOptions::getValues(): "
                << e.what()
                << std::endl;
      std::exit(Exit::FAILURE);
    }
  }

  std::vector<std::string>
  ProgramOptions::getUnrecognized() const
  {
    return unrecognizedArgs;
  }

  void
  ProgramOptions::pipedInputFile(sfs::path const& inputFilePath)
  {
    if (sfs::exists(inputFilePath)) {
      LOG_ERROR << "File already exists: "
                << inputFilePath.string()
                << std::endl;
      std::exit(Exit::FAILURE);
    }

    std::ofstream f{inputFilePath.string()};
    for (std::string line; std::getline(std::cin, line); ) {
      f << line << std::endl;
    }
    f.close();
  }

  void
  ProgramOptions::removeOption(const std::string& map, const std::string& key)
  {
    if (optionsMaps.count(map)) {
      optionsMaps[map].erase(key);
    } else {
      LOG_WARN << "ProgramOptions::removeOption failed to remove "
               << key << " from " << map
               << std::endl;
    }
  }

  void
  ProgramOptions::removeRequiredOption(const std::string& key)
  {
    removeOption(REQUIRED, key);
  }

  void
  ProgramOptions::removeOptionalOption(const std::string& key)
  {
    removeOption(OPTIONAL, key);
  }

  void
  ProgramOptions::removeAdvancedOption(const std::string& key)
  {
    removeOption(ADVANCED, key);
  }

  int
  ProgramOptions::validateOptions(int argc, char** argv)
  {
    std::vector<std::string> args {argv, argv+argc};
    commandLine = boost::algorithm::join(args, " ");

    if (optionsMaps.count(REQUIRED) && !optionsMaps[REQUIRED].empty()) {
      cmdLineOptions.add(getMappedOptions("Required Options",
                         optionsMaps[REQUIRED]));
      if (optionsMaps[REQUIRED].count("data-path")) {
        addPositionalOption("data-path", -1);
      }
    }

    if (optionsMaps.count(OPTIONAL) && !optionsMaps[OPTIONAL].empty()) {
      cmdLineOptions.add(getMappedOptions("Optional Options",
                                          optionsMaps[OPTIONAL]));
    }

    if (optionsMaps.count(ADVANCED) && !optionsMaps[ADVANCED].empty()) {
      cmdLineOptions.add(getMappedOptions("Advanced Options",
                                          optionsMaps[ADVANCED]));
    }

    for (const auto& map : optionsMaps) {
      if (!(map.second.empty())) {
        if (  map.first == REQUIRED
           || map.first == OPTIONAL
           || map.first == ADVANCED
           || map.first == CONFIG) {
          // Skip the option groups manually added to the front of the output
          continue;
        }

        cmdLineOptions.add(getMappedOptions(map.first + " Options",
                                            map.second));
      }
    }

    auto parsedCmdOpts = po::command_line_parser(argc, argv)
                           .options(cmdLineOptions)
                           .positional(positionalOptionsLocation)
                           .allow_unregistered()
                           .run();

    unrecognizedArgs =
      po::collect_unrecognized(parsedCmdOpts.options, po::include_positional);

    po::store(parsedCmdOpts, varMap);

    if (optionsMaps.count(CONFIG) && !optionsMaps[CONFIG].empty()) {
      confFileOptions.add(getMappedOptions("Configuration File Options",
                                           optionsMaps[CONFIG]));
      sfs::path const configPath {confFile};
      std::ifstream config {configPath.string()};
      po::store(po::parse_config_file(config, confFileOptions), varMap);
    }

    if (varMap.count("help")) {
      return HELP;
    }
    if (varMap.count("version")) {
      return VERSION;
    }

    po::notify(varMap);

    if (varMap.count("data-path")) {
      sfs::path const dataFile = varMap.at("data-path").as<std::string>();
      if (varMap.count("pipe")) {
        pipedInputFile(dataFile);
      }
      if (!sfs::exists(dataFile)) {
      LOG_ERROR << "Specified DATA_PATH does not exist: "
                << dataFile.string()
                << std::endl;
      std::exit(Exit::FAILURE);
      }
    }

    return NONE;
  }

  void
  ProgramOptions::addBaseOptions()
  {
    addRequiredOption("db-name", std::make_tuple(
          "db-name",
          po::value<std::string>()->required()->default_value(DEFAULT_DB_NAME),
          "Database to connect to.")
        );
    addAdvancedOption("db-args", std::make_tuple(
          "db-args",
          po::value<std::string>()->default_value(""),
          "Additional database connection args."
          " Space separated `key=value` libpqxx connection string parameters.")
        );

    // Add on prefix to attempt to push to bottom
    addOptionalOption("zzzzzzzzzzhelp", std::make_tuple(
          "help,h",
          NULL_SEMANTIC,
          "Show this help message, then exit.")
        );
    addOptionalOption("zzzzzzzzzzversion", std::make_tuple(
          "version,V",
          NULL_SEMANTIC,
          "Show version information, then exit.")
        );

    addAdvancedOption("zzzzzzzzzzverbosity", std::make_tuple(
          "verbosity,v",
          po::value<Severity>()->default_value(
            LoggerSingleton::getInstance().getLevel()),
          "Alter verbosity level of tool.  See `man syslog` for levels."
          )
        );
  }

  void
  ProgramOptions::addExportOptions()
  {}

  void
  ProgramOptions::addGraphOptions()
  {}

  void
  ProgramOptions::addImportOptions()
  {
    addRequiredDeviceId();
    addRequiredOption("data-path", std::make_tuple(
          "data-path",
          po::value<std::string>()->required(),
          "Data to parse. Either --data-path param or implicit last argument.")
        );

    addOptionalOption("pipe", std::make_tuple(
          "pipe",
          NULL_SEMANTIC,
          "Read input from STDIN; Save a copy to DATA_PATH for parsing.")
        );

    addAdvancedOption("tool-run-id", std::make_tuple(
          "tool-run-id",
          po::value<std::string>(),
          "UUID for this run of the tool.")
        );
    addAdvancedOption("tool-run-metadata", std::make_tuple(
          "tool-run-metadata",
          NULL_SEMANTIC,
          "Insert data into tool_run tables instead of device tables.")
        );

    addPositionalOption("data-path", -1);
  }

  void
  ProgramOptions::addInsertOptions()
  {
    addAdvancedOption("tool-run-id", std::make_tuple(
          "tool-run-id",
          po::value<std::string>(),
          "UUID for this run of the tool.")
        );
  }
}
