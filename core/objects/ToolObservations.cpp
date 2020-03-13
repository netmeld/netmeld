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

#include <netmeld/core/objects/ToolObservations.hpp>

#include <netmeld/core/utils/StringUtilities.hpp>


namespace netmeld::core::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  ToolObservations::ToolObservations()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  ToolObservations::addNotable(const std::string& _observation)
  {
    notables.emplace(_observation);
  }

  void
  ToolObservations::addUnsupportedFeature(const std::string& _observation)
  {
    unsupportedFeatures.emplace(_observation);
  }

  bool
  ToolObservations::isValid() const
  {
    return !(   notables.empty()
             && unsupportedFeatures.empty()
            );
  }

  void
  ToolObservations::saveQuiet(pqxx::transaction_base& t,
                              const Uuid& toolRunId,
                              const std::string& deviceId)
  {
    quiet = true;
    save(t, toolRunId, deviceId);
  }

  void
  ToolObservations::save(pqxx::transaction_base& t,
                         const Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "ToolObservations object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    saveObservations(t, toolRunId, "notable", notables);
    saveObservations(t, toolRunId, "unsupported feature", unsupportedFeatures);
  }

  void
  ToolObservations::saveObservations(pqxx::transaction_base& t,
                                     const Uuid& toolRunId,
                                     const std::string& category,
                                     const std::set<std::string>& observations)
  {
    for (const auto& observation : observations) {
      if (!quiet) {
        LOG_INFO << nmcu::toUpper(category) << ": " << observation << '\n';
      }
      t.exec_prepared("insert_raw_tool_observation",
          toolRunId,
          category,
          observation);
    }
  }

  std::string
  ToolObservations::toDebugString() const
  {
    std::ostringstream oss;

    oss << "["; // opening bracket

    oss << "[ NOTABLES: " << notables << "]";
    oss << "[ UNSUPPORTED FEATURES: " << unsupportedFeatures << "]";

    oss << "]"; // closing bracket

    return oss.str();
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
