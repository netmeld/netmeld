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
Parser::Parser() : Parser::base_type(prestart)
{
  prestart = start [(qi::_val = pnx::bind(&Parser::getData, this))];

  start =
    headers > 
    *(packageLine [(pnx::bind(&Parser::addPackage, this, qi::_1))]
      | ignoredLine
      | qi::eol
    ) 
    ;
  
headers = 
    // will get the first 4 lines of dpkg
    qi::lit("Desired") > +token > -qi::eol 
    > qi::lit("|") > +token > -qi::eol
    > qi::lit("|/") > +token > -qi::eol
    > qi::lit("||/") > +token > -qi::eol
    > qi::lit("+++") > +token > -qi::eol
;

//rpm set of states
//keep going down the file till eof
// ii packageName version arch description
//parse other statuses in dpkg instead of 'ii' there is 3 characters
//flag as tool observ ( object), some error because its supposed to be ii
  // https://linuxprograms.wordpress.com/2010/05/11/status-dpkg-list/

  packageLine = 
    packageStatus [(qi::_val = pnx::construct<nmdo::Package>(qi::_1))]
    > packageName [(pnx::bind(&nmdo::Package::setName, &qi::_val, qi::_1))]
    > version [(pnx::bind(&nmdo::Package::setVersion, &qi::_val, qi::_1))]
    > architecture [(pnx::bind(&nmdo::Package::setArch, &qi::_val, qi::_1))]
    > desc [(pnx::bind(&nmdo::Package::setDesc, &qi::_val, qi::_1))]
    > qi::eol
    ;

  packageStatus = 
    // +qi::ascii::graph 
    // if status is anything except ii then add observation
    *(+qi::lit("ii") 
    | (+qi::ascii::graph)) 
  ;

  packageName = 
    +qi::ascii::graph
    ;
  version = 
    +qi::ascii::graph
  ;

  architecture = 
    +qi::ascii::graph
  ;

  desc = 
    +qi::ascii::print
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
      (packageStatus)
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
Parser::addPackage(const nmdo::Package& packg)
{
  data.packages.push_back(packg);
}
void
Parser::addObservation(const std::vector<std::string>& observations,
                       const nmdo::Package& package)
{
  std::ostringstream oss;
  oss << "Irregular package status " << package.getStatus() << ":";
  for (auto& observation : observations) {
    oss << " " << observation;
  }
  data.observations.addNotable(oss.str());
}

Result
Parser::getData()
{
  Result r;
  r.push_back(data);
  return r;
}
