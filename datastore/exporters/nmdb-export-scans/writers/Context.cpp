// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include <sstream>
#include <regex>

#include <boost/algorithm/string.hpp>

#include "Context.hpp"

#include <netmeld/core/utils/ContainerUtilities.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::datastore::exporters::scans {
  // ==========================================================================
  // Constructors
  // ==========================================================================
  Context::Context(bool _toFile) :
    Writer(_toFile)
  {}


  // ==========================================================================
  // Methods
  // ==========================================================================
  std::string
  Context::getExtension() const
  {
    return ".tex";
  }

  std::string
  Context::escapeSpecial(const std::string& oldText) const
  {
    // ConTeXt special characters
    const std::vector<std::pair<std::regex, std::string>> patterns {
      // backslash replace
      {std::regex(R"(\\)"), R"({\backslash})"},
      // escape special characters
      {std::regex(R"(#|_|\{|\}|\$|\|)"), R"(\$&)"},
      // greater than
      {std::regex(R"(>)"), R"(&gt;)"},
      // less than
      {std::regex(R"(<)"), R"(&lt;)"},
      // new paragraph
      {std::regex(R"((\r\n|\r|\n){2})"),
        "\n\n\\PortionMark{TODO--Caption Classification}{}\n"},
      //{std::regex(R"()", R"()")},
    };

    std::string cleanedText {oldText};
    for (const auto& p : patterns) {
      cleanedText = std::regex_replace(cleanedText, p.first, p.second);
    }

    return cleanedText;
  }

  std::string
  Context::getIntraNetwork(const std::string& srcIp) const
  {
    std::ostringstream oss(std::ios_base::binary | std::ios_base::trunc);

    codeSetup(oss);
    codeTableIntra(oss, srcIp);

    std::string lastIpName;
    for (const auto& row : rows) {
      std::string ip          {""}; // row[0]
      std::string hostname    {""}; // row[1]
      std::string portProto   {row[2] + '/' + row[3]};
      std::string pps         {row[4] + '/' + row[5]};
      std::string serviceName {row[6]};
      std::string serviceDesc {row[7]};

      std::string rowFrame    {""};
      std::string nextIpName  {row[0] + row[1]};
      if (lastIpName != nextIpName) {
        rowFrame  = "[topframe=on]";
        ip        = row[0];
        hostname  = '(' + row[1] + ')';
      }
      lastIpName = nextIpName;

      pps = replaceAll(pps, "|", "\\|");

      codeRowIntra(oss,
          rowFrame, ip, hostname, portProto, pps, serviceName, serviceDesc
        );
    }

    codeTableClose(oss);
    codeTeardown(oss);

    return oss.str();
  }

  std::string
  Context::getInterNetwork(const std::string& srcIp) const
  {
    std::ostringstream oss(std::ios_base::binary | std::ios_base::trunc);

    codeSetup(oss);
    codeTableInter(oss, srcIp);

    std::string lastHopIpName;
    std::string lastIpName;
    for (const auto& row : rows) {
      std::string nextHopIp   {""}; // row[0]
      std::string nextHopName {""}; // row[1]
      std::string destIp      {""}; // row[2]
      std::string destName    {""}; // row[3]
      std::string portProto   {row[4] + '/' + row[5]};
      std::string pps         {row[6] + '/' + row[7]};

      std::string rowFrame {""};

      std::string nextHopIpName {row[0] + row[1]};
      if (lastHopIpName != nextHopIpName) {
        rowFrame    = "[topframe=on]";
        nextHopIp   = row[0];
        nextHopName = '(' + row[1] + ')';
      }
      lastHopIpName = nextHopIpName;

      std::string nextIpName {row[2] + row[3]};
      if ("" != rowFrame || lastIpName != nextIpName) {
        destIp    = row[2];
        destName  ='(' + row[3] + ')';
      }
      lastIpName = nextIpName;

      pps = replaceAll(pps, "|", "\\|");

      codeRowInter(oss,
          rowFrame, nextHopIp, nextHopName, destIp, destName, portProto, pps
        );
    }

    codeTableClose(oss);
    codeTeardown(oss);

    return oss.str();
  }

  std::string
  Context::getNessus() const
  {
    std::ostringstream oss(std::ios_base::binary | std::ios_base::trunc);

    codeSetup(oss);

    for (const auto& row : rows) {
      std::string pluginId        {row[0]};
      std::string pluginSeverity  {row[1]};
      std::string pluginName      {row[2]};
      std::string pluginDesc      {escapeSpecial(row[3])};

      codeParagraphNessus(oss,
          pluginId, pluginSeverity, pluginName, pluginDesc
        );

      std::vector<std::string> rest(row.begin()+4, row.end());
      size_t count {rest.size()};
      for (size_t i {0}; i < count;) {
        std::string ip    {rest[i++]}; // intentional post increment
        std::string name  {rest[i++]}; // intentional post increment

        oss << R"(  \type{)" << ip << '}';

        if ("" != name) {
          oss << R"( \type{()" << name << R"()})";
        }

        if (i < count) {
          oss << ",\n";
        } else {
          oss << ".\n\n";
        }
      }
    }

    codeTeardown(oss);

    return oss.str();
  }

  std::string
  Context::getProwler() const
  {
    std::ostringstream oss(std::ios_base::binary | std::ios_base::trunc);

    codeSetup(oss);

    std::string curProvider;
    std::string curAccountId;
    std::string curServiceName;
    std::string curSubServiceName;
    for (const auto& row : rows) {
      size_t i {0};

      std::string provider        {row.at(i++)};
      std::string accountId       {row.at(i++)};
      std::string serviceName     {row.at(i++)};
      std::string subServiceName  {row.at(i++)};
      std::string severity        {row.at(i++)};
      std::string checkId         {row.at(i++)};
      std::string description     {row.at(i++)};
      std::string risk            {row.at(i++)};
      std::string recommendation  {row.at(i++)};
      std::string url             {escapeSpecial(row.at(i++))};
      std::string code            {row.at(i++)};

      std::vector<std::string> impactedResources(row.begin()+i, row.end());

      bool newChapter {  curProvider != provider
                      || curAccountId != accountId
                      };
      if (newChapter)
      {
        LOG_DEBUG << "Adding new chapter and section\n";
        curProvider = provider;
        curAccountId = accountId;
        codeChapterProwler(oss);
        codeSectionProwler(oss, curProvider, curAccountId);
      }

      bool newSection {  curServiceName != serviceName
                      || curSubServiceName != subServiceName
                      };
      if (newSection) {
        LOG_DEBUG << "Adding new subsection\n";
        curServiceName = serviceName;
        curSubServiceName = subServiceName;
        codeSubSectionProwler(oss, curServiceName, curSubServiceName);
      }

      codeSubSubSectionProwler(oss
          , severity, checkId, description, risk, recommendation, url
          , code, impactedResources
        );
    }

    codeTeardown(oss);

    return oss.str();
  }

  std::string
  Context::getSshAlgorithms() const
  {
    std::ostringstream oss(std::ios_base::binary | std::ios_base::trunc);

    codeSetup(oss);
    codeTableSsh(oss);

    std::string lastServer;
    std::string lastAlgo;
    for (const auto& row : rows) {
      std::string ip        {""}; // row[0]
      std::string name      {""}; // row[1]
      std::string algoType  {""}; // row[2]
      std::string algoName  {row[3]};
      std::string color     {row[4]};

      std::string nextAlgoType {row[2]};

      // adds comment of `IP -- ALG_TYPE` to data for easier navigation
      if (lastAlgo != nextAlgoType) {
        oss << "\n% " << row[0] << " -- " << nextAlgoType << '\n';
      }

      std::string rowFrame {""};
      bool newRow {false};
      std::string nextServer {row[0] + row[1]};
      if (lastServer != nextServer) {
        newRow    = true;
        rowFrame  = "[topframe=on]";
        ip        = row[0];
        name      = '(' + row[1] + ')';
      }
      lastServer = nextServer;

      std::string cellFrameType {"[leftframe=on]"};
      std::string cellFrameName {"[leftframe=on, rightframe=on]"};
      if (newRow || lastAlgo != nextAlgoType) {
        // newRow        = true;
        algoType      = nextAlgoType;
        cellFrameType = "[topframe=on, leftframe=on]";
        cellFrameName = "[topframe=on, leftframe=on, rightframe=on]";
      }
      lastAlgo = nextAlgoType;

      codeRowSsh(oss,
          rowFrame, ip, name, cellFrameType, algoType,
          cellFrameName, color, algoName
        );
    }

    codeTableClose(oss);
    codeTeardown(oss);

    return oss.str();
  }


  // ==========================================================================
  // ConTeXt formatting code
  // NOTE: keep raw string literal indentation (or lack of) for
  //       cleaner output and easier editing
  // ==========================================================================

  // ------------------------------------------------------------
  // General
  // ------------------------------------------------------------
  void
  Context::codeSetup(auto& oss) const
  {
	  oss << R"(
% pre-config
\doifundefined{DefaultFontName}{
  \defineexpandable\DefaultFontName{modern}
  \defineexpandable\DefaultFontSize{10pt}
  \setupwhitespace[big]
}
\doifundefined{PortionMark}{
  %\defineexpandable[2]\PortionMark{(#1 - #2)}
  \defineexpandable[2]\PortionMark{}
}
\usetypescript[\DefaultFontName]
\setupbodyfont[\DefaultFontName, \DefaultFontSize]
\setupinteraction[state=start, color=blue, style=\tf]
\setbreakpoints[compound]

% start of document
\starttext
)";
  }

  void
  Context::codeTeardown(auto& oss) const
  {
    oss << R"(
% end of document
\stoptext
)";
  }

  void
  Context::codeTableClose(auto& oss) const
  {
    oss << R"(
    % end of rows
    \stopxtablebody
    \switchtobodyfont[\DefaultFontSize]
  \stopxtable
}
)";
  }

  // ------------------------------------------------------------
  // Intra-network
  // ------------------------------------------------------------
  void
  Context::codeTableIntra(auto& oss, const auto& srcIp) const
  {
    oss << R"(
% table
\placetable[here,split]
{
  \PortionMark{TODO--Caption Classification}{}
  Reachable ports from \type{)" << srcIp << R"(}.
  \startmode[TabFigClassStmt]
    Table is TODO--Overall Table Classification
  \stopmode
}
{
  \startxtable[frame=off, split=yes, header=repeat, option={stretch,width}]
    \switchtobodyfont[7pt]

    % table header
    \startxtablehead[topframe=on, bottomframe=on]
      \startmode[PortMark]
        \startxrow
          \startxcell[nx=5, frame=on]
            {\midaligned{\bf TODO--Overall Table Classification}}
          \stopxcell
        \stopxrow
      \stopmode

      % table columns
      \startxrow
        \startxcell[width=0.27\makeupwidth, leftframe=on]
          {\bf Destination IP (Name)}
        \stopxcell
        \startxcell[width=0.08\makeupwidth, leftframe=on]
          {\bf Port}
        \stopxcell
        \startxcell[width=0.19\makeupwidth]
          {\bf (State/Reason)}
        \stopxcell
        \startxcell[width=0.14\makeupwidth, leftframe=on]
          {\bf Service}
        \stopxcell
        \startxcell[width=0.32\makeupwidth, rightframe=on]
          {}
        \stopxcell
      \stopxrow
    \stopxtablehead

    % table footer
    \startxtablefoot[topframe=on, bottomframe=on]
      \startxrow[topframe=off]
        \startxcell[leftframe=on]\stopxcell
        \startxcell[leftframe=on]\stopxcell
        \startxcell\stopxcell
        \startxcell[leftframe=on]\stopxcell
        \startxcell[rightframe=on]\stopxcell
      \stopxrow
      \startmode[PortMark]
        \startxrow
          \startxcell[nx=5, frame=on]
            {\midaligned{\bf TODO--Overall Table Classification}}
          \stopxcell
        \stopxrow
      \stopmode
    \stopxtablefoot

    % table rows
    \startxtablebody
)";
  }

  void
  Context::codeRowIntra(auto& oss,
      const auto& rowFrame,
      const auto& ip, const auto& hostname,
      const auto& portProto, const auto& pps,
      const auto& serviceName, const auto& serviceDesc
    ) const
  {
    oss << R"(
      % row
      \startxrow)" << rowFrame << R"(
        \startxcell[leftframe=on]   % ip (hostname)
          \type{)" << ip << R"(} \type{)" << hostname << R"(}
        \stopxcell
        \startxcell[leftframe=on]   % port/protocol
          \type{)" << portProto << R"(}
        \stopxcell
        \startxcell                 % port state/reason
          {\tt ()" << pps << R"()}
        \stopxcell
        \startxcell[leftframe=on]   % service name
          \type{)" << serviceName << R"(}
        \stopxcell
        \startxcell[rightframe=on]  % service description
          \type{)" << serviceDesc << R"(}
        \stopxcell
      \stopxrow
)";
  }

  // ------------------------------------------------------------
  // Inter-network
  // ------------------------------------------------------------
  void
  Context::codeTableInter(auto& oss, const auto& srcIp) const
  {
    oss << R"(
% table
\placetable[here,split]
{
  \PortionMark{TODO--Caption Classification}{}
  Reachable ports from \type{)" << srcIp << R"(}.
  \startmode[TabFigClassStmt]
    Table is TODO--Overall Table Classification
  \stopmode
}
{
  \startxtable[frame=off, split=yes, header=repeat, option={stretch,width}]
    \switchtobodyfont[7pt]

    % table header
    \startxtablehead[topframe=on, bottomframe=on]
      \startmode[PortMark]
        \startxrow
          \startxcell[nx=4, frame=on]
            {\midaligned{\bf TODO--Overall Table Classification}}
          \stopxcell
        \stopxrow
      \stopmode

      % table columns
      \startxrow
        \startxcell[width=0.34\makeupwidth, leftframe=on]
          {\bf Gateway IP (Name)}
        \stopxcell
        \startxcell[width=0.34\makeupwidth, leftframe=on]
          {\bf Destination IP (Name)}
        \stopxcell
        \startxcell[width=0.11\makeupwidth, leftframe=on]
          {\bf Port}
        \stopxcell
        \startxcell[width=0.21\makeupwidth, rightframe=on]
          {\bf (State/Reason)}
        \stopxcell
      \stopxrow
    \stopxtablehead

    % table footer
    \startxtablefoot[topframe=on, bottomframe=on]
      \startxrow[topframe=off]
        \startxcell[leftframe=on]\stopxcell
        \startxcell[leftframe=on]\stopxcell
        \startxcell[leftframe=on]\stopxcell
        \startxcell[rightframe=on]\stopxcell
      \stopxrow
      \startmode[PortMark]
        \startxrow
          \startxcell[nx=4, frame=on]
            {\midaligned{\bf TODO--Overall Table Classification}}
          \stopxcell
        \stopxrow
      \stopmode
    \stopxtablefoot

    % table rows
    \startxtablebody
)";
  }

  void
  Context::codeRowInter(auto& oss,
      const auto& rowFrame,
      const auto& nextHopIp, const auto& nextHopName,
      const auto& destIp, const auto& destName,
      const auto& portProto, const auto& pps
    ) const
  {
    std::string cellFrameDest {"[leftframe=on"};
    if ("" != destIp) {
      cellFrameDest = "[topframe=on, leftframe=on";
    }
    oss << R"(
      % row
      \startxrow)" << rowFrame << R"(
        \startxcell[leftframe=on]   % next hop ip (hostname)
          \type{)" << nextHopIp << R"(} \type{)" << nextHopName << R"(}
        \stopxcell
        \startxcell)" << cellFrameDest << R"(]   % destination ip (hostname)
          \type{)" << destIp << R"(} \type{)" << destName << R"(}
        \stopxcell
        \startxcell)" << cellFrameDest << R"(]   % port/protocol
          \type{)" << portProto << R"(}
        \stopxcell
        \startxcell)" << cellFrameDest << R"(, rightframe=on]  % port state/reason
          {\tt ()" << pps << R"()}
        \stopxcell
      \stopxrow
)";
  }

  // ------------------------------------------------------------
  // Nessus
  // ------------------------------------------------------------
  void
  Context::codeParagraphNessus(auto& oss,
      const auto& pluginId, const auto& pluginSeverity,
      const auto& pluginName, const auto& pluginDesc
    ) const
  {
    oss << R"(
% plugin data section
\section[section:nessus-plugin-)" << pluginId << R"(]
{
  \PortionMark{TODO--Caption Classification}{}
  Plugin: )" << pluginId << R"(;
  Severity: )" << pluginSeverity << R"(;
  )" << pluginName << R"(
}

\PortionMark{TODO--Caption Classification}{}
)" << pluginDesc << R"(

\PortionMark{TODO--Caption Classification}{}
{\bf Affected systems}:
)";
  }

  // ------------------------------------------------------------
  // Prowler
  // ------------------------------------------------------------
  void
  Context::codeChapterProwler(auto& oss) const
  {
    oss << R"(
% tool chapter
\chapter[appendix:prowler]
{\PortionMark{U}{} Prowler Consolidated Results}
The following contains a summarized output of the
Prowler\footnote{\goto{https://github.com/prowler-cloud/prowler}[url(https://github.com/prowler-cloud/prowler)]}
tool results.
These are only the checks which the tool flagged as fails.
They are organized by cloud provider, account identifier, and service.
)";
  }

  void
  Context::codeSectionProwler(auto& oss,
      const auto& provider, const auto& accountId
    ) const
  {
    oss << R"(
% provider-account section
\section[section:prowler-)" << provider << '-' << accountId << R"(]
{
  \PortionMark{TODO--Caption Classification}{}
  Prowler Checks: )" << provider << " -- " << accountId << R"(
})";
  }

  void
  Context::codeSubSectionProwler(auto& oss,
      const auto& service, const auto& subService
    ) const
  {
    std::string sectionAlias;
    std::string serviceFullName;

    if (subService.empty()) {
      sectionAlias = service;
      serviceFullName = service;
    } else {
      sectionAlias = service + '-' + subService;
      serviceFullName = service + ", " + subService;
    }

    oss << R"(
% service section
\subsection[section:prowler-service-)" << sectionAlias << R"(]
{
  \PortionMark{TODO--Caption Classification}{}
  Service: )" << serviceFullName << R"(
})";
  }

  void
  Context::codeSubSubSectionProwler(auto& oss,
      const auto& severity, const auto& checkId,
      const auto& description, const auto& risk, const auto& recommendation,
      const auto& url, const auto& code, const auto& impactedResources
    ) const
  {
    std::vector<std::string> lines;
    boost::split(lines, code, boost::is_any_of("\n"));
    std::map<std::string, std::string> codes;
    for (const auto& line : lines) {
      if (line.empty()) {continue;}
      const auto idx {line.find_first_of(":")};
      if (std::string::npos == idx) {
        LOG_ERROR << "Code line is not in 'key: value' format: "
                  << line << std::endl
                  ;
        std::exit(nmcu::Exit::FAILURE);
      } else {
        codes.emplace( line.substr(0, idx)
                     , line.substr(idx + 2)
                     );
      }
    }
    oss << R"(
\subsubsection[section:prowler-check-)" << checkId << R"(]
{
  \PortionMark{TODO--Caption Classification}{}
  Severity: )" << severity << R"(;\\
  Check ID: )" << checkId << R"(
}

\PortionMark{TODO--Caption Classification}{}
Check: )" << description << R"(

\PortionMark{TODO--Caption Classification}{}
Risk: )" << (risk.empty() ? "N/A" : risk) << R"(

\PortionMark{TODO--Caption Classification}{}
Recommendation: )" << recommendation << R"(

\startalignment[nothyphenated,hanging,table]
\startnarrower[0.15in]
\startitemize[packed]
  \item \goto{)" << url << R"(}[url()" << url << R"()]
)";
    for (const auto& [key, value] : codes) {
      oss << R"(  \item )" << key << ":~";
      if (value.starts_with("http")) {
        const auto nvalue {escapeSpecial(value)};
        oss << R"(\goto{)" << nvalue << R"(}[url()" << nvalue << R"()])";
      } else {
        oss << R"(\type<<)" << value << R"(>>)";
      }
      oss << '\n';
    }
    oss << R"(\stopitemize
