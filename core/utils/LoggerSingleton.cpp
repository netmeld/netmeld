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

#include <netmeld/core/utils/LoggerSingleton.hpp>


namespace netmeld::core::utils {

  LoggerSingleton::LoggerSingleton()
  {
    loggers.emplace(
        Severity::EMERGENCY,
        Logger(Severity::EMERGENCY, std::cerr, "EMER: ", true)
        );
    loggers.emplace(
        Severity::ALERT,
        Logger(Severity::ALERT, std::cerr, "ALERT: ", true)
        );
    loggers.emplace(
        Severity::CRITICAL,
        Logger(Severity::CRITICAL, std::cerr, "CRIT: ", true)
        );
    loggers.emplace(
        Severity::ERROR,
        Logger(Severity::ERROR, std::cerr, "ERROR: ", true)
        );
    loggers.emplace(
        Severity::WARNING,
        Logger(Severity::WARNING, std::cerr, "WARN: ", true)
        );
    loggers.emplace(
        Severity::NOTICE,
        Logger(Severity::NOTICE, std::cout, "", true)
        );
    loggers.emplace(
        Severity::INFORMATIONAL,
        Logger(Severity::INFORMATIONAL, std::cout, "", true)
        );
    loggers.emplace(
        Severity::DEBUG,
        Logger(Severity::DEBUG, std::cout, "DEBUG: ", false)
        );
    loggers.emplace(
        Severity::DEBUG_SPIRIT,
        Logger(Severity::DEBUG_SPIRIT, std::cout, "DEBUG_SPIRIT: ", false)
        );
  }

  LoggerSingleton&
  LoggerSingleton::getInstance()
  {
    static LoggerSingleton instance;
    return instance;
  }

  void
  LoggerSingleton::setLevel(const Severity& _severity)
  {
    logLevel = _severity;
    for (auto& keyValue : loggers) {
      Severity severity =  keyValue.first;
      Logger*  logger   = &keyValue.second;

      if (severity <= logLevel) {
        logger->enable();
      }
      else {
        logger->disable();
      }
    }
  }

  const Severity&
  LoggerSingleton::getLevel() const
  {
    return logLevel;
  }

  const Logger&
  LoggerSingleton::getLogger(const Severity& severity) const
  {
    return loggers.at(severity);
  }
}
