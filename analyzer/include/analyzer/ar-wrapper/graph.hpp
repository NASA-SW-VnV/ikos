/*******************************************************************************
 *
 * Convert some ARBOS AR nodes into BGL (Boost Graph Library) via
 * traits.
 *
 * The idea is to convert some ARBOS AR nodes (actually two: functions
 * and bundles) to BGL graphs *without* having an intermediate graph
 * representation that translates explicitly from AR to a BGL
 * graph. We would like to avoid this since it can be expensive in
 * terms of memory. We use C++ traits for that.
 *
 * Author: Jorge A. Navas
 *         Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#ifndef ANALYZER_GRAPH_HPP
#define ANALYZER_GRAPH_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {
namespace graph {

template < typename G >
struct MkOutEdge : public std::unary_function<
                       typename boost::graph_traits< G >::vertex_descriptor,
                       typename boost::graph_traits< G >::edge_descriptor > {
  typedef typename boost::graph_traits< G >::vertex_descriptor Node;
  typedef typename boost::graph_traits< G >::edge_descriptor Edge;

  Node _src;

  MkOutEdge() : _src(Null_ref) {}
  MkOutEdge(const Node& src) : _src(src) {}
  Edge operator()(const Node& dst) const { return Edge(_src, dst); }
};

template < typename G >
struct MkInEdge : public std::unary_function<
                      typename boost::graph_traits< G >::vertex_descriptor,
                      typename boost::graph_traits< G >::edge_descriptor > {
  typedef typename boost::graph_traits< G >::vertex_descriptor Node;
  typedef typename boost::graph_traits< G >::edge_descriptor Edge;

  Node _dst;

  MkInEdge() : _dst(Null_ref) {}
  MkInEdge(const Node& dst) : _dst(dst) {}
  Edge operator()(const Node& src) const { return Edge(src, _dst); }
};

} // end namespace graph
} // end namespace arbos

namespace boost {

// Convert a Bundle to a BGL graph
template <>
struct graph_traits< arbos::Bundle_ref > {
  typedef arbos::Bundle_ref graph_t;

  typedef arbos::Function_ref vertex_descriptor;
  typedef std::pair< vertex_descriptor, vertex_descriptor > edge_descriptor;
  typedef std::pair< const vertex_descriptor, const vertex_descriptor >
      const_edge_descriptor;

  typedef arbos::FuncRange::iterator vertex_iterator;
  typedef boost::transform_iterator< arbos::graph::MkOutEdge< graph_t >,
                                     arbos::function_iterator >
      out_edge_iterator;
  typedef boost::transform_iterator< arbos::graph::MkInEdge< graph_t >,
                                     arbos::function_iterator >
      in_edge_iterator;

  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_tag directed_category;
  struct this_graph_tag : virtual bidirectional_graph_tag,
                          virtual vertex_list_graph_tag {};
  typedef this_graph_tag traversal_category;

  typedef std::size_t vertices_size_type;
  typedef std::size_t edges_size_type;
  typedef std::size_t degree_size_type;

  static inline vertex_descriptor null_vertex() { return arbos::Null_ref; }

}; // end class graph_traits

} // end namespace boost

namespace arbos {

// For graph = Bundle_ref
boost::graph_traits< Bundle_ref >::vertex_descriptor source(
    boost::graph_traits< Bundle_ref >::edge_descriptor e, Bundle_ref g) {
  return e.first;
}

boost::graph_traits< Bundle_ref >::vertex_descriptor target(
    boost::graph_traits< Bundle_ref >::edge_descriptor e, Bundle_ref g) {
  return e.second;
}

std::pair< boost::graph_traits< Bundle_ref >::in_edge_iterator,
           boost::graph_traits< Bundle_ref >::in_edge_iterator >
in_edges(boost::graph_traits< Bundle_ref >::vertex_descriptor v, Bundle_ref g) {
  std::pair< function_iterator, function_iterator > callers = v->getCallers();
  return std::make_pair(boost::make_transform_iterator(callers.first,
                                                       graph::MkInEdge<
                                                           Bundle_ref >(v)),
                        boost::make_transform_iterator(callers.second,
                                                       graph::MkInEdge<
                                                           Bundle_ref >(v)));
}

std::size_t in_degree(boost::graph_traits< Bundle_ref >::vertex_descriptor v,
                      Bundle_ref g) {
  std::pair< function_iterator, function_iterator > callers = v->getCallers();
  return std::distance(callers.first, callers.second);
}

std::pair< boost::graph_traits< Bundle_ref >::out_edge_iterator,
           boost::graph_traits< Bundle_ref >::out_edge_iterator >
out_edges(boost::graph_traits< Bundle_ref >::vertex_descriptor v,
          Bundle_ref g) {
  std::pair< function_iterator, function_iterator > callees = v->getCallees();
  return std::make_pair(boost::make_transform_iterator(callees.first,
                                                       graph::MkOutEdge<
                                                           Bundle_ref >(v)),
                        boost::make_transform_iterator(callees.second,
                                                       graph::MkOutEdge<
                                                           Bundle_ref >(v)));
}

std::size_t out_degree(boost::graph_traits< Bundle_ref >::vertex_descriptor v,
                       Bundle_ref g) {
  std::pair< function_iterator, function_iterator > callees = v->getCallees();
  return std::distance(callees.first, callees.second);
}

std::size_t degree(boost::graph_traits< Bundle_ref >::vertex_descriptor v,
                   Bundle_ref g) {
  return out_degree(v, g) + in_degree(v, g);
}

std::pair< boost::graph_traits< Bundle_ref >::vertex_iterator,
           boost::graph_traits< Bundle_ref >::vertex_iterator >
vertices(Bundle_ref g) {
  return std::make_pair(g->getFunctions().begin(), g->getFunctions().end());
}

std::size_t num_vertices(Bundle_ref g) {
  return g->getFunctions().size();
}

} // end namespace arbos

namespace boost {

// Convert a Function to a BGL graph
template <>
struct graph_traits< arbos::Function_ref > {
  typedef arbos::Function_ref graph_t;

  typedef arbos::Basic_Block_ref vertex_descriptor;
  typedef std::pair< vertex_descriptor, vertex_descriptor > edge_descriptor;
  typedef std::pair< const vertex_descriptor, const vertex_descriptor >
      const_edge_descriptor;

  typedef arbos::BBRange::iterator vertex_iterator;
  typedef boost::transform_iterator< arbos::graph::MkOutEdge< graph_t >,
                                     arbos::bblock_iterator > out_edge_iterator;
  typedef boost::transform_iterator< arbos::graph::MkInEdge< graph_t >,
                                     arbos::bblock_iterator > in_edge_iterator;

  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_tag directed_category;
  struct this_graph_tag : virtual bidirectional_graph_tag,
                          virtual vertex_list_graph_tag {};
  typedef this_graph_tag traversal_category;

  typedef std::size_t vertices_size_type;
  typedef std::size_t edges_size_type;
  typedef std::size_t degree_size_type;

  static inline vertex_descriptor null_vertex() { return arbos::Null_ref; }

}; // end class graph_traits

} // end namespace boost

namespace arbos {

// For graph = Function_ref
boost::graph_traits< Function_ref >::vertex_descriptor source(
    boost::graph_traits< Function_ref >::edge_descriptor e, Function_ref g) {
  return e.first;
}

boost::graph_traits< Function_ref >::vertex_descriptor target(
    boost::graph_traits< Function_ref >::edge_descriptor e, Function_ref g) {
  return e.second;
}

std::pair< boost::graph_traits< Function_ref >::in_edge_iterator,
           boost::graph_traits< Function_ref >::in_edge_iterator >
in_edges(boost::graph_traits< Function_ref >::vertex_descriptor v,
         Function_ref g) {
  std::pair< bblock_iterator, bblock_iterator > prev_blocks =
      v->getPreviousBlocks();
  return std::make_pair(boost::make_transform_iterator(prev_blocks.first,
                                                       graph::MkInEdge<
                                                           Function_ref >(v)),
                        boost::make_transform_iterator(prev_blocks.second,
                                                       graph::MkInEdge<
                                                           Function_ref >(v)));
}

std::size_t in_degree(boost::graph_traits< Function_ref >::vertex_descriptor v,
                      Function_ref g) {
  std::pair< bblock_iterator, bblock_iterator > prev_blocks =
      v->getPreviousBlocks();
  return std::distance(prev_blocks.first, prev_blocks.second);
}

std::pair< boost::graph_traits< Function_ref >::out_edge_iterator,
           boost::graph_traits< Function_ref >::out_edge_iterator >
out_edges(boost::graph_traits< Function_ref >::vertex_descriptor v,
          Function_ref g) {
  std::pair< bblock_iterator, bblock_iterator > succs_blocks =
      v->getNextBlocks();
  return std::make_pair(boost::make_transform_iterator(succs_blocks.first,
                                                       graph::MkOutEdge<
                                                           Function_ref >(v)),
                        boost::make_transform_iterator(succs_blocks.second,
                                                       graph::MkOutEdge<
                                                           Function_ref >(v)));
}

std::size_t out_degree(boost::graph_traits< Function_ref >::vertex_descriptor v,
                       Function_ref g) {
  std::pair< bblock_iterator, bblock_iterator > succs_blocks =
      v->getNextBlocks();
  return std::distance(succs_blocks.first, succs_blocks.second);
}

std::size_t degree(boost::graph_traits< Function_ref >::vertex_descriptor v,
                   Function_ref g) {
  return out_degree(v, g) + in_degree(v, g);
}

std::pair< boost::graph_traits< Function_ref >::vertex_iterator,
           boost::graph_traits< Function_ref >::vertex_iterator >
vertices(Function_ref g) {
  return std::make_pair(g->getFunctionBody()->getBlocks().begin(),
                        g->getFunctionBody()->getBlocks().end());
}

std::size_t num_vertices(Function_ref g) {
  return g->getFunctionBody()->getBlocks().size();
}

} // end namespace arbos

namespace boost {

template < typename Graph >
void write_graph(Graph g) {
  std::size_t n = num_vertices(g);
  std::cout << "Num of vertices: " << n << std::endl;

  typedef
      typename boost::graph_traits< Graph >::vertex_iterator vertex_iterator_t;
  typedef
      typename boost::graph_traits< Graph >::out_edge_iterator succ_iterator_t;
  typedef
      typename boost::graph_traits< Graph >::in_edge_iterator pred_iterator_t;

  boost::iterator_range< vertex_iterator_t > range =
      boost::make_iterator_range(vertices(g));
  for (vertex_iterator_t it = range.begin(), et = range.end(); it != et; ++it) {
    std::cout << "Vertex: " << *it << std::endl;
    std::cout << " Num of predecessors=" << in_degree(*it, g) << std::endl;
    std::cout << " Num of successors=" << out_degree(*it, g) << std::endl;
    std::cout << " Num of neighbors=" << degree(*it, g) << std::endl;
    std::cout << " Succs={";
    std::pair< succ_iterator_t, succ_iterator_t > succs = out_edges(*it, g);
    for (succ_iterator_t s_it = succs.first; s_it != succs.second; ++s_it)
      std::cout << target(*s_it, g) << ";";
    std::cout << "}" << std::endl << " Preds={ ";
    std::pair< pred_iterator_t, pred_iterator_t > preds = in_edges(*it, g);
    for (pred_iterator_t p_it = preds.first; p_it != preds.second; ++p_it)
      std::cout << source(*p_it, g) << ";";
    std::cout << "}" << std::endl;
  }
}

} // end namespace boost

#endif // ANALYZER_GRAPH_HPP
