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

#include <netmeld/playbook/utils/QueriesPlaybook.hpp>

namespace netmeld::playbook::utils {

  PlaybookQueries::PlaybookQueries()
  {}

  void
  PlaybookQueries::addQuery(const std::string& _name,
                            const std::string& _query)
  {
    queries[_name] = _query;
    LOG_DEBUG << "PSQL query added: "
              << _name << " -- " << _query
              << "\n";
  }

  void
  PlaybookQueries::dbPrepare(pqxx::connection& db)
  {
    for (const auto& [_name, _query] : queries) {
      db.prepare(_name, _query);
    }
  }

  void
  PlaybookQueries::init(const std::string& _queryFilePath)
  {
    YAML::Node yConfig {YAML::LoadFile(_queryFilePath)};

    const auto& yQueries {yConfig["queries"]};
    for (const auto& yQuery : yQueries) {
      const auto& name  {yQuery["id"].as<std::string>()};
      const auto& query {yQuery["psql"].as<std::string>()};
      addQuery(name, query);
    }
  }
}
