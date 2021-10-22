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

#include "TracerouteHop.hpp"

namespace netmeld::datastore::objects {

bool
TracerouteHop::isValid() const
{
  return rtrIpAddr.isValid()
      && dstIpAddr.isValid()
      && (hopCount != -1);
}

void
TracerouteHop::save(pqxx::transaction_base& t,
                    const nmco::Uuid& toolRunId, const std::string& deviceId)
{
  if (!isValid()) {
    LOG_DEBUG << "TracerouteHop object is not saving: " << toDebugString()
              << std::endl;
    return;
  }

  rtrIpAddr.save(t, toolRunId, deviceId);
  dstIpAddr.save(t, toolRunId, deviceId);

  t.exec_prepared("insert_raw_ip_traceroute",
      toolRunId,
      hopCount,
      rtrIpAddr.toString(),
      dstIpAddr.toString());
}

std::string
TracerouteHop::toDebugString() const
{
  std::ostringstream oss;
  oss << "[";
  oss << rtrIpAddr << ", "
      << dstIpAddr << ", "
      << hopCount;
  oss << "]";

  return oss.str();
}

std::string
TracerouteHop::toString() const
{
  return this->toDebugString();
}

}