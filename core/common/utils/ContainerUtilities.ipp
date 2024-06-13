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

// NOTE This implementation is included in the header (at the end) since it
//      leverages templating.

#include <algorithm>

namespace netmeld::core::utils {

  // ==========================================================================
  // toString()
  template<typename Iterator>
  std::string
  toString(Iterator begin, Iterator end, const std::string& sep)
  {
    std::ostringstream oss;
    if (begin != end) {
      oss << *begin;
      ++begin;
      for (; begin != end; ++begin) {
        oss << sep << *begin;
      }
    }

    return oss.str();
  }
  template<typename SequenceContainer>
  std::string
  toString(const SequenceContainer& con, const char sep)
  {
    const std::string sSep {sep};
    return toString(con.begin(), con.end(), sSep);
  }
  template<typename SequenceContainer>
  std::string
  toString(const SequenceContainer& con, const std::string& sep)
  {
    return toString(con.begin(), con.end(), sep);
  }
  // ==========================================================================

  template<typename SequenceContainer, typename T>
  void
  addIfUnique(SequenceContainer* const con, const T& item)
  {
    if (std::find(con->begin(), con->end(), item) == con->end()) {
      con->emplace_back(item);
    }
  }
}
