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

#include <format>

#include "GraphHelper.hpp"


//==============================================================================
// IsRedundantEdge
//==============================================================================
IsRedundantEdge::IsRedundantEdge(const RouteGraph& _g) : graph(_g)
{}

bool
IsRedundantEdge::operator()(const Edge& e) const
{
  const Vertex s {boost::source(e, graph)};
  const Vertex t {boost::target(e, graph)};

  bool otherEdgeExists  {std::get<1>(boost::edge(t, s, graph))};
  bool sFarther1        {graph[s].distance > graph[t].distance};
  bool sFarther2        {(s > t) && (graph[s].distance >= graph[t].distance)};

  return otherEdgeExists && (sFarther1 || sFarther2);
}


//==============================================================================
// LabelWriter
//==============================================================================
LabelWriter::LabelWriter(const RouteGraph& _graph) : graph(_graph)
{}

void
LabelWriter::operator()(std::ostream& os, const Vertex& v) const
{
  os << std::format(R"([shape="{}", style="{}", fillcolor="{}", label="{}"])"
                   , graph[v].shape
                   , graph[v].style
                   , graph[v].fillcolor
                   , graph[v].label
                   )
     ;
}

void
LabelWriter::operator()(std::ostream& os, const Edge& e) const
{
  const Vertex s {boost::source(e, graph)};
  const Vertex t {boost::target(e, graph)};

  const double distanceRatio {(graph[s].distance / graph[t].distance)};

  os << std::format(R"([constraint="{}", style="{}", dir="{}")"
                    R"(, arrowhead="{}", arrowtail="{}", label="{}"])"
                   , !((0.999 < distanceRatio) && (distanceRatio < 1.001))
                   , graph[e].style
                   , graph[e].direction
                   , graph[e].arrowhead
                   , graph[e].arrowtail
                   , graph[e].label
                   )
     ;
}


//==============================================================================
// GraphWriter
//==============================================================================
void
GraphWriter::operator()(std::ostream& os) const
{
  os << "rankdir=LR\n"
     << "//splines=ortho;\n"
     << "splines=true;\n"
     << "nodesep=1.00;\n"
     << "ranksep=2.50;\n"
     << "overlap=false;\n"
     << "beautify=true;\n"
     << "concentrate=true;\n"
     ;
}
