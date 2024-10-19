###############################################################################
#
# Handle ikos toolchain: running clang, ikos-pp and ikos-analyzer.
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
import atexit
import datetime
import json
import os
import os.path
import shlex
import shutil
import signal
import subprocess
import sys
import tempfile
import threading

from ikos import args
from ikos import colors
from ikos import log
from ikos import report
from ikos import settings
from ikos import stats
from ikos.log import printf
from ikos.output_db import OutputDatabase


def parse_arguments(argv):
    usage = '%(prog)s [options] file[.c|.cpp|.bc|.ll]'
    description = 'ikos static analyzer'
    formatter_class = argparse.RawTextHelpFormatter
    parser = argparse.ArgumentParser(usage=usage,
                                     description=description,
                                     formatter_class=formatter_class)

    # Positional arguments
    parser.add_argument('file',
                        metavar='file[.c|.cpp|.bc|.ll]',
                        help='File to analyze')

    # Optional arguments
    parser.add_argument('-o', '--output-db',
                        dest='output_db',
                        metavar='<file>',
                        help='Output database file (default: output.db)',
                        default='output.db')
    parser.add_argument('-v',
                        dest='verbosity',
                        help='Increase verbosity',
                        action='count',
                        default=1)
    parser.add_argument('-q',
                        dest='verbosity',
                        help='Be quiet',
                        action='store_const',
                        const=0)
    parser.add_argument('--version',
                        action=args.VersionAction,
                        nargs=0,
                        help='Show ikos version')

    # Analysis options
    analysis = parser.add_argument_group('Analysis Options')
    analysis.add_argument('-a', '--analyses',
                          dest='analyses',
                          metavar='',
                          help=args.help('Available analyses:',
                                         args.analyses,
                                         args.default_analyses),
                          action='append')
    analysis.add_argument('-d', '--domain',
                          dest='domain',
                          metavar='',
                          help=args.help('Available abstract domains:',
                                         args.domains,
                                         args.default_domain),
                          choices=args.choices(args.domains),
                          default=args.default_domain)
    analysis.add_argument('-e', '--entry-points',
                          dest='entry_points',
                          metavar='<function>',
                          help='List of program entry points (default: main)',
                          action='append')
    analysis.add_argument('--globals-init',
                          dest='globals_init',
                          metavar='',
                          help=args.help(
                              'Policy of initialization for global variables:',
                              args.globals_init_policies,
                              args.default_globals_init_policy),
                          choices=args.choices(args.globals_init_policies),
                          default=args.default_globals_init_policy)
    analysis.add_argument('--no-init-globals',
                          dest='no_init_globals',
                          metavar='<function>',
                          help='Do not initialize global variables for the '
                               'given entry points',
                          action='append')
    analysis.add_argument('--no-liveness',
                          dest='no_liveness',
                          help='Disable the liveness analysis',
                          action='store_true',
                          default=False)
    analysis.add_argument('--no-pointer',
                          dest='no_pointer',
                          help='Disable the pointer analysis',
                          action='store_true',
                          default=False)
    analysis.add_argument('--no-widening-hints',
                          dest='no_widening_hints',
                          help='Disable the widening hint analysis',
                          action='store_true',
                          default=False)
    analysis.add_argument('--no-fixpoint-cache',
                          dest='no_fixpoint_cache',
                          help='Disable the cache of fixpoints',
                          action='store_true',
                          default=False)
    analysis.add_argument('--no-checks',
                          dest='no_checks',
                          help='Disable all the checks',
                          action='store_true',
                          default=False)
    analysis.add_argument('--proc',
                          dest='procedural',
                          metavar='',
                          help=args.help('Procedural:',
                                         args.proceduralities,
                                         args.default_procedurality),
                          choices=args.choices(args.proceduralities),
                          default=args.default_procedurality)
    analysis.add_argument('-j', '--jobs',
                          dest='jobs',
                          metavar='',
                          nargs='?',
                          help='Number of threads',
                          type=int,
                          const=0,
                          default=1)
    analysis.add_argument('--widening-strategy',
                          dest='widening_strategy',
                          metavar='',
                          help=args.help('Strategy for increasing iterations:',
                                         args.widening_strategies,
                                         args.default_widening_strategy),
                          choices=args.choices(args.widening_strategies),
                          default=args.default_widening_strategy)
    analysis.add_argument('--narrowing-strategy',
                          dest='narrowing_strategy',
                          metavar='',
                          help=args.help('Strategy for decreasing iterations:',
                                         args.narrowing_strategies,
                                         args.default_narrowing_strategy),
                          choices=args.choices(args.narrowing_strategies),
                          default=args.default_narrowing_strategy)
    analysis.add_argument('--widening-delay',
                          dest='widening_delay',
                          metavar='',
                          help='Number of loop iterations before using the'
                               ' widening strategy (default: %d)'
                               % args.default_widening_delay,
                          default=args.default_widening_delay,
                          type=args.Integer(min=0))
    analysis.add_argument('--widening-delay-functions',
                          dest='widening_delay_functions',
                          metavar='<function:int>',
                          help='Widening delay for specific functions',
                          action='append')
    analysis.add_argument('--widening-period',
                          dest='widening_period',
                          metavar='',
                          help='Number of loop iterations between each'
                               ' widening (default: %d)'
                               % args.default_widening_period,
                          default=args.default_widening_period,
                          type=args.Integer(min=1))
    analysis.add_argument('--narrowing-iterations',
                          dest='narrowing_iterations',
                          metavar='',
                          help='Perform a fixed number of narrowing'
                               ' iterations',
                          type=args.Integer(min=0))
    analysis.add_argument('--partitioning',
                          dest='partitioning',
                          metavar='',
                          help=args.help('Partitioning strategy:',
                                         args.partitioning_strategies,
                                         args.default_partitioning_strategy),
                          choices=args.choices(args.partitioning_strategies),
                          default=args.default_partitioning_strategy)
    analysis.add_argument('--hardware-addresses',
                          dest='hardware_addresses',
                          metavar='',
                          action='append',
                          help='Specify ranges (x-y) of hardware addresses'
                               ', separated by a comma')
    analysis.add_argument('--hardware-addresses-file',
                          dest='hardware_addresses_file',
                          metavar='',
                          help='Specify ranges (x-y) of hardware addresses'
                               ' from a file (one range per line)')
    analysis.add_argument('--argc',
                          dest='argc',
                          metavar='',
                          help='Specify a value for argc',
                          type=args.Integer(min=0))

    # Compile options
    compiler = parser.add_argument_group('Compile Options')
    compiler.add_argument('-I',
                          dest='compiler_include_flags',
                          metavar='',
                          help='Add the specified directory to the search '
                               'path for include files',
                          action='append')
    compiler.add_argument('-D',
                          dest='compiler_define_flags',
                          metavar='',
                          help='Add an implicit #define into the source file',
                          action='append')
    compiler.add_argument('-W',
                          dest='compiler_warning_flags',
                          metavar='',
                          help='Use the specified warning options',
                          action='append')
    compiler.add_argument('-w',
                          dest='compiler_disable_warnings',
                          help='Suppress all compiler warnings',
                          action='store_true',
                          default=False)
    compiler.add_argument('-m',
                          dest='compiler_machine_flags',
                          metavar='',
                          help='Use the specified machine options',
                          action='append')

    # Preprocessing options
    preprocess = parser.add_argument_group('Preprocessing Options')
    preprocess.add_argument('--opt',
                            dest='opt_level',
                            metavar='',
                            help=args.help('Optimization level:',
                                           args.opt_levels,
                                           args.default_opt_level),
                            choices=args.choices(args.opt_levels),
                            default=args.default_opt_level)
    preprocess.add_argument('--inline-all',
                            dest='inline_all',
                            help='Front-end inline all functions',
                            action='store_true',
                            default=False)
    preprocess.add_argument('--no-bc-verify',
                            dest='no_bc_verify',
                            help='Do not run the LLVM bitcode verifier',
                            action='store_true',
                            default=False)

    # Import options
    imports = parser.add_argument_group('Import Options')
    imports.add_argument('--no-libc',
                         dest='no_libc',
                         help='Do not use libc intrinsics '
                              '(malloc, free, etc.)',
                         action='store_true',
                         default=False)
    imports.add_argument('--no-libcpp',
                         dest='no_libcpp',
                         help='Do not use libcpp intrinsics '
                              '(__cxa_throw, etc.)',
                         action='store_true',
                         default=False)
    imports.add_argument('--no-libikos',
                         dest='no_libikos',
                         help='Do not use ikos intrinsics '
                              '(__ikos_assert, etc.)',
                         action='store_true',
                         default=False)

    # AR passes options
    passes = parser.add_argument_group('AR Passes Options')
    passes.add_argument('--no-type-check',
                        dest='no_type_check',
                        help='Do not run the AR type checker',
                        action='store_true',
                        default=False)
    passes.add_argument('--no-simplify-cfg',
                        dest='no_simplify_cfg',
                        help='Do not run the simplify-cfg pass',
                        action='store_true',
                        default=False)
    passes.add_argument('--no-simplify-upcast-comparison',
                        dest='no_simplify_upcast_comparison',
                        help='Do not run the simplify-upcast-comparison pass',
                        action='store_true',
                        default=False)

    # Debug options
    debug = parser.add_argument_group('Debug Options')
    debug.add_argument('--display-llvm',
                       dest='display_llvm',
                       help='Display the LLVM bitcode as text',
                       action='store_true',
                       default=False)
    debug.add_argument('--display-ar',
                       dest='display_ar',
                       help='Display the Abstract Representation as text',
                       action='store_true',
                       default=False)
    debug.add_argument('--trace-ar-stmts',
                       dest='trace_ar_stmts',
                       help='Trace analysis of ar statements',
                       action='store_true',
                       default=False)
    debug.add_argument('--display-liveness',
                       dest='display_liveness',
                       help='Display liveness analysis results',
                       action='store_true',
                       default=False)
    debug.add_argument('--display-function-pointer',
                       dest='display_function_pointer',
                       help='Display function pointer analysis results',
                       action='store_true',
                       default=False)
    debug.add_argument('--display-pointer',
                       dest='display_pointer',
                       help='Display pointer analysis results',
                       action='store_true',
                       default=False)
    debug.add_argument('--display-fixpoint-parameters',
                       dest='display_fixpoint_parameters',
                       help='Display fixpoint parameters',
                       action='store_true',
                       default=False)
    debug.add_argument('--display-checks',
                       dest='display_checks',
                       metavar='',
                       help=args.help('Display checks:',
                                      args.display_checks_choices,
                                      'no'),
                       choices=args.choices(args.display_checks_choices),
                       default='no')
    debug.add_argument('--display-inv',
                       dest='display_inv',
                       metavar='',
                       help=args.help('Display computed invariants:',
                                      args.display_inv_choices,
                                      'no'),
                       choices=args.choices(args.display_inv_choices),
                       default='no')
    debug.add_argument('--display-raw-checks',
                       dest='display_raw_checks',
                       help='Display analysis raw checks',
                       action='store_true',
                       default=False)
    debug.add_argument('--generate-dot',
                       dest='generate_dot',
                       help='Generate a .dot file for each function',
                       action='store_true',
                       default=False)
    debug.add_argument('--generate-dot-dir',
                       dest='generate_dot_dir',
                       metavar='<directory>',
                       help='Output directory for .dot files',
                       default=None)
    debug.add_argument('--save-temps',
                       dest='save_temps',
                       help='Do not delete temporary files',
                       action='store_true',
                       default=False)
    debug.add_argument('--temp-dir',
                       dest='temp_dir',
                       metavar='<directory>',
                       help='Temporary directory',
                       default=None)

    # Misc.
    misc = parser.add_argument_group('Miscellaneous')
    misc.add_argument('--rm-db',
                      dest='remove_db',
                      help='Remove the output database file after use',
                      action='store_true',
                      default=False)
    misc.add_argument('--color',
                      dest='color',
                      metavar='',
                      help=args.help('Enable terminal colors:',
                                     args.color_choices,
                                     args.default_color),
                      choices=args.choices(args.color_choices),
                      default=args.default_color)
    misc.add_argument('--log',
                      dest='log_level',
                      metavar='',
                      help=args.help('Log level:',
                                     args.log_levels,
                                     args.default_log_level),
                      choices=args.choices(args.log_levels),
                      default=None)
    misc.add_argument('--progress',
                      dest='progress',
                      metavar='',
                      help=args.help('Progress report:',
                                     args.progress_choices,
                                     args.default_progress),
                      choices=args.choices(args.progress_choices),
                      default=args.default_progress)

    # Report options
    report = parser.add_argument_group('Report Options')
    report.add_argument('--display-times',
                        dest='display_times',
                        metavar='',
                        help=args.help('Display timing results',
                                       args.display_times_choices,
                                       'short'),
                        choices=args.choices(args.display_times_choices),
                        default='short')
    report.add_argument('--display-summary',
                        dest='display_summary',
                        metavar='',
                        help=args.help('Display the analysis summary',
                                       args.display_summary_choices,
                                       'full'),
                        choices=args.choices(args.display_summary_choices),
                        default='full')
    report.add_argument('-f', '--format',
                        dest='format',
                        metavar='',
                        help=args.help('Available report formats:',
                                       args.report_formats,
                                       'auto'),
                        choices=args.choices(args.report_formats),
                        default='auto')
    report.add_argument('--report-file',
                        dest='report_file',
                        metavar='<file>',
                        help='Write the report into a file (default: stdout)',
                        default=sys.stdout,
                        type=argparse.FileType('w'))
    report.add_argument('--status-filter',
                        dest='status_filter',
                        metavar='',
                        help=args.help('Available status filters:',
                                       args.status_filters,
                                       args.default_status_filter),
                        action='append')
    report.add_argument('--report-verbosity',
                        dest='report_verbosity',
                        metavar='[1-4]',
                        help='Report verbosity (default: 1)',
                        type=args.Integer(min=1, max=4))

    # Resource options
    resource = parser.add_argument_group('Resources Options')
    resource.add_argument('--cpu',
                          dest='cpu',
                          help='CPU time limit (seconds)',
                          type=args.Integer(min=1))
    resource.add_argument('--mem',
                          dest='mem',
                          help='MEM limit (MB)',
                          type=args.Integer(min=1))

    opt = parser.parse_args(argv)

    # parse --analyses
    opt.analyses = args.parse_argument(parser,
                                       'analyses',
                                       choices=args.analyses,
                                       groups=None,
                                       default=args.default_analyses,
                                       value=opt.analyses)

    # by default, the entry point is main
    if not opt.entry_points:
        opt.entry_points = ('main',)

    # verbosity changes the log level, if --log is not specified
    if opt.log_level is None:
        if opt.verbosity <= 0:
            opt.log_level = 'error'
        elif opt.verbosity == 1:
            opt.log_level = 'info'
        elif opt.verbosity == 2:
            opt.log_level = 'debug'
        else:
            opt.log_level = 'all'

    # quiet mode does not display the timing results and the analysis summary
    if opt.verbosity <= 0:
        opt.display_times = 'no'
        opt.display_summary = 'no'

    # default value for generate-dot-dir
    if opt.generate_dot and not opt.generate_dot_dir:
        if opt.temp_dir and opt.save_temps:
            opt.generate_dot_dir = opt.temp_dir
        else:
            opt.generate_dot_dir = '.'

    # parse --status-filter
    opt.status_filter = args.parse_argument(parser,
                                            'status-filter',
                                            choices=args.status_filters,
                                            groups=None,
                                            default=args.default_status_filter,
                                            value=opt.status_filter)

    # verbosity changes the report verbosity level,
    # if --report-verbosity is not specified
    if opt.report_verbosity is None:
        opt.report_verbosity = max(opt.verbosity, 1)

    return opt


