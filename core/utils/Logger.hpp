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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

#include <netmeld/core/utils/Exit.hpp>
#include <netmeld/core/utils/Severity.hpp>


namespace netmeld::core::utils {

  static std::mutex nmLogMutex;

  class Logger {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
      Severity                              value;
      std::reference_wrapper<std::ostream>  stream;
      std::string                           prefix;
      bool                                  enabled;

    protected:
    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Logger() = delete;
      Logger(const Severity&, std::ostream&, const std::string&, bool = true);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
      static std::ostream& getBadStream();

    protected:
    public:
      void enable();
      void disable();

      std::ostream& getStream() const;

      template<typename T>
      friend std::ostream& operator<<(const Logger&, const T&);
      friend std::ostream& operator<<(const Logger&,
                                      std::ostream& (*F)(std::ostream&));
  };

  // ===========================================================================
  // Template Definition
  // ===========================================================================
  template<typename T>
  std::ostream&
  operator<<(const Logger& l, const T& data)
  {
    std::lock_guard<std::mutex> lock(nmLogMutex);
    return l.getStream() << l.prefix << data;
  }
}

#endif // LOGGER_HPP
