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
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmcu = netmeld::core::utils;

std::strong_ordering
ParserOutput::operator<=>(const ParserOutput& rhs) const
{
    return devInfo <=> rhs.devInfo;
}

bool
ParserOutput::operator==(const ParserOutput& rhs) const
{
    return 0 == operator<=>(rhs);
}

Parser::Parser() : Parser::base_type(start)
{

    auto extractKeyVals = [](
                            std::string &keyLine,
                            std::vector<int> &kvLengths,
                            std::vector<std::string> &valueLines,
                            KeyValVec &keyVals
                            )
    {

        keyVals.resize(0 >= valueLines.size() ? 1 : valueLines.size());
        for (size_t i = 0; i < valueLines.size(); ++i) {
            const auto& valueLine {valueLines[i]};
            size_t lastPos {0};
            for (const auto& kvLength : kvLengths) {
                const auto key   {nmcu::trim(keyLine.substr(lastPos, kvLength))};
                const auto value {nmcu::trim(valueLine.substr(lastPos, kvLength))};
                keyVals[i].emplace(key, value);
                lastPos += kvLength + 1;  // +1 for space separator
            }
        }

    };


    start =
        *qi::eol >>
        -(deviceInfo % +qi::eol)
         >>
        *qi::eol
    ;

    deviceInfo =
        +(
            systemInformationStart
            [(
                pnx::bind(
                    [this](auto &a, auto &b){
                        a.devInfo = b;
                    }, qi::_val, qi::_1
                )
            )] ||
            systemPowerSupplyStart ||
            systemFanModuleStart ||
            systemPortStart ||
            systemTransceiverStart ||
            systemStorageStart
        )
    ;

    entryRuleBase =
        (
            !(&qi::lit("System ")) >> grabLine >> qi::eol >>
            +countDashes >> qi::eol >>
            ((!qi::eol >> grabLine) % qi::eol)
        )
        [
            pnx::bind(
                [](std::string &keyLine,
                   std::vector<int> &kvLengths,
                   std::vector<std::string> &valueLines,
                   std::tuple<std::string, std::vector<int>, std::vector<std::string>>& entryData)
                {
                    entryData = {
                        std::move(keyLine),
                        std::move(kvLengths),
                        std::move(valueLines)
                    };
                }, qi::_1, qi::_2, qi::_3, qi::_val
            )
        ]
    ;

    systemInformationStart =
        qi::lit("System information") >>
        qi::eol >>
        systemInformation
        [(
            pnx::bind(
                [this](DevInfo &devInfo){
                    devInfo.setVendor(this->VENDOR);
                    devInfo.setModel(systemInformationEntry.at(0)["Model"]);
                    devInfo.setDescription(systemInformationEntry.at(0)["Description"]);
                    devInfo.setHardwareRevision(systemInformationEntry.at(0)["HW Version"]);
                    devInfo.setSerialNumber(systemInformationEntry.at(0)["Serial Number"]);
                }, qi::_val
            )
        )]
    ;

    systemInformation =
        +(systemInformationEntryRule >> *qi::eol)
    ;

    systemInformationEntryRule =
        entryRuleBase
        [(
            pnx::bind
            (
                [this, extractKeyVals](auto &entryData)
                {
                    extractKeyVals(
                        std::get<std::string>(entryData),
                        std::get<std::vector<int>>(entryData),
                        std::get<std::vector<std::string>>(entryData),
                        systemInformationEntry
                    );
                }, qi::_1
            )
        )]
    ;

    systemPowerSupplyStart =
        (
            qi::lit("System has") >>
            qi::int_

            >>
            qi::lit("power supply slot") >> -qi::lit('s') >> qi::eol
        ) >>
        systemPowerSupply
    ;

    systemPowerSupply =
        +(systemPowerSupplyEntryRule >> *qi::eol)
    ;

    systemPowerSupplyEntryRule =
        entryRuleBase
        [(
            pnx::bind
            (
                [this, extractKeyVals](auto &entryData)
                {
                    extractKeyVals(
                        std::get<std::string>(entryData),
                        std::get<std::vector<int>>(entryData),
                        std::get<std::vector<std::string>>(entryData),
                        powerSupplyEntries
                    );
                }, qi::_1
            )
        )]
    ;

    systemFanModuleStart =
        (
            (
                qi::lit("System has") >>
                qi::int_

                >>
                qi::lit("fan module") >> -qi::lit('s') >> qi::eol
            ) >>
            systemFanModule
        )
    ;

    systemFanModule =
        +(systemFanModuleEntryRule >> *qi::eol)
    ;

    systemFanModuleEntryRule =
        entryRuleBase
        [(
            pnx::bind
            (
                [this, extractKeyVals](auto &entryData)
                {
                    extractKeyVals(
                        std::get<std::string>(entryData),
                        std::get<std::vector<int>>(entryData),
                        std::get<std::vector<std::string>>(entryData),
                        fanModuleEntries
                    );
                }, qi::_1
            )
        )]
    ;

    systemPortStart =
        (
            (
                qi::lit("System has") >>
                qi::int_

                >>
                qi::lit("port") >> -qi::lit('s') >> qi::eol
            ) >>
            systemPort
        )
    ;

    systemPort =
        +(systemPortEntryRule >> *qi::eol)
    ;

    systemPortEntryRule =
        entryRuleBase
        [(
            pnx::bind
            (
                [this, extractKeyVals](auto &entryData)
                {
                    extractKeyVals(
                        std::get<std::string>(entryData),
                        std::get<std::vector<int>>(entryData),
                        std::get<std::vector<std::string>>(entryData),
                        portEntries
                    );
                }, qi::_1
            )
        )]
    ;

    systemTransceiverStart =
        (
            (
                qi::lit("System has") >>
                qi::int_

                >>
                -qi::lit("switched") >> qi::lit("transceiver slot") >>
                -qi::lit('s') >> qi::eol
            ) >>
            systemTransceiver
        )
    ;

    systemTransceiver =
        +(systemTransceiverEntryRule >> *qi::eol)

    ;

    systemTransceiverEntryRule =
        entryRuleBase
        [(
            pnx::bind
            (
                [this, extractKeyVals](auto &entryData)
                {
                    extractKeyVals(
                        std::get<std::string>(entryData),
                        std::get<std::vector<int>>(entryData),
                        std::get<std::vector<std::string>>(entryData),
                        transceiverSlotEntries
                    );
                }, qi::_1
            )
        )]
    ;

    systemStorageStart =
        (
            (
                qi::lit("System has") >>
                qi::int_

                >>
                qi::lit("storage device") >> -qi::lit('s') >> qi::eol
            )>>
            systemStorage
        )
    ;

    systemStorage =
        +(systemStorageEntryRule >> *qi::eol)

    ;

    systemStorageEntryRule =
        entryRuleBase
        [(
            pnx::bind
            (
                [this, extractKeyVals](auto &entryData)
                {
                    extractKeyVals(
                        std::get<std::string>(entryData),
                        std::get<std::vector<int>>(entryData),
                        std::get<std::vector<std::string>>(entryData),
                        storageDeviceEntries
                    );
                }, qi::_1
            )
        )]
    ;

    grabLine =
        *qi::print
    ;

    countDashes =
        (+qi::lit('-')[qi::_a++]) [qi::_val = qi::_a]
    ;

    token =
        +qi::ascii::graph
    ;

    ignoredLine =
        (+token > -qi::eol) | +qi::eol
    ;

    BOOST_SPIRIT_DEBUG_NODES(
        (start)
        (deviceInfo)
        (systemInformation)
        //(systemInformationEntryRule)
        (systemPowerSupply)
        (systemFanModule)
        (systemPort)
        (systemTransceiver)
        (systemStorage)
        (grabLine)
        (countDashes)
        //(ignoredLine)
    );
}