sh_quote = shlex.quote


def command_string(cmd):
    return ' '.join(map(sh_quote, cmd))


def path_ext(path):
    ''' Return the filename extension.

    >>> path_ext('test.c')
    '.c'
    >>> path_ext('/tmp/file.my.ext')
    '.ext'
    '''
    return os.path.splitext(path)[1]


c_extensions = ('.c', '.h', '.i')
cpp_extensions = ('.cpp', '.cc', '.cxx', '.cppm', '.c++', '.cp', '.C', '.CPP',
                  '.hpp', '.hh', '.hxx',
                  '.ii', '.iim')
llvm_extensions = ('.bc', '.ll')


def create_working_directory(wd=None, save=False):
    ''' Create a temporary working directory '''
    if not wd:
        wd = tempfile.mkdtemp(prefix='ikos-')

    if not os.path.exists(wd):
        try:
            os.makedirs(wd)
        except OSError as e:
            printf('error: %s: %s\n', wd, e.strerror, file=sys.stderr)
            sys.exit(1)

    if not os.path.isdir(wd):
        printf('error: %s: Not a directory\n', wd, file=sys.stderr)
        sys.exit(1)

    if not save:
        atexit.register(shutil.rmtree, path=wd)
    else:
        log.info('Temporary files will be kept in directory: %s' % wd)

    return wd


