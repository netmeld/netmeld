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

#define BOOST_NO_CXX11_SCOPED_ENUMS
#define BOOST_NO_SCOPED_ENUMS

#include <augment_args.hpp>

#include <netmeld/common/fork_exec.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

extern "C" {
#include <fcntl.h>
#include <pty.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
}


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::exception;
using std::ofstream;
using std::string;
using std::vector;

using boost::format;
using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;


void
create_timestamp_file(filesystem::path const& p,
                      posix_time::ptime const& t);

void
create_uuid_file(filesystem::path const& p,
                 uuid const& u);

void
create_command_line_file(filesystem::path const& p,
                         vector<string> const& args);

void
create_network_files(filesystem::path const& d);

void
create_config_files(filesystem::path const& d);


void
create_timestamp_file(filesystem::path const& p,
                      posix_time::ptime const& t)
{
  ofstream f{p.string()};
  f << to_iso_extended_string(t) << endl;
  f.close();
}


void
create_uuid_file(filesystem::path const& p,
                 uuid const& u)
{
  ofstream f{p.string()};
  f << u << endl;
  f.close();
}


void
create_command_line_file(filesystem::path const& p,
                         vector<string> const& args)
{
  ofstream f{p.string()};

  f << args.at(0);
  for (size_t i = 1; i < args.size(); ++i) {
    f << ' ' << args.at(i);
  }
  f << endl;

  f.close();
}


