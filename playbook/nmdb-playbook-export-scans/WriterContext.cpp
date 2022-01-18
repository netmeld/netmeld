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
WriterContext::WriterContext()
{}


// =============================================================================
// Methods
// =============================================================================
std::string
WriterContext::addContextSetup() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

	oss << "\\doifundefined{DefaultFontName}{\n"
      << "  \\define\\DefaultFontName{modern}\n"
      << "  \\define\\DefaultFontSize{10pt}\n"
      << "}\n"
      << "\\doifundefined{PortionMark}{\n"
      << "  %\\define[2]\\PortionMark{(#1 - #2)}\n"
      << "  \\define[2]\\PortionMark{}\n"
      << "}\n"
      << "\\usetypescript[\\DefaultFontName]\n"
      << "\\setupbodyfont[\\DefaultFontName, \\DefaultFontSize]\n"
      << "\\starttext\n"
      ;

  return oss.str();
}

std::string
WriterContext::addContextTeardown() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << "\\stoptext\n";

  return oss.str();
}

void
WriterContext::addRow(std::vector<std::string> _row)
{
  rows.push_back(_row);
}

std::string
WriterContext::writeIntraNetwork(const std::string& srcIp) const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << addContextSetup();

  // add table
  oss << "\\placetable[here,split]\n"
      << "{\n"
      << "\\PortionMark{TODO--Caption Classification}{}\n"
      << "Reachable ports from \\type{" << srcIp << "}.\n"
      << "\\startmode[TabFigClassStmt]\n"
      << "Table is TODO--Overall Table Classification\n"
      << "\\stopmode\n"
      << "}\n"
      << "{\n"
      << "\\startxtable[frame=off, split=yes, header=repeat, "
        << "option={stretch,width}]\n"
      << "\\switchtobodyfont[7pt]\n"

  // start of table header
      << "\\startxtablehead[topframe=on, bottomframe=on]\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=5, frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n";

  // add columns
  oss << "\\startxrow\n"
      << "\\startxcell[width=0.27\\makeupwidth, leftframe=on]\n"
      << "{\\bf Destination IP (Name)}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.08\\makeupwidth, leftframe=on]\n"
      << "{\\bf Port}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.19\\makeupwidth]\n"
      << "{\\bf (State/Reason)}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.14\\makeupwidth, leftframe=on]\n"
      << "{\\bf Service}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.32\\makeupwidth, rightframe=on]\n"
      << "{}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopxtablehead\n";

  // start of table footer
  oss << "\\startxtablefoot[topframe=on, bottomframe=on]\n"
      << "\\startxrow[topframe=off]\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell\\stopxcell\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[rightframe=on]\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=5, frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n"
      << "\\stopxtablefoot\n";

  // add table rows
  oss << "\\startxtablebody\n";
  std::string lastIpName;
  for (const auto& row : rows) {
    std::string rowFrame {""};
    std::string ipName {"{} {}"};
    if (lastIpName != std::string(row[0] + row[1])) {
      rowFrame = "[topframe=on]";
      ipName   = "\\type{" + row[0] + "} \\type{(" + row[1] + ")}";
    }
    oss << "\\startxrow" << rowFrame << "\n"
        // ip (name)
        << "\\startxcell[leftframe=on]\n"
        << ipName << "\n"
        //<< "\\type{" << row[0] << "} \\type{(" << row[1] << ")} \n"
        << "\\stopxcell\n"
        // port/protocol
        << "\\startxcell[leftframe=on]\n"
        << "\\type{" << row[2] << "/" << row[3] << "}\n"
        << "\\stopxcell\n";

    std::string pps = row[4] + "/" + row[5];
    pps = replaceAll(pps, "|", "\\|");
        // port state/reason
    oss << "\\startxcell\n"
        << "{\\tt (" << pps << ")}\n"
        << "\\stopxcell\n"
        // service name
        << "\\startxcell[leftframe=on]\n"
        << "\\type{" << row[6] << "}\n"
        << "\\stopxcell\n"
        // service description
        << "\\startxcell[rightframe=on]\n"
        << "\\type{" << row[7] << "}\n"
        << "\\stopxcell\n"
        << "\\stopxrow\n";
    lastIpName = std::string(row[0] + row[1]);
  }
  oss << "\\stopxtablebody\n"
      << "\\switchtobodyfont[\\DefaultFontSize]\n"
      << "\\stopxtable\n"
      << "}\n"
      ;

  oss << addContextTeardown();

  return oss.str();
}

