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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <pugixml.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "NmapXmlParser.hpp"

class TestNmapXmlParser : public NmapXmlParser
{
  public:
    using NmapXmlParser::extractHostIsResponding;
    using NmapXmlParser::extractHostMacAddr;
    using NmapXmlParser::extractHostIpAddr;
};
/*
  For reference, the XML schema is:
  - https://nmap.org/book/nmap-dtd.html
  - /usr/share/nmap/nmap.dtd
*/

BOOST_AUTO_TEST_CASE(testExtractHostIsResponding)
{
  TestNmapXmlParser tnxp;

  {
    std::vector<std::string> testsOk {
      R"STR(
      <host> <status state="up" reason="host-response" /> </host>
      )STR",
      R"STR(
      <host> <status state="up" reason="user-set" />
      <ports> <port>
      <state state="open" />
      </port> </ports> </host>
      )STR",
      R"STR(
      <host> <status state="up" reason="user-set" />
      <ports> <port>
      <state state="closed" />
      </port> </ports> </host>
      )STR",
    };

    pugi::xml_document doc;

    for (const auto& test : testsOk) {
      doc.load_string(test.c_str());
      const pugi::xml_node testNode {doc.document_element()};

      BOOST_TEST(tnxp.extractHostIsResponding(testNode));
    }
  }
  {
    std::vector<std::string> testsBad {
      R"STR(
      <host> <status state="up" reason="user-set" />
      <ports> <port>
      <state state="other" reason="syn-ack" />
      </port> </ports> </host>
      )STR",
    };

    pugi::xml_document doc;

    for (const auto& test : testsBad) {
      doc.load_string(test.c_str());
      const pugi::xml_node testNode {doc.document_element()};

      BOOST_TEST(!tnxp.extractHostIsResponding(testNode));
    }
  }
}

BOOST_AUTO_TEST_CASE(testExtractHostMacAddr)
{
  TestNmapXmlParser tnxp;

  {
    pugi::xml_document doc;
    doc.load_string(
      R"STR(
      <address addr="00:11:22:33:44:55" addrtype="mac" vendor="NO-ONE"/>
      )STR");
    const pugi::xml_node testNode {doc.document_element().root()};

    auto ipa {tnxp.extractHostMacAddr(testNode)};

    BOOST_TEST("00:11:22:33:44:55" == ipa.toString());
  }
}

BOOST_AUTO_TEST_CASE(testExtractHostIpAddr)
{
  TestNmapXmlParser tnxp;

  {
    pugi::xml_document doc;
    doc.load_string(
      R"STR(
      <address addr="1.2.3.4" addrtype="ipv4"/>
      )STR");
    const pugi::xml_node testNode {doc.document_element().root()};

    auto ipa {tnxp.extractHostIpAddr(testNode)};

    BOOST_TEST("1.2.3.4/32" == ipa.toString());
  }
  {
    pugi::xml_document doc;
    doc.load_string(
      R"STR(
      <address addr="1::2" addrtype="ipv6"/>
      )STR");
    const pugi::xml_node testNode {doc.document_element().root()};

    auto ipa {tnxp.extractHostIpAddr(testNode)};

    BOOST_TEST("1::2/128" == ipa.toString());
  }
}

BOOST_AUTO_TEST_CASE(testExtractHostnames)
{
  TestNmapXmlParser tnxp;

  {
    pugi::xml_document doc;
    doc.load_string(
      R"STR(
      <host> <address addr="1.2.3.4" addrtype="ipv4"/>
      <hostnames>                                                                     
      <hostname name="some_host" type="user"/>                                           
      <hostname name="some_host" type="PTR"/>                                            
      </hostnames> </host>
      )STR");
    const pugi::xml_node testNode {doc.document_element().root()};

    Data d;
    tnxp.extractHostnames(testNode, d);

    BOOST_TEST(2 == d.ipAddrs.size());
    for (const auto& ipa : d.ipAddrs) {
      BOOST_TEST("1.2.3.4/32" == ipa.toString());
      const auto aliases {ipa.getAliases()};
      BOOST_TEST(1 == aliases.size());
      BOOST_TEST(1 == aliases.count("some_host"));
    }
    BOOST_TEST("[1.2.3.4/32, 0, nmap user, 0, [some_host], ]"
               == d.ipAddrs[0].toDebugString());
    BOOST_TEST("[1.2.3.4/32, 0, nmap ptr, 0, [some_host], ]"
               == d.ipAddrs[1].toDebugString());
  }

  {
    pugi::xml_document doc;
    doc.load_string(
      R"STR(
      <host> <address addr="1.2.3.4" addrtype="ipv4"/>
      <hostscript>
      <script id="nbstat" output="NetBIOS name: some_host, NetBIOS user: &lt;unknown&gt;, NetBIOS MAC: 00:11:22:33:44:55 (NO-ONE)"/>
      </hostscript> </host>
      )STR");
    const pugi::xml_node testNode {doc.document_element().root()};

    Data d;
    tnxp.extractHostnames(testNode, d);

    BOOST_TEST(1 == d.ipAddrs.size());
    const auto ipa {d.ipAddrs[0]};
    BOOST_TEST("1.2.3.4/32" == ipa.toString());
    const auto aliases {ipa.getAliases()};
    BOOST_TEST(1 == aliases.size());
    BOOST_TEST(1 == aliases.count("some_host"));
    BOOST_TEST("[1.2.3.4/32, 0, nmap nbstat, 0, [some_host], ]"
               == ipa.toDebugString());
  }

  {
    pugi::xml_document doc;

    Data d;
    doc.load_string(
      R"STR(
      <host> <address addr="1.2.3.4" addrtype="ipv4"/>
      <hostscript>
      <script id="smb-os-discovery" output="&#xa;  OS: Some OS (SomeOsFlavor 1.2.3)&#xa;  OS CPE: cpe:/o:some_vendor:some_os:::&#xa;  Computer name: some_host&#xa;  NetBIOS computer name: SOME_HOST\x00&#xa;  Domain name: some_domain.some_forest&#xa;  Forest name: some_forest&#xa;  FQDN: some_host.some_domain.some_forest&#xa;  System time: 2000-01-01T00:00:01+00:00&#xa;">
      <elem key="os">Some OS</elem>
      <elem key="lanmanager">SomeOsFlavor 1.2.3</elem>
      <elem key="server">SOME_HOST\x00</elem>
      <elem key="date">2000-01-01T00:00:01+00:00</elem>
      <elem key="fqdn">some_host.some_domain.some_forest</elem>
      <elem key="domain_dns">some_domain.some_forest</elem>
      <elem key="forest_dns">some_forest</elem>
      <elem key="workgroup">SOME_WORKGROUP\x00</elem>
      <elem key="cpe">cpe:/o:::::</elem>
      </hostscript> </host>
      )STR");
    const pugi::xml_node testNode {doc.document_element().root()};
    tnxp.extractHostnames(testNode, d);

    BOOST_TEST(1 == d.ipAddrs.size());
    const auto ipa {d.ipAddrs[0]};
    BOOST_TEST("1.2.3.4/32" == ipa.toString());
    const auto aliases {ipa.getAliases()};
    BOOST_TEST(2 == aliases.size());
    BOOST_TEST(1 == aliases.count("some_host"));
    BOOST_TEST(1 == aliases.count("some_host.some_domain.some_forest"));
    BOOST_TEST("[1.2.3.4/32, 0, nmap smb-os-discovery, 0, [some_host, some_host.some_domain.some_forest], ]"
               == ipa.toDebugString());
  }
}
