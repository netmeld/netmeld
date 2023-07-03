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

#ifndef STREAM_UTILITIES_HPP
#define STREAM_UTILITIES_HPP

#include <any>
#include <map>
#include <ostream>
#include <set>
#include <vector>
#include <tuple>


namespace netmeld::core::utils {

  template<size_t Idx = 0, typename... Types>
  std::ostream& operator<<(std::ostream& os, const std::tuple<Types...>& tup)
  {
    if constexpr(Idx == 0) {
      os << "[";
    }
    os << std::get<Idx>(tup);
    if constexpr(Idx + 1 < sizeof...(Types)) {
      os << ", ";
      return operator<<<Idx + 1, Types...>(os, tup);
    } else {
      return os << "]";
    }
  }

  template<typename T>
  std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
  {
    os << "[";
    if (!vec.empty()) {
      os << vec.at(0);
      for (size_t i {1}; i < vec.size(); ++i) {
        os << ", " << vec[i];
      }
    }

    return os << "]";
  }

  template<typename T>
  std::ostream& operator<<(std::ostream& os, const std::set<T>& set)
  {
    os << "[";
    auto iter {set.begin()};
    if (iter != set.end()) {
      os << *iter;
      iter++;
      for (; iter != set.end(); ++iter) {
        os << ", " << *iter;
      }
    }

    return os << "]";
  }

  template<typename K, typename V>
  std::ostream& operator<<(std::ostream& os, const std::map<K,V>& map)
  {
    os << "[";
    for (const auto& [k, v] : map) {
      os << "{" << k << ", " << v << "}, ";
    }
    return os << "]";
  }

  std::ostream& operator<<(std::ostream&, const std::any&);

}
#endif // STREAM_UTILITIES_HPP
