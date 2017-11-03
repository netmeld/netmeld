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

#include <algorithm>
#include <functional>
#include <map>

using std::find;
using std::function;
using std::map;
using std::string;
using std::vector;

namespace filesystem = boost::filesystem;


// Ensure dumpcap writes output file (-w) to the tool run directory.
void
augment_args_dumpcap
(vector<string>& args, filesystem::path const& tool_run_results);

void
augment_args_dumpcap
(vector<string>& args, filesystem::path const& tool_run_results)
{
  // Unconditionally added to end so it overrides any user-specified.
  args.push_back("-w");
  args.push_back((tool_run_results/"results.pcapng").string());
}


// Ensure nmap outputs all formats (-oA) to the tool run directory.
// Ensure nmap always uses "--reason" and "--stats-every".
void
augment_args_nmap
(vector<string>& args, filesystem::path const& tool_run_results);

void
augment_args_nmap
(vector<string>& args, filesystem::path const& tool_run_results)
{
  // Save a copy of any targets file specified with "-iL".
  if (true) {
    auto arg_i = find(args.begin(), args.end(), "-iL");
    if (args.end() != arg_i) {
      ++arg_i;  // Increment from "-iL" to the filename.
      if ((args.end() != arg_i) && (filesystem::exists(*arg_i))) {
        filesystem::path targets{*arg_i};
        filesystem::create_directories(tool_run_results/"iL");
        filesystem::copy_file
          (targets, tool_run_results/"iL"/(targets.filename()));
      }
    }
  }

  // Option "--reason" only needs to be present once.
  if (args.end() == find(args.begin(), args.end(), "--reason")) {
    args.push_back("--reason");
  }

  // Keep user-specified "--stats-every", or add a default of "60s".
  if (args.end() == find(args.begin(), args.end(), "--stats-every")) {
    args.push_back("--stats-every");
    args.push_back("60s");
  }

  // Keep user-specified "--min-hostgroup", or add a default of "256".
  if (args.end() == find(args.begin(), args.end(), "--min-hostgroup")) {
    args.push_back("--min-hostgroup");
    args.push_back("256");
  }

  // Keep user-specified "--min-rate", or add a default of "500".
  //
  // We want to be able to run ~20 parallel scans.
  // (5 VLANs * (IPv4:TCP, IPv4:UDP, IPv6:TCP, IPv6:UDP))
  //
  // During Nmap port scans:
  //   IPv4 Ethernet frames average 58 bytes.
  //   IPv6 Ethernet frames average 78 bytes.
  // We'll go with the higher 78 bytes/frame.
  //
  // T1: 1.544 Mb/s (193 KB/s).
  // So (193 KB/s)/(78 B/frame) = 2474 frames/s.
  // And (2474 frames/s)/(20 parallel scans) = 123 frames/s/scan.
  //
  // LAN: 100 Mb/s (12.5 MB/s).
  // So (12.5 MB/s)/(78 B/frame) = 160256 frames/s.
  // And (160256 frames/s)/(20 parallel scans) = 8012 frames/s/scan.

  if (args.end() == find(args.begin(), args.end(), "--min-rate")) {
    args.push_back("--min-rate");
    args.push_back("500");
  }

  // Output all file formats.
  // Unconditionally added to end so it overrides any user-specified.
  args.push_back("-oA");
  args.push_back((tool_run_results/"results").string());
}


// Ensure ping and ping6 always use "-n" to standardize parsing
// and so we always have the IP addresses involved.
void
augment_args_ping
(vector<string>& args, filesystem::path const&);

void
augment_args_ping
(vector<string>& args, filesystem::path const&)
{
  // Option "-n" only needs to be present once.
  if (args.end() == find(args.begin(), args.end(), "-n")) {
    args.push_back("-n");
  }
}


void
augment_args(vector<string>& args, filesystem::path const& tool_run_results)
{
  map<string, function<void(vector<string>&, filesystem::path const&)> >
    tool_handlers;

  tool_handlers["dumpcap"]  = augment_args_dumpcap;
  tool_handlers["nmap"]     = augment_args_nmap;
  tool_handlers["ping"]     = augment_args_ping;
  tool_handlers["ping6"]    = augment_args_ping;

  if (tool_handlers.count(args.at(0))) {
    tool_handlers.at(args.at(0))(args, tool_run_results);
  }
}
