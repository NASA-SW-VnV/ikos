/*******************************************************************************
 *
 * \file
 * \brief Machine integer abstract domain used by the analyses
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <ikos/core/domain/machine_int/polymorphic_domain.hpp>

#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/analysis/variable.hpp>

namespace ikos {
namespace analyzer {
namespace value {

/// \brief Machine integer abstract domain used for the value analysis
using MachineIntAbstractDomain =
    core::machine_int::PolymorphicDomain< Variable* >;

/// \name Constructors of machine integer abstract domains
/// @{

MachineIntAbstractDomain make_top_machine_int_interval();
MachineIntAbstractDomain make_top_machine_int_congruence();
MachineIntAbstractDomain make_top_machine_int_interval_congruence();
MachineIntAbstractDomain make_top_machine_int_dbm();
MachineIntAbstractDomain make_top_machine_int_var_pack_dbm();
MachineIntAbstractDomain make_top_machine_int_var_pack_dbm_congruence();
MachineIntAbstractDomain make_top_machine_int_gauge();
MachineIntAbstractDomain make_top_machine_int_gauge_interval_congruence();
MachineIntAbstractDomain make_top_machine_int_apron_interval();
MachineIntAbstractDomain make_top_machine_int_apron_octagon();
MachineIntAbstractDomain make_top_machine_int_apron_polka_polyhedra();
MachineIntAbstractDomain make_top_machine_int_apron_polka_linear_equalities();
MachineIntAbstractDomain make_top_machine_int_apron_ppl_polyhedra();
MachineIntAbstractDomain make_top_machine_int_apron_ppl_linear_congruences();
MachineIntAbstractDomain make_top_machine_int_apron_pkgrid_polyhedra_lin_cong();
MachineIntAbstractDomain make_top_machine_int_var_pack_apron_octagon();
MachineIntAbstractDomain make_top_machine_int_var_pack_apron_polka_polyhedra();
MachineIntAbstractDomain
make_top_machine_int_var_pack_apron_polka_linear_equalities();
MachineIntAbstractDomain make_top_machine_int_var_pack_apron_ppl_polyhedra();
MachineIntAbstractDomain
make_top_machine_int_var_pack_apron_ppl_linear_congruences();
MachineIntAbstractDomain
make_top_machine_int_var_pack_apron_pkgrid_polyhedra_lin_cong();

/// @}

/// \brief Create the top machine integer domain of the given choice
inline MachineIntAbstractDomain make_top_machine_int_domain(
    MachineIntDomainOption d) {
  switch (d) {
    case MachineIntDomainOption::Interval:
      return make_top_machine_int_interval();
    case MachineIntDomainOption::Congruence:
      return make_top_machine_int_congruence();
    case MachineIntDomainOption::IntervalCongruence:
      return make_top_machine_int_interval_congruence();
    case MachineIntDomainOption::DBM:
      return make_top_machine_int_dbm();
    case MachineIntDomainOption::VarPackDBM:
      return make_top_machine_int_var_pack_dbm();
    case MachineIntDomainOption::VarPackDBMCongruence:
      return make_top_machine_int_var_pack_dbm_congruence();
    case MachineIntDomainOption::Gauge:
      return make_top_machine_int_gauge();
    case MachineIntDomainOption::GaugeIntervalCongruence:
      return make_top_machine_int_gauge_interval_congruence();
    case MachineIntDomainOption::ApronInterval:
      return make_top_machine_int_apron_interval();
    case MachineIntDomainOption::ApronOctagon:
      return make_top_machine_int_apron_octagon();
    case MachineIntDomainOption::ApronPolkaPolyhedra:
      return make_top_machine_int_apron_polka_polyhedra();
    case MachineIntDomainOption::ApronPolkaLinearEqualities:
      return make_top_machine_int_apron_polka_linear_equalities();
    case MachineIntDomainOption::ApronPplPolyhedra:
      return make_top_machine_int_apron_ppl_polyhedra();
    case MachineIntDomainOption::ApronPplLinearCongruences:
      return make_top_machine_int_apron_ppl_linear_congruences();
    case MachineIntDomainOption::ApronPkgridPolyhedraLinearCongruences:
      return make_top_machine_int_apron_pkgrid_polyhedra_lin_cong();
    case MachineIntDomainOption::VarPackApronOctagon:
      return make_top_machine_int_var_pack_apron_octagon();
    case MachineIntDomainOption::VarPackApronPolkaPolyhedra:
      return make_top_machine_int_var_pack_apron_polka_polyhedra();
    case MachineIntDomainOption::VarPackApronPolkaLinearEqualities:
      return make_top_machine_int_var_pack_apron_polka_linear_equalities();
    case MachineIntDomainOption::VarPackApronPplPolyhedra:
      return make_top_machine_int_var_pack_apron_ppl_polyhedra();
    case MachineIntDomainOption::VarPackApronPplLinearCongruences:
      return make_top_machine_int_var_pack_apron_ppl_linear_congruences();
    case MachineIntDomainOption::VarPackApronPkgridPolyhedraLinearCongruences:
      return make_top_machine_int_var_pack_apron_pkgrid_polyhedra_lin_cong();
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

} // end namespace value
} // end namespace analyzer
} // end namespace ikos
