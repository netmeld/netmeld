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

#include <netmeld/core/utils/Logger.hpp>


namespace netmeld::core::utils {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  Logger::Logger(const Severity& _value,
                 std::ostream& _stream,
                 const std::string& _prefix,
                 bool _enabled) :
    value(_value),
    stream(_stream),
    prefix(_prefix),
    enabled(_enabled)
  { }


  // ===========================================================================
  // Methods
  // ===========================================================================
  void
  Logger::enable()
  {
    std::lock_guard<std::mutex> lock(nmLogMutex);
    enabled = true;
  }

  void
  Logger::disable()
  {
    std::lock_guard<std::mutex> lock(nmLogMutex);
    enabled = false;
  }

  std::ostream&
  Logger::getStream() const
  {
    if (enabled) {
      return stream.get();
    } else {
      return getBadStream();
    }
  }

  std::ostream&
  Logger::getBadStream()
  {
    static std::ostringstream badStream;
    badStream.setstate(std::ios_base::badbit);

    return badStream;
  }


  // ===========================================================================
  // Friends
  // ===========================================================================
  std::ostream&
  // cppcheck-suppress constParameter
  operator<<(const Logger& l, std::ostream& (*F)(std::ostream&))
  {
    std::lock_guard<std::mutex> lock(nmLogMutex);
    // only really handles cases like Logger << std::endl;
    return F(l.getStream());
  }
}
