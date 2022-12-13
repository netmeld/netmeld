// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include "Parser.hpp"

// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data)
{
  try {
    for (const auto& routeTable : _data.at("RouteTables")) {
      processRouteTable(routeTable);
    }
  } catch (json::out_of_range& ex) {
    LOG_ERROR << "Parse error " << ex.what() << std::endl;
  }
}

void
Parser::processRouteTable(const json& _routeTable)
{
  nmdoa::RouteTable art;
  art.setId(_routeTable.value("RouteTableId", ""));
  art.setVpcId(_routeTable.value("VpcId", ""));

  processAssociations(_routeTable, art);
  processRoutes(_routeTable, art);

  d.routeTables.emplace_back(art);
}

void
Parser::processAssociations(const json& _rt,
                            nmdoa::RouteTable& _routeTable)
{
  std::set<std::string> keys {
        "GatewayId"
      , "SubnetId"
    };

  for (const auto& key : keys) {
    for (const auto& association : _rt.at("Associations")) {
      if ("associated" != association.at("AssociationState").at("State")) {
        LOG_DEBUG << "RouteTable not associated: "
                  << _routeTable.toDebugString()
                  ;
        continue;
      }
      if (association.contains(key)) {
        _routeTable.addAssociation(association.at(key));
      }
    }
  }
}

void
Parser::processRoutes(const json& _rt, nmdoa::RouteTable& _routeTable)
{
  for (const auto& route : _rt.at("Routes")) {
    const std::string rId {getRouteId(route)};

    nmdoa::Route ar;
    ar.setId(rId);
    ar.setState(route.value("State", ""));

    bool routeDestinationFound {false};

    {
      std::set keys {
            "DestinationCidrBlock"
          , "DestinationIpv6CidrBlock"
        };
      for (const auto& key : keys) {
        if (route.contains(key)) {
          ar.addCidrBlock(route.value(key, ""));
          routeDestinationFound = true;
        }
      }
    }
    {
      std::set keys {
            "DestinationPrefixListId"
        };
      for (const auto& key : keys) {
        if (route.contains(key)) {
          ar.addNonCidrBlock(route.value(key, ""));
          routeDestinationFound = true;
        }
      }
    }

    if (!routeDestinationFound) {
      std::ostringstream oss;
      oss << "Route (destination ID: "
          << rId
          << ") has no known destination type"
          ;
      d.observations.addUnsupportedFeature(oss.str());
    }

    _routeTable.addRoute(ar);
  }
}

std::string
Parser::getRouteId(const json& _route)
{
  std::set<std::string> keys {
        "CarrierGatewayId"
      , "CoreNetworkArn"
      , "EgressOnlyInternetGatewayId"
      , "GatewayId"
      , "LocalGatewayId"
      , "NatGatewayId"
      , "NetworkInterfaceId"
      , "TransitGatewayId"
      , "VpcPeeringConnectionId"
      // NOTE The following two instance Ids seem to always appear together
      // and when NetworkInterfaceId is present, prefer NetworkInterfaceId
      //, "InstanceId"
      //, "InstanceOwnerId"
    };

  std::string found;

  for (const auto& key : keys) {
    if (_route.contains(key)) {
      if (!found.empty()) {
        std::ostringstream oss;
        oss << "Route contains multiple IDs: "
            << found << " (Netmeld used) and "
            << _route.at(key) << " (Netmeld unused)"
            ;
        d.observations.addUnsupportedFeature(oss.str());
      } else {
        found = _route.at(key);
      }
    }
  }

  return found;
}


// =============================================================================
// Parser helper methods
// =============================================================================
Result
Parser::getData()
{
  Result r;
  r.emplace_back(d);
  return r;
}
