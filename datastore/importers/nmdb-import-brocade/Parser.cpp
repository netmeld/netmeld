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

#include "Parser.hpp"

#include <netmeld/datastore/utils/ServiceFactory.hpp>

namespace nmdu = netmeld::datastore::utils;


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
  start =
    (config)
      [(pnx::bind(&Parser::setVendor, this, "brocade"),
        qi::_val = pnx::bind(&Parser::getData, this))]
    ;

  config =
    *(
        (qi::lit("SwitchName =") >> domainName >> qi::eol)
           [(pnx::bind(&Parser::setDevId, this, qi::_1))]
      | (qi::lit("[Boot Parameters]") >> qi::eol >> bootIface)
           [(pnx::bind(&Parser::addIface, this, qi::_1))]
      | (qi::lit("fc4.fcp.vendorId:") >> token >> qi::eol)
           [(pnx::bind(&Parser::updateVendor, this, qi::_1))]
      | (qi::lit("fc4.fcp.productId:") >> tokens >> qi::eol)
           [(pnx::bind(&Parser::updateProduct, this, qi::_1))]
      | (qi::lit("ts.clockServer:") >> ipAddr >> qi::eol)
           [(pnx::bind(&Parser::addNtpService, this, qi::_1))]
      | (qi::lit("ts.clockServerList:") >> +(ipAddr >> -qi::lit(";"))
           [(pnx::bind(&Parser::addNtpService, this, qi::_1))])
      | ignoredLine
     )
    ;

  bootIface =
    *(!qi::lit("[") >>
      (  (qi::lit("boot.ipa:") >> ipAddr)
            [(pnx::bind(&nmdo::InterfaceNetwork::addIpAddress, &qi::_val, qi::_1))]
       | (qi::lit("boot.mac:10:00:") >> macAddr) // macaddr8...great
            [(pnx::bind(&nmdo::InterfaceNetwork::setMacAddress, &qi::_val, qi::_1))]
       | (qi::lit("boot.device:") >> token)
            [(qi::_a = qi::_1,
              pnx::bind(&nmdo::InterfaceNetwork::setName, &qi::_val, qi::_1))]
       | (qi::omit[+token])
      ) >> qi::eol
     )
    ;

  ignoredLine =
      (+token >> qi::eol) // ignored config lines
    | (+qi::eol)          // ignored empty lines
    ;

  tokens =
    qi::as_string[+(token >> *qi::ascii::blank)]
    ;

  token =
    +(qi::ascii::graph)
    ;


  BOOST_SPIRIT_DEBUG_NODES(
      //(start)
      (config)
      (bootIface)
      (ignoredLine)
      //(tokens) (token)
      );
}

// =============================================================================
// Parser helper methods
// =============================================================================
void
Parser::setVendor(const std::string& vendor)
{
  d.devInfo.setVendor(vendor);
}

void
Parser::setDevId(const std::string& id)
{
  d.devInfo.setDeviceId(id);
}

void
Parser::addNtpService(const nmdo::IpAddress& ip)
{
  auto service = nmdu::ServiceFactory::makeNtp();
  service.setDstAddress(ip);
  service.setServiceReason(d.devInfo.getDeviceId() + "'s config");
  d.services.push_back(service);
}

void
Parser::updateVendor(const std::string& vendor)
{
  d.devInfo.setVendor(vendor);
}

void
Parser::updateProduct(const std::string& product)
{
  d.devInfo.setDeviceType(product);
}

// InterfaceNetwork related
void
Parser::addIface(nmdo::InterfaceNetwork& iface)
{
  d.ifaces.push_back(iface);
}

// Object return
Result
Parser::getData()
{
  Result r;

  if (d != Data()) {
    r.push_back(d);
  }

  return r;
}