std::string
WriterContext::writeInterNetwork(const std::string& srcIp) const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << addContextSetup();

  // add table
  oss << "\\placetable[here,split]\n"
      << "{\n"
      << "\\PortionMark{TODO--Caption Classification}{}\n"
      << "Reachable ports from \\type{" << srcIp << "}.\n"
      << "\\startmode[TabFigClassStmt]\n"
      << "Table is TODO--Overall Table Classification\n"
      << "\\stopmode\n"
      << "}\n"
      << "{\n"
      << "\\startxtable[frame=off, split=yes, header=repeat, "
        << "option={stretch,width}]\n"
      << "\\switchtobodyfont[7pt]\n"

  // start of table header
      << "\\startxtablehead[topframe=on, bottomframe=on]\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=4, frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n";

  // add columns
  oss << "\\startxrow\n"
      << "\\startxcell[width=0.34\\makeupwidth, leftframe=on]\n"
      << "{\\bf Gateway IP (Name)}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.34\\makeupwidth, leftframe=on]\n"
      << "{\\bf Destination IP (Name)}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.11\\makeupwidth, leftframe=on]\n"
      << "{\\bf Port}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.21\\makeupwidth, rightframe=on]\n"
      << "{\\bf (State/Reason)}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopxtablehead\n";

  // start of table footer
  oss << "\\startxtablefoot[topframe=on, bottomframe=on]\n"
      << "\\startxrow[topframe=off]\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[rightframe=on]\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=4, frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n"
      << "\\stopxtablefoot\n";

  // add table rows
  oss << "\\startxtablebody\n";
  std::string lastRouter;
  std::string lastIpName;
  for (const auto& row : rows) {
    std::string rowFrame {""};
    std::string ipName {"{} {}"};
    if (lastRouter != std::string(row[0] + row[1])) {
      rowFrame = "[topframe=on]";
      ipName   = "\\type{" + row[0] + "} \\type{(" + row[1] + ")}";
    }
    oss << "\\startxrow" << rowFrame << "\n"
        // gateway (name)
        << "\\startxcell[leftframe=on]\n"
        << ipName << "\n"
        << "\\stopxcell\n";

    ipName = "{} {}";
    if ("" != rowFrame || lastIpName != std::string(row[2] + row[3])) {
      ipName   = "\\type{" + row[2] + "} \\type{(" + row[3] + ")}";
    }

        // desination (name)
    oss << "\\startxcell[leftframe=on]\n"
        << ipName << "\n"
        << "\\stopxcell\n"
        // port/protocol
        << "\\startxcell[leftframe=on]\n"
        << "\\type{" << row[4] << "/" << row[5] << "}\n"
        << "\\stopxcell\n"
        // port state/reason
        << "\\startxcell[rightframe=on]\n"
        << "\\type{(" << row[6] << "/" << row[7] << ")}\n"
        << "\\stopxcell\n"
        << "\\stopxrow\n";
    lastRouter = std::string(row[0] + row[1]);
    lastIpName = std::string(row[2] + row[3]);
  }
  oss << "\\stopxtablebody\n"
      << "\\switchtobodyfont[\\DefaultFontSize]\n"
      << "\\stopxtable\n"
      << "}\n"
      ;

  return oss.str();
}

std::string
WriterContext::writeNessus() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );
  
  oss << addContextSetup();

  for (const auto& row : rows) {
    // reference and title
    oss << "\\section[section:nessus-plugin-" << row[0] << "]\n"
        << "{\\PortionMark{TODO--Caption Classification}{}\n"
        << " Plugin: " << row[0] << ";"
          << " Severity: " << row[1] << ";"
          << " " << row[2] << "\n"
        << "}\n\n";

    // description
    std::string desc = row[3];
    std::regex bs {"\\\\"};
    desc = std::regex_replace(desc, bs, "{\\backslash}");
    std::regex escape {"#|_|\\$|\\|"};
    desc = std::regex_replace(desc, escape, "\\$&");
    std::regex gt {"&gt;"};
    desc = std::regex_replace(desc, gt, ">");
    std::regex dnl {"\n\n"};
    desc = std::regex_replace(desc, dnl, "\n\n\\PortionMark{TODO--Caption Classification}{}\n");
    oss << "\\PortionMark{TODO--Caption Classification}{}\n"
        << desc << "\n\n";

    // affected systems
    oss << "\\PortionMark{TODO--Caption Classification}{}\n"
        << "{\\bf Affected systems}: ";

    std::vector<std::string> rest(row.begin()+4, row.end());
    for (size_t i {0}; i < rest.size();) {
      oss << "\\type{" << rest[i] << "}";
      i++;

      if (!rest[i].empty()) {
        oss << " (\\type{" << rest[i] << "})";
      }
      i++;

      if (i < rest.size()) {
        oss << ", ";
      }
    }
    oss << ".\n\n";
  }

  oss << addContextTeardown();

  return oss.str();
}

