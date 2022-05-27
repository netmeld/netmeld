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

/* Notes:
   - This unit is part of the complilation process to help ensure consistency
     between templates and the actual data
   - Various data is included and most is commented solely for educational
     purposes
     - In non-template, remove data as makes sense

   Guidelines:
   - If using a custom Parser
     - Data ordering is different as the focus is the parsing logic, not rule
       instantiation
     - It occasionally is more reasonable to interact and place data with an
       intermediary object
       - The code can be collocated or a separate file, depending on complexity
*/

#ifndef PARSER_HPP
#define PARSER_HPP //idk what this is

#include "Parser.hpp"

// #include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/parsers/ParserHelper.hpp> // if parser not needed
#include <netmeld/datastore/objects/Package.hpp>
#include <map>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

// =============================================================================
// Data containers
// =============================================================================
// typedef nmdo::AbstractDatastoreObject  Data;

// A struct of a map that contains packages and the key is the packagename
struct Data { 
  std::map<std::string, nmdo::Package> packages;
};
typedef std::vector<Data>    Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables are always private
    Data data;

    // variables for current package

  // Rules
  protected:
    std::string curpackagename;
    std::string curversion;
    std::string curarch;
    std::string curdescription;

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;


    // blank skipper without eol
    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      headers, packageLine, ignoredLine;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
    packages;
    //iterator with string skipper

    // skipp white space iterator
    qi::rule<nmdp::IstreamIter>
      packageName,
      version,
      architecture, 
      desc,
      token
    ;
      // bring in already made parsers (ie version parser)
  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
    void addPackage(const std::string&);
    void setPackageName(const std::string&);
    void setPackageVersion(const std::string&);
    void setPackageArch(const std::string&);
    void setPackageDesc(const std::string&);

  protected:
  public:
    Result getData();
};
#endif // PARSER_HPP
