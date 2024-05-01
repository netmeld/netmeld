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

#include <nlohmann/json.hpp>
#include <string>

#include "Parser.hpp"

using json = nlohmann::json;

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJsonV2(std::ifstream& _file)
{
  // V2 output is a JSON lines file
  Data d;
  std::string line;
  while (std::getline(_file, line)) {
    json jline = json::parse(line);
    if (!jline.empty()) {
      nmdop::ProwlerV2Data v2d {jline};
      if (v2d != nmdop::ProwlerV2Data()) {
        d.v2Data.emplace_back(v2d);
      }
    } else {
      LOG_WARN << "Malformed input: Empty JSON data." << std::endl;
    }
  }
  if (d != Data()) {
    r.emplace_back(d);
  }
}

void
Parser::fromJsonV3(std::ifstream& _file)
{
  // V3 output is a JSON array
  Data d;
  auto dataArray = json::parse(_file);
  for (const auto& entry : dataArray) {
    if (!entry.empty()) {
      nmdop::ProwlerV3Data v3d {entry};
      if (v3d != nmdop::ProwlerV3Data()) {
        d.v3Data.emplace_back(v3d);
      }
    } else {
      LOG_WARN << "Malformed input: Empty JSON data." << std::endl;
    }
  }
  if (d != Data()) {
    r.emplace_back(d);
  }
}



// =============================================================================
// Parser helper methods
// =============================================================================
Result
Parser::getData()
{
  return r;
}
