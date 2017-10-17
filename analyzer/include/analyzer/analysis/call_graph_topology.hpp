/******************************************************************************
 *
 * Call graph analysis (Strongly connected components, Topological order, ...)
 *
 * Author: Maxime Arthaud
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

#ifndef ANALYZER_CALL_GRAPH_TOPOLOGY_HPP
#define ANALYZER_CALL_GRAPH_TOPOLOGY_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

#include <analyzer/ar-wrapper/graph.hpp>

namespace analyzer {

/*
 * The graph of strongly connected components of a bundle
 *
 * Uses boost::strong_components to compute the set of strongly connected
 * components of the Bundle. It gives us, for each component, a root node that
 * represents the component. The set of vertices of the StrongComponentsGraph
 * is thus the set of root nodes.
 */
class StrongComponentsGraph {
public:
  typedef std::unordered_set< Function_ref > vertices_t;
  typedef std::unordered_map< Function_ref, vertices_t > edges_map_t;

private:
  typedef std::unordered_map< Function_ref, std::size_t > component_map_t;
  typedef std::unordered_map< Function_ref, boost::default_color_type >
      color_map_t;
  typedef std::unordered_map< Function_ref, Function_ref > root_map_t;

private:
  Bundle_ref _bundle;

  /* Map a function to the root function of its component */
  root_map_t _root_map;

  /* Graph elements */
  vertices_t _vertices;
  edges_map_t _in_edges;
  edges_map_t _out_edges;

public:
  StrongComponentsGraph(Bundle_ref bundle) : _bundle(bundle) {}

  vertices_t& vertices() { return _vertices; }
  const vertices_t& vertices() const { return _vertices; }

  vertices_t& in_edges(Function_ref fun) { return _in_edges[fun]; }
  const vertices_t& in_edges(Function_ref fun) const {
    return _in_edges.at(fun);
  }

  vertices_t& out_edges(Function_ref fun) { return _out_edges[fun]; }
  const vertices_t& out_edges(Function_ref fun) const {
    return _out_edges.at(fun);
  }

  // build the graph of strongly connected components
  void build() {
    typedef boost::associative_property_map< component_map_t >
        property_component_map_t;
    typedef boost::associative_property_map< root_map_t > property_root_map_t;
    typedef boost::associative_property_map< color_map_t > property_color_map_t;

    typedef boost::graph_traits< Bundle_ref >::out_edge_iterator
        succ_iterator_t;
    typedef boost::graph_traits< Bundle_ref >::in_edge_iterator pred_iterator_t;

    // initialization
    component_map_t component_map;
    color_map_t color;
    component_map_t discover_time;

    FuncRange functions = ar::getFunctions(_bundle);
    for (FuncRange::iterator it = functions.begin(); it != functions.end();
         ++it) {
      component_map.insert(component_map_t::value_type(*it, 0));
      _root_map.insert(root_map_t::value_type(*it, Null_ref));
      color.insert(color_map_t::value_type(*it, boost::default_color_type()));
      discover_time.insert(component_map_t::value_type(*it, 0));
    }

    // call to boost::strong_components
    boost::strong_components(_bundle,
                             property_component_map_t(component_map),
                             root_map(property_root_map_t(_root_map))
                                 .color_map(property_color_map_t(color))
                                 .discover_time_map(
                                     property_component_map_t(discover_time)));

    // build all graph elements (_vertices, _in_edges, _out_edges)
    for (FuncRange::iterator it = functions.begin(); it != functions.end();
         ++it) {
      Function_ref fun = *it;
      Function_ref root = _root_map.find(fun)->second;

      _vertices.insert(root);
      vertices_t& root_in_edges = _in_edges[root];
      vertices_t& root_out_edges = _out_edges[root];

      std::pair< succ_iterator_t, succ_iterator_t > succs =
          arbos::out_edges(fun, _bundle);
      for (succ_iterator_t s_it = succs.first; s_it != succs.second; ++s_it) {
        Function_ref out = _root_map.find(target(*s_it, _bundle))->second;
        if (out != root) {
          root_out_edges.insert(out);
        }
      }

      std::pair< pred_iterator_t, pred_iterator_t > preds =
          arbos::in_edges(fun, _bundle);
      for (pred_iterator_t p_it = preds.first; p_it != preds.second; ++p_it) {
        Function_ref in = _root_map.find(source(*p_it, _bundle))->second;
        if (in != root) {
          root_in_edges.insert(in);
        }
      }
    }
  }

  // Return all functions in the same strongly connected component
  std::vector< Function_ref > component(Function_ref root) const {
    std::vector< Function_ref > functions;

    for (root_map_t::const_iterator it = _root_map.begin();
         it != _root_map.end();
         ++it) {
      if (it->second == root) {
        functions.push_back(it->first);
      }
    }

    return functions;
  }

}; // end class StrongComponentsGraph

/*
 * http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/faq.html
 * BGL uses calls by value. To improve performance, we use std::shared_ptr
 */
