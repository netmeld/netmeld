// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include "NessusResult.hpp"

bool
NessusResult::isValid() const
{
  // pluginId is also a PK (also all fields are NOT NULL)
  return port.isValid();
}

void
NessusResult::save( pqxx::transaction_base& t
                  , const nmco::Uuid& toolRunId, const std::string& deviceId)
{
  if (!isValid()) {
    LOG_DEBUG << "NessusResult object is not saving: " << toDebugString()
              << std::endl;
    return;
  }

  port.save(t, toolRunId, deviceId);

  t.exec_prepared( "insert_raw_nessus_result"
                 , toolRunId
                 , port.getIpAddress().toString()
                 , port.getProtocol()
                 , port.getPort()
                 , pluginId
                 , pluginName
                 , pluginFamily
                 , pluginType
                 , pluginOutput
                 , severity
                 , description
                 , solution
                 );
}

std::string
NessusResult::toDebugString() const
{
  const auto& textSnippet = [&](const std::string& text) {
      size_t count {50};
      std::string snippet;
      if (text.size() > count) {
        snippet = text.substr(0, count) + "...SNIPPED...";
      } else {
        snippet = text;
      }
      return snippet;
    };
  std::ostringstream oss;
  oss << "["
      << "port: " << port.toDebugString()
      << ", pluginId: " << pluginId
      << ", pluginName: " << pluginName
      << ", pluginFamily: " << pluginFamily
      << ", pluginType: " << pluginType
      << ", pluginOutput: " << textSnippet(pluginOutput)
      << ", severity: " << severity
      << ", description: " << textSnippet(description)
      << ", solution: " << solution
      << "]"
      ;

  return oss.str();
}

std::partial_ordering
NessusResult::operator<=>(const NessusResult& rhs) const
{
  return std::tie( port
                 , pluginId
                 , pluginName
                 , pluginFamily
                 , pluginType
                 , pluginOutput
                 , severity
                 , description
                 , solution
                 )
     <=> std::tie( rhs.port
                 , rhs.pluginId
                 , rhs.pluginName
                 , rhs.pluginFamily
                 , rhs.pluginType
                 , rhs.pluginOutput
                 , rhs.severity
                 , rhs.description
                 , rhs.solution
                 )
    ;
}

bool
NessusResult::operator==(const NessusResult& rhs) const
{
  return 0 == operator<=>(rhs);
}
