###############################################################################
#
# Helpers for arguments
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2019 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
# Disclaimers:
#
# No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
# ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
# TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
# ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
# ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
# THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
# ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
# RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
# RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
# DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
# IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
#
# Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
# THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
# AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
# IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
# USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
# RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
# HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
# AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
# RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
# UNILATERAL TERMINATION OF THIS AGREEMENT.
#
###############################################################################
import argparse

from ikos import settings


def help(title, choices, default=None):
    ''' Build a help message for an argument '''
    maxlen = max(len(opt) for opt, _ in choices)

    s = title
    for opt, description in choices:
        s += '\n  %s  - %s' % (opt.ljust(maxlen), description)
        if opt == default:
            s += ' (default)'

    if isinstance(default, (list, tuple)):
        s += '\n(default: %s)' % ', '.join(default)

    return s


def choices(choices):
    ''' Build the tuple of choices '''
    return list(opt for opt, _ in choices)


def parse_argument(parser, name, choices, groups, default, value):
    '''
    Parse an argument with commas

    >>> choices = (('a', 'desc'), ('b','desc'), ('c', 'desc'), ('d', 'desc'))
    >>> default = ('a', 'b', 'c')
    >>> parse_argument(p, 'name', choices, None, default, None)
    set(['a', 'b', 'c'])
    >>> parse_argument(p, 'name', choices, None, default, 'a,b')
    set(['a', 'b'])
    >>> parse_argument(p, 'name', choices, None, default, '*,-a,-c')
    set(['b', 'd'])
    '''
    if not value:
        value = default

    if isinstance(value, (list, tuple)):
        value = ','.join(value)

    choices = set(choice[0] for choice in choices)
    choices.discard('*')  # prevent a bug

    result = set()
    for param in value.split(','):
        param = param.strip()
        if not param:
            pass

        # Parse +/-
        qualifier = '+'
        if param[0] == '+':
            param = param[1:]
        elif param[0] == '-':
            qualifier = '-'
            param = param[1:]

        param = param.strip()
        if not param:
            pass

        param_set = set()
        if param == '*':
            param_set = choices
        elif param in choices:
            param_set = {param}
        elif groups and param in groups:
            param_set = set(groups[param])
        else:
            parser.error("argument --%s: invalid choice: '%s'" % (name, param))

        if qualifier == '+':
            result.update(param_set)
        elif qualifier == '-':
            result.difference_update(param_set)

    return result


class VersionAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        print('ikos %s' % settings.VERSION)
        print('Copyright (c) 2011-2023 United States Government as represented'
              ' by the')
        print('Administrator of the National Aeronautics and Space '
              'Administration.')
        print('All Rights Reserved.')
        parser.exit()


def Integer(min=None, max=None):
    def parser(string):
        try:
            value = int(string)
        except ValueError:
            raise argparse.ArgumentTypeError(
                "invalid integer value: '%s'" % string
            )

        if min is not None and value < min:
            raise argparse.ArgumentTypeError(
                "invalid integer value: %d < %d" % (value, min)
            )

        if max is not None and value > max:
            raise argparse.ArgumentTypeError(
                "invalid integer value: %d > %d" % (value, max)
            )

        return value

    return parser


# Analysis options choices

analyses = (
    ('boa', 'Buffer Overflow Analysis'),
    ('dbz', 'Division by Zero Analysis'),
    ('nullity', 'Null Pointer Dereference Analysis'),
    ('prover', 'Assertion Prover'),
    ('upa', 'Unaligned Pointer Analysis'),
    ('uva', 'Uninitialized Variables Analysis'),
    ('sio', 'Signed Integer Overflow Analysis'),
    ('uio', 'Unsigned Integer Overflow Analysis'),
    ('shc', 'Shift Count Analysis'),
    ('poa', 'Pointer Overflow Analysis'),
    ('pcmp', 'Pointer Comparison Analysis'),
    ('sound', 'Soundness Analysis'),
    ('fca', 'Function Call Analysis'),
    ('dca', 'Dead Code Analysis'),
    ('dfa', 'Double Free Analysis'),
    ('dbg', 'Debugger'),
    ('watch', 'Memory Watcher'),
)

default_analyses = (
    'boa',
    'dbz',
    'nullity',
    'prover',
    'uva',
    'sio',
    'shc',
    'pcmp',
    'sound',
    'fca',
    'dca',
    'dfa',
    'dbg',
)