def namer(path, ext, wd):
    '''
    Return the path to a file with the given extension,
    in the given working directory.

    >>> namer('/home/me/test.c', '.bc', '/tmp/ikos-xxx')
    '/tmp/ikos-xxx/test.bc'
    '''
    base = os.path.basename(path)
    return os.path.join(wd, os.path.splitext(base)[0] + ext)


def signal_name(signum):
    ''' Return the signal name given the signal number '''
    for name, value in signal.__dict__.items():
        if (name.startswith('SIG') and
                not name.startswith('SIG_') and
                value == signum):
            return name
    return str(signum)


def is_apron_domain(domain):
    ''' Return True if the given domain is an APRON numerical domain '''
    return 'apron-' in domain


domains_without_narrowing = (
    'apron-polka-polyhedra',
    'apron-polka-linear-equalities',
    'apron-ppl-polyhedra',
    'apron-ppl-linear-congruences',
    'apron-pkgrid-polyhedra-lin-cong',
    'var-pack-apron-polka-polyhedra',
    'var-pack-apron-polka-linear-equalities',
    'var-pack-apron-ppl-polyhedra',
    'var-pack-apron-ppl-linear-congruences',
    'var-pack-apron-pkgrid-polyhedra-lin-cong',
)


def clang_emit_llvm_flags():
    ''' Clang flags to emit llvm bitcode '''
    return ['-c', '-emit-llvm']


