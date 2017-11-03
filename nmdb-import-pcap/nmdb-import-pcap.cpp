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

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <netmeld/common/networking.hpp>
#include <netmeld/common/piped_input.hpp>
#include <netmeld/common/queries_common.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>

extern "C" {
#include <pcap/pcap.h>
}


using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::copy_n;
using std::get;
using std::ifstream;
using std::map;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;

using boost::format;
using boost::numeric_cast;
using boost::uuids::uuid;

namespace filesystem = boost::filesystem;
namespace posix_time = boost::posix_time;
namespace program_options = boost::program_options;


boost::uuids::random_generator uuid_generator;


struct Ethernet_Header
{
  uint8_t  dst_addr[6];
  uint8_t  src_addr[6];
  uint16_t payload_protocol_;

  uint16_t payload_protocol() const;
};


uint16_t
Ethernet_Header::
payload_protocol() const
{
  return ntohs(payload_protocol_);
}


struct VLAN_Header
{
  uint16_t tag_control; // control 1 byte, VLAN ID 3 bytes
  uint16_t payload_protocol;
};

struct ARP_Header
{
  uint16_t hardware_type;
  uint16_t protocol_type;
  uint8_t hardware_size;
  uint8_t protocol_size;
  uint16_t opcode;
  uint8_t src_mac[6];
  uint8_t src_addr[4];
  uint8_t dst_mac[6];
  uint8_t dst_addr[4];
};


struct IPv4_Header
{
  uint8_t  version;
  uint8_t  type_of_service;
  uint16_t length;
  uint16_t id;
  uint16_t fragment_offset;
  uint8_t  hop_limit;
  uint8_t  payload_protocol;
  uint16_t checksum;
  uint8_t  src_addr[4];
  uint8_t  dst_addr[4];
};


struct IPv6_Header
{
  uint8_t  version;
  uint8_t  traffic_class;
  uint16_t flow_label;
  uint16_t payload_length;
  uint8_t  payload_protocol;
  uint8_t  hop_limit;
  uint8_t  src_addr[16];
  uint8_t  dst_addr[16];
};


