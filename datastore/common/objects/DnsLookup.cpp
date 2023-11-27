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

#include <netmeld/datastore/objects/DnsLookup.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmco = netmeld::core::objects;
namespace nmcu = netmeld::core::utils;


namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================

  // ===========================================================================
  // Methods
  // ===========================================================================

  void
  DnsLookup::setResolver(const Port& _resolver)
  {
    resolver = _resolver;
  }

  void
  DnsLookup::setQuestion(const nmco::DnsQuestion& _question)
  {
    question = _question;
  }

  void
  DnsLookup::setStatus(const std::string& _status)
  {
    status = _status;
  }

  void
  DnsLookup::addResponseSection(const std::pair<std::string, nmco::DnsResponses>& _responseSection)
  {
    responseSections.emplace(_responseSection);
  }

	void
  DnsLookup::save(pqxx::transaction_base& t,
                  const nmco::Uuid& toolRunId, const std::string&)
	{
    for (auto& [sectionName, responses] : responseSections) {
      for (auto& response : responses) {
        t.exec_prepared("insert_raw_dns_lookup",
            toolRunId,
            resolver.getIpAddress().toString(),
            resolver.getPort(),
            question.getFqdn(),
            question.getClass(),
            question.getType(),
            status,
            sectionName,
            response.getFqdn(),
            response.getClass(),
            response.getType(),
            response.getTtl(),
            response.getData());
      }
    }
	}

  std::string
  DnsLookup::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["
        << "resolver: " << resolver << ", "
        << "question: " << question << ", "
        << "status: " << status << ", "
        << "responseSections: " << responseSections <<
        "]";

    return oss.str();
  }

  std::partial_ordering
  DnsLookup::operator<=>(const DnsLookup& rhs) const
  {
    return std::tie( resolver
                   , question
                   , status
                   , responseSections
                   )
       <=> std::tie( rhs.resolver
                   , rhs.question
                   , rhs.status
                   , rhs.responseSections
                   )
      ;
  }

  bool
  DnsLookup::operator==(const DnsLookup& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
