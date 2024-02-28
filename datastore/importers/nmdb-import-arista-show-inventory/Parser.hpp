#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/InterfaceNetwork.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/objects/Service.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>
#include <netmeld/datastore/objects/Vlan.hpp>
#include <netmeld/datastore/parsers/ParserDomainName.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/parsers/ParserMacAddress.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

typedef nmdo::DeviceInformation  DevInfo;
typedef std::vector<DevInfo>     Result;

class Parser:
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  private:
    std::string VENDOR = "Arista";

  public:

    Parser();

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, DevInfo(), qi::ascii::blank_type>
      deviceInfo;

    qi::rule<nmdp::IstreamIter, DevInfo(), qi::ascii::blank_type>
      systemInformation;

    qi::rule<nmdp::IstreamIter, std::vector<std::string>, qi::ascii::blank_type>
      systemInformationRow1, systemInformationRow2;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemPowerSupply, powerSupplyRule, repeatPowerSupplyRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemFanModule, fanModuleRule, repeatFanModuleRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemPorts, portRule, repeatPortRule;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      systemTransceiverSlots, transceiverRule, repeatTransceiverRule;

    qi::rule<nmdp::IstreamIter, std::string()>
      token;

    std::array<qi::rule<nmdp::IstreamIter, std::string()>, 4>
      stringFieldRules;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      ignoredLine;
};