int
main(int argc, char** argv)
{
  try {
    program_options::options_description general_opts_desc("Options");
    general_opts_desc.add_options()
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to connect to.")
      ("tool-run-id",
       program_options::value<string>(),
       "UUID for this run of the tool.")
      ("help,h",
       "Show this help message, then exit.")
      ("version,v",
       "Show version information, then exit.")
      ("pipe",
       "Read input from STDIN; Save a copy to {input-file}.")
      ;

    program_options::options_description hidden_opts_desc("Hidden options");
    hidden_opts_desc.add_options()
      ("input-file",
       program_options::value<string>()->required(),
       "Input data file to parse.")
      ;
    program_options::positional_options_description position_opts_desc;
    position_opts_desc.add("input-file", -1);

    // Command-line options (accepted on the command-line)
    program_options::options_description cl_opts_desc;
    cl_opts_desc.add(general_opts_desc).add(hidden_opts_desc);

    // Visible options (shown in help message)
    program_options::options_description visible_opts_desc;
    visible_opts_desc.add(general_opts_desc);

    // Parse command-line options
    program_options::variables_map opts;
    program_options::store
      (program_options::command_line_parser(argc, argv)
       .options(cl_opts_desc)
       .positional(position_opts_desc)
       .run(), opts);

    if (opts.count("help")) {
      cerr << "Import pcap file." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input-file}" << endl
           << visible_opts_desc << endl;
      return (0);
    }

    if (opts.count("version")) {
      cerr << "nmdb-import-pcap (Netmeld)" << endl;
      return 0;
    }

    // Enforce required options and other sanity checks.
    program_options::notify(opts);


    filesystem::path const ifp = opts.at("input-file").as<string>();
    if (opts.count("pipe")) {
      piped_input_file(ifp);
    }
    if (!filesystem::exists(ifp)) {
      throw std::runtime_error("Specified input-file does not exist: " +
                               ifp.string());
    }
    filesystem::path const input_file_path = filesystem::canonical(ifp);

    uuid const tool_run_id =
      opts.count("tool-run-id")
      ? (boost::uuids::string_generator()(opts.at("tool-run-id").as<string>()))
      : (uuid_generator());

    string const command_line; // Empty, as unknown source 

    string const db_name = opts.at("db-name").as<string>();
    pqxx::connection db{string("dbname=") + db_name};
    db_prepare_common(db);

    char pcap_errbuf[PCAP_ERRBUF_SIZE];
    shared_ptr<pcap_t> pcap_handle
    {pcap_open_offline(input_file_path.string().c_str(), pcap_errbuf),
     pcap_close};

    if (true) {
      posix_time::ptime const execute_time_lower =
        posix_time::microsec_clock::universal_time();

      posix_time::ptime const execute_time_upper =
        posix_time::microsec_clock::universal_time();

      pqxx::transaction<> t{db};

      if (!opts.count("tool-run-id")) {
        t.prepared("insert_tool_run")
          (tool_run_id)
          (PROGRAM_NAME)
          (command_line)
          (input_file_path.string())
          (execute_time_lower)
          (execute_time_upper)
          .exec();
      }

      map<uint16_t, size_t> vlans;      // VLAN ID,  count
      map<MAC_Addr, size_t> mac_addrs;  // MAC addr, count
      map<IP_Addr,  size_t> ip_addrs;   // IP addr,  count

      uint8_t const* packet_data = nullptr;
      pcap_pkthdr* packet_header = nullptr;
      while (1 == pcap_next_ex(pcap_handle.get(),
                               &packet_header, &packet_data)) {
        Ethernet_Header const* ethernet =
          reinterpret_cast<Ethernet_Header const*>(packet_data);

        MAC_Addr src_mac_addr(&(ethernet->src_addr[0]),
                              &(ethernet->src_addr[6]));
        ++mac_addrs[src_mac_addr];

        // Get string representation so it can be used for DB inserts 
        string const mac_addr_str =
          (format("%02x:%02x:%02x:%02x:%02x:%02x")
           % static_cast<uint16_t>(src_mac_addr[0])
           % static_cast<uint16_t>(src_mac_addr[1])
           % static_cast<uint16_t>(src_mac_addr[2])
           % static_cast<uint16_t>(src_mac_addr[3])
           % static_cast<uint16_t>(src_mac_addr[4])
           % static_cast<uint16_t>(src_mac_addr[5])).str();

        if (true) {
          bool const is_responding = true;

          t.prepared("insert_raw_mac_addr")
            (tool_run_id)
            (mac_addr_str)
            (is_responding)
            .exec();
        }

        // Create placeholders to handle data shifting
        auto payload_type = ethernet->payload_protocol();
        auto offset = packet_data + sizeof(Ethernet_Header);

Payload_Process:
        switch (payload_type) {
        case 0x8100: { // 802.1Q VLAN tag
          VLAN_Header const* vlan_h =
            reinterpret_cast<VLAN_Header const*>(offset);

          uint16_t vlan_id = ntohs(vlan_h->tag_control) & 0x0FFF;

          ++vlans[vlan_id];

          if (true) {
            t.prepared("insert_raw_vlan")
              (tool_run_id)
              (vlan_id)
              ()
              .exec();
          }

          // Update and reset payload processing to handle VLAN payload
          payload_type = ntohs(vlan_h->payload_protocol);
          offset += sizeof(VLAN_Header);
          goto Payload_Process;

          // should never get here...but just in case
          cerr << "WARNING: VLAN payload processing skipped" << endl;
          break;
        }
        case 0x0806: { // ARP
          ARP_Header const* arp =
            reinterpret_cast<ARP_Header const*>(offset);

          IPv4_Addr::bytes_type addr_bytes;
          copy_n(arp->src_addr, addr_bytes.size(), addr_bytes.begin());
          IPv4_Addr src_ip_addr = IPv4_Addr{addr_bytes};
          ++ip_addrs[src_ip_addr];
        
          if (true) {
            bool const is_responding = true;
            
            t.prepared("insert_raw_ip_addr")
              (tool_run_id)
              (src_ip_addr)
              (is_responding)
              .exec();

            t.prepared("insert_raw_mac_addr_ip_addr")
              (tool_run_id)
              (mac_addr_str)
              (src_ip_addr)
              .exec();
          }

          break;
        }
        case 0x0800: { // IPv4
          IPv4_Header const* ipv4 =
            reinterpret_cast<IPv4_Header const*>(offset);

          IPv4_Addr::bytes_type addr_bytes;
          copy_n(ipv4->src_addr, addr_bytes.size(), addr_bytes.begin());
          IPv4_Addr src_ip_addr = IPv4_Addr{addr_bytes};
          ++ip_addrs[src_ip_addr];
        
          if (true) {
            bool const is_responding = true;
            
            t.prepared("insert_raw_ip_addr")
              (tool_run_id)
              (src_ip_addr)
              (is_responding)
              .exec();

            t.prepared("insert_raw_mac_addr_ip_addr")
              (tool_run_id)
              (mac_addr_str)
              (src_ip_addr)
              .exec();
          }

          break;
        }
        case 0x86DD: { // IPv6
          IPv6_Header const* ipv6 =
            reinterpret_cast<IPv6_Header const*>(offset);

          IPv6_Addr::bytes_type addr_bytes;
          copy_n(ipv6->src_addr, addr_bytes.size(), addr_bytes.begin());
          IPv6_Addr src_ip_addr = IPv6_Addr{addr_bytes};
          ++ip_addrs[src_ip_addr];

          if (true) {
            bool const is_responding = true;
            
            t.prepared("insert_raw_ip_addr")
              (tool_run_id)
              (src_ip_addr)
              (is_responding)
              .exec();

            t.prepared("insert_raw_mac_addr_ip_addr")
              (tool_run_id)
              (mac_addr_str)
              (src_ip_addr)
              .exec();
          }

          break;
        }
        default: {
          break;
        }
        }

      }

      for (auto const& mac_addr_x : mac_addrs) {
        MAC_Addr const& mac_addr{get<0>(mac_addr_x)};
        cout << (format("%02x:%02x:%02x:%02x:%02x:%02x %d")
                 % static_cast<uint16_t>(mac_addr.at(0))
                 % static_cast<uint16_t>(mac_addr.at(1))
                 % static_cast<uint16_t>(mac_addr.at(2))
                 % static_cast<uint16_t>(mac_addr.at(3))
                 % static_cast<uint16_t>(mac_addr.at(4))
                 % static_cast<uint16_t>(mac_addr.at(5))
                 % get<1>(mac_addr_x))
             << endl;

      }

      for (auto const& ip_addr_x : ip_addrs) {
        IP_Addr const& ip_addr{get<0>(ip_addr_x)};
        cout << ip_addr << " " << get<1>(ip_addr_x) << endl;

        bool const is_responding = true;

        t.prepared("insert_raw_ip_addr")
          (tool_run_id)
          (ip_addr)
          (is_responding)
          .exec();
      }

      for (auto const& vlan_x : vlans) {
        cout << "VLAN " << get<0>(vlan_x) 
             << " "     << get<1>(vlan_x)
             << endl;
      }

      t.commit();

      if (!opts.count("tool-run-id")) {
        cerr << "tool_run_id: " << tool_run_id << endl;
      }
    }
  }
  catch (std::exception& e) {
    cerr << "Error: " << e.what() << endl;
    return (-1);
  }

  return (0);
}
