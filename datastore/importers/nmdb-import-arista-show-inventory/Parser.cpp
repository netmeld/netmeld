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

typedef nmdo::DeviceInformation  DevInfo;
typedef std::vector<DevInfo>     Result;

Parser::Parser() : Parser::base_type(start)
{

    auto extractKeyVals = [](
                            std::string &keyLine,
                            std::vector<int> &kvLengths,
                            std::vector<std::string> &valueLines,
                            KeyValVec &keyVals
                            )
    {

        keyVals.resize(valueLines.size());
        std::vector<std::string> keys;
        auto keyLineIt = keyLine.begin();
        for (size_t i = 0; i < kvLengths.size(); ++i) {
            qi::phrase_parse(
                keyLineIt,
                keyLine.end(),
                (
                    qi::lexeme
                    [
                        qi::as_string[qi::repeat(0, kvLengths[i])[qi::print]]
                    ]
                    [(
                        pnx::bind(
                            [&keys](auto str){
                                keys.emplace_back(nmcu::trim(str));
                            }, qi::_1
                        )
                    )]
                
                ),
                qi::ascii::space
                
            );
        }
        for (size_t i = 0; i < valueLines.size(); ++i) {
            auto valueLine = valueLines[i];
            auto valueLineIt = valueLine.begin();
            for (size_t j = 0; j < kvLengths.size(); ++j) {
                std::string key = keys[j];
                qi::phrase_parse(
                    valueLineIt,
                    valueLine.end(),
                    (
                        qi::lexeme
                        [
                            qi::as_string[qi::repeat(0, kvLengths[j])[qi::print]]
                        ]
                        [(
                            pnx::bind(
                                [i, key, &keyVals](auto str){
                                    std::string trimmedStr{nmcu::trim(str)};
                                    keyVals[i].emplace(key, trimmedStr);
                                }, qi::_1
                            )
                        )]
                    
                    ),
                    qi::ascii::space
                    
                );
            }
        }

    };


    start =
        *qi::eol >>
        -(
        deviceInfo
        [
            pnx::bind(
                [this](auto &devInfoVec, auto &devInfo)
                {
                    a.emplace_back(std::move(b));
                }, qi::_val, qi::_1
            )
        ] % +qi::eol)
         >>
        *qi::eol
    ;

    deviceInfo =
        handleSection
        [
            pnx::bind(
                [](auto &a, auto &b){
                    a = b.devInfo;
                }, qi::_val, qi::_1
            )
        ]
    ;

    placeholder = qi::repeat(0)[!qi::lit("__PLACEHOLDER__")];
    
    handleSection =
        systemInformationStart
        [(
            pnx::bind(
                [this](auto &a, auto &b){
                    a.devInfo = b;
                    std::cout << a.devInfo.toDebugString() << std::endl;
                    std::cout << b.toDebugString() << std::endl;
                }, qi::_val, qi::_1
            )
        )] ||
        systemPowerSupplyStart ||
        systemFanModuleStart ||
        systemPortStart ||
        systemTransceiverStart ||
        systemStorageStart ||
        placeholder
    ;

    entryRuleBase =
        (
            grabLine >> qi::eol >>
            qi::repeat
            [
                countDashes
            ]
        
            >> qi::eol >>
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
                    devInfo.setModel(systemInformationEntry.at(0)["Model"]);
                    devInfo.setDescription(systemInformationEntry.at(0)["Description"]);
                    devInfo.setHardwareRevision(systemInformationEntry.at(0)["HW Version"]);
                    devInfo.setSerialNumber(systemInformationEntry.at(0)["Serial Number"]);
                }, qi::_val
            )
        )] 
    ;

    systemInformation =
        systemInformationEntryRule >>
        (
            *qi::eol >> 
            (
                handleSection || 
                systemInformation
                
            )
        )
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
        systemPowerSupplyEntryRule >>
        (
            *qi::eol >> (handleSection || systemPowerSupply)
        )
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
        systemFanModuleEntryRule >>
        (
            *qi::eol >> (handleSection || systemFanModule)
        )
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
        systemPortEntryRule >>
        (
            *qi::eol >> (handleSection || systemPort)
        )
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
        systemTransceiverEntryRule >>
        (
            *qi::eol >> (handleSection || systemTransceiver)
        )
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
        systemStorageEntryRule >>
        (
            *qi::eol >> (handleSection || systemStorage)
        )
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
        qi::lexeme
        [
            qi::as_string[qi::repeat[qi::print]]
        ]
        [
            qi::_val = qi::_1
        ]
    ;

    countDashes =
        qi::lexeme
        [
            qi::as_string[+qi::char_('-')]
        ]
        [(
            qi::_val = pnx::bind([](auto &str){
                return str.size();
            }, qi::_1)
        )]
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
        (handleSection)
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
