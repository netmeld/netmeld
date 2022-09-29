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

#ifndef EXPORT_SCAN_NESSUS_HPP
#define EXPORT_SCAN_NESSUS_HPP

#include "ExportScan.hpp"

namespace netmeld::export_scans {
// ============================================================================
// Primary object
// ============================================================================
class Nessus : public ExportScan {
  // ========================================================================
  // Variables
  // ========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope

  // ========================================================================
  // Constructors
  // ========================================================================
  private: // Constructors which should be hidden from API users
  protected: // Constructors part of subclass API
  public: // Constructors part of public API
    Nessus() = delete;
    explicit Nessus(const std::string&);

  // ========================================================================
  // Methods
  // ========================================================================
  private: // Methods which should be hidden from API users
    void exportTemplate(const auto&) const;
    void exportFromDb(const auto&, const pqxx::result&);

  protected: // Methods part of subclass API
  public: // Methods part of public API
    void exportScan(const std::unique_ptr<Writer>&)override;
};
}
#endif // EXPORT_SCAN_NESSUS_HPP
