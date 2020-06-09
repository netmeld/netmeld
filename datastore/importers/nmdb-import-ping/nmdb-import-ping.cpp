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
#include <netmeld/datastore/parsers/ParserIpAddress.hpp>
#include <netmeld/datastore/tools/AbstractImportTool.hpp>

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdt = netmeld::datastore::tools;


typedef std::vector<nmdo::IpAddress>       Result;

class Parser :
  public qi::grammar<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
{
  public:
    Parser() : Parser::base_type(start)
    {
      start =
        pingResponsesHeader >> pingResponses >> *qi::eol >>
        -(pingStatisticsHeader >> pingStatistics >> *qi::eol)
        ;

      pingResponsesHeader =
        qi::lit("PING") >> +token >> qi::eol
        ;

      pingResponses =
        *pingResponse
        ;

      pingResponse =
        ( qi::uint_ >> qi::lit("bytes") >>
          qi::lit("from") >> ipAddr[qi::_val = qi::_1] >>
          -(qi::lit("%") >> ifaceName) >> qi::lit(':') >>
          (qi::lit("icmp_req=") | qi::lit("icmp_seq=")) >> qi::uint_ >>
          qi::lit("ttl=") >> qi::uint_ >>
          qi::lit("time=") >> qi::float_ >>
          (qi::lit("us") | qi::lit("ms") | qi::lit("s")) >>
          -qi::lit("(DUP!)") >> qi::eol
        )
        | qi::lit("^") >> +token >> -qi::eol
        | qi::eol
        ;

      pingStatisticsHeader =
        qi::lit("---") >> +token >> qi::eol
        ;

      pingStatistics =
        +(+token >> -qi::eol)
        ;

      ifaceName =
        +(  qi::ascii::alnum
          | qi::ascii::char_("-_.@")
         )
        ;

      token =
        +qi::ascii::graph
        ;

      BOOST_SPIRIT_DEBUG_NODES(
          (start)
          (pingResponsesHeader) (pingResponses) (pingResponse)
          (pingStatisticsHeader) (pingStatistics)
          (ifaceName)
          //(token)
          );
    }

    qi::rule<nmdp::IstreamIter, Result(), qi::ascii::blank_type>
      start,
      pingResponses;

    qi::rule<nmdp::IstreamIter, nmdo::IpAddress(), qi::ascii::blank_type>
      pingResponse;

    qi::rule<nmdp::IstreamIter, qi::ascii::blank_type>
      pingResponsesHeader,
      pingStatisticsHeader,
      pingStatistics;

    qi::rule<nmdp::IstreamIter, std::string()>
      ifaceName,
      token;

    nmdp::ParserIpAddress
      ipAddr;
};

template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P, R>
{
  public:
    Tool() : nmdt::AbstractImportTool<P, R>
      ("ping", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    modifyToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addOptionalOption("a-device-id", std::make_tuple(
          "device-id",
          po::value<std::string>(),
          "Name of device.")
        );
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      for (auto& result : this->tResults) {
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
