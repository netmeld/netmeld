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

#include <netmeld/core/utils/StringUtilities.hpp>
#include <netmeld/datastore/objects/AcServiceBook.hpp>
#include <netmeld/datastore/utils/AcBookUtilities.hpp>

#include "RulesCommon.hpp"

namespace netmeld::datastore::importers::cisco {

  namespace nmdo = netmeld::datastore::objects;
  namespace nmdp = netmeld::datastore::parsers;
  namespace nmdu = netmeld::datastore::utils;


  // ===========================================================================
  // Data containers
  // ===========================================================================
  typedef std::map<std::string, nmdo::AcServiceBook>  ServiceBook;
  typedef std::map<std::string, ServiceBook>          ServiceBooks;


  // ===========================================================================
  // Parser definition
  // ===========================================================================
  class CiscoServiceBook :
    public qi::grammar<nmdp::IstreamIter, ServiceBooks(), qi::ascii::blank_type>
  {
    // =========================================================================
    // Variables
    // =========================================================================
    public:
      // Rules
      qi::rule<nmdp::IstreamIter, ServiceBooks(), qi::ascii::blank_type>
        start;

      qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
        ciscoServiceBook,
        objectService,
          objectServiceLine,
          sourceDestinationArgument,
        objectGroupService,
          portObjectArgumentLine,
          serviceObjectLine,
          protocolPortLine,
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

      qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
        portArgument;

      qi::rule<nmdp::IstreamIter>
        icmpArgument,
          icmpTypeCode,
          icmpMessage;

    protected:
      // Supporting data structures
      ServiceBook   serviceBooks;
      nmdo::AcServiceBook curBook;
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
