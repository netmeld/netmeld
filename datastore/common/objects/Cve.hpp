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

#ifndef CVE_HPP
#define CVE_HPP

#include <compare>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/Port.hpp>


namespace netmeld::datastore::objects {

  class Cve : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      short           year     {-1};
      int             number   {-1};
      Port            port;
      unsigned int    pluginId {UINT_MAX};

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Cve();
      Cve(short, int);
      explicit Cve(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      void setPort(const Port&);
      void setPluginId(const unsigned int);

      short getYear() const;
      int getNumber() const;

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toString() const;
      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const Cve&) const;
      bool operator==(const Cve&) const;
  };
}

// pqxx mappings between nmdo::Cve and PostgreSQL CVE
// See QueriesCommon.hpp for boost type mappings
namespace pqxx {
  namespace nmdo = netmeld::datastore::objects;

  template<>
  struct PQXX_LIBEXPORT string_traits<nmdo::Cve>
  {
    static const char* name();
    static bool has_null();
    static bool is_null(nmdo::Cve const&);
    static nmdo::Cve null();
    static void from_string(const char str[], nmdo::Cve& obj);
    static std::string to_string(nmdo::Cve const& obj);
  };
}

#endif //CVE_HPP
