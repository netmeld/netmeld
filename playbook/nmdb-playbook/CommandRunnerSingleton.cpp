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

#include <chrono>
#include <thread>
#include <regex>

#include <netmeld/core/utils/CmdExec.hpp>
#include <netmeld/core/utils/ForkExec.hpp>
#include <netmeld/core/utils/LoggerSingleton.hpp>

#include "CommandRunnerSingleton.hpp"

namespace nmcu = netmeld::core::utils;

namespace netmeld::playbook {

  // ===========================================================================
  // Constructors
  // ===========================================================================
  CommandRunnerSingleton::CommandRunnerSingleton()
  {}

  // ===========================================================================
  // Methods
  // ===========================================================================
  CommandRunnerSingleton&
  CommandRunnerSingleton::getInstance()
  {
    static CommandRunnerSingleton instance;
    return instance;
  }

  void
  CommandRunnerSingleton::disableCommands(
      std::set<uint32_t> const& _disabledCommands)
  {
    disabledCommands = _disabledCommands;
  }

  void
  CommandRunnerSingleton::setExecute(bool const state)
  {
    execute = state;
  }

  void
  CommandRunnerSingleton::setHeadless(bool const state)
  {
    headless = state;
  }

  bool
  CommandRunnerSingleton::isEnabled(uint32_t const commandId) const
  {
    if (!disabledCommands.empty() && disabledCommands.count(commandId)) {
      return false;
    }

    return true;
  }

  bool
  CommandRunnerSingleton::systemExec(std::string const& command)
  {
    if (isEnabled(++commandIdNumber)) {
      LOG_INFO << commandIdNumber << ": " << command << std::endl;
      if (execute) {
        return nmcu::cmdExecOrExit(command);
      }
    }

    return true;
  }

  void
  CommandRunnerSingleton::threadExec(
      std::vector<std::tuple<std::string, std::string>> const& commands)
  {
    std::vector<std::thread> threadVector;

    for (const auto& [commandTitle, command]: commands) {
      if (isEnabled(++commandIdNumber)) {
        LOG_DEBUG << "# " << commandTitle << std::endl;
        LOG_INFO << commandIdNumber << ": " << command << std::endl;

        if (execute) {
          auto threadActions = &CommandRunnerSingleton::xtermThreadActions;
          if(headless) {
            threadActions = &CommandRunnerSingleton::tmuxThreadActions;
          }
          threadVector.emplace_back(threadActions, this, commandTitle, command);
        }
      }
    }

    for (auto& tv : threadVector) {
      tv.join();
    }
  }

  void
  CommandRunnerSingleton::scheduleSleep(uint64_t const _ms)
  {
    if (isEnabled(++commandIdNumber)) {
      double seconds {_ms/1000.0};
      LOG_INFO << commandIdNumber << ": " << "sleep " << seconds << "s"
               << std::endl;
      if (execute) {
        std::this_thread::sleep_for(std::chrono::milliseconds(_ms));
      }
    }
  }

  void
  CommandRunnerSingleton::xtermThreadActions(std::string const& title,
      std::string const& command) const
  {
    std::vector<std::string> xtermArgs = {
      "lxterm",
      "-iconic",
      "-geometry", "110x25",
      "-bg", "black",
      "-fg", "red",
      "-T", title,
      "-e", command  // "-e command" must be the last option.
    };

    nmcu::forkExecWait(xtermArgs);
  }

  void
  CommandRunnerSingleton::tmuxThreadActions(std::string const& title,
      std::string const& command) const
  {
    std::string tmuxSafeTitle {title};
    std::vector<std::tuple<std::regex, std::string>> substitutions {
      {std::regex("\\."), "-"},
      {std::regex(":"), ""},
    };
    for (const auto& [find, replace] : substitutions) {
      tmuxSafeTitle = std::regex_replace(tmuxSafeTitle, find, replace);
    }

    std::vector<std::string> tmuxCommandArgs = {
      "tmux",
      "new-session", "-d",
      "-s", tmuxSafeTitle + "-session",
      "-n", "playbook-window" + commandIdNumber,
      command
    };
    nmcu::forkExecWait(tmuxCommandArgs);


    std::vector<std::string> tmuxStyleArgs = {
      "tmux",
      "set-window",
      "-t", "playbook-window" + commandIdNumber,
      "window-style", "bg=black,fg=red"
    };
    nmcu::forkExecWait(tmuxStyleArgs);


    std::vector<std::string> tmuxWaitArgs = {
      "tmux",
      "wait",
      tmuxSafeTitle + "-session"
    };
    nmcu::forkExecWait(tmuxWaitArgs);
  }

  // ===========================================================================
  // Friends
  // ===========================================================================
}
