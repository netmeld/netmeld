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

#ifndef LOGGER_SINGLETON_HPP
#define LOGGER_SINGLETON_HPP

#include <map>

#include <netmeld/core/utils/Logger.hpp>
#include <netmeld/core/utils/StreamUtilities.hpp>

namespace nmcu = netmeld::core::utils;

using nmcu::operator<<;


namespace netmeld::core::utils {

  class LoggerSingleton {
    private:
      Severity logLevel {Severity::INFORMATIONAL};

      std::map<Severity, Logger> loggers;

    public:
      static LoggerSingleton& getInstance();

    private:
      LoggerSingleton();

    public:
      LoggerSingleton(const LoggerSingleton&) = delete;
      void operator=(const LoggerSingleton&)  = delete;

      const Logger& getLogger(const Severity&) const;
      const Severity& getLevel() const;
      void setLevel(const Severity&);
  };
}

// START OF LOGGER DEFINES
// Expose to object users
#define LOG_EMER \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::EMERGENCY)
#define LOG_ALERT \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::ALERT)
#define LOG_CRIT \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::CRIT)
#define LOG_ERROR \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::ERROR)
#define LOG_WARN  \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::WARNING)
#define LOG_NOTICE  \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::NOTICE)
#define LOG_INFO  \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::INFORMATIONAL)
#define LOG_DEBUG \
  nmcu::LoggerSingleton::getInstance().getLogger(nmcu::Severity::DEBUG)
/* NOTE: Need to explicitly use netmeld::utils for re-defining
         BOOST_SPIRIT_DEBUG_OUT.  Boost appears to have a boost::utils with
         which the compiler may use instead.
 */
#define BOOST_SPIRIT_DEBUG_OUT \
   netmeld::core::utils::LoggerSingleton::getInstance().getLogger( \
      netmeld::core::utils::Severity::DEBUG_SPIRIT).getStream()
// END OF LOGGER DEFINES

#endif // LOGGER_SINGLETON_HPP
