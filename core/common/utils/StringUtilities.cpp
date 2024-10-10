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

#include <netmeld/core/utils/StringUtilities.hpp>


namespace netmeld::core::utils {

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
    std::string ifaceName{toLower(_ifaceName)};

    trim(ifaceName);

    // Ethernet variations (ordered by speed)
    if (ifaceName.starts_with("et") &&
        !ifaceName.starts_with("ethernet")) {
      ifaceName.replace(0, 2, "ethernet");
    }
    else if (ifaceName.starts_with("fa") &&
             !ifaceName.starts_with("fastethernet")) {
      ifaceName.replace(0, 2, "fastethernet");
    }
    else if (ifaceName.starts_with("gi") &&
             !ifaceName.starts_with("gigabitethernet")) {
      ifaceName.replace(0, 2, "gigabitethernet");
    }
    else if (ifaceName.starts_with("te") &&
             !ifaceName.starts_with("tengigabitethernet")) {
      ifaceName.replace(0, 2, "tengigabitethernet");
    }
    else if (ifaceName.starts_with("fo") &&
             !ifaceName.starts_with("fortygigabitethernet")) {
      ifaceName.replace(0, 2, "fortygigabitethernet");
    }
    // Other interface types (ordered alphabetically)
    else if (ifaceName.starts_with("lo") &&
             !ifaceName.starts_with("loopback")) {
      ifaceName.replace(0, 2, "loopback");
    }
    else if (ifaceName.starts_with("po") &&
             !ifaceName.starts_with("port-channel")) {
      ifaceName.replace(0, 2, "port-channel");
    }
    else if (ifaceName.starts_with("se") &&
             !ifaceName.starts_with("serial")) {
      ifaceName.replace(0, 2, "serial");
    }
    else if (ifaceName.starts_with("tu") &&
             !ifaceName.starts_with("tunnel")) {
      ifaceName.replace(0, 2, "tunnel");
    }
    else if (ifaceName.starts_with("vl") &&
             !ifaceName.starts_with("vlan")) {
      ifaceName.replace(0, 2, "vlan");
    }

    return ifaceName;
  }

  std::string
  compileCPE(const std::string& _part, const std::string& _vendor,
             const std::string& _product, const std::string& _version)
  {
    // CPE format follows NMAP standard: 
    // cpe:/{part}:{vendor}:{product}:{version}

    return std::format("cpe:/{}:{}:{}:{}", _part, _vendor, _product, _version);
  }

  std::string replaceSpacesWithUnderscores(const std::string& input) {
    std::string result = input; // Create a copy of the input string
    for (char& c : result) { // Iterate through each character in the string
        if (c == ' ') { 
            c = '_'; 
        }
    }
    return result; 
}
}
