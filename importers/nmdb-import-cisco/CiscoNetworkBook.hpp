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

#include <netmeld/core/objects/AcNetworkBook.hpp>
#include <netmeld/core/parsers/ParserIpAddress.hpp>
#include <netmeld/core/utils/AcBookUtilities.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>

#include "CommonRules.hpp"

namespace netmeld::datastore::importers::cisco {

namespace nmco = netmeld::core::objects;
namespace nmcp = netmeld::core::parsers;
namespace nmcu = netmeld::core::utils;


// =============================================================================
// Data containers
// =============================================================================
typedef std::map<std::string, nmco::AcNetworkBook>  NetworkBook;
typedef std::map<std::string, NetworkBook>          NetworkBooks;


// =============================================================================
// Parser definition
// =============================================================================
class CiscoNetworkBook :
  public qi::grammar<nmcp::IstreamIter, NetworkBooks(), qi::ascii::blank_type>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  public:
    // Rules
    qi::rule<nmcp::IstreamIter, NetworkBooks(), qi::ascii::blank_type>
      start;

    qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
      config,
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

    qi::rule<nmcp::IstreamIter>
      dataIpMask,
      dataIpPrefix,
      dataIpRange,
      ipNoPrefix;

    nmcp::ParserIpAddress   ipAddr;

  protected:
    // Supporting data structures
    NetworkBook networkBooks;
    nmco::AcNetworkBook curBook;
    const std::string ZONE  {"global"};

    std::set<std::string> ignoredRuleData;

  private:

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    CiscoNetworkBook();

  // ===========================================================================
  // Methods
  // ===========================================================================
  public:
    NetworkBooks getFinalVersion();

  protected:
  private: // Methods which should be hidden from API users
    void addData(const std::string&);
    void fromIp(const nmco::IpAddress&);
    void fromIpMask(const nmco::IpAddress&, const nmco::IpAddress&);
    void fromIpRange(const nmco::IpAddress&, const nmco::IpAddress&);
    void fromNetworkObjectMask(const std::string&, const nmco::IpAddress&);
    void finalizeCurBook();

    // Object return
    NetworkBooks getData();
};
}
#endif // CISO_NETWORK_BOOK_HPP