def clang_ikos_flags():
    ''' Clang flags for ikos '''
    return [
        # enable clang warnings
        '-Wall',
        # disable source code fortification
        '-U_FORTIFY_SOURCE',
        '-D_FORTIFY_SOURCE=0',
        # flag for intrinsic.h
        '-D__IKOS__',
        # compile in debug mode
        '-g',
        # disable optimizations
        '-O0',
        # disable the 'optnone' attribute
        # see https://bugs.llvm.org/show_bug.cgi?id=35950#c10
        '-Xclang',
        '-disable-O0-optnone',
    ]


##################
# ikos toolchain #
##################

def clang(
    bc_path,
    cpp_path,
    include_flags=None,
    define_flags=None,
    warning_flags=None,
    disable_warnings=False,
    machine_flags=None,
    colors=True,
):
    cmd = [settings.clang()]
    cmd += clang_emit_llvm_flags()
    cmd += clang_ikos_flags()
    cmd += [cpp_path,
            '-o',
            bc_path]

    # For #include <ikos/analyzer/intrinsic.hpp>
    cmd += ['-isystem', settings.INCLUDE_DIR]

    if include_flags:
        cmd += ['-I%s' % i for i in include_flags]
    if define_flags:
        cmd += ['-D%s' % d for d in define_flags]
    if warning_flags:
        cmd += ['-W%s' % w for w in warning_flags]
    if disable_warnings:
        cmd.append('-w')
    if machine_flags:
        cmd += ['-m%s' % m for m in machine_flags]

    if colors:
        cmd.append('-fcolor-diagnostics')
    else:
        cmd.append('-fno-color-diagnostics')

    if path_ext(cpp_path) in cpp_extensions:
        cmd.append('-std=c++17')  # available because clang >= 7.0

    log.info('Compiling %s' % cpp_path)
    log.debug('Running %s' % command_string(cmd))
    subprocess.check_call(cmd)


