// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <sstream>
#include <boost/algorithm/string.hpp>

#include <netmeld/core/objects/DnsQuestion.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmcu = netmeld::core::utils;


namespace netmeld::core::objects {

  DnsQuestion::DnsQuestion() :
    questionFqdn{},
    questionClass{},
    questionType{}
  { }

  DnsQuestion::DnsQuestion(const std::string& strQuestionFqdn,
                           const std::string& strQuestionClass,
                           const std::string& strQuestionType) :
    questionFqdn{strQuestionFqdn},
    questionClass{strQuestionClass},
    questionType{strQuestionType}
  { }

  void
  DnsQuestion::setFqdn(const std::string& _fqdn)
  {
    questionFqdn = nmcu::toLower(_fqdn);
    boost::trim_right_if(questionFqdn, boost::is_any_of("."));
  }

  void
  DnsQuestion::setClass(const std::string& _class)
  {
    questionClass = nmcu::toUpper(_class);
  }

  void
  DnsQuestion::setType(const std::string& _type)
  {
    questionType = nmcu::toUpper(_type);
  }

  const std::string&
  DnsQuestion::getFqdn() const
  {
    return questionFqdn;
  }

  const std::string&
  DnsQuestion::getClass() const
  {
    return questionClass;
  }

  const std::string&
  DnsQuestion::getType() const
  {
    return questionType;
  }

  std::string
  DnsQuestion::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "questionFqdn: " << questionFqdn << ", "
        << "questionClass: " << questionClass << ", "
        << "questionType: " << questionType
        << "]";

    return oss.str();
  }

  std::strong_ordering
  DnsQuestion::operator<=>(const DnsQuestion& rhs) const
  {
    return std::tie( questionFqdn
                   , questionClass
                   , questionType
                   )
       <=> std::tie( rhs.questionFqdn
                   , rhs.questionClass
                   , rhs.questionType
                   )
      ;
  }

  bool
  DnsQuestion::operator==(const DnsQuestion& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
