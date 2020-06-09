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

#ifndef CISO_NETWORK_BOOK_HPP
#define CISO_NETWORK_BOOK_HPP

#include <netmeld/datastore/objects/AcNetworkBook.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/utils/AcBookUtilities.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include "RulesCommon.hpp"

namespace netmeld::datastore::importers::cisco {

  namespace nmdo = netmeld::datastore::objects;
  namespace nmdp = netmeld::datastore::parsers;
  namespace nmdu = netmeld::datastore::utils;


  // ===========================================================================
  // Data containers
  // ===========================================================================
  typedef std::map<std::string, nmdo::AcNetworkBook>  NetworkBook;
  typedef std::map<std::string, NetworkBook>          NetworkBooks;


  // ===========================================================================
  // Parser definition
  // ===========================================================================
  class CiscoNetworkBook :
    public qi::grammar<nmdp::IstreamIter, NetworkBooks(), qi::ascii::blank_type>
  {
    // =========================================================================
    // Variables
    // =========================================================================
    public:
      // Rules
      qi::rule<nmdp::IstreamIter, NetworkBooks(), qi::ascii::blank_type>
        start;

      qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
        ciscoNetworkBook,
        nameLine,
        objectNetwork,
          objectNetworkHostLine,
          objectNetworkSubnetLine,
          objectNetworkRangeLine,
          objectNetworkNatLine,
          objectNetworkFqdnLine,
        objectGroupNetwork,
          networkObjectLine,
          groupObjectLine,
        description,
        objectArgument,
        dataNetworkObjectMask,
        bookName,
        hostArgument,
        dataIp,
        dataString;

      qi::rule<nmdp::IstreamIter>
        dataIpMask,
        dataIpPrefix,
        dataIpRange,
        ipNoPrefix;

      nmdp::ParserIpAddress   ipAddr;

    protected:
      // Supporting data structures
      NetworkBook networkBooks;
      nmdo::AcNetworkBook curBook;
      const std::string ZONE  {"global"};

      std::set<std::string> ignoredRuleData;

    private:

    // =========================================================================
    // Constructors
    // =========================================================================
    public: // Constructor is only default and must be public
      CiscoNetworkBook();

    // =========================================================================
    // Methods
    // =========================================================================
    public:
      NetworkBooks getFinalVersion();

    protected:
    private: // Methods which should be hidden from API users
      void addData(const std::string&);
      void fromIp(const nmdo::IpAddress&);
      void fromIpMask(const nmdo::IpAddress&, const nmdo::IpAddress&);
      void fromIpRange(const nmdo::IpAddress&, const nmdo::IpAddress&);
      void fromNetworkObjectMask(const std::string&, const nmdo::IpAddress&);
      void finalizeCurBook();

      // Object return
      NetworkBooks getData();
  };
}
#endif // CISO_NETWORK_BOOK_HPP