def ikos_pp(pp_path, bc_path, entry_points, opt_level, inline_all, verify):
    if opt_level == 'aggressive':
        log.warning('Using aggressive optimizations is not recommended')
        log.warning('The translation from LLVM bitcode to AR might fail')

    cmd = [settings.ikos_pp(),
           '-opt=%s' % opt_level,
           '-entry-points=%s' % ','.join(entry_points)]

    if inline_all:
        cmd.append('-inline-all')

    if not verify:
        cmd.append('-no-verify')

    cmd += [bc_path, '-o', pp_path]

    log.info('Running ikos preprocessor')
    log.debug('Running %s' % command_string(cmd))
    subprocess.check_call(cmd)


def display_llvm(pp_path):
    log.info('Printing LLVM')
    cmd = [settings.opt(), '-S', pp_path]
    subprocess.check_call(cmd)


class AnalyzerError(Exception):
    def __init__(self, message, cmd, returncode):
        super(AnalyzerError, self).__init__(message)
        self.cmd = cmd
        self.returncode = returncode


def ikos_analyzer(db_path, pp_path, opt):
    if settings.BUILD_MODE == 'Debug':
        log.warning('ikos was built in debug mode, the analysis might be slow')
    if is_apron_domain(opt.domain) and opt.jobs != 1:
        log.warning('apron abstract domains are not thread-safe, '
                    'the analysis might crash')

    # Fix huge slow down when ikos-analyzer uses DROP TABLE on an existing db
    if os.path.isfile(db_path):
        os.remove(db_path)

    cmd = [settings.ikos_analyzer()]

    # analysis options
    cmd += ['-a=%s' % ','.join(opt.analyses),
            '-d=%s' % opt.domain,
            '-entry-points=%s' % ','.join(opt.entry_points),
            '-globals-init=%s' % opt.globals_init,
            '-proc=%s' % opt.procedural,
            '-j=%d' % opt.jobs,
            '-widening-strategy=%s' % opt.widening_strategy,
            '-widening-delay=%d' % opt.widening_delay,
            '-widening-period=%d' % opt.widening_period]

    if opt.narrowing_strategy == 'auto':
        if opt.domain in domains_without_narrowing:
            cmd.append('-narrowing-strategy=meet')
        else:
            cmd.append('-narrowing-strategy=narrow')
    else:
        cmd.append('-narrowing-strategy=%s' % opt.narrowing_strategy)

    if opt.narrowing_iterations is not None:
        cmd.append('-narrowing-iterations=%d' % opt.narrowing_iterations)
    elif (opt.narrowing_strategy == 'auto' and
            opt.domain in domains_without_narrowing):
        cmd.append('-narrowing-iterations=%d'
                   % args.meet_iterations_if_no_narrowing)

    if opt.widening_delay_functions:
        cmd.append('-widening-delay-functions=%s'
                   % ','.join(opt.widening_delay_functions))

    if opt.no_init_globals:
        cmd.append('-no-init-globals=%s' % ','.join(opt.no_init_globals))
    if opt.no_liveness:
        cmd.append('-no-liveness')
    if opt.no_pointer:
        cmd.append('-no-pointer')
    if opt.no_widening_hints:
        cmd.append('-no-widening-hints')
    if opt.partitioning != 'no':
        cmd.append('-enable-partitioning-domain')
    if opt.no_fixpoint_cache:
        cmd.append('-no-fixpoint-cache')
    if opt.no_checks:
        cmd.append('-no-checks')
    if opt.hardware_addresses:
        cmd.append('-hardware-addresses=%s' % ','.join(opt.hardware_addresses))
    if opt.hardware_addresses_file:
        cmd.append('-hardware-addresses-file=%s' % opt.hardware_addresses_file)
    if opt.argc is not None:
        cmd.append('-argc=%d' % opt.argc)

    # import options
    cmd.append('-allow-dbg-mismatch')
    if opt.no_bc_verify:
        cmd.append('-no-verify')
    if opt.no_libc:
        cmd.append('-no-libc')
    if opt.no_libcpp:
        cmd.append('-no-libcpp')
    if opt.no_libikos:
        cmd.append('-no-libikos')

    # AR passes options
    if opt.no_type_check:
        cmd.append('-no-type-check')
    if opt.no_simplify_cfg:
        cmd.append('-no-simplify-cfg')
    if opt.no_simplify_upcast_comparison:
        cmd.append('-no-simplify-upcast-comparison')
    if 'gauge' in opt.domain:
        cmd.append('-add-loop-counters')
    if opt.partitioning == 'return':
        cmd.append('-add-partitioning-variables')

    # debug options
    cmd += ['-display-checks=%s' % opt.display_checks,
            '-display-inv=%s' % opt.display_inv]

    if opt.display_ar:
        cmd.append('-display-ar')
    if opt.trace_ar_stmts:
        cmd.append('-trace-ar-stmts')
    if opt.display_liveness:
        cmd.append('-display-liveness')
    if opt.display_function_pointer:
        cmd.append('-display-function-pointer')
    if opt.display_pointer:
        cmd.append('-display-pointer')
    if opt.display_fixpoint_parameters:
        cmd.append('-display-fixpoint-parameters')
    if opt.generate_dot:
        cmd += ['-generate-dot', '-generate-dot-dir', opt.generate_dot_dir]

    # add -name-values if necessary
    if (opt.display_checks in ('all', 'fail') or
            opt.display_inv in ('all', 'fail') or
            opt.display_liveness or
            opt.display_fixpoint_parameters or
            opt.display_function_pointer or
            opt.display_pointer or
            opt.display_raw_checks):
        cmd.append('-name-values')

    # misc. options
    if opt.color == 'yes':
        cmd.append('-color=1')
    elif opt.color == 'no':
        cmd.append('-color=0')

    cmd.append('-log=%s' % opt.log_level)
    cmd.append('-progress=%s' % opt.progress)

    # input/output
    cmd += [pp_path, '-o', db_path]

    # set resource limit, if requested
    if opt.mem:
        import resource  # fails on Windows

        def set_limits():
            mem_bytes = opt.mem * 1024 * 1024
            resource.setrlimit(resource.RLIMIT_AS, [mem_bytes, mem_bytes])
    else:
        set_limits = None

    # called after timeout
    def kill(p):
        try:
            log.error('Timeout')
            p.send_signal(signal.SIGALRM)
        except OSError:
            pass

    log.info('Running ikos analyzer')
    log.debug('Running %s' % command_string(cmd))
    p = subprocess.Popen(cmd, preexec_fn=set_limits)
    timer = threading.Timer(opt.cpu, kill, [p])

    if opt.cpu:
        timer.start()

    try:
        if sys.platform.startswith('win'):
            return_status = p.wait()
        else:
            _, return_status = os.waitpid(p.pid, 0)
    finally:
        # kill the timer if the process has terminated already
        if timer.is_alive():
            timer.cancel()

    # special case for Windows, since it does not define WIFEXITED & co.
    if sys.platform.startswith('win'):
        if return_status != 0:
            raise AnalyzerError('a run-time error occurred', cmd, return_status)
        else:
            return

    # if it did not terminate properly, propagate this error code
    if os.WIFEXITED(return_status) and os.WEXITSTATUS(return_status) != 0:
        exit_status = os.WEXITSTATUS(return_status)
        raise AnalyzerError('a run-time error occurred', cmd, exit_status)

    if os.WIFSIGNALED(return_status):
        signum = os.WTERMSIG(return_status)
        raise AnalyzerError('exited with signal %s' % signal_name(signum),
                            cmd,
                            signum)

    if os.WIFSTOPPED(return_status):
        signum = os.WSTOPSIG(return_status)
        raise AnalyzerError('exited with signal %d' % signal_name(signum),
                            cmd,
                            signum)


