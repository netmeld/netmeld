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

#include <netmeld/core/tools/AbstractTool.hpp>
#include <netmeld/core/utils/ForkExec.hpp>
#include <netmeld/core/objects/Time.hpp>
#include <netmeld/core/objects/Uuid.hpp>

#include "AugmentArgs.hpp"

extern "C" {
#include <fcntl.h>
#include <pty.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
}

namespace nmct = netmeld::core::tools;
namespace nmcu = netmeld::core::utils;
namespace nmco = netmeld::core::objects;


class Tool : public nmct::AbstractTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
    nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};
    nmco::Time timestampStart;
    nmco::Uuid toolRunId;

    sfs::path toolRunResults;

    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmct::AbstractTool
      (
       "Netmeld command line wrapper",  // short description
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractTool
    void
    addToolBaseOptions() override // Pre-subclass operations
    {
      // Remove all standard options, no short opts wanted
      opts.removeRequiredOption("db-name");
      opts.removeOptionalOption("zzzzzzzzzzhelp");
      opts.removeOptionalOption("zzzzzzzzzzversion");
      opts.removeAdvancedOption("zzzzzzzzzzverbosity");

      // Re-add select standard options, no short opts
      opts.addOptionalOption("zzzzzzzzzzhelp", std::make_tuple(
            "help",
            NULL_SEMANTIC,
            "Show this help message, then exit.")
          );
      opts.addOptionalOption("zzzzzzzzzzversion", std::make_tuple(
            "version",
            NULL_SEMANTIC,
            "Show version information, then exit.")
          );
      opts.addAdvancedOption("zzzzzzzzzzverbosity", std::make_tuple(
            "verbosity",
            po::value<nmcu::Severity>()->default_value(
              nmcu::LoggerSingleton::getInstance().getLevel()),
            "Alter verbosity level of tool.  See `man syslog` for levels."
            )
          );

      // Add tool specific option(s)
      opts.addRequiredOption("command", std::make_tuple(
            "command",
            po::value<std::vector<std::string>>()->multitoken()->required(),
            "Command to wrap")
          );
      opts.addPositionalOption("command", -1);
    }

    // Overriden from AbstractTool
    void
    modifyToolOptions() override { } // Private means no intention of allowing a subclass

    template<typename Data>
    void
    writeToFile(sfs::path const& path, const Data& data)
    {
      std::ofstream ofs {path};
      ofs << data << std::endl;
      ofs.close();
    }

    void
    createConfigFiles(sfs::path const& d)
    {
      std::string dir {d.string() + '/'};

      // Environment variables
      if (isCommandAvailable("/usr/bin/env")) {
        systemExec("/usr/bin/env >> " + dir + "env.txt");
      } else if (isCommandAvailable("env")) {
        systemExec("env >> " + dir + "env.txt");
      } else {
        LOG_WARN << "env not found, no environment information collected"
                 << std::endl;
      }

      // Kernel parameters
      if (isCommandAvailable("/sbin/sysctl")) {
        systemExec("/sbin/sysctl -a >> " + dir + "sysctl.txt");
      } else if (isCommandAvailable("sysctl")) {
        systemExec("sysctl -a >> " + dir + "sysctl.txt");
      } else {
        LOG_WARN << "sysctl not found, no kernel parameters collected"
                 << std::endl;
      }

      // Important configuration files in /etc/
      sfs::create_directories(dir + "etc");
      std::vector<std::string> etc_files = {
            "/etc/hosts",
            "/etc/resolv.conf"
        };

      for (const auto& file : etc_files) {
        if (sfs::exists(file)) {
          sfs::copy_file(file, dir + file);
        }
        else {
          LOG_WARN << "File not found (on copy): " << file << std::endl;
        }
      }
    }

    void
    createNetworkFiles(sfs::path const& d)
    {
      std::string dir {d.string() + '/'};
      bool limitedNetworkCommands {false};

      // Network interfaces (interface names, MAC addrs, and IP addrs, etc)
      if (isCommandAvailable("/bin/ip")) {
        systemExec("/bin/ip addr show >> " + dir + "ip_addr_show.txt");
      } else if (isCommandAvailable("ip")) {
        systemExec("ip addr show >> " + dir + "ip_addr_show.txt");
      } else if (isCommandAvailable("/sbin/ifconfig")) {
        systemExec("/sbin/ifconfig -a >> "  + dir + "ifconfig.txt");
      } else if (isCommandAvailable("ifconfig")) {
        systemExec("ifconfig -a >> "  + dir + "ifconfig.txt");
      } else {
        LOG_WARN << "ip or ifconfig not found, attempting limited"
                 << " network interface information collection from /proc\n";
        limitedNetworkCommands = true;
      }

      // Network routes
      if (isCommandAvailable("/bin/ip")) {
        systemExec("/bin/ip -4 route show >> " + dir + "ip4_route_show.txt");
        systemExec("/bin/ip -6 route show >> " + dir + "ip6_route_show.txt");
      } else if (isCommandAvailable("ip")) {
        systemExec("ip -4 route show >> " + dir + "ip4_route_show.txt");
        systemExec("ip -6 route show >> " + dir + "ip6_route_show.txt");
      } else if (isCommandAvailable("/bin/netstat")) {
        systemExec("/bin/netstat -nr >> " + dir + "netstat-nr.txt");
      } else if (isCommandAvailable("netstat")) {
        systemExec("netstat -nr >> " + dir + "netstat-nr.txt");
      } else {
        LOG_WARN << "ip or netstat not found, attempting limited"
                 << " network route information collection from /proc\n";
        limitedNetworkCommands = true;
      }

      if (limitedNetworkCommands) {
        sfs::create_directories(dir + "/proc/net");
        std::vector<std::string> procFiles = {
              "/proc/net/arp",
              "/proc/net/dev",
              "/proc/net/fib_trie",
              "/proc/net/route",
              "/proc/net/wireless"
          };
        for (const auto& file : procFiles) {
          if (sfs::exists(file)) {
            sfs::copy_file(file, dir + file);
          }
          else {
            LOG_WARN << "File not found (on copy): " << file << std::endl;
          }
        }
      }

      // Firewall rules
      if (isCommandAvailable("/sbin/iptables")) {
        systemExec("/sbin/iptables-save --counters >> " + dir
                   + "ip4_tables_save.txt");
        systemExec("/sbin/ip6tables-save --counters >> " + dir
                  + "ip6_tables_save.txt");
      } else if (isCommandAvailable("iptables")) {
        systemExec("iptables-save --counters >> " + dir
                   + "ip4_tables_save.txt");
        systemExec("ip6tables-save --counters >> " + dir
                  + "ip6_tables_save.txt");
      } else {
        // warn the following message instead
        LOG_WARN << "iptables not found, no firewall information collected"
                 << std::endl;
      }
    }

    bool
    isCommandAvailable(const std::string& command)
    {
      return (0 == systemExec("type " + command + " >/dev/null 2>/dev/null"));
    }

    int
    systemExec(const std::string& command)
    {
      int exitCode;

      exitCode = system(command.c_str());
      if (-1 == exitCode) {
        LOG_ERROR << "Execution failure (" << exitCode << ") for: " << command
                  << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      return exitCode;
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printHelp() const;
      // virtual void printVersion() const;
    int
    runTool() override
    {
      auto args1 {opts.getUnrecognized()};
      auto& args {args1};

      std::string const toolName {args.at(0)};

      std::stringstream oss;
      oss << toolName << '_' << timestampStart.toIsoString() << '_' << toolRunId;
      std::string const toolRunName {oss.str()};

      // Create directory for meta-data about and results from this tool run.
      toolRunResults = nmfm.getSavePath()/"clw"/toolRunName;
      bool dirCreated {sfs::create_directories(toolRunResults)};

      if (dirCreated) {
        // TODO 11DEC18 To do, or not?  Issue only occurs with sudo -E
        //nmfm.giveAll(toolRunResults);
      }

      return doWork(args);
    }

    int
    doWork(std::vector<std::string>& args)
    {
      // Open pseudo-tty (pty) master (for child's stdin and stdout).
      int ptmInOut = posix_openpt(O_RDWR | O_NOCTTY);
      if ((-1 == ptmInOut) ||
          (-1 == grantpt(ptmInOut)) ||
          (-1 == unlockpt(ptmInOut))) {
        LOG_ERROR << "Error on pseudo-tty master" << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      // Open a pipe (for child's stderr).
      int ptErr[2];
      if (-1 == pipe(ptErr)) {
        LOG_ERROR << "Error on pipe()" << std::endl;
        std::exit(nmcu::Exit::FAILURE);
      }

      // Fork child process and manage I/O with the child process.
      pid_t pid = fork();

      switch (pid)
      {
        case -1:
          {  // Error.
            LOG_ERROR << "Error on fork()" << std::endl;
            std::exit(nmcu::Exit::FAILURE);
          }

        case 0:
          {   // In child.
            std::string pstDevice {ptsname(ptmInOut)};
            close(ptmInOut);  // Child has no further use for the pty master.
            close(ptErr[0]);  // Child won't read from stderr pipe.

            // Open pseudo-tty (pty) slave.
            int ptsInOut = open(pstDevice.c_str(), O_RDWR | O_NOCTTY);
            if (-1 == ptsInOut) {
              LOG_ERROR << "Error on pseudo-tty slave" << std::endl;
              std::exit(nmcu::Exit::FAILURE);
            }

            // Replace child's stdin and stdout with the pty slave.
            // Replace child's stderr with the stderr pipe.
            dup2(ptsInOut, STDIN_FILENO);
            dup2(ptsInOut, STDOUT_FILENO);
            dup2(ptErr[1], STDERR_FILENO);

            // Don't need pty slave or stderr pipe after they've been dup'd.
            close(ptsInOut);
            close(ptErr[1]);

            setsid();
            ioctl(STDIN_FILENO, TIOCSCTTY, 1);

            // Record meta-data about this tool run.
            writeToFile<nmco::Uuid>
              (toolRunResults/"tool_run_id.txt", toolRunId);
            writeToFile<nmco::Time>
              (toolRunResults/"timestamp_start.txt", timestampStart);

            writeToFile<std::string>
              (toolRunResults/"command_line_original.txt",
              nmcu::toString(args));
            netmeld::utils::augmentArgs(args, toolRunResults);
            writeToFile<std::string>
              (toolRunResults/"command_line_modified.txt",
               nmcu::toString(args));

            createConfigFiles(toolRunResults);
            createNetworkFiles(toolRunResults);

            nmcu::exec(args.at(0), args); // does not return
          }
        default:
          {  // In parent.
            std::ofstream logChildStdIn
              {(toolRunResults/"stdin.txt").string()};

            std::ofstream logChildStdOut
              {(toolRunResults/"stdout.txt").string()};

            std::ofstream logChildStdErr
              {(toolRunResults/"stderr.txt").string()};

            // Store copy of terminal setting to be restored later.
            termios ptySettingsOriginal;
            tcgetattr(STDIN_FILENO, &ptySettingsOriginal);

            // Immediately pass characters to/from the child,
            // ignore signaling (pass through to child),
            // and don't echo input (child controls echo settings).
            termios ptySettings;
            tcgetattr(STDIN_FILENO, &ptySettings);
            // The following pragmas are to silence an accepted warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
            ptySettings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
#pragma GCC diagnostic pop
            tcsetattr(STDIN_FILENO, TCSANOW, &ptySettings);

            // Handle forwarding I/O between parent and child.
            pollfd pfds[3];

            pfds[0].fd = STDIN_FILENO;
            pfds[0].events = (POLLIN | POLLPRI);

            pfds[1].fd = ptmInOut;
            pfds[1].events = (POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL);

            pfds[2].fd = ptErr[0];
            pfds[2].events = (POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL);


            size_t const bufferSize = 160;
            char buffer[bufferSize];

            while (0 < poll(pfds, 3, -1)) {

              // Forward parent's stdin -> child's stdin (pty) and log files.
              if (pfds[0].revents & (POLLIN | POLLPRI)) {
                ssize_t readCount = read(pfds[0].fd, buffer, bufferSize);
                if (0 < readCount) {
                  ssize_t writeCount =
                    write(ptmInOut, buffer, static_cast<size_t>(readCount));
                  if (writeCount != readCount) {
                    LOG_WARN << "STDIN read and write counts do not match,"
                             << " r:" << readCount << " vs w:" << writeCount
                             << std::endl;
                  }

                  logChildStdIn.write(buffer, readCount);
                  logChildStdIn.flush();
                }
              }

              // Forward child's stdout (pty) -> parent's stdout and log files.
              if (pfds[1].revents & (POLLIN | POLLPRI)) {
                ssize_t readCount = read(pfds[1].fd, buffer, bufferSize);
                if (0 < readCount) {
                  ssize_t writeCount =
                    write(STDOUT_FILENO, buffer, static_cast<size_t>(readCount));
                  if (writeCount != readCount) {
                    LOG_WARN << "STDOUT read and write counts do not match,"
                             << " r:" << readCount << " vs w:" << writeCount
                             << std::endl;
                  }

                  logChildStdOut.write(buffer, readCount);
                  logChildStdOut.flush();
                }
              }

              // Forward child's stderr (pipe) -> parent's stderr and log files.
              if (pfds[2].revents & (POLLIN | POLLPRI)) {
                ssize_t readCount = read(pfds[2].fd, buffer, bufferSize);
                if (0 < readCount) {
                  ssize_t writeCount =
                    write(STDERR_FILENO, buffer, static_cast<size_t>(readCount));
                  if (writeCount != readCount) {
                    LOG_WARN << "STDERR read and write counts do not match,"
                             << " r:" << readCount << " vs w:" << writeCount
                             << std::endl;
                  }

                  logChildStdErr.write(buffer, readCount);
                  logChildStdErr.flush();
                }
              }

              // The child closed the pty/pipe, hung-up, or had I/O errors.
              if (pfds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                break;
              }
              if (pfds[2].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                break;
              }
            }

            logChildStdIn.flush();
            logChildStdOut.flush();
            logChildStdErr.flush();

            logChildStdIn.close();
            logChildStdOut.close();
            logChildStdErr.close();

            // Restore original terminal settings.
            tcsetattr(STDIN_FILENO, TCSANOW, &ptySettingsOriginal);

            // Record meta-data and cleanup after child.
            int childResult = -1;
            waitpid(pid, &childResult, 0);

            writeToFile<nmco::Time>
              (toolRunResults/"timestamp_end.txt", nmco::Time());

            // Make toolRunResults (and everything in it) read-only.
            if (true) {
              nmfm.removeWrite(toolRunResults, true);
            }

            LOG_INFO << "clw results stored in: " << toolRunResults
                     << std::endl;

            // Import toolRunResults into the default database.
            if (isCommandAvailable("nmdb-import-clw")) {
              std::vector<std::string> importCmd {
                "nmdb-import-clw",
                toolRunResults.string()
              };

              nmcu::forkExecWait(importCmd);
            }

            // Return the child's return value so that "clw <command>"
            // can be used wherever "<command>" would be used.
            return childResult;
          }
      }  // end of switch(pid).
    }

  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
