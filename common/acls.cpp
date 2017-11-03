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

//#define BOOST_SPIRIT_DEBUG
//bool skip_print(false);

#include <netmeld/common/acls.hpp>

using std::cout;
using std::endl;


//
// ACL_Policy
//
ACL_Policy::
ACL_Policy()
{
}

ACL_Policy::
ACL_Policy(int id, const std::string& name):
  acl_number(id),
  acl_set(name)
{
}


void 
ACL_Policy::
add_src_net(const std::string& net)
{
  src_ip_nets.insert(net);
}

void 
ACL_Policy::
add_dst_net(const std::string& net)
{
  dst_ip_nets.insert(net);
}

void 
ACL_Policy::
add_service(const std::string& service)
{
  services.insert(service);
}

void 
ACL_Policy::
add_action(const std::string& action)
{
  if (!action.empty()) {
    actions.insert(action);

    if (action_set.empty()) {
      action_set = action;
    }
    else {
      action_set += " " + action;
    }
  }
}

void 
ACL_Policy::
set_disabled(const bool val)
{
  disabled = val;
}

bool 
ACL_Policy::
is_disabled()
{
  return disabled;
}




//
// ACL_Policies
//

ACL_Policies::
ACL_Policies()
{
}


void
ACL_Policies::
add_zone(const std::string& zone_name, 
         const std::string& zone_desc)
{
  Zone z{
    zone_name,
    zone_desc,
    std::map<std::string, Address>(),
    std::map<std::string, Address_Set>(),
    std::map<std::string, Service>(),
    std::map<std::string, Service_Set>()
  };

  zone_map.emplace(zone_name, z);
}

void 
ACL_Policies::
add_address(const IP_Addr_with_Prefix ip_addr,
            const std::string& ip_net_name,
            const std::string& zone_name,
            const std::string& addr_desc)
{
  // Find zone, if not found make it
  auto iter = zone_map.find(zone_name);
  if (iter == zone_map.end()) {
    add_zone(zone_name);
    iter = zone_map.find(zone_name);
  }
  
  // Add address information to zone
  auto& addr_map = (std::get<1>(*iter)).addr_map;
  Address addr{ip_addr, ip_net_name, addr_desc};
  addr_map.emplace(ip_net_name, addr);
}

void 
ACL_Policies::
add_address_set(const std::string& address_set_name,
                const std::string& ip_net_name,
                const std::string& zone_name)
{
  // Find zone, if not found make it
  auto iter = zone_map.find(zone_name);
  if (iter == zone_map.end()) {
    add_zone(zone_name);
    iter = zone_map.find(zone_name);
  }

  // Find address set, if not found make it
  auto& addr_set_map = (std::get<1>(*iter)).addr_set_map;
  auto aiter = addr_set_map.find(address_set_name);
  if (aiter == addr_set_map.end()) {
    Address_Set addrs_set{address_set_name, std::set<std::string>()};
    addr_set_map.emplace(address_set_name, addrs_set);
    aiter = addr_set_map.find(address_set_name);
  }

  // Add address information to address set
  auto& addrs_set = (std::get<1>(*aiter)).addrs_set;
  addrs_set.emplace(ip_net_name);
}

// TODO this needs to handle multiple same entries, e.g. DNS tcp and upd 53
void 
ACL_Policies::
add_service(const std::string& service_name,
            const std::string& protocol,
            const std::string& dst_ports,
            const std::string& src_ports,
            const std::string& zone_name)
{
  // Find zone, if not found make it
  auto iter = zone_map.find(zone_name);
  if (iter == zone_map.end()) {
    add_zone(zone_name);
    iter = zone_map.find(zone_name);
  }
  
  // Add service information to zone
  auto& service_map = (std::get<1>(*iter)).service_map;
  Service service{service_name, protocol, dst_ports, src_ports};
  service_map.emplace(service_name, service);
}

void 
ACL_Policies::
add_service_set(const std::string& service_set_name,
                const std::string& service_name,
                const std::string& zone_name)
{
  // Find zone, if not found make it
  auto iter = zone_map.find(zone_name);
  if (iter == zone_map.end()) {
    add_zone(zone_name);
    iter = zone_map.find(zone_name);
  }

  // Find service set, if not found make it
  auto& service_set_map = (std::get<1>(*iter)).service_set_map;
  auto aiter = service_set_map.find(service_set_name);
  if (aiter == service_set_map.end()) {
    Service_Set service_set{service_set_name, std::set<std::string>()};
    service_set_map.emplace(service_set_name, service_set);
    aiter = service_set_map.find(service_set_name);
  }

  // Add service information to services set
  auto& services_set = (std::get<1>(*aiter)).services_set;
  services_set.emplace(service_name);
}

/*
void 
ACL_Policies::
add_policy(ACL_Policy policy)
{
  // TODO {policy}
}
*/

void 
ACL_Policies::
dump_data()
{
 for (auto const& ziter : zone_map) {
   cout << "----------" << endl;
   auto zone = std::get<1>(ziter);
   cout << zone.zone_name << ":" << zone.zone_desc << endl;

   cout << "Addresses:" << endl;
   for (auto const& aiter : zone.addr_map) {
     auto addr = std::get<1>(aiter);
     cout << "\t" << addr.addr_name << ":" << addr.addr_desc << endl;
   }
   cout << "Address Sets:" << endl;
   for (auto const& asiter : zone.addr_set_map) {
     auto addr_set = std::get<1>(asiter);
     cout << "\t" << addr_set.addr_set_name << endl;
     for (auto const& as_name : addr_set.addrs_set) {
       cout << "\t\t" << as_name << endl;
     }
   }
   cout << "Services:" << endl;
   for (auto const& siter : zone.service_map) {
     auto service = std::get<1>(siter);
     cout << "\t" << service.service_name << ":" << service.protocol
          << ":" << service.src_ports << "->" << service.dst_ports << endl;
   }
   cout << "Service Sets:" << endl;
   for (auto const& ssiter : zone.service_set_map) {
     auto service_set = std::get<1>(ssiter);
     cout << "\t" << service_set.service_set_name << endl;
     for (auto const& ss_name : service_set.services_set) {
       cout << "\t\t" << ss_name << endl;
     }
   }
 }
}

// ------------------------------------------------------------------