def ikos_view(opt, db):
    from ikos import view
    v = view.View(db)
    v.serve()


#################
# main for ikos #
#################

def main(argv):
    progname = os.path.basename(argv[0])

    start_date = datetime.datetime.now()

    # parse arguments
    opt = parse_arguments(argv[1:])

    # setup colors and logging
    colors.setup(opt.color, file=log.out)
    log.setup(opt.log_level)

    if is_apron_domain(opt.domain) and not settings.HAS_APRON:
        printf('%s: error: cannot use apron abstract domains.\n'
               'ikos was compiled without apron support, '
               'see analyzer/README.md\n',
               progname, file=sys.stderr)
        sys.exit(1)

    # create working directory
    wd = create_working_directory(opt.temp_dir, opt.save_temps)

    input_path = opt.file

    # compile c/c++ code
    if path_ext(input_path) in c_extensions + cpp_extensions:
        bc_path = namer(opt.file, '.bc', wd)

        try:
            with stats.timer('clang'):
                clang(bc_path, input_path,
                      opt.compiler_include_flags,
                      opt.compiler_define_flags,
                      opt.compiler_warning_flags,
                      opt.compiler_disable_warnings,
                      opt.compiler_machine_flags,
                      colors.ENABLE)
        except subprocess.CalledProcessError as e:
            printf('%s: error while compiling %s, abort.\n',
                   progname, input_path, file=sys.stderr)
            sys.exit(e.returncode)

        input_path = bc_path

    if path_ext(input_path) not in llvm_extensions:
        printf('%s: error: unexpected file extension.\n',
               progname, file=sys.stderr)
        sys.exit(1)

    # ikos-pp: preprocess llvm bitcode
    pp_path = namer(opt.file, '.pp.bc', wd)
    try:
        with stats.timer('ikos-pp'):
            ikos_pp(pp_path, input_path,
                    opt.entry_points, opt.opt_level,
                    opt.inline_all, not opt.no_bc_verify)
    except subprocess.CalledProcessError as e:
        printf('%s: error while preprocessing llvm bitcode, abort.\n',
               progname, file=sys.stderr)
        sys.exit(e.returncode)

    # display the llvm bitcode, if requested
    if opt.display_llvm:
        display_llvm(pp_path)

    # ikos-analyzer: analyze llvm bitcode
    try:
        with stats.timer('ikos-analyzer'):
            ikos_analyzer(opt.output_db, pp_path, opt)
    except AnalyzerError as e:
        printf('%s: error: %s\n', progname, e, file=sys.stderr)
        sys.exit(e.returncode)

    # open output database
    db = OutputDatabase(path=opt.output_db)

    # insert timing results in the database
    db.insert_timing_results(stats.rows())

    # insert settings in the database
    settings_rows = [
        ('version', settings.VERSION),
        ('start-date', start_date.isoformat(' ')),
        ('end-date', datetime.datetime.now().isoformat(' ')),
        ('working-directory', wd),
        ('input', opt.file),
        ('bc-file', input_path),
        ('pp-bc-file', pp_path),
        ('clang', settings.clang()),
        ('ikos-pp', settings.ikos_pp()),
        ('opt-level', opt.opt_level),
        ('inline-all', json.dumps(opt.inline_all)),
        ('use-libc-intrinsics', json.dumps(not opt.no_libc)),
        ('use-libcpp-intrinsics', json.dumps(not opt.no_libcpp)),
        ('use-libikos-intrinsics', json.dumps(not opt.no_libikos)),
        ('use-simplify-cfg', json.dumps(not opt.no_simplify_cfg)),
        ('use-simplify-upcast-comparison',
         json.dumps(not opt.no_simplify_upcast_comparison)),
    ]
    if opt.cpu:
        settings_rows.append(('cpu-limit', opt.cpu))
    if opt.mem:
        settings_rows.append(('mem-limit', opt.mem))
    db.insert_settings(settings_rows)

    first = (log.LEVEL >= log.ERROR)

    # display timing results
    if opt.display_times != 'no':
        if not first:
            printf('\n')
        report.print_timing_results(db, opt.display_times == 'full')
        first = False

    # display summary
    if opt.display_summary != 'no':
        if not first:
            printf('\n')
        report.print_summary(db, opt.display_summary == 'full')
        first = False

    # display raw checks
    if opt.display_raw_checks:
        if not first:
            printf('\n')
        report.print_raw_checks(db, opt.procedural == 'inter')
        first = False

    # start ikos-view
    if opt.format == 'web':
        ikos_view(opt, db)
        return

    # report
    if opt.format != 'no':
        if not first and opt.report_file is sys.stdout:
            printf('\n' + colors.bold('# Results') + '\n')
            first = False

        # setup colors again (in case opt.color = 'auto')
        colors.setup(opt.color, file=opt.report_file)

        # generate report
        rep = report.generate_report(db,
                                     status_filter=opt.status_filter,
                                     analyses_filter=None)

        # format report
        formatter_class = report.formats[opt.format]
        formatter = formatter_class(opt.report_file, opt.report_verbosity)
        formatter.format(rep)

    # close database
    db.close()

    if opt.remove_db:
        os.remove(opt.output_db)