void
create_network_files(filesystem::path const& d)
{
  // Network interfaces (interface names, MAC addrs, and IP addrs, etc)
  if (-1 == system((format("ip addr show >> '%s'")
                    % (d/"ip_addr_show.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: ip addr show");
  }
  if (-1 == system((format("ifconfig -a >> '%s'")
                    % (d/"ifconfig.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: ifconfig -a");
  }

  // Network routes
  if (-1 == system((format("ip -4 route show >> '%s'")
                    % (d/"ip4_route_show.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: ip -4 route show");
  }
  if (-1 == system((format("ip -6 route show >> '%s'")
                    % (d/"ip6_route_show.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: ip -6 route show");
  }
  if (-1 == system((format("netstat -nr >> '%s'")
                    % (d/"netstat-nr.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: netstat -nr");
  }

  // Firewall rules
  if (-1 == system((format("iptables-save --counters >> '%s'")
                    % (d/"ip4_tables_save.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: iptables-save --counters");
  }
  if (-1 == system((format("ip6tables-save --counters >> '%s'")
                    % (d/"ip6_tables_save.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: ip6tables-save --counters");
  }
}


void
create_config_files(filesystem::path const& d)
{
  // Environment variables
  if (-1 == system((format("env >> '%s'")
                    % (d/"env.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: env");
  }

  // Kernel parameters
  if (-1 == system((format("sysctl -a >> '%s'")
                    % (d/"sysctl.txt").string())
                   .str().c_str())) {
    throw std::runtime_error("Error executing: sysctl -a");
  }

  // Important configuration files in /etc/
  filesystem::create_directories(d/"etc");
  vector<string> etc_files = {
        "/etc/hosts", 
        "/etc/resolv.conf"
    };

  for (auto const& file : etc_files) {
    if (filesystem::exists(file)) {
      filesystem::copy_file(file, d/file);
    }
    else {
      cerr << "File not found (on copy): " << file << endl;
    }
  }
}


int
main(int argc, char* argv[])
{
  try {
    // Simple args and usage (either there or not)
    string arg = (argc < 2) ? "" : argv[1];
    if (argc < 2 || arg == "-h" || arg == "--help") {
      cerr << "Capture context about a command's execution.\n"
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {command}" << endl
           << endl
           << "Options:\n"
           << "  -h [ --help ]          Show the help message, then exit.\n"
           << "  -v [ --version ]       Show version information, then exit.\n"
           << endl;
      return (argc < 2) ? 1 : 0;
    }
    if (arg == "-v" || arg == "--version") {
      cerr << "clw (Netmeld)" << endl;
      return 0;
    }

    // Copy the wrapped command and arguments into a vector that we can modify.
    vector<string> args;
    for (size_t i = 1; nullptr != argv[i]; ++i) {
      args.push_back(argv[i]);
    }

    // Create (if it doesn't exist) the ~/.clw directory.
    filesystem::path const clw_dir{filesystem::path{getenv("HOME")}/".clw"};
    filesystem::create_directory(clw_dir);

    // Record timestamp and UUID info for this tool run.
    posix_time::ptime const timestamp_start =
      posix_time::microsec_clock::universal_time();

    uuid const tool_run_id = uuid_generator();

    string const tool_name = string(args.at(0));

    string const tool_run_name =
      tool_name + '_' +
      to_iso_string(timestamp_start) + '_' +
      to_string(tool_run_id);

    // Create directory for meta-data about and results from this tool run.
    filesystem::path const tool_run_results{clw_dir/tool_run_name};
    filesystem::create_directory(tool_run_results);

    // Record meta-data about this tool run.
    create_uuid_file
      (tool_run_results/"tool_run_id.txt", tool_run_id);

    create_timestamp_file
      (tool_run_results/"timestamp_start.txt", timestamp_start);

    create_command_line_file
      (tool_run_results/"command_line_original.txt", args);

    augment_args(args, tool_run_results);

    create_command_line_file
      (tool_run_results/"command_line_modified.txt", args);

    create_config_files(tool_run_results);

    create_network_files(tool_run_results);


    // Open pseudo-tty (pty) master (for child's stdin and stdout).
    int ptm_inout = posix_openpt(O_RDWR | O_NOCTTY);
    if ((-1 == ptm_inout) ||
        (-1 == grantpt(ptm_inout)) ||
        (-1 == unlockpt(ptm_inout))) {
      perror("Error on pseudo-tty master");
      throw exception();
    }

    // Open a pipe (for child's stderr).
    int pt_err[2];
    if (-1 == pipe(pt_err)) {
      perror("Error on pipe()");
      throw exception();
    }

    // Fork child process and manage I/O with the child process.
    pid_t pid = fork();

    switch (pid) {

    case -1: {  // Error.
      perror("Error on fork()");
      throw exception();
    }

    case 0: {   // In child.
      string pts_device(ptsname(ptm_inout));
      close(ptm_inout);  // Child has no further use for the pty master.
      close(pt_err[0]);  // Child won't read from stderr pipe.

      // Open pseudo-tty (pty) slave.
      int pts_inout = open(pts_device.c_str(), O_RDWR | O_NOCTTY);
      if (-1 == pts_inout) {
        perror("Error on pseudo-tty slave");
        throw exception();
      }

      // Replace child's stdin and stdout with the pty slave.
      // Replace child's stderr with the stderr pipe.
      dup2(pts_inout, STDIN_FILENO);
      dup2(pts_inout, STDOUT_FILENO);
      dup2(pt_err[1], STDERR_FILENO);

      // Don't need pty slave or stderr pipe after they've been dup'd.
      close(pts_inout);
      close(pt_err[1]);

      setsid();
      ioctl(STDIN_FILENO, TIOCSCTTY, 1);

      // Extract modified args into the format needed by execvp.
      vector<char*> argv_modified;
      for (string& arg : args) {
        argv_modified.push_back(strdup(arg.c_str()));
      }
      argv_modified.push_back(nullptr);

      // Execute the wrapped command using the modified args.
      execvp(argv_modified[0], &argv_modified[0]);
      perror("Error on execvp()");
      throw exception();
    }

    default: {  // In parent.
      ofstream log_child_stdin
      {(tool_run_results/"stdin.txt").string()};

      ofstream log_child_stdout
      {(tool_run_results/"stdout.txt").string()};

      ofstream log_child_stderr
      {(tool_run_results/"stderr.txt").string()};

      // Store copy of terminal setting to be restored later.
      termios pty_settings_original;
      tcgetattr(STDIN_FILENO, &pty_settings_original);

      // Immediately pass characters to/from the child,
      // ignore signaling (pass through to child),
      // and don't echo input (child controls echo settings).
      termios pty_settings;
      tcgetattr(STDIN_FILENO, &pty_settings);
      // The following pragmas are to silence an accepted warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
      pty_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
#pragma GCC diagnostic pop
      tcsetattr(STDIN_FILENO, TCSANOW, &pty_settings);

      // Handle forwarding I/O between parent and child.
      pollfd pfds[3];

      pfds[0].fd = STDIN_FILENO;
      pfds[0].events = (POLLIN | POLLPRI);

      pfds[1].fd = ptm_inout;
      pfds[1].events = (POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL);

      pfds[2].fd = pt_err[0];
      pfds[2].events = (POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL);


      size_t const buffer_size = 160;
      char buffer[buffer_size];

      while (0 < poll(pfds, 3, -1)) {

        // Forward parent's stdin -> child's stdin (pty) and log files.
        if (pfds[0].revents & (POLLIN | POLLPRI)) {
          ssize_t read_count = read(pfds[0].fd, buffer, buffer_size);
          if (0 < read_count) {
            ssize_t write_count =
              write(ptm_inout, buffer, numeric_cast<size_t>(read_count));
            if (write_count != read_count) {
              // TODO: What's the best way to handle this?
            }

            log_child_stdin.write(buffer, read_count);
            log_child_stdin.flush();
          }
        }

        // Forward child's stdout (pty) -> parent's stdout and log files.
        if (pfds[1].revents & (POLLIN | POLLPRI)) {
          ssize_t read_count = read(pfds[1].fd, buffer, buffer_size);
          if (0 < read_count) {
            ssize_t write_count =
              write(STDOUT_FILENO, buffer, numeric_cast<size_t>(read_count));
            if (write_count != read_count) {
              // TODO: What's the best way to handle this?
            }

            log_child_stdout.write(buffer, read_count);
            log_child_stdout.flush();
          }
        }

        // Forward child's stderr (pipe) -> parent's stderr and log files.
        if (pfds[2].revents & (POLLIN | POLLPRI)) {
          ssize_t read_count = read(pfds[2].fd, buffer, buffer_size);
          if (0 < read_count) {
            ssize_t write_count =
              write(STDERR_FILENO, buffer, numeric_cast<size_t>(read_count));
            if (write_count != read_count) {
              // TODO: What's the best way to handle this?
            }

            log_child_stderr.write(buffer, read_count);
            log_child_stderr.flush();
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

      log_child_stdin.flush();
      log_child_stdout.flush();
      log_child_stderr.flush();

      log_child_stdin.close();
      log_child_stdout.close();
      log_child_stderr.close();

      // Restore original terminal settings.
      tcsetattr(STDIN_FILENO, TCSANOW, &pty_settings_original);

      // Record meta-data and cleanup after child.
      int child_result = -1;
      waitpid(pid, &child_result, 0);

      posix_time::ptime const timestamp_end =
        posix_time::microsec_clock::universal_time();

      create_timestamp_file
        (tool_run_results/"timestamp_end.txt", timestamp_end);

      // Make tool_run_results (and everything in it) read-only.
      if (true) {
        using filesystem::remove_perms;
        using filesystem::owner_write;
        using filesystem::group_write;
        using filesystem::others_write;

        filesystem::perms const read_only =
          (remove_perms | owner_write | group_write | others_write);

        for (filesystem::recursive_directory_iterator
               path_i(tool_run_results), path_e; path_i != path_e; ++path_i) {
          filesystem::permissions(*path_i, read_only);
        }

        filesystem::permissions(tool_run_results, read_only);
      }

      cout << "clw results stored in: " << tool_run_results.string() << endl;

      // Import tool_run_results into the default database.
      if (true) {
        vector<string> args = {
          "nmdb-import-clw",
          tool_run_results.string()
        };

        fork_exec_wait(args);
      }

      // Return the child's return value so that "clw <command>"
      // can be used wherever "<command>" would be used.
      return child_result;
    }

    }  // end of switch(pid).
  }
  catch (exception const& e) {
    cerr << e.what() << endl;
  }
  catch (...) {
    cerr << "Non-exception was thrown." << endl;
  }

  return 0;
}
