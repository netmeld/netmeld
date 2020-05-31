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

#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/Route.hpp>
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;

typedef std::vector<nmdo::Route>  Result;


class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  public:
    Parser() : Parser::base_type(start)
    {
      start =
        *(defaultRoute | route | unreachable)
        ;

      defaultRoute =
        dstIpNet
          [pnx::bind(&nmdo::Route::setDstNet, &qi::_val, qi::_1)] >>
        qi::lit("via") >>
        ipAddr
          [pnx::bind(&nmdo::Route::setRtrIp, &qi::_val, qi::_1)] >>
        ifaceName
          [pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1)] >>
        qi::omit[*token] >> qi::eol
        ;

      route =
        dstIpNet
          [pnx::bind(&nmdo::Route::setDstNet, &qi::_val, qi::_1)] >>
        ifaceName
          [pnx::bind(&nmdo::Route::setIfaceName, &qi::_val, qi::_1)] >>
        // IPv6 doesn't seem to do this, so needs to be optional
        -(qi::lit("proto kernel scope link src") >>
          ipAddr
            [pnx::bind(&nmdo::Route::setRtrIp, &qi::_val, qi::_1)]
         ) >>
        qi::omit[*token] >> qi::eol
        ;

      unreachable =
        qi::lit("unreachable") >> *token >> qi::eol
        ;

      dstIpNet =
        (qi::lit("default") | ipAddr [qi::_val = qi::_1])
          [pnx::bind(&nmdo::IpAddress::setReason, &qi::_val, "ip route show")]
        ;

      ifaceName =
        qi::lit("dev") >> token
        ;

      token =
        +qi::ascii::graph
        ;

      BOOST_SPIRIT_DEBUG_NODES(
          (start)
          (defaultRoute) (route)
          (dstIpNet) (rtrIpAddr)
          (ifaceName)
          //(token)
          );
    }

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start;

    qi::rule<nmdp::IstreamIter, nmdo::Route(), qi::ascii::blank_type>
      defaultRoute, route;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      unreachable;

    qi::rule<nmdp::IstreamIter, nmdo::IpAddress(), qi::ascii::blank_type>
      dstIpNet, rtrIpAddr;

    qi::rule<nmdp::IstreamIter, std::string(), qi::ascii::blank_type>
      ifaceName;

    qi::rule<nmdp::IstreamIter, std::string()>
      token;

    nmdp::ParserIpAddress
      ipAddr;
};


template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  public:
    Tool() : nmdt::AbstractImportTool<P,R>
      ("ip route show", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void toolRunMetadataInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      auto& results         {this->tResults};

      for (auto& result : results) {
        result.saveAsMetadata(t, toolRunId);
        LOG_DEBUG << "[TRM] " << result.toDebugString() << std::endl;
      }
    }

    void specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->devInfo.getDeviceId()};
      auto& results         {this->tResults};

      for (auto& result : results) {
        result.save(t, toolRunId, deviceId);
        LOG_DEBUG << result.toDebugString() << std::endl;
      }
    }
};


int
main(int argc, char** argv)
{
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
