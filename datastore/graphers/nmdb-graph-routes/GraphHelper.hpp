// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef GRAPH_HELPER_HPP
#define GRAPH_HELPER_HPP

#include <boost/graph/adjacency_list.hpp>

#include <string>

//==============================================================================
// Graphviz helper objects
//==============================================================================
struct VertexProperties
{
  std::string name;
  std::string label;
  std::string shape;
  std::string style;
  std::string fillcolor;

  double distance = std::numeric_limits<double>::infinity();
  double extra_weight = 0.0;
};

struct EdgeProperties
{
  std::string label;

  double weight = 1.0;
};


using RouteGraph =
boost::adjacency_list<
  boost::listS,        // OutEdgeList
  boost::vecS,         // VertexList
  //boost::directedS,    // Directed/Undirected
  boost::undirectedS,    // Directed/Undirected
  VertexProperties,    // VertexProperties
  EdgeProperties,      // EdgeProperties
  boost::no_property   // GraphProperties
  >;

using Vertex = RouteGraph::vertex_descriptor;
using Edge   = RouteGraph::edge_descriptor;


class IsRedundantEdge
{
  private:
    const RouteGraph& g_;
  public:
    explicit IsRedundantEdge(const RouteGraph& g) : g_(g) { }

    bool operator()(Edge const& e) const
    {
      const Vertex s {boost::source(e, g_)};
      const Vertex t {boost::target(e, g_)};

      return (get<1>(boost::edge(t, s, g_))
          && (   (g_[s].distance > g_[t].distance)
              || ((s > t) && (g_[s].distance >= g_[t].distance))
             ));
    }
};


//==============================================================================
// Writer helper objects
//==============================================================================
class LabelWriter
{
  private:
    RouteGraph const& graph;

  public:
    explicit LabelWriter(const RouteGraph&);

    void operator()(std::ostream&, Vertex const&) const;
    void operator()(std::ostream&, Edge const&) const;

};


class GraphWriter
{
  public:
    void operator()(std::ostream&) const;
};
#endif // GRAPH_HELPER_HPP
