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

#include "GraphHelper.hpp"

//==============================================================================
// LabelWriter
//==============================================================================
LabelWriter::LabelWriter(const AcGraph& _graph) :
  graph(_graph)
{}

void
LabelWriter::operator()(std::ostream& os, Vertex const& v) const
{
  os << "["
     << "shape=\"" << graph[v].shape << "\""
     << ", "
     << "style=\"" << graph[v].style << "\""
     << ", "
     << "fillcolor=\"" << graph[v].fillcolor << "\""
     << ", "
     << "label=\"" << graph[v].label << "\""
     << "]";
}

void
LabelWriter::operator()(std::ostream& os, Edge const& e) const
{
  Vertex const s = boost::source(e, graph);
  Vertex const t = boost::target(e, graph);

  double const distance_ratio = (graph[s].distance / graph[t].distance);
  std::string constraint;
  if ((0.999 < distance_ratio) && (distance_ratio < 1.001)) {
    constraint = "constraint=false, ";
  }

  os << "["
     << constraint
     << "label=\"" << graph[e].label << "\""
     << "]";
}


//==============================================================================
// GraphWriter
//==============================================================================
void
GraphWriter::operator()(std::ostream& os) const
{
  os << "rankdir=LR" << std::endl;
  os << "splines=true;" << std::endl;
  os << "nodesep=1.00;" << std::endl;
  os << "ranksep=2.50;" << std::endl;
  os << "overlap=false;" << std::endl;
}
