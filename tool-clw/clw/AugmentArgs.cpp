// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <algorithm>
#include <functional>
#include <map>

#include "AugmentArgs.hpp"

namespace sfs = std::filesystem;


namespace netmeld::tools::clw {

  // Ensure dumpcap writes output file (-w) to the tool run directory.
  void
  augmentArgsDumpcap
  (std::vector<std::string>& args, const sfs::path& toolRunResults);

  void
  augmentArgsDumpcap
  (std::vector<std::string>& args, const sfs::path& toolRunResults)
  {
    // Unconditionally added to end so it overrides any user-specified.
    args.emplace_back("-w");
    args.emplace_back((toolRunResults/"results.pcapng").string());
  }


  // Ensure nmap outputs all formats (-oA) to the tool run directory.
  // Ensure nmap always uses "--reason" and "--stats-every".
  void
  augmentArgsNmap
  (std::vector<std::string>& args, const sfs::path& toolRunResults);

  void
  augmentArgsNmap
  (std::vector<std::string>& args, const sfs::path& toolRunResults)
  {
    // Save a copy of any targets file specified with "-iL".
    auto argI = std::find(args.begin(), args.end(), "-iL");
    if (args.end() != argI) {
      ++argI;  // Increment from "-iL" to the filename.
      if ((args.end() != argI) && (sfs::exists(*argI))) {
        sfs::path targets{*argI};
        sfs::create_directories(toolRunResults/"iL");
        sfs::copy_file
          (targets, toolRunResults/"iL"/(targets.filename()));
      }
    }

    // Option "--reason" only needs to be present once.
    if (args.end() == std::find(args.begin(), args.end(), "--reason")) {
      args.emplace_back("--reason");
    }

    // Keep user-specified "--stats-every", or add a default of "60s".
    if (args.end() == std::find(args.begin(), args.end(), "--stats-every")) {
      args.emplace_back("--stats-every");
      args.emplace_back("60s");
    }

    // Keep user-specified "--min-hostgroup", or add a default of "256".
    if (args.end() == std::find(args.begin(), args.end(), "--min-hostgroup")) {
      args.emplace_back("--min-hostgroup");
      args.emplace_back("256");
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

    if (args.end() == std::find(args.begin(), args.end(), "--min-rate")) {
      args.emplace_back("--min-rate");
      args.emplace_back("500");
    }

    // Output all file formats.
    // Unconditionally added to end so it overrides any user-specified.
    args.emplace_back("-oA");
    args.emplace_back((toolRunResults/"results").string());
  }


  // Ensure ping and ping6 always use "-n" to standardize parsing
  // and so we always have the IP addresses involved.
  void
  augmentArgsPing
  (std::vector<std::string>& args, const sfs::path&);

  void
  augmentArgsPing
  (std::vector<std::string>& args, const sfs::path&)
  {
    // Option "-n" only needs to be present once.
    if (args.end() == std::find(args.begin(), args.end(), "-n")) {
      args.emplace_back("-n");
    }
  }


  // Actual entry point
  void
  augmentArgs(std::vector<std::string>& args, const sfs::path& toolRunResults)
  {
    std::map<std::string,
             std::function<
               void(std::vector<std::string>&, const sfs::path&)
            >>
      toolHandlers;

    toolHandlers["dumpcap"]  = augmentArgsDumpcap;
    toolHandlers["nmap"]     = augmentArgsNmap;
    toolHandlers["ping"]     = augmentArgsPing;
    toolHandlers["ping6"]    = augmentArgsPing;

    if (toolHandlers.count(args.at(0))) {
      toolHandlers.at(args.at(0))(args, toolRunResults);
    }
  }
}
