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

#ifndef ACLS_HPP
#define ACLS_HPP

//bool skip_print(false);

#include <netmeld/common/networking.hpp>

#include <boost/operators.hpp>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>


struct Address
{
  IP_Addr_with_Prefix ip_addr;
  std::string addr_name;
  std::string addr_desc;
};
struct Address_Set
{
  std::string addr_set_name;
  std::set<std::string> addrs_set;
};
struct Service
{
  std::string service_name;
  std::string protocol;
  std::string dst_ports;
  std::string src_ports;
};
struct Service_Set
{
  std::string service_set_name;
  std::set<std::string> services_set;
};
struct Zone
{
  std::string zone_name;
  std::string zone_desc;
  std::map<std::string, Address> addr_map;
  std::map<std::string, Address_Set> addr_set_map;
  std::map<std::string, Service> service_map;
  std::map<std::string, Service_Set> service_set_map;
};


class ACL_Policy :
  boost::totally_ordered<ACL_Policy>
{
public:
  virtual ~ACL_Policy() = default;
  ACL_Policy();
  ACL_Policy(int id, const std::string& name);

  void add_src_net(const std::string& net);
  void add_dst_net(const std::string& net);
  void add_service(const std::string& service);
  void add_action(const std::string& action);
  void set_disabled(const bool val);

  bool is_disabled();
  bool operator==(ACL_Policy const& other) const;
  bool operator<(ACL_Policy const& other) const;

//private:
  int acl_number = 0;
  std::string 
    acl_set = "",
    action_set = "";
  std::set<std::string> 
    src_ip_nets,
    dst_ip_nets,
    services,
    actions;
  bool disabled = false;
};


class ACL_Policies :
  boost::totally_ordered<ACL_Policies>
{
public:
  virtual ~ACL_Policies() = default;
  ACL_Policies();

  void add_zone(const std::string& zone_name=GLOBAL, 
                const std::string& zone_desc="");
  void add_address(const IP_Addr_with_Prefix ip_addr,
                   const std::string& ip_net_name,
                   const std::string& zone_name=GLOBAL,
                   const std::string& addr_desc="");
  void add_address_set(const std::string& address_set_name,
                       const std::string& ip_net_name,
                       const std::string& zone_name=GLOBAL);
  void add_service(const std::string& service_name,
                   const std::string& protocol,
                   const std::string& dst_ports,
                   const std::string& src_ports=ALL_PORTS,
                   const std::string& zone_name=GLOBAL);
  void add_service_set(const std::string& service_set_name,
                       const std::string& service_name,
                       const std::string& zone_name=GLOBAL);
  //void add_policy(ACL_Policy policy);

  void dump_data();

private:
  static constexpr const char* GLOBAL = "global";
  static constexpr const char* ALL_PORTS = "[0-65535]";

  std::map<std::string, Zone> zone_map;
};

#endif /* ACLS_HPP */