std::string
WriterContext::writeSshAlgorithms() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  oss << addContextSetup();

  // add table
  oss << "\\placetable[here,split][table:observed-ssh-algorithms]\n"
      << "{\n"
      << "\\PortionMark{TODO--Caption Classification}{}\n"
      << "Supported SSH algorithms observed by the assessment team.\n"
      << "\\startmode[TabFigClassStmt]\n"
      << "Table is TODO--Overall Table Classification\n"
      << "\\stopmode\n"
      << "}\n"
      << "{\n"
      << "\\startxtable[frame=off, split=yes, header=repeat, "
        << "option={stretch,width}]\n"
      << "\\switchtobodyfont[7pt]\n"

  // start of table header
      << "\\startxtablehead[topframe=on, bottomframe=on]\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=3, frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n";

  // add columns
  oss << "\\startxrow\n"
      << "\\startxcell[width=0.40\\makeupwidth, leftframe=on]\n"
      << "{\\bf Server IP (Name)}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.25\\makeupwidth, leftframe=on]\n"
      << "{\\bf Algorithm Type}\n"
      << "\\stopxcell\n"
      << "\\startxcell[width=0.35\\makeupwidth, leftframe=on, rightframe=on]\n"
      << "{\\bf Algorithm}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopxtablehead\n";

  // start of table footer
  oss << "\\startxtablefoot[topframe=on, bottomframe=on]\n"
      << "\\startxrow[topframe=off]\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[leftframe=on]\\stopxcell\n"
      << "\\startxcell[leftframe=on, rightframe=on]\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=3, frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n"
      << "\\stopxtablefoot\n";

  // add table rows
  oss << "\\startxtablebody\n";
  std::string lastServer;
  std::string lastAlgo;
  for (const auto& row : rows) {
    if (lastAlgo != row[2]) {
      oss << "\n% " << row[0] << " -- " << row[2] << "\n";
    }

    std::string rowFrame {""};
    std::string ipName   {"{} {}"};
    bool newRow {false};
    if (lastServer != std::string(row[0] + row[1])) {
      rowFrame  = "[topframe=on]";
      ipName    = "\\type{" + row[0] + "} \\type{(" + row[1] + ")}";
      newRow    = true;
    }

    oss << "\\startxrow" << rowFrame << "\n"
        << "\\startxcell[leftframe=on]\n"
        << ipName << "\n"
        << "\\stopxcell\n";

    rowFrame  = "[leftframe=on]";
    ipName    = "{}";
    if (newRow || lastAlgo != row[2]) {
      rowFrame  = "[topframe=on, leftframe=on]";
      ipName    = "\\type{" + row[2] + "}";
      newRow    = true;
    }

    oss << "\\startxcell" << rowFrame << "\n"
        << ipName << "\n"
        << "\\stopxcell\n";

    rowFrame = "[leftframe=on, rightframe=on]";
    if (newRow || lastAlgo != row[2]) {
      rowFrame = "[topframe=on, leftframe=on, rightframe=on]";
    }

    oss << "\\startxcell" << rowFrame << "\n"
        << "\\startcolor[" << row[4] << "] "
        << "\\type{" << row[3] << "} "
        << "\\stopcolor\n"
        << "\\stopxcell\n"
        << "\\stopxrow\n";


    lastServer = std::string(row[0] + row[1]);
    lastAlgo   = row[2];
  }
  oss << "\\stopxtablebody\n"
      << "\\switchtobodyfont[\\DefaultFontSize]\n"
      << "\\stopxtable\n"
      << "}\n"
      ;

  oss << addContextTeardown();

  return oss.str();
}

std::string
WriterContext::replaceAll(
    const std::string& source, std::string from, std::string to
    ) const
{
  std::string str = source;
  for (auto pos {str.find(from)};
       pos != std::string::npos;
       pos = str.find(from, pos))
  {
    str.replace(pos, from.length(), to);
    pos += to.length();
  }

  return str;
}
