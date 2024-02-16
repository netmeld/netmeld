
#include "Parser.hpp"
#include <netmeld/core/utils/StringUtilities.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmcu = netmeld::core::utils;

typedef nmdo::DeviceInformation  DevInfo;
typedef std::vector<DevInfo>     Result;

Parser::Parser() : Parser::base_type(start)
{
    auto generateStringFieldRule = [this](size_t idx, std::string dashes)
    {
        this->stringFieldRules[idx] = 
        qi::lexeme[
            qi::as_string[
            qi::repeat(1, static_cast<int>(dashes.size()))[qi::print]
            ]
        ][qi::_val = qi::_1];

        if (idx == 0) {
            BOOST_SPIRIT_DEBUG_NODES(
            (stringFieldRules[0])
            )
        } else if (idx == 1) {
            BOOST_SPIRIT_DEBUG_NODES(
            (stringFieldRules[1])
            )
        } else if (idx == 2) {
            BOOST_SPIRIT_DEBUG_NODES(
            (stringFieldRules[2])
            )
        } else if (idx == 3) {
            BOOST_SPIRIT_DEBUG_NODES(
            (stringFieldRules[3])
            )
        } else {
            BOOST_SPIRIT_DEBUG_NODES(
            (stringFieldRules[idx])
            )
        }
    };

    auto generatePowerSupplyRule = [this](int repeatCount)
    {
        auto &repeatRule = this->repeatPowerSupplyRule;
        auto &rule = this->powerSupplyRule;
        repeatRule = 
            qi::repeat(repeatCount - 1)[rule >> qi::eol] >> rule;
    };

    auto generateFanModuleRule = [this](int repeatCount)
    {
        auto &repeatRule = this->repeatFanModuleRule;
        auto &rule = this->fanModuleRule;
        repeatRule = 
            qi::repeat(repeatCount - 1)[rule >> qi::eol] >> rule;
    };

    auto generatePortRule = [this](int repeatCount)
    {
        auto &repeatRule = this->repeatPortRule;
        auto &rule = this->portRule;
        if (repeatCount == 1) {
            repeatRule = rule;
            return;
        }
        repeatRule = 
            qi::repeat(repeatCount - 1)[rule >> qi::eol] >> rule;
    };

    auto generateTransceiverRule = [this](int repeatCount)
    {
        auto &repeatRule = this->repeatTransceiverRule;
        auto &rule = this->transceiverRule;
        repeatRule = 
            qi::repeat(0, repeatCount - 1)[rule >> qi::eol] >> rule;
    };

    start =
        *qi::eol >> -(deviceInfo % +qi::eol) >> *qi::eol
    ;

    deviceInfo =
        systemInformation[qi::_val = qi::_1] > +qi::eol >
        systemPowerSupply > +qi::eol >>
        systemFanModule > +qi::eol >
        systemPorts > +qi::eol >
        systemTransceiverSlots
    ;

    systemInformation =
        qi::lit("System information") >> qi::eol >>
        (systemInformationRow1 >> *qi::eol >>
        systemInformationRow2)[(
            pnx::bind([](auto &val, auto &row1, auto &row2, std::string vendor){
                val.setModel(row1[0]);
                val.setDescription(row1[1]);
                val.setHardwareRevision(row2[0]);
                val.setSerialNumber(row2[1]);
                val.setVendor(vendor);
                for (auto x : row1) {
                    std::cout << "1: [" << x << "]";
                }
                for (auto x : row2) {
                    std::cout << "1: [" << x << "]";
                }
            }, qi::_val, qi::_1, qi::_2, this->VENDOR)
        )]
    ;

    systemInformationRow1 =
        qi::lit("Model") >> qi::lit("Description") >> qi::eol >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 0, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 1, qi::_1)
        ]] >>
        qi::eol >>
        stringFieldRules[0][(
            pnx::bind(
                [](auto &vec, auto model){
                    vec.emplace_back(nmcu::trim(model));
                }, qi::_val, qi::_1
            )
        )] >>
        stringFieldRules[1][(
            pnx::bind(
                [](auto &vec, auto desc){
                    vec.emplace_back(nmcu::trim(desc));
                }, qi::_val, qi::_1
            )
        )]
    ;

    systemInformationRow2 =
        qi::lit("HW Version") >> qi::lit("Serial Number") >> qi::lit("Mfg Date") >> qi::eol >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 0, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 1, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 2, qi::_1)
        ]] >>
        qi::eol >>
        stringFieldRules[0][(
            pnx::bind(
                [](auto &vec, auto hwVer){
                    vec.emplace_back(nmcu::trim(hwVer));
                }, qi::_val, qi::_1
            )
        )] >>
        stringFieldRules[1][(
            pnx::bind(
                [](auto &vec, auto serNum){
                    vec.emplace_back(nmcu::trim(serNum));
                }, qi::_val, qi::_1
            )
        )] >>
        stringFieldRules[2][(
            pnx::bind(
                [](auto &vec, auto mfgDate){
                    vec.emplace_back(nmcu::trim(mfgDate));
                }, qi::_val, qi::_1
            )
        )]
    ;

    systemPowerSupply =
        qi::lit("System has") >> 
        qi::int_[
            pnx::bind(generatePowerSupplyRule, qi::_1)
        ] >> 
        "power supply slots" >> qi::eol >>
        qi::lit("Slot") >> qi::lit("Model") >> qi::lit("Serial Number")  >> qi::eol >>
        qi::lexeme[qi::as_string[+qi::string("-")]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 0, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 1, qi::_1)
        ]] >> 
        qi::eol >> 
        repeatPowerSupplyRule
    ;

    powerSupplyRule =
        qi::int_ >>
        stringFieldRules[0] >>
        stringFieldRules[1]
    ;

    systemFanModule =
        qi::lit("System has") >> 
        qi::int_[
            pnx::bind(generateFanModuleRule, qi::_1)
        ] >> 
        "fan modules" >> qi::eol >>
        qi::lit("Module") >> qi::lit("Number of Fans") >>
        qi::lit("Model") >> qi::lit("Serial Number")  >> qi::eol >>
        qi::lexeme[qi::as_string[+qi::string("-")]] >>
        qi::lexeme[qi::as_string[+qi::string("-")]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 0, qi::_1)
        ]] >> 
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 1, qi::_1)
        ]] >> 
        qi::eol >> 
        repeatFanModuleRule
    ;

    fanModuleRule =
        qi::int_ >>
        qi::int_ >>
        stringFieldRules[0] >>
        stringFieldRules[1]
    ;

    systemPorts =
        qi::lit("System has") >> 
        qi::int_[
            pnx::bind(generatePortRule, 2)
        ] >> 
        "ports" >> qi::eol >>
        qi::lit("Type") >> qi::lit("Count") >> qi::eol >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 0, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")]] >>
        qi::eol >>
        repeatPortRule
    ;

    portRule = 
        stringFieldRules[0] >>
        qi::int_
    ;

    systemTransceiverSlots =
        qi::lit("System has") >> 
        qi::int_[
            pnx::bind(generateTransceiverRule, qi::_1)

        ] >> 
        "transceiver slots" >> qi::eol >>
        qi::lit("Port") >> qi::lit("Manufacturer") >>
        qi::lit("Model") >> qi::lit("Serial Number") >>
        qi::lit("Rev") >> qi::eol >>
        qi::lexeme[qi::as_string[+qi::string("-")]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 0, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 1, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 2, qi::_1)
        ]] >>
        qi::lexeme[qi::as_string[+qi::string("-")][
            pnx::bind(generateStringFieldRule, 3, qi::_1)
        ]] >>
        qi::eol >>
        repeatTransceiverRule
    ;

    transceiverRule = 
        qi::int_ >>
        stringFieldRules[0] >>
        stringFieldRules[1] >>
        stringFieldRules[2] >>
        stringFieldRules[3]
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
        (systemInformationRow1)
        (systemInformationRow2)
        (systemPowerSupply)
        (powerSupplyRule)
        (systemFanModule)
        (fanModuleRule)
        (systemPorts)
        (portRule)
        (systemTransceiverSlots)
        (transceiverRule)
        //(token)
        );
}
