/*******************************************************************************
 * Generic abstract domain for reasoning about array cells based on
 * computing all the feasible partial orderings between array
 * indexes. For each array variable we keep a graph where vertices are
 * the array indexes and edges are labelled with weights. The weight
 * domain must be distributive so we can compute efficiently the least
 * fixed point by applying the Floyd-Warshall's algorithm. An edge
 * (i,j) with weight w denotes that the property w holds for the all
 * elements in the array between [i,j).
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * This domain is based on the paper "A Partial-Order Approach to
 * Array Content Analysis" by Gange, Navas, Schachte, Sondergaard, and
 * Stuckey (http://arxiv.org/pdf/1408.1754v1.pdf)
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

#ifndef ANALYZER_ARRAY_GRAPH_HPP
#define ANALYZER_ARRAY_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <ikos/algorithms/mergeable_map.hpp>
#include <ikos/domains/abstract_domains_api.hpp>

#include <analyzer/analysis/common.hpp>

namespace ikos {

/*
   A weighted array graph is a graph (V,E,L) where V is the set of
   vertices, E the edges and L is a label function: E -> W such that:
  - If there is an edge e from vertex i to j whose weight is not
    bottom (i.e., L(e) != bot) then it means that i < j.
  - It is possible one non-bottom edge from i to j and another
    non-bottom one from j to i. This means that both i < j and j < i
    are possible.
  - If the both edges from i to j and from j to i are bottom then it
    must be that (i>=j) && (j>=i) = (i==j)
   W must be a bounded idempotent semiring or equivalently, a
   distributive lattice.
 */
template < typename VertexName, typename Weight, typename ScalarNumDomain >
class array_graph : public ikos::writeable {
  // generic wrapper for using VertexName as key in almost any
  // container (std::unordered_map/set, mergeable_map, etc)
  struct VertexNameKey {
    VertexName _v;
    VertexNameKey(VertexName v) : _v(v) {}
    VertexNameKey(const VertexNameKey& other) : _v(other._v) {}
    VertexNameKey& operator=(VertexNameKey other) {
      this->_v = other._v;
      return *this;
    }
    VertexName name() const { return this->_v; }
    bool operator==(const VertexNameKey& other) const {
      return (this->index() == other.index());
    }
    bool operator<(const VertexNameKey& other) const {
      return (this->index() < other.index());
    }
    index64_t index() const {
      VertexName x(this->_v);
      return x.index();
    }
    void write(std::ostream& o) const { name().write(o); }
  }; // end struct VertexNameKey

  struct KeyHasher {// key hasher for VertexNameKey
    size_t operator()(const VertexNameKey& a) const { return a.index(); }
  };

  template < typename Any1, typename Any2, typename Any3, typename Any4 >
  friend class array_graph_domain;

private:
  typedef uint64_t key_t;
  typedef std::shared_ptr< VertexName > VertexNamePtr;
  typedef std::shared_ptr< Weight > WeightPtr;
  struct graph_vertex_t {
    VertexNamePtr name;
  };
  struct graph_edge_t {
    WeightPtr weight;
  };

public:
  typedef array_graph< VertexName, Weight, ScalarNumDomain > array_graph_t;
  typedef std::tuple< VertexName, VertexName, Weight > edge_t;

private:
  typedef boost::adjacency_list< boost::listS,
                                 boost::listS,
                                 boost::bidirectionalS,
                                 graph_vertex_t,
                                 graph_edge_t > graph_t;
  typedef std::shared_ptr< graph_t > graph_ptr;

private:
  typedef typename boost::graph_traits< graph_t >::edge_iterator edge_iterator;
  typedef
      typename boost::graph_traits< graph_t >::vertex_iterator vertex_iterator;
  typedef typename boost::graph_traits< graph_t >::edge_descriptor
      edge_descriptor_t;
  typedef typename boost::graph_traits< graph_t >::vertex_descriptor
      vertex_descriptor_t;
  typedef typename boost::graph_traits< graph_t >::out_edge_iterator
      out_edge_iterator;
  typedef typename boost::graph_traits< graph_t >::in_edge_iterator
      in_edge_iterator;

private:
  typedef typename ScalarNumDomain::linear_constraint_t linear_constraint_t;
  typedef typename ScalarNumDomain::variable_t variable_t;

private:
  typedef std::unordered_map< key_t, vertex_descriptor_t > vertex_map_t;
  typedef std::shared_ptr< vertex_map_t > vertex_map_ptr;

private:
  typedef std::set< VertexNameKey > vertex_names_set_t;
  typedef std::shared_ptr< vertex_names_set_t > vertex_names_set_ptr;

private:
  bool _is_bottom;
  graph_ptr _graph;
  vertex_map_ptr _vertex_map; // map a VertexName to a graph vertex
  vertex_names_set_ptr _vertices_set;

private:
  bool find_vertex_map(VertexName v) {
    return (this->_vertex_map->find(v.index()) != this->_vertex_map->end());
  }

  void insert_vertex_map(VertexName key, vertex_descriptor_t value) {
    if (find_vertex_map(key)) {
      std::ostringstream err;
      err << key << " already in the vertex map" << std::endl;
      throw logic_error(err.str());
    } else {
      this->_vertex_map->insert(std::make_pair(key.index(), value));
      this->_vertices_set->insert(key);
    }
  }

  void remove_vertex_map(VertexName key) {
    this->_vertex_map->erase(key.index());
    this->_vertices_set->erase(key);
  }

