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

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser() : Parser::base_type(start)
{
    start =
        (*qi::eol > systemInfo > -qi::eol)[(qi::_val = pnx::bind(&Parser::getData, this))];

    tokens =
        +qi::ascii::print;

    hostName =
        ("Host Name: " 
        > tokens[pnx::bind(&nmdo::DeviceInformation::setDeviceId, &data.devInfo, qi::_1)] 
        > qi::eol);

    osName =
        ("OS Name: " 
        > tokens[( pnx::bind(&nmdo::OperatingSystem::setProductName, &data.os, qi::_1)
               ,  pnx::bind(&nmdo::OperatingSystem::setCpe, &data.os, qi::_1)
               )]
        > qi::eol);

    osVersion =
        ("OS Version: " 
        > tokens[pnx::bind(&nmdo::OperatingSystem::setProductVersion, &data.os, qi::_1)] 
        > qi::eol);

    osManufacturer =
        ("OS Manufacturer: " 
        > tokens[pnx::bind(&nmdo::OperatingSystem::setVendorName, &data.os, qi::_1)] 
        > qi::eol);

    osConfiguration =
        ("OS Configuration: " 
        > tokens[pnx::bind(&nmdo::DeviceInformation::setDescription, &data.devInfo, qi::_1)] 
        > qi::eol);

    systemManufacturer =
        ("System Manufacturer: " 
        > tokens[pnx::bind(&nmdo::DeviceInformation::setVendor, &data.devInfo, qi::_1)] 
        > qi::eol);

    systemModel =
        ("System Model: " 
        > tokens[pnx::bind(&nmdo::DeviceInformation::setModel, &data.devInfo, qi::_1)] 
        > qi::eol);

    systemType =
        ("System Type: "
        > tokens[pnx::bind(&nmdo::DeviceInformation::setDeviceType, &data.devInfo, qi::_1)] 
        > qi::eol);

    domain =
        ("Domain: " 
        > tokens 
        > qi::eol); 

    hotfix =
        ("[" 
        >> +qi::ascii::digit 
        >> "]:" 
        >> qi::as_string[qi::ascii::string("KB") 
        >> +qi::ascii::digit][pnx::bind(&Parser::addHotfix, this, qi::_1)]);

    hotfixes =
        ("Hotfix(s): " 
        >> (+qi::ascii::print - qi::eol) 
        >> qi::eol
        > ( *(hotfix > qi::eol) | ignoredLine));

    networkCardName =
        qi::lexeme[+(qi::ascii::char_ - qi::eol)];

    networkCardConnectionName =
        qi::lexeme[+(qi::ascii::char_ - qi::eol)];

    dhcpServer =
        qi::lexeme[+(qi::char_ - qi::eol)];
    ;
    dhcpEnabledStatus =
        qi::lit("DHCP Enabled:") 
        >> (qi::lit("Yes") | qi::lit("No"));

    networkCardStatus =
        +qi::ascii::graph;

    networkCard =
        ('[' >> qi::lexeme[+qi::char_("0-9")] >> ']' >> qi::lit(':') 
        >> networkCardName[(pnx::bind(&Parser::addInterface, this, qi::_1))] 
        >> qi::eol 
        >> "Connection Name: " 
        > networkCardConnectionName[(pnx::bind(&Parser::addIfaceConnectName, this, qi::_1))] 
        >> qi::eol 
        >> -(dhcpEnabledStatus 
        >> qi::eol) 
        >> -("DHCP Server: " 
        >> dhcpServer >> qi::eol) 
        >> -("IP address(es)" 
        >> qi::eol 
        > +('[' >> qi::lexeme[+qi::char_("0-9")] >> "]: "
        >> ipAddr[(pnx::bind(&Parser::addIfaceIp, this, qi::_1))] >> qi::eol)) 
        >> -("Status: " 
            >> networkCardStatus[(pnx::bind(&Parser::setIfaceStatus, this, qi::_1))] 
            >> qi::eol)
        );
    ;
    networkCards =
        qi::lit("Network Card(s):") 
        >> +qi::ascii::print 
        > qi::eol 
        > +networkCard;

    systemInfo =
        hostName 
        > osName
        > osVersion 
        > osManufacturer 
        > osConfiguration 
        > *(qi::char_ - qi::lit("System Manufacturer:")) //Skip to system manufacturer
        > systemManufacturer 
        > systemModel 
        > systemType 
        > *(qi::char_ - qi::lit("Domain:")) //Skip to domain
        > -domain[pnx::ref(curDomain) = qi::_1] 
        > *(qi::char_ - qi::lit("Hotfix")) //Skip to hotfixes
        > -hotfixes 
        > -networkCards 
        > qi::omit[*(qi::char_ - qi::eoi)]; //omit the rest of sysinfo (aka hyper-v)
    ;
    ignoredLine =
        (qi::ascii::print > -qi::eol) | +qi::eol;

    // Allows for error handling and debugging of qi.
    BOOST_SPIRIT_DEBUG_NODES(
        (start)
        (systemInfo)
        (hostName)
        (osName)
        (osVersion)
        (osManufacturer)
        (osConfiguration)
        (systemManufacturer)
        (systemModel)
        (systemType)
        (domain)
        (hotfixes)
        (hotfix)
        (networkCards)
        (networkCard)
        (dhcpServer)
        (dhcpEnabledStatus)
        (networkCardName)
        (networkCardConnectionName)
        (networkCardStatus));
}

// =============================================================================
// Parser helper methods
// =============================================================================

void Parser::addHotfix(const std::string &_string)
{
    data.hotfixes.push_back(_string);
}

void Parser::addInterface(const std::string &_name)
{
    nmdo::Interface iface;
    iface.setName(_name);
    curIfaceName = iface.getName();
    data.network_cards[curIfaceName] = iface;
}

void Parser::addIfaceConnectName(const std::string &_connectionname)
{
    auto &iface{data.network_cards[curIfaceName]};
    if (_connectionname.find("Wi-Fi") != std::string::npos || _connectionname.find("Bluetooth") != std::string::npos)
    {
        iface.setMediaType(_connectionname);
    }
    iface.setUp();
    iface.setDescription(_connectionname);
}

void Parser::addIfaceIp(nmdo::IpAddress &_ipAddr)
{
    auto &iface{data.network_cards[curIfaceName]};
    std::string fqdn {data.devInfo.getDeviceId() + '.' + curDomain};
    _ipAddr.addAlias(fqdn, "from systeminfo");
    data.os.setIpAddr(_ipAddr);
    iface.addIpAddress(_ipAddr);
}

void Parser::setIfaceStatus(const std::string &_status)
{
    auto &iface{data.network_cards[curIfaceName]};
    iface.setDown();
}

void Parser::clearHotfixes()
{
    data.hotfixes.clear();
}

void Parser::clearNetworkCards()
{
    data.network_cards.clear();
}

Result
Parser::getData()
{
    Result r;
    r.push_back(data);
    return r;
}
