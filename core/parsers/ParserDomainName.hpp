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

#ifndef PARSER_DOMAIN_NAME_HPP
#define PARSER_DOMAIN_NAME_HPP

#include <netmeld/core/parsers/ParserHelper.hpp>


namespace netmeld::core::parsers {

  class ParserDomainName :
    public qi::grammar<IstreamIter, std::string()>
  {
    public:
      ParserDomainName() : ParserDomainName::base_type(start)
      {
        // A domain name is between 1 and 127 labels, each separated by a period
        start =
          (label >> qi::repeat(0,126)[qi::char_('.') >> label])
          ;

        // See: https://tools.ietf.org/html/rfc2181#section-11
        // See: https://www.ietf.org/rfc/rfc1035.txt#section-3.1
        /* A label must be between 1 and 63 characters long. While most places
           say alphanumeric, hyphens, and underscores...the above references
           basically says there is not a character enforcement outside of length
           requirements. Many domain owners might enforce requirements, but it
           is not a protocol enforcement. Since we've been using this for a
           while now with no issue, we will only change if it becomes one as
           many parsers used this and may require logic changes to take the
           change with no impact.
        */
        label =
          (qi::repeat(1,63)[qi::ascii::alnum | qi::char_('_') | qi::char_('-')])
          ;

        BOOST_SPIRIT_DEBUG_NODES((start)(label));
      }

      qi::rule<IstreamIter, std::string()>
        start, label;
  };
}
#endif // PARSER_DOMAIN_NAME_HPP