  vertex_descriptor_t lookup_vertex_map(key_t key) const {
    typename vertex_map_t::const_iterator it = this->_vertex_map->find(key);
    if (it != this->_vertex_map->end()) {
      return it->second;
    } else {
      std::ostringstream err;
      array_graph_t tmp(*this);
      err << "No vertex with id " << key << " found in the graph: " << tmp;
      throw logic_error(err.str());
    }
  }

  vertex_descriptor_t lookup_vertex_map(VertexName key) const {
    typename vertex_map_t::const_iterator it =
        this->_vertex_map->find(key.index());
    if (it != this->_vertex_map->end()) {
      return it->second;
    } else {
      std::ostringstream err;
      array_graph_t tmp(*this);
      err << "No vertex with name " << key << " found in the graph: " << tmp;
      throw logic_error(err.str());
    }
  }

  // All methods that add new vertices should call this one.
  void add(std::vector< VertexName > vertices, std::vector< edge_t > edges) {
    for (unsigned int i = 0; i < vertices.size(); i++) {
      vertex_descriptor_t u = add_vertex(*this->_graph);
      (*this->_graph)[u].name = VertexNamePtr(new VertexName(vertices[i]));
      insert_vertex_map(vertices[i], u);
    }
    for (unsigned int i = 0; i < edges.size(); i++) {
      vertex_descriptor_t u = lookup_vertex_map(std::get< 0 >(edges[i]));
      vertex_descriptor_t v = lookup_vertex_map(std::get< 1 >(edges[i]));
      edge_descriptor_t e;
      bool b;
      boost::tie(e, b) = add_edge(u, v, *this->_graph);
      if (!b) {
        throw logic_error("edge is already in the graph");
      }
      (*this->_graph)[e].weight =
          WeightPtr(new Weight(std::get< 2 >(edges[i])));
    }
    this->canonical();
  }

  // All methods that remove vertices should call this one.
  void remove(VertexName v) {
    this->canonical();
    vertex_descriptor_t u = lookup_vertex_map(v);
    /*
    in_edge_iterator in_it, in_et;
    for (boost::tie(in_it, in_et) = boost::in_edges(u, *this->_graph); in_it !=
    in_et;
    ++in_it){
      out_edge_iterator out_it, out_et;
      for (boost::tie(out_it, out_et) = boost::out_edges(u, *this->_graph);
    out_it != out_et;
    ++out_it){
        vertex_descriptor_t x = source(*in_it, *this->_graph);
        vertex_descriptor_t y = target(*out_it, *this->_graph);
          if (x != y){
            if (boost::edge(x, y, *this->_graph).second){
              edge_descriptor_t e = boost::edge(x, y, *this->_graph).first;
              binary_join_op join;
              (*this->_graph)[e].weight = join((*this->_graph)[*in_it].weight
    ,(*this->_graph)[*out_it].weight);
            }
          }
      }
    }
    */
    // remove all in and out edges to/from u
    clear_vertex(u, *this->_graph);
    // remove the vertex
    remove_vertex(u, *this->_graph);
    remove_vertex_map(v);
  }

  ///////////////////////////////////////////////////////////////////////
  // For our canonical form, we would like to compute the greatest
  // fixed point to the set of inequalities:
  //    \forall i,j,k. G[i,j] \subseteq G[i,k] \cup G[k,j]
  // Since the weight domain is distributive we can solve this set of
  // inequations by solving:
  //    \forall i,j,k. G[i,j] = G[i,j] \cap G[i,k] \cup G[k,j]
  // The Floyd-Warshall algorithm does exactly that.
  ///////////////////////////////////////////////////////////////////////
  void canonical() {
    // Floyd-Warshall algorithm
    binary_join_op join;
    binary_meet_op meet;
    vertex_iterator ki, ke, ii, ie, ji, je;
    for (boost::tie(ki, ke) = boost::vertices(*this->_graph); ki != ke; ++ki) {
      for (boost::tie(ii, ie) = boost::vertices(*this->_graph); ii != ie;
           ++ii) {
        if (boost::edge(*ii, *ki, *this->_graph).second)
          for (boost::tie(ji, je) = boost::vertices(*this->_graph); ji != je;
               ++ji) {
            if (boost::edge(*ii, *ji, *this->_graph).second &&
                boost::edge(*ki, *ji, *this->_graph).second) {
              edge_descriptor_t e_ij =
                  boost::edge(*ii, *ji, *this->_graph).first;
              edge_descriptor_t e_ik =
                  boost::edge(*ii, *ki, *this->_graph).first;
              edge_descriptor_t e_kj =
                  boost::edge(*ki, *ji, *this->_graph).first;
              (*this->_graph)[e_ij].weight =
                  meet((*this->_graph)[e_ij].weight,
                       (join((*this->_graph)[e_ik].weight,
                             (*this->_graph)[e_kj].weight)));
            }
          }
      }
    }
  }

private:
  array_graph(bool is_bot)
      : _is_bottom(is_bot),
        _graph(new graph_t(0)),
        _vertex_map(new vertex_map_t()),
        _vertices_set(new vertex_names_set_t()) {}

public:
  static array_graph_t bottom() { return array_graph(true); }

  static array_graph_t top() { return array_graph(false); }

