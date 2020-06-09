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

#include <regex>

#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;

typedef nmdo::DeviceInformation  DevInfo;
typedef std::vector<DevInfo>     Result;


class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  private:
    std::string VENDOR = "Cisco";

  public:
    Parser() : Parser::base_type(start)
    {
      start =
        *qi::eol >> -(deviceInfo % +qi::eol) >> *qi::eol
        ;

      deviceInfo =
        (qi::lit("NAME:") >> token >> -qi::lit(',') >>
         qi::lit("DESCR:") >> token >> qi::eol >>
         qi::lit("PID:") >> token >> -qi::lit(',') >>
         qi::lit("VID:") >> token >> -qi::lit(',') >>
         qi::lit("SN:") >> token)
           [qi::_val = pnx::construct<DevInfo>(),
            pnx::bind(&DevInfo::setVendor, &qi::_val, VENDOR),
            pnx::bind(&DevInfo::setDeviceType, &qi::_val, qi::_1),
            pnx::bind(&DevInfo::setDescription, &qi::_val, qi::_2),
            pnx::bind(&DevInfo::setModel, &qi::_val, qi::_3),
            pnx::bind(&DevInfo::setHardwareRevision, &qi::_val, qi::_4),
            pnx::bind(&DevInfo::setSerialNumber, &qi::_val, qi::_5)
           ]
        ;

      token =
          (qi::lit('"') >> +(~qi::char_('"')) >> qi::lit('"'))
        | (+(~qi::char_(" ,") - qi::eol) | qi::attr(""))
        ;

      BOOST_SPIRIT_DEBUG_NODES(
          (start)
          (deviceInfo)
          //(token)
          );
    }

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, DevInfo, qi::ascii::blank_type>
      deviceInfo;

    qi::rule<nmdp::IstreamIter, std::string()>
      token;
};

template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportTool<P,R>
      ("show inventory", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& result : this->tResults) {
        result.setDeviceId(deviceId);

        // Standardize on chassis for variations
        std::regex chassisTest("chassis|1", std::regex::icase);
        if (std::regex_match(result.getDeviceType(), chassisTest)) {
          result.setDeviceType("chassis");
        }

        result.save(t, toolRunId);
        LOG_DEBUG << result.toDebugString() << std::endl;
      }
    }
};


int
main (int argc, char** argv)
{
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
