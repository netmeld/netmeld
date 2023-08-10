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

#include "ProwlerData.hpp"

namespace netmeld::datastore::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  /*{"Profile":"","Account Number":"","Control":"","Message":"","Severity":"","Status":"","Scored":"","Level":"","Control ID":"","Region":"","Timestamp":"","Compliance":"","Service":"","CAF Epic":"","Risk":"","Remediation":"","Doc link":"","Resource ID":"","Account Email":"","Account Name":"","Account ARN":"","Account Organization":"","Account tags":""}*/
  ProwlerData::ProwlerData(const json& jline) :
      accountNumber(jline["Account Number"])
    , region(jline["Region"])
    , control(jline["Control"])
    , severity(jline["Severity"])
    , status(jline["Status"])
    , level(jline["Level"])
    , controlId(jline["Control ID"])
    , service(jline["Service"])
    , risk(jline["Risk"])
    , remediation(jline["Remediation"])
    , documentationLink(jline["Doc link"])
    , resourceId(jline["Resource ID"])
  {
    timestamp.readFormatted(jline["Timestamp"], "%Y-%m-%dT%H:%M:%SZ"); // 2022-01-01T01:01:01Z
  }

  // ===========================================================================
  // Methods
  // ===========================================================================

  bool
  ProwlerData::isValid() const
  {
    return !(
           accountNumber.empty()
        || timestamp.isNull()
        || region.empty()
        || level.empty()
        || controlId.empty()
        || service.empty()
      );
  }

  void
  ProwlerData::save(pqxx::transaction_base& t,
                         const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "ProwlerData object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    // NOTE: The following are the suspected minimum for unique:
    //         accountNumber, timestamp, region, level, controlId, service,
    //         resourceId
    //       However, resourceId can be NULL so problematic for the DB
    t.exec_prepared("insert_raw_prowler_check",
          toolRunId
        , accountNumber
        , timestamp
        , region
        , level
        , controlId
        , service
        , status
        , severity
        , control
        , risk
        , remediation
        , documentationLink
        , resourceId
      );
  }

  std::string
  ProwlerData::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["; // opening bracket

    oss << R"("accountNumber": ")" << accountNumber << R"(", )";
    oss << R"("timestamp": ")" << timestamp << R"(", )";
    oss << R"("region": ")" << region << R"(", )";
    oss << R"("control": ")" << control << R"(", )";
    oss << R"("severity": ")" << severity << R"(", )";
    oss << R"("status": ")" << status << R"(", )";
    oss << R"("level": ")" << level << R"(", )";
    oss << R"("controlId": ")" << controlId << R"(", )";
    oss << R"("service": ")" << service << R"(", )";
    oss << R"("risk": ")" << risk << R"(", )";
    oss << R"("remediation": ")" << remediation << R"(", )";
    oss << R"("documentationLink": ")" << documentationLink << R"(", )";
    oss << R"("resourceId": ")" << resourceId << R"(")";

    oss << "]"; // closing bracket

    return oss.str();
  }

  std::partial_ordering
  ProwlerData::operator<=>(const ProwlerData& rhs) const
  {
    return std::tie( accountNumber
                   , timestamp
                   , region
                   , control
                   , severity
                   , status
                   , level
                   , controlId
                   , service
                   , risk
                   , remediation
                   , documentationLink
                   , resourceId
                   )
       <=> std::tie( rhs.accountNumber
                   , rhs.timestamp
                   , rhs.region
                   , rhs.control
                   , rhs.severity
                   , rhs.status
                   , rhs.level
                   , rhs.controlId
                   , rhs.service
                   , rhs.risk
                   , rhs.remediation
                   , rhs.documentationLink
                   , rhs.resourceId
                   )
      ;
  }

  bool
  ProwlerData::operator==(const ProwlerData& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