  array_graph(const array_graph_t& other)
      : _is_bottom(other._is_bottom),
        //_graph(new graph_t(*other._graph)),
        //_vertex_map(new vertex_map_t(*other._vertex_map)),
        //_vertices_set( new vertex_names_set_t(*other._vertices_set))
        _graph(new graph_t(0)),
        _vertex_map(new vertex_map_t()),
        _vertices_set(new vertex_names_set_t()) {
    if (!this->_is_bottom) {
      // copy vertices and internal datastructures (_vertex_map and
      // _vertices_set)
      vertex_iterator i, e;
      for (boost::tie(i, e) = boost::vertices(*other._graph); i != e; ++i) {
        vertex_descriptor_t u = add_vertex(*this->_graph);
        VertexName u_name = *((*other._graph)[*i].name);
        (*this->_graph)[u].name = VertexNamePtr(new VertexName(u_name));
        insert_vertex_map(u_name, u);
      }
      // copy edges
      edge_iterator ie, ee;
      for (boost::tie(ie, ee) = boost::edges(*other._graph); ie != ee; ++ie) {
        VertexName u = *((*other._graph)[source(*ie, *other._graph)].name);
        VertexName v = *((*other._graph)[target(*ie, *other._graph)].name);
        Weight w = *((*other._graph)[*ie].weight);
        vertex_descriptor_t _u = lookup_vertex_map(u);
        vertex_descriptor_t _v = lookup_vertex_map(v);
        edge_descriptor_t _e;
        bool b;
        boost::tie(_e, b) = add_edge(_u, _v, *this->_graph);
        (*this->_graph)[_e].weight = WeightPtr(new Weight(w));
      }
    }
  }

  array_graph_t& operator=(const array_graph_t& other) {
    this->_is_bottom = other._is_bottom;
    this->_graph = other._graph;
    this->_vertex_map = other._vertex_map;
    this->_vertices_set = other._vertices_set;
    return *this;
  }

  void insert_vertex(VertexName u, Weight val = Weight::top()) {
    if (!this->_is_bottom && !find_vertex_map(u)) {
      std::vector< VertexName > new_vertices;
      new_vertices.push_back(u);
      std::vector< edge_t > new_edges;
      vertex_iterator i, e;
      for (boost::tie(i, e) = boost::vertices(*this->_graph); i != e; ++i) {
        VertexNamePtr v = (*this->_graph)[*i].name;
        // add two edges in both directions
        new_edges.push_back(edge_t(u, *v, val));
        new_edges.push_back(edge_t(*v, u, val));
      }
      add(new_vertices, new_edges);
    }
  }

  bool is_bottom() { return this->_is_bottom; }

  bool is_top() {
    if (this->is_bottom())
      return false;
    else {
      // FIXME: speedup this operation
      this->canonical();
      edge_iterator it, et;
      for (boost::tie(it, et) = boost::edges(*this->_graph); it != et; ++it) {
        edge_descriptor_t e = *it;
        if ((*(*this->_graph)[e].weight) == Weight::top())
          continue;
        else
          return false;
      }
      return true;
    }
  }

  void reduce(ScalarNumDomain scalar) {
    if (!this->_is_bottom) {
      this->canonical();
      edge_iterator it, et;
      for (boost::tie(it, et) = boost::edges(*this->_graph); it != et; ++it) {
        edge_descriptor_t e = *it;
        VertexNamePtr u = (*this->_graph)[source(e, *this->_graph)].name;
        VertexNamePtr v = (*this->_graph)[target(e, *this->_graph)].name;
        ScalarNumDomain tmp(scalar);
        linear_constraint_t cst(variable_t(*u) <= variable_t(*v) - 1);
        tmp += cst;
        if (tmp.is_bottom()) {
          (*this->_graph)[e].weight = WeightPtr(new Weight(Weight::bottom()));
        }
      }
      this->canonical();
    }
  }

