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

#include <sstream>
#include <regex>

#include "WriterContext.hpp"

// =============================================================================
// Constructors
// =============================================================================
WriterContext::WriterContext(bool _toFile) :
  Writer(_toFile)
{}


// =============================================================================
// Methods
// =============================================================================
std::string
WriterContext::getExtension() const
{
  return ".tex";
}

std::string
WriterContext::addContextSetup() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

	oss << R"(
% pre-config
\doifundefined{DefaultFontName}{
  \define\DefaultFontName{modern}
  \define\DefaultFontSize{10pt}
}
\doifundefined{PortionMark}{
  %\define[2]\PortionMark{(#1 - #2)}
  \define[2]\PortionMark{}
}
\usetypescript[\DefaultFontName]
\setupbodyfont[\DefaultFontName, \DefaultFontSize]

% start of document
\starttext
)";

  return oss.str();
}

std::string
WriterContext::addContextTeardown() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << R"(
% end of document
\stoptext
)";

  return oss.str();
}

std::string
WriterContext::getIntraNetwork(const std::string& srcIp) const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << addContextSetup()
      << R"(
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

  oss << R"(
    \stopxtablebody
    \switchtobodyfont[\DefaultFontSize]
  \stopxtable
}
)";

  oss << addContextTeardown();

  return oss.str();
}

std::string
WriterContext::getInterNetwork(const std::string& srcIp) const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << addContextSetup()
      << R"(
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

    oss << R"(
      % row
      \startxrow)" << rowFrame << R"(
        \startxcell[leftframe=on]   % next hop ip (hostname)
          \type{)" << nextHopIp << R"(} \type{)" << nextHopName << R"(}
        \stopxcell
        \startxcell[leftframe=on]   % destination ip (hostname)
          \type{)" << destIp << R"(} \type{)" << destName << R"(}
        \stopxcell
        \startxcell[leftframe=on]   % port/protocol
          \type{)" << portProto << R"(}
        \stopxcell
        \startxcell[rightframe=on]  % port state/reason
          {\tt ()" << pps << R"()}
        \stopxcell
      \stopxrow
)";

  }

  oss << R"(
    \stopxtablebody
    \switchtobodyfont[\DefaultFontSize]
  \stopxtable
}
)";

  oss << addContextTeardown();

  return oss.str();
}

std::string
WriterContext::getNessus() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );
  
  oss << addContextSetup();

  // ConTeXt special characters
  std::vector<std::pair<std::regex, std::string>> patterns {
    // backslash replace
    {std::regex(R"(\\)"), R"({\backslash})"},
    // escape special characters
    {std::regex(R"(#|_|\$|\|)"), R"(\$&)"},
    // greater than
    {std::regex(R"(&gt;)"), R"(>)"},
    // less than
    {std::regex(R"(&lt;)"), R"(<)"},
    // new paragraph
    {std::regex(R"(\n\n)"),
     R"(\n\n\PortionMark{TODO--Caption Classification}{}\n)"},
    //{R"()", R"()"},
  };

  for (const auto& row : rows) {
    std::string pluginId        {row[0]};
    std::string pluginSeverity  {row[1]};
    std::string pluginName      {row[2]};
    std::string pluginDesc      {row[3]};

    // replace special characters in description
    for (const auto& p : patterns) {
      pluginDesc = std::regex_replace(pluginDesc, p.first, p.second);
    }

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

  oss << addContextTeardown();

  return oss.str();
}

std::string
WriterContext::getSshAlgorithms() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << addContextSetup()
      << R"(
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

  std::string lastServer;
  std::string lastAlgo;
  for (const auto& row : rows) {
    std::string ip        {""}; // row[0]
    std::string name      {""}; // row[1]
    std::string algoType  {""}; // row[2]
    std::string algoName  {row[3]};
    std::string color     {row[4]};

    std::string nextAlgoType {row[2]};
    if (lastAlgo != nextAlgoType) {
      oss << "\n% " << row[0] << " -- " << nextAlgoType << '\n';
    }

    std::string rowFrame {""};
    std::string ipName   {"{} {}"};
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
      newRow        = true;
      algoType      = nextAlgoType;
      cellFrameType = "[topframe=on, leftframe=on]";
      cellFrameName = "[topframe=on, leftframe=on, rightframe=on]";
    }
    lastAlgo = nextAlgoType;

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

  oss << R"(
    \stopxtablebody
    \switchtobodyfont[\DefaultFontSize]
  \stopxtable
}
)";

  oss << addContextTeardown();

  return oss.str();
}
