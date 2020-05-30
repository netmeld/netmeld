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

#include "Parser.hpp"


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    config
      [qi::_val = pnx::bind(&Parser::getData, this)]
    ;

  config =
    *(qi::eol) >>
    *(header >>
      devIdAddr >>
      *(!header >> -qi::omit[+token] >> qi::eol)
    )
    ;

  header =
    +qi::ascii::char_("-") >> qi::eol
    ;

  devIdAddr =
    (qi::lit("Device ID:") >>
      hostname >> qi::eol >>
     qi::lit("Entry address(es):") >> qi::eol >>
     qi::lit("IP address:") >>
      ipAddr >> qi::eol >>
     qi::lit("Platform:") >>
      token >>
      token >> qi::omit[+token] >> qi::eol >>
     qi::lit("Interface:") >>
      token >> qi::lit("Port ID (outgoing port):") >>
      token >> qi::eol
    ) [
       pnx::bind(&Parser::addIp, this, qi::_1, qi::_2),
       pnx::bind(&Parser::addHwInfo, this, qi::_1, qi::_3, qi::_4),
       pnx::bind(&Parser::addCon, this, qi::_1, qi::_6, qi::_2)
      ]
    ;

  token =
    +qi::ascii::graph
    ;

  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (header)
      (devIdAddr)
      (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
std::string
Parser::getDevice(const std::string& hostname)
{
  return hostname.substr(0, hostname.find("."));
}

void
Parser::addIp(const std::string& hostname, const nmco::IpAddress& _ip)
{
  nmco::IpAddress ip = _ip;
  ip.addAlias(hostname, "cdp");

  d.ipAddrs.push_back(ip);
}

void
Parser::addHwInfo(const std::string& hostname, const std::string& vendor,
                  std::string& model)
{
  nmco::DeviceInformation devInfo;

  devInfo.setDeviceId(getDevice(hostname));
  devInfo.setVendor(vendor);
  if (model.back() == ',') {
    model.pop_back();
  }
  devInfo.setModel(model);

  d.devInfos.push_back(devInfo);
}

void
Parser::addCon(const std::string& hostname,
               const std::string& toIface, const nmco::IpAddress& _ip)
{
  nmco::InterfaceNetwork iface {toIface};
  iface.setPartial(true);

  nmco::IpAddress ip = _ip; // copy to ensure we don't alter original

  nmco::DeviceInformation devInfo;
  const auto& did {getDevice(hostname)};
  devInfo.setDeviceId(did);
  d.devInfos.push_back(devInfo);

  ip.addAlias(hostname, "cdp");
  iface.addIpAddress(ip);

  d.interfaces.emplace_back(iface, did);
}

// Object return
Result
Parser::getData()
{
  Result r;
  r.push_back(d);

  return r;
}