  // Point-wise application of <= in the weight domain
  bool operator<=(array_graph_t other) {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      this->canonical();
      other.canonical();
      edge_iterator it_1, et_1;
      edge_iterator it_2, et_2;
      for (boost::tie(it_1, et_1) = boost::edges(*this->_graph); it_1 != et_1;
           ++it_1) {
        edge_descriptor_t e_1 = *it_1;
        vertex_descriptor_t u_1 = source(e_1, *this->_graph);
        vertex_descriptor_t v_1 = target(e_1, *this->_graph);
        VertexNamePtr u_name_1 = (*this->_graph)[u_1].name;
        VertexNamePtr v_name_1 = (*this->_graph)[v_1].name;
        WeightPtr weight_1 = (*this->_graph)[e_1].weight;
        vertex_descriptor_t u_2 = other.lookup_vertex_map(*u_name_1);
        vertex_descriptor_t v_2 = other.lookup_vertex_map(*v_name_1);

        if (boost::edge(u_2, v_2, *other._graph).second) {
          edge_descriptor_t e_2 = boost::edge(u_2, v_2, *other._graph).first;
          WeightPtr weight_2 = (*other._graph)[e_2].weight;
          if (!(*weight_1 <= *weight_2))
            return false;
        } else {
          throw logic_error(
              "operator<= with graphs with different adjacency structure");
        }
      } // end for
      return true;
    }
  }

  bool operator==(array_graph_t other) {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else {
      return (*(this->_vertices_set) == *(other._vertices_set) &&
              (this->operator<=(other) && other.operator<=(*this)));
    }
  }

  // pre: caller must ensure the graph is in canonical form
  void set_incoming(const VertexName& v, const Weight& weight) {
    if (!this->_is_bottom) {
      vertex_descriptor_t u = lookup_vertex_map(v);
      in_edge_iterator in_it, in_et;
      for (boost::tie(in_it, in_et) = boost::in_edges(u, *this->_graph);
           in_it != in_et;
           ++in_it) {
        (*this->_graph)[*in_it].weight = WeightPtr(new Weight(weight));
      }
    }
  }

  // pre: caller must ensure the graph is in canonical form
  void set_outgoing(const VertexName& v, const Weight& weight) {
    if (!this->_is_bottom) {
      vertex_descriptor_t u = lookup_vertex_map(v);
      out_edge_iterator out_it, out_et;
      for (boost::tie(out_it, out_et) = boost::out_edges(u, *this->_graph);
           out_it != out_et;
           ++out_it) {
        (*this->_graph)[*out_it].weight = WeightPtr(new Weight(weight));
      }
    }
  }

  void operator-=(VertexName v) {
    if (!this->_is_bottom) {
      this->remove(v);
    }
  }

  struct binary_join_op {
    WeightPtr operator()(WeightPtr w1, WeightPtr w2) {
      return WeightPtr(new Weight(*w1 | *w2));
    }
  };

  struct binary_meet_op {
    WeightPtr operator()(WeightPtr w1, WeightPtr w2) {
      return WeightPtr(new Weight(*w1 & *w2));
    }
  };

  template < typename BinaryOp >
  void pointwise_binary_operator_helper(array_graph_t& g1,
                                        const array_graph_t& g2) {
    // pre: g1 and g2 have the same adjacency structure
    edge_iterator it_1, et_1;
    for (boost::tie(it_1, et_1) = boost::edges(*g1._graph); it_1 != et_1;
         ++it_1) {
      edge_descriptor_t e_1 = *it_1;
      vertex_descriptor_t u_1 = source(e_1, *g1._graph);
      vertex_descriptor_t v_1 = target(e_1, *g1._graph);
      VertexNamePtr u_name_1 = (*g1._graph)[u_1].name;
      VertexNamePtr v_name_1 = (*g1._graph)[v_1].name;
      WeightPtr weight_1 = (*g1._graph)[e_1].weight;
      vertex_descriptor_t u_2 = g2.lookup_vertex_map(*u_name_1);
      vertex_descriptor_t v_2 = g2.lookup_vertex_map(*v_name_1);
      if (boost::edge(u_2, v_2, *g2._graph).second) {
        BinaryOp op;
        edge_descriptor_t e_2 = boost::edge(u_2, v_2, *g2._graph).first;
        (*g1._graph)[e_1].weight =
            op((*g1._graph)[e_1].weight, (*g2._graph)[e_2].weight);
      } else {
        ikos_unreachable("unreachable");
      }
    } // end for
  }

  template < typename Iterator >
  void insert_vertices(array_graph_t& g, Iterator begin, Iterator end) {
    for (; begin != end; ++begin) {
      g.insert_vertex(begin->name());
    }
  }

  template < typename BinaryOp >
  array_graph_t pointwise_binary_operator(array_graph_t g1, array_graph_t g2) {
    g1.canonical();
    g2.canonical();

    // if (*(g1._vertices_set) != *(g2._vertices_set)){
    //   set<VertexNameKey> all_vs;
    //   set_union(g1._vertices_set->begin(), g1._vertices_set->end(),
    //             g2._vertices_set->begin(), g2._vertices_set->end(),
    //             inserter(all_vs, all_vs.end()));
    //   std::vector<VertexNameKey> new_g1, new_g2;
    //   set_difference(all_vs.begin(), all_vs.end(),
    //                  g1._vertices_set->begin(), g1._vertices_set->end(),
    //                  inserter(new_g1, new_g1.end()));
    //   set_difference(all_vs.begin(), all_vs.end(),
    //                  g2._vertices_set->begin(), g2._vertices_set->end(),
    //                  inserter(new_g2, new_g2.end()));
    //   insert_vertices<typename std::vector<VertexNameKey>::iterator>(g1,
    //   new_g1.begin(), new_g1.end());
    //   insert_vertices<typename std::vector<VertexNameKey>::iterator>(g2,
    //   new_g2.begin(), new_g2.end());
    // }

    // pre: g1 and g2 have the same set of vertices and edges at this point
    pointwise_binary_operator_helper< BinaryOp >(g1, g2);
    return g1;
  }

  // Point-wise join in the weight domain
  array_graph_t operator|(array_graph_t other) {
    if (this->is_bottom())
      return other;
    else if (other.is_bottom())
      return *this;
    else {
      return pointwise_binary_operator< binary_join_op >(*this, other);
    }
  }

  // Point-wise widening in the weight domain
  array_graph_t operator||(array_graph_t other) {
    return this->operator|(other);
  }

  // Point-wise meet in the weight domain
  array_graph_t operator&(array_graph_t other) {
    if (this->is_bottom())
      return *this;
    else if (other.is_bottom())
      return other;
    else {
      return pointwise_binary_operator< binary_meet_op >(*this, other);
    }
  }

  // Point-wise narrowing in the weight domain
  array_graph_t operator&&(array_graph_t other) {
    return this->operator&(other);
  }

  void update_weight(const VertexName& src,
                     const VertexName& dest,
                     const Weight& weight) {
    if (find_vertex_map(src) && find_vertex_map(dest)) {
      vertex_descriptor_t u = lookup_vertex_map(src);
      vertex_descriptor_t v = lookup_vertex_map(dest);
      if (boost::edge(u, v, *this->_graph).second) {
        edge_descriptor_t e = edge(u, v, *this->_graph).first;
        (*this->_graph)[e].weight = WeightPtr(new Weight(weight));
      } else {
        std::vector< VertexName > vertices;
        std::vector< edge_t > edges;
        edges.push_back(edge_t(src, dest, weight));
        add(vertices, edges);
      }
    } else {
      std::ostringstream err;
      VertexName x(src);
      VertexName y(dest);
      err << "Either vertex " << x << " or " << y
          << " does not exist in the graph" << std::endl;
      throw logic_error(err.str());
    }
  }

  void meet_weight(const VertexName& src,
                   const VertexName& dest,
                   Weight weight) {
    if (find_vertex_map(src) && find_vertex_map(dest)) {
      vertex_descriptor_t u = lookup_vertex_map(src);
      vertex_descriptor_t v = lookup_vertex_map(dest);
      if (boost::edge(u, v, *this->_graph).second) {
        edge_descriptor_t e = boost::edge(u, v, *this->_graph).first;
        Weight meet = weight & (*(*this->_graph)[e].weight);
        (*this->_graph)[e].weight = WeightPtr(new Weight(meet));
      } else {
        std::vector< VertexName > vertices;
        std::vector< edge_t > edges;
        edges.push_back(edge_t(src, dest, weight));
        add(vertices, edges);
      }
    }
  }

  Weight get_weight(const VertexName& src, const VertexName& dest) {
    if (find_vertex_map(src) && find_vertex_map(dest)) {
      vertex_descriptor_t u = lookup_vertex_map(src);
      vertex_descriptor_t v = lookup_vertex_map(dest);
      if (boost::edge(u, v, *this->_graph).second) {
        edge_descriptor_t e = boost::edge(u, v, *this->_graph).first;
        return *((*this->_graph)[e].weight);
      }
    }
    std::ostringstream err;
    VertexName x(src);
    VertexName y(dest);
    err << "No edge associated with " << x << " and " << y << std::endl;
    throw logic_error(err.str());
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      {
        vertex_iterator it, et;
        o << "(V={";
        for (boost::tie(it, et) = boost::vertices(*this->_graph); it != et;
             ++it) {
          vertex_descriptor_t u = *it;
          VertexNamePtr u_name = (*this->_graph)[u].name;
#if 0
          // more verbose mode
          o << *u_name << " (index=" << (*u_name).index() << ") ";
#else
          o << *u_name << " ";
#endif
        }
        o << "},";
      }
      {
        edge_iterator it, et;
        o << "E={";
        for (boost::tie(it, et) = boost::edges(*this->_graph); it != et; ++it) {
          edge_descriptor_t e = *it;
          vertex_descriptor_t u = source(e, *this->_graph);
          vertex_descriptor_t v = target(e, *this->_graph);
          VertexNamePtr u_name = (*this->_graph)[u].name;
          VertexNamePtr v_name = (*this->_graph)[v].name;
          WeightPtr weight = (*this->_graph)[e].weight;
          if (!weight->is_bottom()) {
            o << "(" << *u_name << "," << *v_name << "," << *weight << ") ";
          }
        }
        o << "})";
      }
    }
  }
}; // end class array_graph

