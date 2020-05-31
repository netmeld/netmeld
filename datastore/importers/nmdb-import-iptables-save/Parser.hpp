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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/AcNetworkBook.hpp>
#include <netmeld/datastore/objects/AcServiceBook.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;


typedef std::map<std::string, nmdo::AcNetworkBook> NetworkBook;
typedef std::map<std::string, nmdo::AcServiceBook> ServiceBook;
typedef std::map<size_t, nmdo::AcRule> RuleBook;

// =============================================================================
// Data containers
// =============================================================================
struct Data {
  std::map<std::string, NetworkBook> networkBooks;
  std::map<std::string, ServiceBook> serviceBooks;
  std::map<std::string, RuleBook> ruleBooks;
};
typedef std::vector<Data>  Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private:
    Data d;

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type,
             qi::locals<std::string>>
      table, chain, rule;

    qi::rule<nmdp::IstreamIter, std::string()>
      optionValue;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      sportModule, dportModule, icmpModule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      counts,
      commentLine;

    qi::rule<nmdp::IstreamIter, std::string()>
      optionSwitch,
      token;

    const std::string GLOBAL {"global"};
    std::string tableName;
    std::string bookName;
    std::map<std::string, size_t>   ruleIds;
    size_t curRuleId;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void setTableName(const std::string&);
    void setBookName(const std::string&);
    void updateCurRuleId(const std::string&);
    void updateRulePort(const std::string&, const std::string&,
                        const std::string&);
    void updateRule(const bool, const std::string&, const std::string&);
    void finalizeRule();
    void updateChainPolicy(const std::string&, const std::string&);
    Result getData();
};
#endif // PARSER_HPP
