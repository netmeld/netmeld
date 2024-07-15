// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

  /*
  void
  ProgramOptions::addConfFileOption(const std::string& key,
                                    const OptionsValue& value)
  {
    addOption(CONFIG, key, value);
  }
  */

  void
  ProgramOptions::addRequiredOption(const std::string& key,
                                    const OptionsValue& value)
  {
    addOption(REQUIRED, key, value);
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

  /*
  void
  ProgramOptions::setConfFile(const std::string& _confFile)
  {
    confFile = _confFile;
  }
  */

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

  const std::string&
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

  const std::vector<std::string>&
  ProgramOptions::getUnrecognized() const
  {
    return unrecognizedArgs;
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

    if (varMap.count("data-path") && optionsMaps[REQUIRED].count("data-path")) {
      sfs::path const dataFile {varMap.at("data-path").as<std::string>()};
      if (varMap.count("pipe")) {
        nmfm.pipedInputFile(dataFile);
      }
      if (!sfs::exists(dataFile)) {
        LOG_ERROR << "Specified DATA_PATH does not exist: "
                  << dataFile.string() << '\n';
        std::exit(Exit::FAILURE);
      }
    }

    return NONE;
  }
}