typedef std::shared_ptr< StrongComponentsGraph > StrongComponentsGraph_ref;

} // end namespace analyzer

namespace boost {

// Convert a StrongComponentsGraph_ref to a BGL graph

template <>
struct graph_traits< analyzer::StrongComponentsGraph_ref > {
  typedef analyzer::StrongComponentsGraph_ref graph_t;

  typedef arbos::Function_ref vertex_descriptor;
  typedef std::pair< vertex_descriptor, vertex_descriptor > edge_descriptor;
  typedef std::pair< const vertex_descriptor, const vertex_descriptor >
      const_edge_descriptor;

  typedef analyzer::StrongComponentsGraph::vertices_t::iterator vertex_iterator;
  typedef boost::transform_iterator< arbos::graph::MkOutEdge< graph_t >,
                                     vertex_iterator > out_edge_iterator;
  typedef boost::transform_iterator< arbos::graph::MkInEdge< graph_t >,
                                     vertex_iterator > in_edge_iterator;

  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_tag directed_category;
  struct this_graph_tag : virtual bidirectional_graph_tag,
                          virtual vertex_list_graph_tag {};
  typedef this_graph_tag traversal_category;

  typedef std::size_t vertices_size_type;
  typedef std::size_t edges_size_type;
  typedef std::size_t degree_size_type;

  static vertex_descriptor null_vertex() { return arbos::Null_ref; }

}; // end class graph_traits

} // end namespace boost

namespace analyzer {

boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor source(
    boost::graph_traits< StrongComponentsGraph_ref >::edge_descriptor e,
    StrongComponentsGraph_ref g) {
  return e.first;
}

boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor target(
    boost::graph_traits< StrongComponentsGraph_ref >::edge_descriptor e,
    StrongComponentsGraph_ref g) {
  return e.second;
}

std::pair< boost::graph_traits< StrongComponentsGraph_ref >::in_edge_iterator,
           boost::graph_traits< StrongComponentsGraph_ref >::in_edge_iterator >
in_edges(boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor v,
         StrongComponentsGraph_ref g) {
  return std::
      make_pair(boost::make_transform_iterator(g->in_edges(v).begin(),
                                               arbos::graph::MkInEdge<
                                                   StrongComponentsGraph_ref >(
                                                   v)),
                boost::make_transform_iterator(g->in_edges(v).end(),
                                               arbos::graph::MkInEdge<
                                                   StrongComponentsGraph_ref >(
                                                   v)));
}

std::size_t in_degree(
    boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor v,
    StrongComponentsGraph_ref g) {
  return g->in_edges(v).size();
}

std::pair< boost::graph_traits< StrongComponentsGraph_ref >::out_edge_iterator,
           boost::graph_traits< StrongComponentsGraph_ref >::out_edge_iterator >
out_edges(boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor v,
          StrongComponentsGraph_ref g) {
  return std::
      make_pair(boost::make_transform_iterator(g->out_edges(v).begin(),
                                               arbos::graph::MkOutEdge<
                                                   StrongComponentsGraph_ref >(
                                                   v)),
                boost::make_transform_iterator(g->out_edges(v).end(),
                                               arbos::graph::MkOutEdge<
                                                   StrongComponentsGraph_ref >(
                                                   v)));
}

std::size_t out_degree(
    boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor v,
    StrongComponentsGraph_ref g) {
  return g->out_edges(v).size();
}

std::size_t degree(
    boost::graph_traits< StrongComponentsGraph_ref >::vertex_descriptor v,
    StrongComponentsGraph_ref g) {
  return out_degree(v, g) + in_degree(v, g);
}

std::pair< boost::graph_traits< StrongComponentsGraph_ref >::vertex_iterator,
           boost::graph_traits< StrongComponentsGraph_ref >::vertex_iterator >
vertices(StrongComponentsGraph_ref g) {
  return std::make_pair(g->vertices().begin(), g->vertices().end());
}

std::size_t num_vertices(StrongComponentsGraph_ref g) {
  return g->vertices().size();
}

} // end namespace analyzer

namespace analyzer {

// Compute the topological sort of a StrongComponentsGraph.
std::vector< Function_ref > topological_sort(StrongComponentsGraph_ref g) {
  // initialize
  typedef std::unordered_map< Function_ref, boost::default_color_type >
      color_map_t;
  typedef boost::associative_property_map< color_map_t > property_color_map_t;

  color_map_t color;

  for (StrongComponentsGraph::vertices_t::iterator it = g->vertices().begin();
       it != g->vertices().end();
       ++it) {
    color[*it] = boost::default_color_type();
  }

  std::vector< Function_ref > order;
  order.reserve(g->vertices().size());

  // call to boost::topological_sort
  boost::topological_sort(g,
                          std::back_inserter(order),
                          color_map(property_color_map_t(color)));

  return order;
}

} // end namespace analyzer

#endif // ANALYZER_CALL_GRAPH_TOPOLOGY_HPP
