/*******************************************************************************
 *
 * \file
 * \brief Generic API for control-flow graphs.
 *
 * Author: Maxime Arthaud
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

#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {

/// \brief Traits for graphs
///
/// This class should be specialized for all graph types.
///
/// Elements to provide:
///
/// NodeRef - Type of a reference to a node in the graph,
//            should be cheap to copy,
//            should be comparable, ie. define bool NodeRef::operator==(NodeRef)
///
/// SuccessorNodeIterator - Type returned by successor_begin(NodeRef),
///                         dereference to a NodeRef
/// PredecessorNodeIterator - Type returned by predecessor_begin(NodeRef)
///                           dereference to a NodeRef
///
/// static NodeRef entry(GraphRef)
///   Return the entry node of the graph
///
/// static SuccessorNodeIterator successor_begin(NodeRef)
/// static SuccessorNodeIterator successor_end(NodeRef)
///   Return iterators over the successors of the given node
///
/// static PredecessorNodeIterator predecessor_begin(NodeRef)
/// static PredecessorNodeIterator predecessor_end(NodeRef)
///   Return iterators over the predecessors of the given node
///
/// The GraphRef type should also be cheap to copy
template < typename GraphRef >
struct GraphTraits {};

/// \brief Check if a type implements GraphTraits
template < typename GraphRef,
           typename GraphTrait = GraphTraits< GraphRef >,
           typename = void >
struct IsGraph : std::false_type {};

template < typename GraphRef, typename GraphTrait >
struct IsGraph<
    GraphRef,
    GraphTrait,
    void_t<
        // GraphTrait has NodeRef
        typename GraphTrait::NodeRef,
        // GraphTrait::NodeRef has: operator==(NodeRef) -> bool
        decltype(std::declval< typename GraphTrait::NodeRef >() ==
                 std::declval< typename GraphTrait::NodeRef >()),
        // GraphTrait has SuccessorNodeIterator
        typename GraphTrait::SuccessorNodeIterator,
        // SuccessorNodeIterator dereferences to NodeRef
        std::enable_if_t< std::is_same<
            typename std::iterator_traits<
                typename GraphTrait::SuccessorNodeIterator >::value_type,
            typename GraphTrait::NodeRef >::value >,
        // GraphTrait has PredecessorNodeIterator
        typename GraphTrait::PredecessorNodeIterator,
        // PredecessorNodeIterator dereferences to NodeRef
        std::enable_if_t< std::is_same<
            typename std::iterator_traits<
                typename GraphTrait::PredecessorNodeIterator >::value_type,
            typename GraphTrait::NodeRef >::value >,
        // GraphTrait has: entry(GraphRef) -> NodeRef
        std::enable_if_t< std::is_same< decltype(GraphTrait::entry(
                                            std::declval< GraphRef >())),
                                        typename GraphTrait::NodeRef >::value >,
        // GraphTrait has: successor_begin(NodeRef) -> SuccessorNodeIterator
        std::enable_if_t<
            std::is_same< decltype(GraphTrait::successor_begin(
                              std::declval< typename GraphTrait::NodeRef >())),
                          typename GraphTrait::SuccessorNodeIterator >::value >,
        // GraphTrait has: successor_end(NodeRef) -> SuccessorNodeIterator
        std::enable_if_t<
            std::is_same< decltype(GraphTrait::successor_end(
                              std::declval< typename GraphTrait::NodeRef >())),
                          typename GraphTrait::SuccessorNodeIterator >::value >,
        // GraphTrait has: predecessor_begin(NodeRef) -> PredecessorNodeIterator
        std::enable_if_t< std::is_same<
            decltype(GraphTrait::predecessor_begin(
                std::declval< typename GraphTrait::NodeRef >())),
            typename GraphTrait::PredecessorNodeIterator >::value >,
        // GraphTrait has: predecessor_end(NodeRef) -> PredecessorNodeIterator
        std::enable_if_t< std::is_same<
            decltype(GraphTrait::predecessor_end(
                std::declval< typename GraphTrait::NodeRef >())),
            typename GraphTrait::PredecessorNodeIterator >::value > > >
    : std::true_type {};

} // end namespace core
} // end namespace ikos