// Reduced product of a scalar numerical domain with a weighted array
// graph. The reduction goes only in one direction: from the scalar to
// the array graph. A bidirectional reduction is possible and it makes
// sense if the array cells are abstracted with numerical
// constraints. This case is also described in Gange et.al.

template < typename ScalarNumDomain,
           typename Number,
           typename VariableName,
           typename Weight >
class array_graph_domain : public ikos::abstract_domain {
private:
  typedef array_graph_domain< ScalarNumDomain, Number, VariableName, Weight >
      array_graph_domain_t;

private:
  typedef array_graph< VariableName, Weight, ScalarNumDomain > array_graph_t;

public:
  typedef typename ScalarNumDomain::linear_constraint_t linear_constraint_t;
  typedef typename ScalarNumDomain::linear_constraint_system_t
      linear_constraint_system_t;
  typedef typename ScalarNumDomain::linear_expression_t linear_expression_t;
  typedef typename ScalarNumDomain::variable_t variable_t;

private:
  typedef typename array_graph_t::VertexNameKey VariableNameKey;
  typedef mergeable_map< VariableNameKey, VariableName > succ_index_map_t;
  typedef std::shared_ptr< succ_index_map_t > succ_index_map_ptr;

private:
  bool _is_bottom;
  ScalarNumDomain _scalar;
  array_graph_t _g;
  // for each array index i we keep track of a special index that represent i+1
  succ_index_map_ptr _succ_idx_map;

private:
  void abstract(VariableName v) {
    if (this->_g.find_vertex_map(v)) {
      this->_g.set_incoming(v, Weight::top());
      this->_g.set_outgoing(v, Weight::top());
      boost::optional< VariableName > succ_v = get_succ_idx(v);
      if (succ_v) {
        this->_g.set_incoming(*succ_v, Weight::top());
        this->_g.set_outgoing(*succ_v, Weight::top());
      }
    }
  }

  inline VariableName mk_succ_idx(VariableName x,
                                  analyzer::VariableFactory& vfac) {
    std::ostringstream b;
    b << x;
    VariableName x_plus = vfac["\"" + b.str() + "+" + "\""];
    return x_plus;
  }

  inline boost::optional< VariableName > get_succ_idx(VariableName v) const {
    return this->_succ_idx_map->operator[](v);
  }

