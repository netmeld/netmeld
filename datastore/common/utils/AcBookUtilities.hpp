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

#ifndef AC_BOOK_UTILITIES_HPP
#define AC_BOOK_UTILITIES_HPP

#include <map>
#include <string>
#include <vector>


namespace netmeld::datastore::utils {

  template<typename DType>
  bool
  expanded(std::map<std::string, std::map<std::string, DType>>& book,
           const std::string& targetSet, const std::string& targetData,
           const std::string& defaultSet=""
    )
  {
    // book doesn't exist
    if (!book.count(targetSet)) { return false; }
    // book exists, no data
    if (!book[targetSet].count(targetData)) { return false; }
    const auto& set {book[targetSet][targetData].getData()};
    // book and data exist, but empty
    if (set.empty()) { return false; }

    // recursive function
    std::vector<std::string> vec {set.begin(), set.end()};
    for (const auto& data : vec) {
      // skip self-reference data
      if (targetData == data) { continue; }
      if (expanded(book, targetSet, data, defaultSet)) {
        book[targetSet][targetData].removeData(data);
        book[targetSet][targetData].addData(book[targetSet][data].getData());
      } else if (!defaultSet.empty()) {
        expanded(book, defaultSet, data);
      }
    }
    return true;
  }

}
#endif  /* AC_BOOK_UTILITIES_HPP */
