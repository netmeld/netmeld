// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include "WriterCsv.hpp"

// =============================================================================
// Constructors
// =============================================================================
WriterCsv::WriterCsv(bool _toFile) :
  Writer(_toFile)
{}


// =============================================================================
// Methods
// =============================================================================
std::string
WriterCsv::getExtension() const
{
  return ".csv";
}

void
WriterCsv::addRows(std::ostringstream& oss) const
{
  for (const auto& row : rows) {
    bool first {true};
    for (const auto& col : row) {
      if (!first) {
        oss << ',';
      }
      first = false;
      auto ccol {replaceAll(col, "\"", "\\\"")};
      oss << '"' << ccol << '"';
    }
    oss << '\n';
  }
}

std::string
WriterCsv::getIntraNetwork(const std::string&) const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  // add column headers
  oss << R"("Destination IP",)"
      << R"("Hostname",)"
      << R"("Port",)"
      << R"("Protocol",)"
      << R"("State",)"
      << R"("Reason",)"
      << R"("Service Name",)"
      << R"("Service Description")"
      << '\n'
      ;

  // add table rows
  addRows(oss);

  return oss.str();
}

std::string
WriterCsv::getInterNetwork(const std::string&) const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  // add column headers
  oss << R"("Gateway IP",)"
      << R"("Hostname",)"
      << R"("Destination IP",)"
      << R"("Hostname",)"
      << R"("Port",)"
      << R"("Protocol",)"
      << R"("State",)"
      << R"("Reason")"
      << '\n'
      ;

  // add table rows
  addRows(oss);

  return oss.str();
}

std::string
WriterCsv::getNessus() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );
  
  // add column headers
  oss << R"("Plugin ID",)"
      << R"("Severity",)"
      << R"("Name",)"
      << R"("Description",)"
      << R"("Affected Systems")"
      << '\n'
      ;


  // add table rows
  addRows(oss);
  for (const auto& row : rows) {
    size_t count {row.size()};
    for (size_t i {0}; i < 4; i++) {
      auto ccol {replaceAll(row[i], "\"", "\\\"")};
      oss << '"' << ccol << '"' << ',';
    }
    oss << '"';
    for (size_t i {4}; i < count;) {
      oss << row[i];
      i++;

      if (!row[i].empty()) {
        oss << " (" << row[i] << ")";
      }
      i++;

      if (i < count) {
        oss << ", ";
      }
    }
    oss << '"';
  }

  return oss.str();
}

std::string
WriterCsv::getSshAlgorithms() const
{
  std::ostringstream oss(
        std::ios_base::binary
      | std::ios_base::trunc
      );

  // add column headers
  oss << R"("Server IP",)"
      << R"("Hostname",)"
      << R"("Algorithm Type",)"
      << R"("Algorithm",)"
      << R"("Color")"
      << '\n'
      ;

  // add table rows
  addRows(oss);

  return oss.str();
}