  inline VariableName add_variable(Number n, analyzer::VariableFactory& vfac) {
    std::ostringstream buf;
    buf << "v_" << n;
    VariableName var_n = vfac[buf.str()];
    if (n >= 0) {
      this->_g.insert_vertex(var_n);
      this->_scalar.assign(var_n, n);
    }
    return var_n;
  }

  inline void add_variable(VariableName v, analyzer::VariableFactory& vfac) {
    if (is_array_index(v)) {
      VariableName v_succ = mk_succ_idx(v, vfac);
      this->_g.insert_vertex(v);
      this->_g.insert_vertex(v_succ);
      this->_succ_idx_map->set(v, v_succ);
      // all array indexes are non-negative
      // this->_scalar += linear_constraint_t( variable_t(v) >= 0 );
      // forcing "i+" = i + 1
      this->_scalar +=
          linear_constraint_t(variable_t(v_succ) == variable_t(v) + 1);
    }
  }

public: /* temporary public for testing */
  void meet_weight(VariableName i,
                   VariableName j,
                   Weight w,
                   analyzer::VariableFactory& vfac) {
    add_variable(i, vfac);
    add_variable(j, vfac);
    this->_g.meet_weight(i, j, w);
    this->reduce();
  }
  void meet_weight(Number i,
                   Number j,
                   Weight w,
                   analyzer::VariableFactory& vfac) {
    this->_g.meet_weight(add_variable(i, vfac), add_variable(j, vfac), w);
    this->reduce();
  }
  void meet_weight(Number i,
                   VariableName j,
                   Weight w,
                   analyzer::VariableFactory& vfac) {
    add_variable(j, vfac);
    this->_g.meet_weight(add_variable(i, vfac), j, w);
    this->reduce();
  }
  void meet_weight(VariableName i,
                   Number j,
                   Weight w,
                   analyzer::VariableFactory& vfac) {
    add_variable(i, vfac);
    this->_g.meet_weight(i, add_variable(j, vfac), w);
    this->reduce();
  }

private:
  // x := x op k
  template < typename VariableNameOrNumber >
  void apply_helper(operation_t op,
                    VariableName x,
                    VariableNameOrNumber k,
                    analyzer::VariableFactory& vfac) {
    if (this->is_bottom())
      return;

    /// step 1: add x_old in the graph
    VariableName x_old = vfac[boost::lexical_cast< std::string >(x) + "_old"];
    VariableName x_old_succ = mk_succ_idx(x_old, vfac);
    this->_g.insert_vertex(x_old);
    this->_g.insert_vertex(x_old_succ);
    this->_succ_idx_map->set(x_old, x_old_succ);
    /// add some constraints in the scalar domain
    this->_scalar.assign(x_old, linear_expression_t(x));
    this->_scalar +=
        linear_constraint_t(variable_t(x_old_succ) == variable_t(x_old) + 1);
    boost::optional< VariableName > x_succ = this->get_succ_idx(x);
    if (x_succ)
      this->_scalar +=
          linear_constraint_t(variable_t(x_old_succ) == variable_t(*x_succ));
    /* { x_old = x, x_old+ = x+, x_old+ = x_old + 1} */
    this->reduce();

    /// step 2: abstract all incoming/outgoing edges of x
    this->abstract(x);

    /// step 3: update the graph with the scalar domain after applying x = x op
    /// k.
    this->_scalar.apply(op, x, x, k);
    if (x_succ) {
      this->_scalar -= *x_succ;
      this->_scalar +=
          linear_constraint_t(variable_t(*x_succ) == variable_t(x) + 1);
    }
    /* { x = x op k, x+ = x+1} */
    this->reduce();

    /// step 4: delete x_old
    this->_g -= x_old;
    this->_g -= x_old_succ;
    this->_succ_idx_map->operator-=(x_old);
    this->_scalar -= x_old;
    this->_scalar -= x_old_succ;

    // this->reduce();
  }

private:
  inline bool is_array_index(VariableName v) { return true; }

public:
  static array_graph_domain_t top() {
    return array_graph_domain(ScalarNumDomain::top(),
                              array_graph_t::top(),
                              succ_index_map_ptr(new succ_index_map_t()));
  }

  static array_graph_domain_t bottom() {
    return array_graph_domain(ScalarNumDomain::bottom(),
                              array_graph_t::bottom(),
                              succ_index_map_ptr(new succ_index_map_t()));
  }

private:
  void set_to_bottom() {
    this->_is_bottom = true;
    this->_scalar = ScalarNumDomain::bottom();
    this->_g = array_graph_t::bottom();
    this->_succ_idx_map->clear();
  }

  array_graph_domain(ScalarNumDomain scalar,
                     array_graph_t g,
                     succ_index_map_ptr map)
      : _is_bottom(false),
        _scalar(scalar),
        _g(g),
        _succ_idx_map(new succ_index_map_t(*map)) {
    if (this->_scalar.is_bottom() || this->_g.is_bottom())
      set_to_bottom();
    else
      this->reduce();
  }

public:
  array_graph_domain()
      : _is_bottom(false),
        _scalar(ScalarNumDomain::top()),
        _g(array_graph_t::top()),
        _succ_idx_map(new succ_index_map_t()) {}

  array_graph_domain(const array_graph_domain_t& other)
      : _is_bottom(other._is_bottom),
        _scalar(other._scalar),
        _g(other._g),
        _succ_idx_map(new succ_index_map_t(*other._succ_idx_map)) {}

