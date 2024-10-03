// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser {
    public:
      using Parser::start;
};

BOOST_AUTO_TEST_CASE(testWhole)
{
  {
    TestParser tp;
    const auto& parserRule {tp.start};
    std::string test {
      R"STR(
Created by: D-Default, S-Static, G-GVRP, R-Radius Assigned VLAN, V-Voice VLAN
 
Vlan       Name           Tagged Ports      UnTagged Ports      Created by   
---- ----------------- ------------------ ------------------ ----------------
1           1                               gi8-26,Po1-8           V        
54         54             gi1-7,gi28                               S        
55         55                gi7                                   S        
99         99             gi1-6,gi28                               S        
100         100            gi1-7,gi28                               S        
101         101            gi1-7,gi28                               S        
102         102            gi1-7,gi28                               S        
106         106            gi1-7,gi28                               S        
200         200                             gi1-7,gi27-28           S        
300         300            gi1-7,gi28                               S        
310         310            gi1-7,gi28                               S        
320         320            gi1-7,gi28                               S        
350         350            gi1-7,gi28                               S        
360         360            gi1-7,gi28                               S        
370         370            gi1-7,gi28                               S        
3000       3000            gi1-7,gi28                               S    
      )STR",
      };
    Result out;
    BOOST_TEST(nmdp::testAttr(test.c_str(), parserRule, out, blank),
              "Parser rule 'testWhole': " << test);
    BOOST_TEST(1 == out.size());
  }
}