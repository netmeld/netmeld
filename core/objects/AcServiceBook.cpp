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

#include <netmeld/core/objects/AcServiceBook.hpp>


namespace netmeld::core::objects {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  AcServiceBook::AcServiceBook()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  bool
  AcServiceBook::isValid() const
  {
    return !name.empty();
  }

  void
  AcServiceBook::save(pqxx::transaction_base& t,
               const Uuid& toolRunId, const std::string& _deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AcBook object is not saving: " << toDebugString()
                << std::endl;
      return; // Always short circuit if invalid object
    }

    if (0 == data.size()) {
      t.exec_prepared("insert_raw_device_ac_service",
        toolRunId,
        _deviceId,
        name,
        nullptr);
    } else {
      for (const auto& entry : data) {
        t.exec_prepared("insert_raw_device_ac_service",
          toolRunId,
          _deviceId,
          name,
          entry);
      }
    }
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