  array_graph_domain_t& operator=(array_graph_domain_t other) {
    if (this != &other) {
      this->_is_bottom = other._is_bottom;
      this->_scalar = other._scalar;
      this->_g = other._g;
      this->_succ_idx_map = other._succ_idx_map;
    }
    return *this;
  }

  bool is_bottom() { return this->_is_bottom; }

  bool is_top() { return (this->_scalar.is_top() || this->_g.is_top()); }

  void reduce() {
    if (this->_is_bottom)
      return;
    num_domain_traits::normalize(this->_scalar);
    if (this->_scalar.is_bottom() || this->_g.is_bottom())
      set_to_bottom();
    else
      this->_g.reduce(this->_scalar);
  }

  bool operator<=(array_graph_domain_t other) {
    if (this->_is_bottom) {
      return true;
    } else if (other._is_bottom) {
      return false;
    } else {
      return (this->_scalar <= other._scalar && this->_g <= other._g);
    }
  }

  array_graph_domain_t operator|(array_graph_domain_t other) {
    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      succ_index_map_ptr map(new succ_index_map_t(*(this->_succ_idx_map) |
                                                  *(other._succ_idx_map)));
      return array_graph_domain_t(this->_scalar | other._scalar,
                                  this->_g | other._g,
                                  map);
    }
  }

  array_graph_domain_t operator&(array_graph_domain_t other) {
    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      succ_index_map_ptr map(new succ_index_map_t(*(this->_succ_idx_map) |
                                                  *(other._succ_idx_map)));
      return array_graph_domain_t(this->_scalar & other._scalar,
                                  this->_g & other._g,
                                  map);
    }
  }

  array_graph_domain_t operator||(array_graph_domain_t other) {
    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      // after widening the relationships between i and i+ might be
      // also weakened. If this is the case we need to restore them.
      //
      // ScalarNumDomain scalar(this->_scalar | other._scalar) ;
      // std::cout << "before forcing constraints: " << scalar << std::endl;
      // for (typename succ_index_map_t::iterator it =
      // this->_succ_idx_map->begin();
      //      it!= this->_succ_idx_map->end();++it){
      //   VariableName v      = it->first.name();
      //   VariableName v_succ = it->second;
      //   //scalar.apply(OP_ADDITION, v_succ, v, Number(1));
      //   //scalar -= v_succ;
      //   scalar += linear_constraint_t( variable_t(v_succ) == variable_t(v) +
      //   1);
      //   std::cout << "after widening forcing " << v_succ << " = " << v << " +
      //   1"
      //   << std::endl;
      //   std::cout << scalar << std::endl;
      // }
      succ_index_map_ptr map(new succ_index_map_t(*(this->_succ_idx_map) |
                                                  *(other._succ_idx_map)));
      array_graph_domain_t widen(this->_scalar || other._scalar,
                                 this->_g || other._g,
                                 map);
#ifdef DEBUG
      std::cout << "WIDENING: " << *this << std::endl;
#endif
      return widen;
    }
  }

  array_graph_domain_t operator&&(array_graph_domain_t other) {
    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      succ_index_map_ptr map(new succ_index_map_t(*(this->_succ_idx_map) |
                                                  *(other._succ_idx_map)));
      return array_graph_domain_t(this->_scalar && other._scalar,
                                  this->_g && other._g,
                                  map);
    }
  }

  void operator-=(VariableName var) {
    if (this->is_bottom())
      return;
    // We assume that a variable is either an index in the graph or it
    // is in the weight but not both.
    if (is_array_index(var)) {
      this->_scalar -= var;
      this->_g -= var;
      boost::optional< VariableName > var_succ = get_succ_idx(var);
      if (var_succ) {
        this->_scalar -= *var_succ;
        this->_g -= *var_succ;
        this->_succ_idx_map->operator-=(var);
      }
    } else {
      typename array_graph_t::edge_iterator it, et;
      for (boost::tie(it, et) = boost::edges(*((this->_g)._graph)); it != et;
           ++it) {
        typename array_graph_t::edge_descriptor_t e = *it;
        typename array_graph_t::WeightPtr weight =
            (*((this->_g)._graph))[e].weight;
        (*weight) -= var;
      }
    }
    // this->reduce();
  }

  // model array reads: return the weight from the edge i to i+
  Weight operator[](VariableName i) {
    if (this->is_bottom()) {
      return Weight::bottom();
    }
    if (!is_array_index(i)) {
      return Weight::top();
    }
    // this->reduce();
    boost::optional< VariableName > i_succ = get_succ_idx(i);
    if (i_succ)
      return this->_g.get_weight(i, *i_succ);
    else {
      std::cout << *this << std::endl;
      std::ostringstream err;
      err << "There is no successor index associated with " << i << std::endl;
      throw logic_error(err.str());
    }
  }

  /////
  // Transfer functions
  /////

  void assertion(linear_constraint_system_t csts,
                 analyzer::VariableFactory& vfac) {
    if (this->is_bottom())
      return;

    // graph domain: make sure that all the relevant variables
    // (included special "0") are inserted in the graph
    for (typename linear_constraint_system_t::iterator it = csts.begin();
         it != csts.end();
         ++it) {
      linear_constraint_t cst = *it;
      Number n = cst.expression().constant();
      if (n == 0)
        add_variable(n, vfac);
      typename linear_expression_t::variable_set_t vars = cst.variables();
      for (typename linear_expression_t::variable_set_t::iterator vit =
               vars.begin();
           vit != vars.end();
           ++vit) {
        add_variable((*vit).name(), vfac);
      }
    }
    this->_scalar += csts;

    this->reduce();
#ifdef DEBUG
    std::cout << "ASSERT " << csts << " --- " << *this << std::endl;
#endif
  }

  void assign(VariableName x,
              linear_expression_t e,
              analyzer::VariableFactory& vfac) {
    if (this->is_bottom())
      return;
    if (boost::optional< variable_t > y = e.get_variable()) {
      if ((*y).name() == x)
        return;
    }
    // scalar domain
    this->_scalar.assign(x, e);

    // graph domain
    if (e.is_constant() && (e.constant() == 0))
      this->add_variable(e.constant(), vfac);
    if (this->_g.find_vertex_map(x)) {
      this->abstract(x);
      // wrong results if we do not restore the relationship between x
      // and x+ in the scalar domain
      boost::optional< VariableName > x_succ = this->get_succ_idx(x);
      if (x_succ) {
        this->_scalar -= *x_succ;
        this->_scalar +=
            linear_constraint_t(variable_t(*x_succ) == variable_t(x) + 1);
      }
    } else
      this->add_variable(x, vfac);

    this->reduce();
#ifdef DEBUG
    std::cout << "ASSIGN " << x << " := " << e << " --- " << *this << std::endl;
#endif
  }

  void apply(operation_t op,
             VariableName x,
             VariableName y,
             Number z,
             analyzer::VariableFactory& vfac) {
    this->assign(x, linear_expression_t(y), vfac);
    apply_helper< Number >(op, x, z, vfac);

#ifdef DEBUG
    std::cout << "APPLY " << x << " := " << y << " " << op << " " << z
              << " --- " << *this << std::endl;
#endif
  }

  void apply(operation_t op,
             VariableName x,
             VariableName y,
             VariableName z,
             analyzer::VariableFactory& vfac) {
    this->assign(x, linear_expression_t(y), vfac);
    apply_helper< VariableName >(op, x, z, vfac);
#ifdef DEBUG
    std::cout << "APPLY " << x << " := " << y << " " << op << " " << z
              << " --- " << *this << std::endl;
#endif
  }

  void apply(operation_t op,
             VariableName x,
             Number k,
             analyzer::VariableFactory& vfac) {
    apply_helper< Number >(op, x, k, vfac);
#ifdef DEBUG
    std::cout << "APPLY " << x << " := " << x << " " << op << " " << k
              << " --- " << *this << std::endl;
#endif
  }

  // model array writes
  void store(VariableName i, Weight w) {
    if (this->is_bottom())
      return;
    // this->reduce();
    // strong update
    boost::optional< VariableName > i_succ = get_succ_idx(i);
    if (i_succ) {
      Weight old_w = this->_g.get_weight(i, *i_succ);
      if ((old_w | w).is_top()) {
        // If the new weight and the previous one are completely
        // "separate" we meet them in order to keep both. Otherwise,
        // we throw away the old one and keep the new. We do this for
        // cases where we have consecutive assignments to different
        // arrays.
        this->_g.meet_weight(i, *i_succ, w);
      } else
        this->_g.update_weight(i, *i_succ, w);
    } else {
      std::cout << *this << std::endl;
      std::ostringstream err;
      err << "There is no successor index associated with " << i << std::endl;
      throw logic_error(err.str());
    }
    w = this->_g.get_weight(i, *i_succ);
    // weak update:
    // An edge (p,q) must be weakened if p <= i <= q and p < q
    typename array_graph_t::edge_iterator it, et;
    for (boost::tie(it, et) = boost::edges(*this->_g._graph); it != et; ++it) {
      typename array_graph_t::edge_descriptor_t e = *it;
      typename array_graph_t::VertexNamePtr p =
          (*this->_g._graph)[source(e, *this->_g._graph)].name;
      typename array_graph_t::VertexNamePtr q =
          (*this->_g._graph)[target(e, *this->_g._graph)].name;
      typename array_graph_t::WeightPtr weight = (*this->_g._graph)[e].weight;
      if (((*p == i) && (*q == *i_succ)) || weight->is_bottom())
        continue;
      // p < q
      ScalarNumDomain tmp(this->_scalar);
      tmp += linear_constraint_t(variable_t(*p) <= variable_t(i));
      tmp += linear_constraint_t(variable_t(*i_succ) <= variable_t(*q));
      if (tmp.is_bottom())
        continue;
      // p <= i <= q and p < q
      typename array_graph_t::binary_join_op join;
      (*this->_g._graph)[e].weight =
          join(weight, typename array_graph_t::WeightPtr(new Weight(w)));
    }
    this->_g.canonical();
#ifdef DEBUG
    std::cout << "ARRAY WRITE at " << i << " with weight " << w << " --- "
              << *this << std::endl;
#endif
  }

  void write(std::ostream& o) {
    o << "(";
#if 1
    // less verbose: remove the special variables i+ from the scalar
    // domain
    ScalarNumDomain inv(this->_scalar);
    for (typename succ_index_map_t::iterator it = this->_succ_idx_map->begin();
         it != this->_succ_idx_map->end();
         ++it) {
      inv -= it->second;
    }
    o << inv;
#else
    o << this->_scalar;
#endif
    o << "," << this->_g;
    o << ")";
  }

  static std::string domain_name() {
    return "Array Graph of " + ScalarNumDomain::domain_name();
  }

}; // end class array_graph_domain

} // end namespace ikos

#endif // ANALYZER_ARRAY_GRAPH_HPP
