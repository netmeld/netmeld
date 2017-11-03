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

#include <netmeld/common/fork_exec.hpp>

#include <cstring>
#include <exception>
#include <iostream>

extern "C" {
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
}


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::exception;
using std::string;
using std::vector;


void
exec(string const& filename, vector<string> const& args)
{
  // Extract args into the format needed by execvp.
  vector<char*> argv;
  for (string const& arg : args) {
    argv.push_back(strdup(arg.c_str()));
  }
  argv.push_back(nullptr);

  // Execute the command using the args.
  execvp(filename.c_str(), &argv[0]);
  perror("Error on execvp()");
  throw exception();
}


void
exec(vector<string> const& args)
{
  if (args.size() < 1) {
    throw exception();
  }

  exec(args.at(0), args);
}


pid_t
fork_exec(string const& filename, vector<string> const& args)
{
  pid_t pid = fork();
  switch (pid) {
  case -1: {  // Error.
    perror("Error on fork()");
    throw exception();
  }
  case 0: {   // In child.
    exec(filename, args);
  }

  default: {  // In parent.
    // Fall through and return child's PID.
  }
  }

  return pid;
}


int
fork_exec(vector<string> const& args)
{
  if (args.size() < 1) {
    throw exception();
  }

  return fork_exec(args.at(0), args);
}


int
fork_exec_wait(string const& filename, vector<string> const& args)
{
  int child_result = -1;
  pid_t child_pid = fork_exec(filename, args);

  if (-1 == waitpid(child_pid, &child_result, 0)) {
    throw exception();
  }

  return child_result;
}


int
fork_exec_wait(vector<string> const& args)
{
  if (args.size() < 1) {
    throw exception();
  }

  return fork_exec_wait(args.at(0), args);
}


string
args_to_string(vector<string> const& args)
{
  string command;

  for (string arg : args) {
    // Separate arguments by spaces.
    if (!command.empty()) {
      command += ' ';
    }

    // Argument containing spaces need to be quoted.
    if (string::npos != arg.find(' ')) {
      arg = "'" + arg + "'";
    }

    command += arg;
  }

  return command;
}
