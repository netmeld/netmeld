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

#include <netmeld/playbook/utils/QueriesPlaybook.hpp>

namespace netmeld::playbook::utils {

  void
  dbPreparePlaybook(pqxx::connection& db)
  {
    // =========================================================================
    // Common
    // =========================================================================

    // =========================================================================
    // Insert Router
    // =========================================================================
    db.prepare(
        "insert_playbook_ip_router",
        "INSERT INTO playbook_ip_routers"
        " (rtr_ip_addr)"
        " SELECT host(($1)::INET)::INET"
        " WHERE NOT EXISTS ("
        "   SELECT 1 FROM playbook_ip_routers"
        "   WHERE (rtr_ip_addr = host(($1)::INET)::INET)"
        " )"
        );


    // =========================================================================
    // Insert Source
    // =========================================================================
    db.prepare(
        "insert_playbook_inter_network_source",
        "INSERT INTO playbook_inter_network_sources"
        " (playbook_source_id, is_completed, playbook_stage,"
        "  interface_name, vlan, mac_addr,"
        "  ip_addr, ptp_rtr_ip_addr, description)"
        " SELECT $1, $2, $3,"
        "        $4, $5, NULLIF($6, '')::MACADDR,"
        "        ($7)::INET, NULLIF($8, '')::INET, NULLIF($9, '')"
        " WHERE NOT EXISTS ("
        "   SELECT 1 FROM playbook_inter_network_sources"
        "   WHERE (playbook_source_id = $1)"
        " )"
        );

    db.prepare(
        "insert_playbook_intra_network_source",
        "INSERT INTO playbook_intra_network_sources"
        " (playbook_source_id, is_completed, playbook_stage,"
        "  interface_name, vlan, mac_addr,"
        "  ip_addr, description)"
        " SELECT $1, $2, $3,"
        "        $4, $5, NULLIF($6, '')::MACADDR,"
        "        ($7)::INET, NULLIF($8, '')"
        " WHERE NOT EXISTS ("
        "   SELECT 1 FROM playbook_intra_network_sources"
        "   WHERE (playbook_source_id = $1)"
        " )"
        );


    // =========================================================================
    // Playbook
    // =========================================================================
    db.prepare(
        "select_playbook_intra_network",
        "SELECT"
        "   playbook_source_id,"
        "   playbook_stage,"
        "   interface_name,"
        "   vlan,"
        "   mac_addr,"
        "   text(ip_addr) AS ip_addr,"
        "   NULL AS rtr_ip_addr,"
        "   description,"
        "   family(ip_addr) AS addr_family"
        " FROM playbook_intra_network_dashboard"
        " WHERE (NOT is_completed)"
        );
    db.prepare(
        "select_playbook_inter_network",
        "SELECT"
        "   playbook_source_id,"
        "   playbook_stage,"
        "   interface_name,"
        "   vlan,"
        "   mac_addr,"
        "   text(ip_addr) AS ip_addr,"
        "   rtr_ip_addr,"
        "   description,"
        "   family(ip_addr) AS addr_family"
        " FROM playbook_inter_network_dashboard"
        " WHERE (NOT is_completed)"
        );

    db.prepare(
        "select_network_and_broadcast",
        "SELECT"
        "   network(($1)::INET)                AS ip_net,"
        "   host(broadcast(($1)::INET))::INET  AS ip_net_bcast"
        );

    db.prepare(
        "playbook_intra_network_set_completed",
        "UPDATE playbook_intra_network_sources"
        " SET is_completed = true"
        " WHERE (playbook_source_id = $1)"
        );

    db.prepare(
        "playbook_inter_network_set_completed",
        "UPDATE playbook_inter_network_sources"
        " SET is_completed = true"
        " WHERE (playbook_source_id = $1)"
        );
  }
}
