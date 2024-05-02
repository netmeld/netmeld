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

#include <cstring>
#include <exception>
#include <iostream>

extern "C" {
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
}

#include <netmeld/core/utils/ForkExec.hpp>
#include <netmeld/core/utils/LoggerSingleton.hpp>
#include <netmeld/core/utils/ContainerUtilities.hpp>


namespace netmeld::core::utils {

  [[ noreturn ]] void
  exec(std::string const& filename, std::vector<std::string> const& args)
  {
    // Extract args into the format needed by execvp.
    std::vector<char*> argv;
    for (const auto& arg : args) {
      // cppcheck-suppress useStlAlgorithm
      argv.push_back(strdup(arg.c_str()));
    }
    argv.push_back(nullptr);

    // Execute the command using the args.
    execvp(filename.c_str(), &argv[0]);
    LOG_ERROR << "Execution failure"
              << " (" << errno << ": " << std::strerror(errno) << ")"
              << " for command: " << toString(args)
              << std::endl;
    std::exit(Exit::FAILURE);
  }

  [[ noreturn ]] void
  exec(std::vector<std::string> const& args)
  {
    if (args.size() < 1) {
      LOG_ERROR << "No args to execute" << std::endl;
      std::exit(Exit::FAILURE);
    }

    exec(args.at(0), args);
  }

  pid_t
  forkExec(std::string const& filename, std::vector<std::string> const& args)
  {
    pid_t pid = fork();
    switch (pid)
    {
      case -1: // Error
        {
          LOG_ERROR << "Fork error"
                    << " (" << errno << ": " << std::strerror(errno) << ")"
                    << " for command: " << toString(args)
                    << std::endl;
          std::exit(Exit::FAILURE);
        }
      case 0: // In child
        {
          exec(filename, args);
        }
      default: // In parent
        {
          // Fall through and return child's PID.
        }
    }
    return pid;
  }

  int
  forkExec(std::vector<std::string> const& args)
  {
    if (args.size() < 1) {
      LOG_ERROR << "No args to execute" << std::endl;
      std::exit(Exit::FAILURE);
    }

    return forkExec(args.at(0), args);
  }

  int
  forkExecWait(std::string const& filename, std::vector<std::string> const& args)
  {
    int childResult {-1};
    pid_t childPid {forkExec(filename, args)};

    if (-1 == waitpid(childPid, &childResult, 0)) {
      LOG_ERROR << "Abnormal execution termination"
                << " (" << errno << ": " << std::strerror(errno) << ")"
                << " for command: " << toString(args)
                << std::endl;
      std::exit(Exit::FAILURE);
    }

    return childResult;
  }

  int
  forkExecWait(std::vector<std::string> const& args)
  {
    if (args.size() < 1) {
      LOG_ERROR << "No args to execute" << std::endl;
      std::exit(Exit::FAILURE);
    }

    return forkExecWait(args.at(0), args);
  }
}
