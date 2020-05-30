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


#include <algorithm>
#include <cctype>
#include <vector>

#include <netmeld/core/utils/Severity.hpp>


namespace netmeld::core::utils {

  std::vector<std::string> sevTexts {
    "OFF",
    "EMERGENCY",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFORMATIONAL",
    "DEBUG",
    "DEBUG_SPIRIT",
    "ALL"
  };

  std::string toString(const Severity& sev)
  {
    return sevTexts.at(static_cast<unsigned long>(sev));
  }

  std::ostream& operator<<(std::ostream& os, const Severity& sev)
  {
    return os << static_cast<unsigned long>(sev) << ": " << toString(sev);
  }

  std::istream& operator>>(std::istream& is, Severity& sev)
  {
    unsigned long value = 0;
    std::string token;
    try {
      is >> token;

      std::string temp = token;
      std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
      unsigned long i = 0;
      for (auto text : sevTexts) {
        if (text == temp) {
          value = i;
          break;
        }
        i++;
      }

      if (0 == value && 0 != i) {
        value = std::stoul(token);
      }
    } catch (std::exception& e) {
      throw std::runtime_error(
          "Severity::operator>>: Invalid Severity: " + token
          );
    }

    unsigned long lower = static_cast<unsigned long>(Severity::OFF);
    unsigned long upper = static_cast<unsigned long>(Severity::ALL);

    // cppcheck-suppress unsignedPositive
    if (lower <= value && upper >= value) {
      sev = static_cast<Severity>(value);
    } else if (upper < value) {
      sev = Severity::ALL;
    } else {
      is.setstate(std::ios_base::failbit);
    }

    return is;
  }
}
