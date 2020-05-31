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

#ifndef CISCO_SERVICE_BOOK_HPP
#define CISCO_SERVICE_BOOK_HPP

#include <netmeld/core/objects/AcServiceBook.hpp>
#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/core/utils/AcBookUtilities.hpp>

#include "RulesCommon.hpp"

namespace netmeld::datastore::importers::cisco {

  namespace nmco = netmeld::core::objects;
  namespace nmcp = netmeld::core::parsers;
  namespace nmcu = netmeld::core::utils;


  // ===========================================================================
  // Data containers
  // ===========================================================================
  typedef std::map<std::string, nmco::AcServiceBook>  ServiceBook;
  typedef std::map<std::string, ServiceBook>          ServiceBooks;


  // ===========================================================================
  // Parser definition
  // ===========================================================================
  class CiscoServiceBook :
    public qi::grammar<nmcp::IstreamIter, ServiceBooks(), qi::ascii::blank_type>
  {
    // =========================================================================
    // Variables
    // =========================================================================
    public:
      // Rules
      qi::rule<nmcp::IstreamIter, ServiceBooks(), qi::ascii::blank_type>
        start;

      qi::rule<nmcp::IstreamIter, qi::ascii::blank_type>
        ciscoServiceBook,
        objectService,
          objectServiceLine,
          sourceDestinationArgument,
        objectGroupService,
          portObjectArgumentLine,
          serviceObjectLine,
        objectGroupProtocol,
          protocolObjectLine,
          groupObjectLine,
        bookName,
        description,
        protocolArgument,
        sourcePort,
        destinationPort,
        unallocatedPort,
        objectArgument,
        dataString;

      qi::rule<nmcp::IstreamIter, std::string(), qi::ascii::blank_type>
        portArgument;

      qi::rule<nmcp::IstreamIter>
        icmpArgument,
          icmpTypeCode,
          icmpMessage;

    protected:
      // Supporting data structures
      ServiceBook   serviceBooks;
      nmco::AcServiceBook curBook;
      const std::string ZONE  {"global"};

      std::string curProtocol {""};
      std::string curSrcPort  {""};
      std::string curDstPort  {""};

      std::set<std::string> ignoredRuleData;

    private:

    // =========================================================================
    // Constructors
    // =========================================================================
    public: // Constructor is only default and must be public
      CiscoServiceBook();

    // =========================================================================
    // Methods
    // =========================================================================
    public:
      ServiceBooks getFinalVersion();

    protected:
    private: // Methods which should be hidden from API users
      void addData(const std::string&);
      void addCurData();

      void finalizeCurBook();

      // Object return
      ServiceBooks getData();
  };
}
#endif // CISCO_SERVICE_BOOK_HPP
