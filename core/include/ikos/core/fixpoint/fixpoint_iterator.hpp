/*******************************************************************************
 *
 * \file
 * \brief Generic API for fixpoint iterators.
 *
 * Author: Arnaud J. Venet
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#pragma once

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/semantic/graph.hpp>

namespace ikos {
namespace core {

/// \brief Base class for forward fixpoint iterators
template < typename GraphRef,
           typename AbstractValue,
           typename GraphTrait = GraphTraits< GraphRef > >
class ForwardFixpointIterator {
public:
  static_assert(IsGraph< GraphRef, GraphTrait >::value,
                "GraphRef does not implement GraphTraits");
  static_assert(IsAbstractDomain< AbstractValue >::value,
                "AbstractValue does not implement AbstractDomain");

public:
  /// \brief Reference to a node of the graph
  using NodeRef = typename GraphTrait::NodeRef;

public:
  /// \brief Default constructor
  ForwardFixpointIterator() = default;

  /// \brief Copy constructor
  ForwardFixpointIterator(const ForwardFixpointIterator&) noexcept = default;

  /// \brief Move constructor
  ForwardFixpointIterator(ForwardFixpointIterator&&) noexcept = default;

  /// \brief Copy assignment operator
  ForwardFixpointIterator& operator=(const ForwardFixpointIterator&) noexcept =
      default;

  /// \brief Move assignment operator
  ForwardFixpointIterator& operator=(ForwardFixpointIterator&&) noexcept =
      default;

  /// \brief Return the control flow graph
  virtual GraphRef cfg() const = 0;

  /// \brief Return the pre abstract value for the given node
  virtual const AbstractValue& pre(NodeRef node) const = 0;

  /// \brief Return the post invariant for the given node
  virtual const AbstractValue& post(NodeRef node) const = 0;

  /// \brief Compute the fixpoint with the given initial abstract value
  virtual void run(AbstractValue init) = 0;

  /// \brief Return true if the fixpoint is reached
  virtual bool converged() const = 0;

  /// \brief Clear the current fixpoint
  virtual void clear() = 0;

  /// \brief Semantic transformer for a node
  ///
  /// This method is called with the abstract value representing the state
  /// of the program upon entering the node. The method should return an
  /// abstract value representing the state of the program after the node.
  virtual AbstractValue analyze_node(NodeRef node, AbstractValue state) = 0;

  /// \brief Semantic transformer for an edge
  ///
  /// This method is called with the abstract value representing the state of
  /// the program after exiting the source node. The method should return an
  /// abstract value representing the state of the program after the edge, at
  /// the entry of the destination node.
  virtual AbstractValue analyze_edge(NodeRef src,
                                     NodeRef dest,
                                     AbstractValue state) = 0;

  /// \brief Process the computed abstract value for a node
  ///
  /// This method is called when the fixpoint is reached, and with the abstract
  /// value representing the state of the program upon entering the node.
  virtual void process_pre(NodeRef, const AbstractValue&) = 0;

  /// \brief Process the computed abstract value for a node
  ///
  /// This method is called when the fixpoint is reached, and with the abstract
  /// value representing the state of the program after the node.
  virtual void process_post(NodeRef, const AbstractValue&) = 0;

  /// \brief Destructor
  virtual ~ForwardFixpointIterator() = default;

}; // end class ForwardFixpointIterator

/// \brief Kind of fixpoint iteration
enum class FixpointIterationKind { Increasing, Decreasing };

} // end namespace core
} // end namespace ikos
