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

#ifndef PROGRAM_OPTIONS_HPP
#define PROGRAM_OPTIONS_HPP

#include <boost/program_options.hpp>

#include <netmeld/core/utils/FileManager.hpp>
#include <netmeld/core/utils/LoggerSingleton.hpp>


namespace po = boost::program_options;

#define NULL_SEMANTIC static_cast<const po::value_semantic*>(NULL)


namespace netmeld::core::utils {

  typedef std::tuple<const char*, const po::value_semantic*, const char*>
    OptionsValue;

  typedef std::map<std::string, OptionsValue>
    OptionsMap;

  class ProgramOptions {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
      FileManager& nmfm {FileManager::getInstance()};

      po::options_description             cmdLineOptions;
      po::options_description             confFileOptions;
      po::positional_options_description  positionalOptionsLocation;
      std::map<std::string, OptionsMap>   optionsMaps;
      po::variables_map                   varMap;
      std::vector<std::string>            unrecognizedArgs;

      std::string                         commandLine;
      std::string                         confFile;

      const std::string REQUIRED  {"Required"};
      const std::string OPTIONAL  {"Optional"};
      const std::string ADVANCED  {"Advanced"};
      const std::string CONFIG    {"Configuration"};

    public:
      static const int NONE    {0};
      static const int HELP    {1};
      static const int VERSION {2};


    // =========================================================================
    // Constructors
    // =========================================================================
    public:
      ProgramOptions();


    // =========================================================================
    // Methods
    // =========================================================================
    private:
      po::options_description getMappedOptions(const std::string&,
                                               const OptionsMap&) const;

    public:
      void createCustomOptionsMap(const std::string&);

      void addOption(const std::string&, const std::string&,
                     const OptionsValue&);
      void addRequiredOption(const std::string&, const OptionsValue&);
      void addOptionalOption(const std::string&, const OptionsValue&);
      void addAdvancedOption(const std::string&, const OptionsValue&);
      //void addConfFileOption(const std::string&, const OptionsValue&);
      void addPositionalOption(const std::string&, int loc);

      void removeOption(const std::string&, const std::string&);
      void removeRequiredOption(const std::string&);
      void removeOptionalOption(const std::string&);
      void removeAdvancedOption(const std::string&);

      //void setConfFile(const std::string&);

      bool exists(const std::string&) const;
      const std::string& getCommandLine() const;
      std::string getValue(const std::string&) const;
      std::vector<std::string> getValues(const std::string&) const;
      const std::vector<std::string>& getUnrecognized() const;

      int validateOptions(int argc, char** argv);

      friend std::ostream& operator<<(std::ostream&, const ProgramOptions&);

      template<typename T>
      T getValueAs(const std::string& key) const
      {
        try {
          return varMap.at(key).as<T>();
        } catch (std::exception& e) {
          LOG_ERROR << "ProgramOptions::getValueAs<T>(): key lookup issue for "
                    << key
                    << std::endl;
          std::exit(Exit::FAILURE);
        }
      }
  };
}
#endif // PROGRAM_OPTIONS_HPP
