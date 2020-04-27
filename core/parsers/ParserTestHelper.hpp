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

#ifndef PARSER_TEST_HELPER_HPP
#define PARSER_TEST_HELPER_HPP

#include <netmeld/core/parsers/ParserHelper.hpp>

namespace netmeld::core::parsers {

  template<typename Data, typename Parser>
  bool test(const Data* in, const Parser& p,
            bool fullMatch = true)
  {
    qi::what(p);
    std::istringstream dataStream(in);
    dataStream.unsetf(std::ios::skipws);
    IstreamIter i(dataStream), e;
    return qi::parse(i, e, p)
      && (!fullMatch || (i == e));
  }

  template<typename Data, typename Parser, typename Skipper>
  bool test(const Data* in, const Parser& p, const Skipper& s,
            bool fullMatch = true)
  {
    qi::what(p);
    std::istringstream dataStream(in);
    dataStream.unsetf(std::ios::skipws);
    IstreamIter i(dataStream), e;
    return qi::phrase_parse(i, e, p, s)
      && (!fullMatch || (i == e));
  }

  template<typename Data, typename Parser, typename Attr>
  bool testAttr(const Data* in, const Parser& p, Attr& a,
                bool fullMatch = true)
  {
    qi::what(p);
    std::istringstream dataStream(in);
    dataStream.unsetf(std::ios::skipws);
    IstreamIter i(dataStream), e;
    return qi::parse(i, e, p, a)
      && (!fullMatch || (i == e));
  }

  template<typename Data, typename Parser, typename Attr, typename Skipper>
  bool testAttr(const Data in, const Parser& p, Attr& a, const Skipper& s,
                bool fullMatch = true)
  {
    qi::what(p);
    std::istringstream dataStream(in);
    dataStream.unsetf(std::ios::skipws);
    IstreamIter i(dataStream), e;
    return qi::phrase_parse(i, e, p, s, a)
      && (!fullMatch || (i == e));
  }

}
#endif // PARSER_TEST_HELPER_HPP
