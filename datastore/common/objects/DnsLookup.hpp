// =============================================================================
// Copyright 2021 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef DNS_LOOKUP_HPP
#define DNS_LOOKUP_HPP

#include <netmeld/core/objects/DnsQuestion.hpp>
#include <netmeld/core/objects/DnsResponse.hpp>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/Port.hpp>

#include <map>
#include <vector>

namespace nmco = netmeld::core::objects;

namespace netmeld::datastore::objects {

  class DnsLookup : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      Port resolver;
      nmco::DnsQuestion question;
      std::string status;
      std::map<std::string, nmco::DnsResponses> responseSections;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      DnsLookup() = default;
      DnsLookup(const DnsLookup&) = default;

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      void setResolver(const Port&);
      void setQuestion(const nmco::DnsQuestion&);
      void setStatus(const std::string&);
      void addResponseSection(const std::pair<std::string, nmco::DnsResponses>&);
      
      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;
  };
  typedef std::vector<DnsLookup> DnsLookups;
}
#endif // DNS_LOOKUP_HPP