domains = (
    ('interval',
     'Interval domain'),
    ('congruence',
     'Congruence domain'),
    ('interval-congruence',
     'Reduced product of Interval and Congruence'),
    ('dbm',
     'Difference-Bound Matrices domain'),
    ('var-pack-dbm',
     'Difference-Bound Matrices domain with variable packing'),
    ('var-pack-dbm-congruence',
     'Reduced product of DBM with variable packing and Congruence'),
    ('gauge',
     'Gauge domain'),
    ('gauge-interval-congruence',
     'Reduced product of Gauge, Interval and Congruence'),
    ('apron-interval',
     'APRON Interval domain'),
    ('apron-octagon',
     'APRON Octagon domain'),
    ('apron-polka-polyhedra',
     'APRON Polka Polyhedra domain'),
    ('apron-polka-linear-equalities',
     'APRON Polka Linear Equalities domain'),
    ('apron-ppl-polyhedra',
     'APRON PPL Polyhedra domain'),
    ('apron-ppl-linear-congruences',
     'APRON PPL Linear Congruences domain'),
    ('apron-pkgrid-polyhedra-lin-cong',
     'APRON Pkgrid Polyhedra and Linear Congruences domain'),
    ('var-pack-apron-octagon',
     'APRON Octagon domain with variable packing'),
    ('var-pack-apron-polka-polyhedra',
     'APRON Polka Polyhedra domain with variable packing'),
    ('var-pack-apron-polka-linear-equalities',
     'APRON Polka Linear Equalities domain with variable packing'),
    ('var-pack-apron-ppl-polyhedra',
     'APRON PPL Polyhedra domain with variable packing'),
    ('var-pack-apron-ppl-linear-congruences',
     'APRON PPL Linear Congruences domain with variable packing'),
    ('var-pack-apron-pkgrid-polyhedra-lin-cong',
     'APRON Pkgrid Polyhedra and Linear Congruences domain'
     ' with variable packing'),
)

default_domain = 'interval'

globals_init_policies = (
    ('all', 'Initialize all global variables'),
    ('skip-big-arrays', 'Initialize all global variables except big arrays'),
    ('skip-strings', 'Initialize all global variables except strings'),
    ('none', 'Do not initialize any global variable'),
)

default_globals_init_policy = 'skip-big-arrays'

proceduralities = (
    ('inter', 'Interprocedural analysis'),
    ('intra', 'Intraprocedural analysis'),
)

default_procedurality = 'inter'

widening_strategies = (
    ('widen', 'Widening operator'),
    ('join', 'Join operator'),
)

default_widening_strategy = 'widen'

narrowing_strategies = (
    ('narrow', 'Narrowing operator'),
    ('meet', 'Meet operator'),
    ('auto', 'Narrowing if available, otherwise meet'),
)

default_narrowing_strategy = 'auto'

default_widening_delay = 1

default_widening_period = 1

meet_iterations_if_no_narrowing = 2

partitioning_strategies = (
    ('return', 'Partition the states on function return values'),
    ('manual', 'Partition the states manually'),
    ('no', 'Disable partitioning'),
)

default_partitioning_strategy = 'no'

# Preprocessing options choices

opt_levels = (
    ('none', 'Disable all optimizations'),
    ('basic', 'Basic set of optimizations'),
    ('aggressive', 'Aggressive optimizations (not recommended)'),
)

default_opt_level = 'basic'

# Debug options choices

display_checks_choices = (
    ('all', 'Display all checks'),
    ('fail', 'Display only failed checks'),
    ('no', 'Do not display checks'),
)

display_inv_choices = (
    ('all', 'Display all invariants'),
    ('fail', 'Display invariants for failed checks'),
    ('no', 'Do not display invariants'),
)

# Misc. options choices

color_choices = (
    ('yes', 'Enable colors'),
    ('auto', 'Enable colors if the output is a terminal'),
    ('no', 'Disable colors'),
)

default_color = 'auto'

log_levels = (
    ('none', 'Disable logging'),
    ('critical', 'Critical level'),
    ('error', 'Error level'),
    ('warning', 'Warning level'),
    ('info', 'Informative level'),
    ('debug', 'Debug level'),
    ('all', 'Show all messages'),
)

default_log_level = 'info'

progress_choices = (
    ('auto', 'Interactive if the output is a terminal'),
    ('interactive', 'Interactive'),
    ('linear', 'Linear'),
    ('no', 'Disable progress report'),
)

default_progress = 'auto'

# Report options choices

display_times_choices = (
    ('full', 'Display all timing results'),
    ('short', 'Display most meaningful timing results'),
    ('no', 'Do not display any timing results'),
)

display_summary_choices = (
    ('full', 'Display the complete analysis summary'),
    ('short', 'Display a shorter analysis summary'),
    ('no', 'Do not display the analysis summary'),
)

report_formats = (
    ('auto', 'Generate a text report, if less than 15 entries'),
    ('text', 'Generate a text report'),
    ('json', 'Generate a json report'),
    ('sarif', 'Generate a sarif report'),
    ('junit', 'Generate a JUnit.xml report'),
    ('csv', 'Generate a csv report'),
    ('web', 'Generate a web report (ikos-view)'),
    ('no', 'Do not generate a report'),
)

status_filters = (
    ('*', 'All'),
    ('error', 'Error'),
    ('warning', 'Warning'),
    ('safe', 'Safe'),
    ('unreachable', 'Unreachable'),
)

default_status_filter = ('error', 'warning', 'unreachable')