\stopnarrower
\stopalignment

\PortionMark{TODO--Caption Classification}{}
{\bf )" << impactedResources.size() << R"( affected resources}:
\startalignment[nothyphenated,hanging,table]
\startnarrower[0.15in]
{\tt
  )" << nmcu::toString(impactedResources, ",\n  ") << R"(
}
\stopnarrower
\stopalignment
)" << "\n";
  }


  // ------------------------------------------------------------
  // SSH
  // ------------------------------------------------------------
  void
  Context::codeTableSsh(auto& oss) const
  {
    oss << R"(
% table
\placetable[here,split][table:observed-ssh-algorithms]
{
  \PortionMark{TODO--Caption Classification}{}
  Supported SSH algorithms observed by the assessment team.
  \startmode[TabFigClassStmt]
    Table is TODO--Overall Table Classification
  \stopmode
}
{
  \startxtable[frame=off, split=yes, header=repeat, option={stretch,width}]
    \switchtobodyfont[7pt]

    % table header
    \startxtablehead[topframe=on, bottomframe=on]
      \startmode[PortMark]
        \startxrow
          \startxcell[nx=3, frame=on]
            {\midaligned{\bf TODO--Overall Table Classification}}
          \stopxcell
        \stopxrow
      \stopmode

      % table columns
      \startxrow
        \startxcell[width=0.40\makeupwidth, leftframe=on]
          {\bf Server IP (Name)}
        \stopxcell
        \startxcell[width=0.25\makeupwidth, leftframe=on]
          {\bf Algorithm Type}
        \stopxcell
        \startxcell[width=0.35\makeupwidth, leftframe=on, rightframe=on]
          {\bf Algorithm}
        \stopxcell
      \stopxrow
    \stopxtablehead

    % table footer
    \startxtablefoot[topframe=on, bottomframe=on]
      \startxrow[topframe=off]
        \startxcell[leftframe=on]\stopxcell
        \startxcell[leftframe=on]\stopxcell
        \startxcell[leftframe=on, rightframe=on]\stopxcell
      \stopxrow
      \startmode[PortMark]
        \startxrow
          \startxcell[nx=3, frame=on]
            {\midaligned{\bf TODO--Overall Table Classification}}
          \stopxcell
        \stopxrow
      \stopmode
    \stopxtablefoot

    % table rows
    \startxtablebody
)";
  }

  void
  Context::codeRowSsh(auto& oss,
      const auto& rowFrame,
      const auto& ip, const auto& name,
      const auto& cellFrameType, const auto& algoType,
      const auto& cellFrameName, const auto& color, const auto& algoName
    ) const
  {
    oss << R"(
      % row
      \startxrow)" << rowFrame << R"(
        \startxcell[leftframe=on]   % server ip (hostname)
          \type{)" << ip << R"(} \type{)" << name << R"(}
        \stopxcell
        \startxcell)" << cellFrameType << R"(   % algorithm type
          \type{)" << algoType << R"(}
        \stopxcell
        \startxcell)" << cellFrameName << R"(   % algorithm name
          \startcolor[)" << color << R"(]
            \type{)" << algoName << R"(}
          \stopcolor
        \stopxcell
      \stopxrow
)";
  }
}
