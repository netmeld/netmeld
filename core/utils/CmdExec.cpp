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

#include <netmeld/core/utils/CmdExec.hpp>


namespace netmeld::core::utils {

  bool
  isCmdAvailable(const std::string& _cmd)
  {
    return (0 == cmdExec("type " + _cmd + " >/dev/null 2>/dev/null"));
  }

  int
  cmdExecOrExit(const std::string& _cmd)
  {
    auto exitStatus {cmdExec(_cmd)};
    if (-1 == exitStatus) {
      std::exit(nmcu::Exit::FAILURE);
    }
    return exitStatus;
  }

  int
  cmdExec(const std::string& _cmd)
  {
    LOG_DEBUG << _cmd << '\n';

    auto exitStatus {std::system(_cmd.c_str())};
    if (-1 == exitStatus) { LOG_ERROR << "Failure: " << _cmd << '\n'; }
    if (0 != exitStatus)  { LOG_WARN << "Non-Zero: " << _cmd << '\n'; }

    return exitStatus;
  }

  std::string
  cmdExecOut(const std::string& _cmd)
  {
    LOG_DEBUG << _cmd << '\n';

    std::unique_ptr<FILE, decltype(&pclose)>
        pipe(popen(_cmd.c_str(), "r"), pclose);
    if (!pipe) {
      LOG_ERROR << "Failure: " << _cmd << '\n';
      return "";
    }

    std::array<char, 128> buffer;
    std::ostringstream oss;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      oss << buffer.data();
    }

    return oss.str();
  }

}
