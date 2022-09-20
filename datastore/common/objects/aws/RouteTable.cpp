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

#include <netmeld/datastore/objects/aws/RouteTable.hpp>


namespace netmeld::datastore::objects::aws {

  RouteTable::RouteTable()
  {}

  void
  RouteTable::setId(const std::string& _id)
  {
    routeTableId = _id;
  }
  void
  RouteTable::setVpcId(const std::string& _id)
  {
    vpcId = _id;
  }
  void
  RouteTable::addAssociation(const std::string& _association)
  {
    if (_association.empty()) { return; }
    associations.insert(_association);
  }
  void
  RouteTable::addRoute(const Route& _route)
  {
    Route t;
    if (t == _route) { return; }
    routes.insert(_route);
  }

  bool
  RouteTable::isValid() const
  {
    return !(routeTableId.empty())
        ;
  }

  void
  RouteTable::save(pqxx::transaction_base& t,
                    const nmco::Uuid& toolRunId, const std::string&)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS RouteTable object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    t.exec_prepared("insert_raw_aws_route_table"
        , toolRunId
        , routeTableId
      );

    for (const auto& association : associations) {
      t.exec_prepared("insert_raw_aws_route_table_association"
          , toolRunId
          , routeTableId
          , association
        );
    }

    for (auto route : routes) {
      route.save(t, toolRunId, routeTableId);
    }

    if (!vpcId.empty()) {
      t.exec_prepared("insert_raw_aws_vpc"
          , toolRunId
          , vpcId
        );

      t.exec_prepared("insert_raw_aws_vpc_route_table"
          , toolRunId
          , vpcId
          , routeTableId
        );
    }
  }

  std::string
  RouteTable::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "routeTableId: " << routeTableId
        << ", vpcId: " << vpcId
        << ", associations: " << associations
        << ", routes: " << routes
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  RouteTable::operator<=>(const RouteTable& rhs) const
  {
    if (auto cmp = routeTableId <=> rhs.routeTableId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = vpcId <=> rhs.vpcId; 0 != cmp) {
      return cmp;
    }
    if (auto cmp = associations <=> rhs.associations; 0 != cmp) {
      return cmp;
    }

    return routes <=> rhs.routes;
  }

  bool
  RouteTable::operator==(const RouteTable& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
