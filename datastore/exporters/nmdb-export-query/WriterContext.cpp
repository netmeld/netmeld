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
      << "  \\define[2]\\PortionMark{(#1 - #2)}\n"
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
WriterContext::addQueryInfo(std::string _queryInfo, std::string _query)
{
  std::size_t start = _queryInfo.find(_query);
  queryInfo = _queryInfo;
  queryInfo.insert(start + _query.length(), "\"");
  queryInfo.insert(start, "\"");
}

void
WriterContext::addColumn(std::string _colName, float _width)
{
  columns.push_back(_colName);
  columnWidths.emplace(_colName, _width);
}

void
WriterContext::addRow(std::vector<std::string> _row)
{
  rows.push_back(_row);
}


std::string
WriterContext::write() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  // add query
  oss << "% Generated with:\n"
      << "%   " << queryInfo << "\n";

  oss << addContextSetup();

  // add table
  oss << "\\placetable[here,split]\n"
      << "{\n"
      << "\\PortionMark{TODO--Caption Classification}{}\n"
      << "TODO--Table caption.\n"
      << "\\startmode[TabFigClassStmt]\n"
      << "Table is TODO--Overall Table Classification\n"
      << "\\stopmode\n"
      << "}\n"
      << "{\n"
      << "\\startxtable[frame=on, split=yes, header=repeat, "
        << "option={stretch,width}]\n"
      << "\\switchtobodyfont[8pt]\n"

  // start of table header
      << "\\startxtablehead[topframe=on, bottomframe=on]\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=" << columns.size() << ", frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n";

  // add columns
  oss << "\\startxrow\n";
  for (const auto& column : columns) {
    oss << "\\startxcell[width="
          << columnWidths.at(column)
          << "\\makeupwidth]\n"
        << "{\\bf "
          << column
          << "}\n"
        << "\\stopxcell\n";
  }
  oss << "\\stopxrow\n"
      << "\\stopxtablehead\n";

  // start of table footer
  oss << "\\startxtablefoot[topframe=on, bottomframe=on]\n"
      << "\\startmode[PortMark]\n"
      << "\\startxrow\n"
      << "\\startxcell[nx=" << columns.size() << ", frame=on]\n"
      << "{\\midaligned{\\bf TODO--Overall Table Classification}}\n"
      << "\\stopxcell\n"
      << "\\stopxrow\n"
      << "\\stopmode\n"
      << "\\stopxtablefoot\n";

  // setup regex
  std::regex bs {"\\\\"};
  std::regex escape {"#|_|\\$|\\|"};
  std::regex gt {"&gt;"};

  // add table rows
  oss << "\\startxtablebody\n";
  for (const auto& row : rows) {
    oss << "\\startxrow\n";
    for (const auto& cell : row) {
      auto cleanCell {std::regex_replace(cell, bs, "{\\backslash}")};
      cleanCell = std::regex_replace(cleanCell, escape, "\\$&");
      cleanCell = std::regex_replace(cleanCell, gt, ">");
      oss << "\\startxcell\n"
          << "\\type{"
            << cleanCell
            << "}\n"
          << "\\stopxcell\n";
    }
    oss << "\\stopxrow\n";
  }
  oss << "\\stopxtablebody\n"
      << "\\switchtobodyfont[\\DefaultFontSize]\n"
      << "\\stopxtable\n"
      << "}\n"
      ;

  oss << addContextTeardown();

  return oss.str();
}
