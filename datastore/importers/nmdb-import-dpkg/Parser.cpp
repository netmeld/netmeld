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

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    *(packageLine | ignoredLine)
      [(qi::_val = pnx::bind(&Parser::getData, this))]
    ;
  
//keep going down the file till eof
packages = 
  headers > packageLine 
;

headers = 
    // will get the first 4 lines of dpkg
    qi::lit("Desired") > +token > -qi::eol 
    > qi::lit("|") > +token > -qi::eol
    > qi::lit("|/") > +token > -qi::eol
    > qi::lit("||/") > +token > -qi::eol
    > qi::lit("+++") > +token > -qi::eol
    > packageLine >  -qi::eol
;
// ii packageName version arch description
  packageLine = 
    qi::lit("ii") > packageName
    > version 
    > architecture 
    > desc 
    > qi::eol
    ;

  packageName = 
    +qi::graph
      // [(pnx::bind(&Parser::addPackage, this)) = qi::_1]
    ;
  version = 
    +qi::ascii::graph
      // [(pnx::bind(&Parser::setPackageVersion, this)) = qi::_1]
  ;

  architecture = 
    +qi::ascii::graph
      // [(pnx::bind(&Parser::setPackageArch, this)) = qi::_1]
  ;

  desc = 
    +qi::ascii::print
      // [(pnx::bind(&Parser::setPackageDesc, this)) = qi::_1]
  ;

  //soaker 
  token =
    +qi::ascii::graph
    ;

  ignoredLine =
    (+token > -qi::eol) | +qi::eol
    ;

  //Allows for error handling and debugging of qi.
  BOOST_SPIRIT_DEBUG_NODES(
      (start)
      (headers)
      (packages)
      (packageName)
      (packageLine)
      (version)
      (architecture)
      (desc)
      );
}


// =============================================================================
// Parser helper methods
// =============================================================================
void 
Parser::addPackage(const std::string& _name)
{
  //construct a pack object
  nmdo::Package package;
  package.setName(_name);
  // package.setVersion(_version);
  // package.setArch(_arch);
  // package.setDesc(_desc);
  curpackagename = package.getName();
  //put it in our data map
  data.packages[curpackagename] = package;
}
// void 
// setPackageName(const std::string&);

void 
Parser::setPackageVersion(const std::string& _version)
{
  auto& package {data.packages[curpackagename]};
  package.setVersion(_version);
}
void 
Parser::setPackageArch(const std::string& _arch)
{
  auto& package {data.packages[curpackagename]};
  package.setArch(_arch);
}
void 
Parser::setPackageDesc(const std::string& _desc)
{
  auto& package {data.packages[curpackagename]};
  package.setDesc(_desc);
}

Result
Parser::getData()
{
  Result r {data};
  // std::cout << data["acl"].toDebugString();
  return r;
}
