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
        (*qi::eol > systeminfo > -qi::eol)[(qi::_val = pnx::bind(&Parser::getData, this))];

    token =
        +qi::ascii::graph;

    host_name =
        ("Host Name: " > token[pnx::bind(&nmdo::DeviceInformation::setDeviceId, &data.devInfo, qi::_1)] > qi::eol);

    os_name =
        ("OS Name: " > token[pnx::bind(&nmdo::OperatingSystem::setProductName, &data.os, qi::_1)][pnx::bind(&nmdo::OperatingSystem::setCpe, &data.os, qi::_1)] > qi::eol);

    os_version =
        ("OS Version: " > token[pnx::bind(&nmdo::OperatingSystem::setProductVersion, &data.os, qi::_1)] > qi::eol);

    os_manufacturer =
        ("OS Manufacturer: " > token[pnx::bind(&nmdo::OperatingSystem::setVendorName, &data.os, qi::_1)] > qi::eol);

    os_configuration =
        ("OS Configuration: " > token[pnx::bind(&nmdo::DeviceInformation::setDescription, &data.devInfo, qi::_1)] > qi::eol);

    system_manufacturer =
        ("System Manufacturer: " > token[pnx::bind(&nmdo::DeviceInformation::setVendor, &data.devInfo, qi::_1)] > qi::eol);

    system_model =
        ("System Model: " > token[pnx::bind(&nmdo::DeviceInformation::setModel, &data.devInfo, qi::_1)] > qi::eol);

    system_type =
        ("System Type: " > token[pnx::bind(&nmdo::DeviceInformation::setDeviceType, &data.devInfo, qi::_1)] > qi::eol);

    domain =
        ("Domain: " > token > qi::eol); 

    hotfix =
        +("[" >> qi::ascii::digit >> qi::ascii::digit >> "]:" > token[pnx::bind(&Parser::addHotfix, this, qi::_1)] - qi::omit[qi::eol]);
    
    hotfixes =
        ("Hotfix(s): " >> (+qi::ascii::print - qi::eol) >> qi::eol > *(hotfix > qi::eol));

    networkCardName =
        qi::lexeme[+(qi::ascii::char_ - qi::eol)];

    networkCardConnectionName =
        qi::lexeme[+(qi::ascii::char_ - qi::eol)];

    ipAddressLine =
        qi::int_ >> '.' >> qi::int_ >> '.' >> qi::int_ >> '.' >> qi::int_;

    ipaddresssection =
        +qi::ascii::graph;

    dhcpServer =
        qi::lexeme[+(qi::char_ - qi::eol)];
    ;
    dhcpEnabledStatus =
        qi::lit("DHCP Enabled:") >> (qi::lit("Yes") | qi::lit("No"));

    networkCardStatus =
        +qi::ascii::graph;

    network_card =
        ('[' >> qi::lexeme[+qi::char_("0-9")] >> ']' >> qi::lit(':') >> networkCardName[(pnx::bind(&Parser::addInterface, this, qi::_1))] >> qi::eol >> "Connection Name: " > networkCardConnectionName[(pnx::bind(&Parser::addIfaceConnectName, this, qi::_1))] >> qi::eol >> -(dhcpEnabledStatus >> qi::eol) >> -("DHCP Server: " >> dhcpServer >> qi::eol) >> -("IP address(es)" >> qi::eol > +('[' >> qi::lexeme[+qi::char_("0-9")] >> "]: " >> ipAddr[(pnx::bind(&Parser::addIfaceIp, this, qi::_1))] >> qi::eol)) >> -("Status: " >> networkCardStatus[(pnx::bind(&Parser::setIfaceStatus, this, qi::_1))] >> qi::eol));
    ;
    network_cards =
        qi::lit("Network Card(s):") >> +qi::ascii::print > qi::eol > +network_card;

    hyper_v =
        ("Hyper-V Requirements: " > token > qi::eol);

    systeminfo =
        host_name > os_name > os_version > os_manufacturer > os_configuration > *(qi::char_ - qi::lit("System Manufacturer:")) > system_manufacturer > system_model > system_type > *(qi::char_ - qi::lit("Domain:")) > domain[pnx::bind(&Parser::setDomain, this, qi::_1)] > *(qi::char_ - qi::lit("Hotfix")) > hotfixes > network_cards > hyper_v;
    ;
    ignoredLine =
        (qi::ascii::print > -qi::eol) | +qi::eol;

    // Allows for error handling and debugging of qi.
    BOOST_SPIRIT_DEBUG_NODES(
        (start)(systeminfo)(host_name)(os_name)(os_version)(os_manufacturer)(os_configuration)(system_manufacturer)(system_model)(system_type)(domain)(hotfixes)(hotfix)(network_cards)(network_card)(dhcpServer)(dhcpEnabledStatus)(ipAddressLine)(networkCardName)(networkCardConnectionName)(networkCardStatus));
}

// =============================================================================
// Parser helper methods
// =============================================================================

void Parser::setDomain(const std::string &_string)
{
    curDomain = _string;
}

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
    std::string fqdn = data.devInfo.getDeviceId() + '.' + curDomain;
    _ipAddr.addAlias(fqdn, "from systeminfo");
    data.os.setIpAddr(_ipAddr);
    iface.addIpAddress(_ipAddr);
}

void Parser::setIfaceStatus(const std::string &_status)
{
    auto &iface{data.network_cards[curIfaceName]};
    iface.setDown();
}

Result
Parser::getData()
{
    Result r;
    r.push_back(data);
    return r;
}
