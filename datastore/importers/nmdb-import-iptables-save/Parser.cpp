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

#include <netmeld/core/utils/StringUtilities.hpp>

#include "Parser.hpp"

namespace nmcu = netmeld::core::utils;


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    (*(table))
      [(qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  table =
    commentLine >>
    qi::lexeme[qi::lit("*") >> token]
      [(pnx::bind(&Parser::setTableName, this, qi::_1))] >> qi::eol >>
    +(chain) >>
    *(rule) >>
    qi::lit("COMMIT") >> qi::eol >>
    commentLine
    ;

  chain =
    (qi::lit(":") >> token >> token >> counts >> qi::eol)
      [(pnx::bind(&Parser::updateChainPolicy, this, qi::_1, qi::_2))]
    ;

  rule =
    -counts >>
    (qi::lit("-A") >> token
       [(pnx::bind(&Parser::updateCurRuleId, this, qi::_1))] >>
     // TODO 09FEB19 Improve logic to match certain module formats e.g.,
     //      -p tcp -m tcp --sport :1024 ! --dport 1024:2048
     // http://ipset.netfilter.org/iptables-extensions.man.html
     *(  (qi::lexeme[qi::lit("-p tcp -m tcp")] >> sportModule >> dportModule)
           [(pnx::bind(&Parser::updateRulePort, this, "tcp", qi::_1, qi::_2))]
       | (qi::lexeme[qi::lit("-p udp -m udp")] >> sportModule >> dportModule)
           [(pnx::bind(&Parser::updateRulePort, this, "udp", qi::_1, qi::_2))]
       | (qi::lexeme[qi::lit("-p icmp -m icmp")] >> icmpModule)
           [(pnx::bind(&Parser::updateRulePort, this, "icmp", "", qi::_1))]
       | (qi::matches[qi::lit("!")] >> optionSwitch >> optionValue)
           [(pnx::bind(&Parser::updateRule, this, qi::_1, qi::_2, qi::_3))]
      ) >>
     qi::eol)
      [(pnx::bind(&Parser::finalizeRule, this))]
    ;

  sportModule =
    (  (qi::char_("!") >> qi::lit("--sport") >> token)
         [(qi::_val = qi::_1 + qi::_2)]
     | (qi::lit("--sport") >> token)
         [(qi::_val = qi::_1)]
     | (qi::attr(std::string()))
         [(qi::_val = qi::_1)]
    )
    ;
  dportModule =
    (  (qi::char_("!") >> qi::lit("--dport") >> token)
         [(qi::_val = qi::_1 + qi::_2)]
     | (qi::lit("--dport") >> token)
         [(qi::_val = qi::_1)]
     | (qi::attr(std::string()))
         [(qi::_val = qi::_1)]
    )
    ;
  icmpModule =
    (  (qi::char_("!") >>
        (qi::lit("--icmp-type") | qi::lit("--icmpv6-type")) >> token)
         [(qi::_val = qi::_1 + qi::_2)]
     | ((qi::lit("--icmp-type") | qi::lit("--icmpv6-type")) >> token)
         [(qi::_val = qi::_1)]
    )
    ;
    ;

  counts =
    qi::lit("[") >> +qi::ascii::digit >> qi::lit(":") >> +qi::ascii::digit >> qi::lit("]")
    ;

  optionSwitch =
    // appear to be short form (-[a-z]) only
    qi::lit("-") >> qi::ascii::alpha
    ;

  optionValue =
    // mName [module-options]
    // module-options appear to be long form (--) and may contain negated (!)
    *((token - (optionSwitch | (qi::lit("! ") >> optionSwitch))) >>
      *qi::ascii::blank
    )
    ;

  commentLine =
    qi::lit("#") >> qi::omit[*token] >> qi::eol
    ;

  token =
    +(qi::ascii::graph)
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (table)(chain)(rule)
      (optionValue)(optionSwitch)
      (sportModule)(dportModule)
      (counts) (commentLine)
      //(token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::setTableName(const std::string& _tableName)
{
  tableName = _tableName;
}

void
Parser::setBookName(const std::string& _bookName)
{
  bookName = tableName + ":" + _bookName;
}

void
Parser::updateCurRuleId(const std::string& _bookName)
{
  setBookName(_bookName);
  curRuleId = ruleIds[bookName]++;
  d.ruleBooks[bookName][curRuleId].setRuleId(curRuleId);
}

void
Parser::updateRulePort(const std::string& _proto, const std::string& _srcPort,
                       const std::string& _dstPort)
{
  updateRule(false, "p", nmcu::getSrvcString(_proto, _srcPort, _dstPort));
}

void
Parser::updateRule(const bool _neg,
                   const std::string& _key, const std::string& _val)
{
  //LOG_DEBUG << "Parser::updateRule: " <<_neg<<":"<<_key<<":"<<_val<<std::endl;
  std::string value = nmcu::trim(_neg ? "!" + _val : _val);
  auto& ruleObj {d.ruleBooks[bookName][curRuleId]};
  if ("s" == _key) {
    ruleObj.addSrc(value);
    d.networkBooks[bookName][value];
  } else if ("d" == _key) {
    ruleObj.addDst(value);
    d.networkBooks[bookName][value];
  } else if ("i" == _key) {
    ruleObj.addSrcIface(value);
  } else if ("o" == _key) {
    ruleObj.addDstIface(value);
  } else if ("p" == _key) {
    ruleObj.addService(value);
    d.serviceBooks[bookName][value];
  } else if ("m" == _key) {
    ruleObj.addService(value);
    d.serviceBooks[bookName][value];
  } else if ("j" == _key) {
    ruleObj.addAction(value);
  } else if ("g" == _key) {
    ruleObj.addAction(value);
  } else if ("f" == _key) {
    ruleObj.addService(value);
    d.serviceBooks[bookName][value];
  } else {
    LOG_WARN << "Parser::updateRule: Unknown key value pair: "
             << _key << " - " << value << std::endl;
  }
}

void
Parser::finalizeRule()
{
  auto& rbRule {d.ruleBooks[bookName][curRuleId]};

  rbRule.setSrcId(bookName);
  rbRule.setDstId(bookName);

  if (0 == rbRule.getSrcs().size()) {
    rbRule.addSrc("any");
  }
  if (0 == rbRule.getSrcIfaces().size()) {
    rbRule.addSrcIface("any");
  }
  if (0 == rbRule.getDsts().size()) {
    rbRule.addDst("any");
  }
  if (0 == rbRule.getDstIfaces().size()) {
    rbRule.addDstIface("any");
  }
  if (0 == rbRule.getServices().size()) {
    rbRule.addService("any");
  }
  if (0 == rbRule.getActions().size()) {
    rbRule.addAction("none");
  }
}

void
Parser::updateChainPolicy(const std::string& _bookName,
                          const std::string& _policy)
{
  if ("-" != _policy) {
    setBookName(_bookName);

    auto& rbRule {d.ruleBooks[bookName][SIZE_MAX]};

    rbRule.setSrcId(bookName);
    rbRule.setDstId(bookName);
    rbRule.setRuleId(SIZE_MAX);
    rbRule.addSrc("any");
    rbRule.addSrcIface("any");
    rbRule.addDst("any");
    rbRule.addDstIface("any");
    rbRule.addService("any");
    rbRule.addAction(_policy);
  }
}

Result
Parser::getData()
{
  Result r {d};

  return r;
}
