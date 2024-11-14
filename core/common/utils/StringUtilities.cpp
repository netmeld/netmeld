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

#include <algorithm>
#include <format>
#include <vector>
#include <tuple>

#include <netmeld/core/utils/StringUtilities.hpp>


namespace netmeld::core::utils
{
  std::string
  toLower(const std::string& orig)
  {
    std::string text {orig};
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return text;
  }

  std::string
  toUpper(const std::string& orig)
  {
    std::string text {orig};
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c){ return std::toupper(c); });
    return text;
  }

  std::string
  trim(const std::string& orig)
  {
    std::string text {orig};
    if (text.front() == ' ') {
      text.erase(text.begin(),
                 std::find_if(text.begin(),
                              text.end(),
                              [](int ch) { return !std::isspace(ch); }
                             )
                );
    }
    if (text.back() == ' ') {
      text.erase(std::find_if(text.rbegin(),
                              text.rend(),
                              [](int ch) { return !std::isspace(ch); }
                             ).base(),
                 text.end()
                );
    }

    return text;
  }

  std::string
  getSrvcString(const std::string& _proto,
                const std::string& _srcPorts, const std::string& _dstPorts)
  {
    return _proto + ":" + _srcPorts + ":" + _dstPorts;
  }

  std::string
  expandCiscoIfaceName(const std::string& _ifaceName)
  {
    // Various Cisco output uses two-letter interface prefixes.
    // These short interface names need to be expanded in order to match
    // the interface names obtained from the running configuration.
    static const std::vector<std::tuple<std::string, std::string>> lookups {
      // reverse sort (sort!) to ensure shorter spellings don't match first
        {"vl", "vlan"}
      , {"twe", "twentyfivegigabitethernet"}
      , {"tw", "twogigabitethernet"}
      , {"tu", "tunnel"}
      , {"te", "tengigabitethernet"}
      , {"se", "serial"}
      , {"po", "port-channel"}
      , {"lo", "loopback"}
      , {"gi", "gigabitethernet"}
      , {"fo", "fortygigabitethernet"}
      , {"fi", "fivegigabitethernet"}
      , {"fa", "fastethernet"}
      , {"et", "ethernet"}
      };

    std::string ifaceName {toLower(_ifaceName)};
    trim(ifaceName);

    for (const auto& [abbrev, full] : lookups) {
      if (   ifaceName.starts_with(abbrev)
         && !ifaceName.starts_with(full)
         )
      {
        ifaceName.replace(0, abbrev.size(), full);
        break;
      }
    }

    return ifaceName;
  }
}
