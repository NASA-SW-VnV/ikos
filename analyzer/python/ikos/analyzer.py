################################################################################
#
# Handle ikos toolchain, from clang to the output database.
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2017 United States Government as represented by the
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
################################################################################
import argparse
import atexit
import datetime
import json
import os
import os.path
import pipes
import resource
import shlex
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import threading

from ikos import colors
from ikos import render
from ikos import stats
from ikos import settings

analyses = {
    'boa': 'Buffer overflow analysis',
    'dbz': 'Division by zero analysis',
    'upa': 'Unaligned pointer analysis',
    'uva': 'Uninitialized variables analysis',
    'nullity': 'Null dereference analysis',
    'prover': 'Assertion prover',
}

default_analyses = ('boa', 'dbz', 'nullity', 'prover')


def parse_opt(argv):
    usage = '%(prog)s [options] file[.c|.cpp|.bc]'
    parser = argparse.ArgumentParser(usage=usage)

    parser.add_argument('-a', '--analysis', dest='analyses',
                        help='Type of analysis (default: boa, dbz, nullity, prover)',
                        action='append',
                        choices=analyses.keys())
    parser.add_argument('-e', '--entry-points', dest='entry_points',
                        help='The entry point(s) of the program (default: main)',
                        action='append')
    parser.add_argument('--entry-init-gv', dest='entry_points_init_gv',
                        help='Initialize global variables for the given entry points (default: main)',
                        action='append')
    parser.add_argument('--ikos-pp', dest='ikos_pp',
                        help='Enable all preprocessing optimizations',
                        action='store_true',
                        default=False)
    parser.add_argument('--inline-all', dest='inline',
                        help='Front-end inline all functions',
                        action='store_true',
                        default=False)
    parser.add_argument('--intra', dest='intraprocedural',
                        help='Run an intraprocedural analysis instead of an interprocedural analysis',
                        action='store_true',
                        default=False)
    parser.add_argument('--no-liveness', dest='no_liveness',
                        help='Disable the liveness analysis',
                        action='store_true',
                        default=False)
    parser.add_argument('--no-pointer', dest='no_pointer',
                        help='Disable the pointer analysis',
                        action='store_true',
                        default=False)
    parser.add_argument('-p', '--precision-level', dest='precision_level',
                        help='The precision level (reg, ptr, mem) (default: mem)',
                        default='mem',
                        choices=('reg', 'ptr', 'mem'))
    parser.add_argument('--gv-zero-init', dest='gv_zero_initializers',
                        help='Do not ignore zero initialization of global variables',
                        action='store_true',
                        default=False)
    parser.add_argument('--gv-init-all', dest='gv_init_all',
                        help='Initialize all global variables (default: initialize scalars and pointers)',
                        action='store_true',
                        default=False)
    parser.add_argument('--gv-init-scalars-only', dest='gv_init_scalars_only',
                        help='Initialize scalar global variables only',
                        action='store_true',
                        default=False)
    parser.add_argument('--gv-init-pointers-only', dest='gv_init_pointers_only',
                        help='Initialize pointer global variables only',
                        action='store_true',
                        default=False)
    parser.add_argument('--summaries', dest='use_summaries',
                        help='Use function summarization',
                        action='store_true',
                        default=False)
    parser.add_argument('--pointer-summaries', dest='use_pointer_summaries',
                        help='Use function summarization for the pointer analysis',
                        action='store_true',
                        default=False)
    parser.add_argument('--disable-arbos-opt', dest='arbos_optimize',
                        help='Disable arbos optimizations',
                        action='store_false',
                        default=True)
    parser.add_argument('--disable-cfg-opt', dest='arbos_optimize_cfg',
                        help='Disable arbos control flow graph optimizations',
                        action='store_false',
                        default=True)
    parser.add_argument('--verify-bitcode', dest='verify',
                        help='Verify LLVM bitcode is well formed',
                        action='store_true',
                        default=False)
    parser.add_argument('-d', '--dot-cfg', dest='dot_cfg',
                        help="Print CFG of all functions to 'dot' file",
                        action='store_true',
                        default=False)
    parser.add_argument('--save-temps', dest='save_temps',
                        help='Do not delete temporary files',
                        action='store_true',
                        default=False)
    parser.add_argument('--temp-dir', dest='temp_dir', metavar='DIR',
                        help='Temporary directory',
                        default=None)
    parser.add_argument('-o', '--db', '--output-db', dest='output_db', metavar='FILE',
                        help='The output database file',
                        default='output.db')
    parser.add_argument('--rm-db', dest='remove_db',
                        help='Remove the output database file after use',
                        action='store_true',
                        default=False)
    parser.add_argument('--colors', dest='colors',
                        help='Enable colors (default: auto)',
                        default='auto',
                        choices=('off', 'on', 'no', 'yes', 'auto'))
    parser.add_argument('--display-invariants', dest='display_invariants',
                        help='Display invariants (default: off)',
                        default='off',
                        choices=('all', 'fail', 'off'))
    parser.add_argument('--display-checks', dest='display_checks',
                        help='Display checks (default: off)',
                        default='off',
                        choices=('all', 'fail', 'off'))
    parser.add_argument('--display-times', dest='display_timing_results',
                        help='Display timing results (default: short)',
                        default='short',
                        choices=('off', 'short', 'full'))
    parser.add_argument('--display-summary', dest='display_summary',
                        help='Display the analysis summary (default: full)',
                        default='full',
                        choices=('off', 'short', 'full'))
    parser.add_argument('--show-raw-checks', dest='show_raw_checks',
                        help='Print analysis raw checks',
                        action='store_true',
                        default=False)
    parser.add_argument('--export', dest='export',
                        help='Export analysis results into a specific format',
                        action='store_true',
                        default=False)
    parser.add_argument('--export-format', dest='export_format',
                        help='Export format (default: gcc)',
                        default='gcc',
                        choices=render.formats.keys())
    parser.add_argument('--export-file', dest='export_file', metavar='FILE',
                        help='Export into a file (default: stdout)',
                        default=sys.stdout,
                        type=argparse.FileType('w'))
    parser.add_argument('--export-level', dest='export_level',
                        help='Export level (default: warning)',
                        default='warning',
                        choices=('all', 'safe', 'note', 'warning', 'error'))
    parser.add_argument('--export-no-unreachable', dest='export_no_unreachable',
                        help='Do not export unreachable statements',
                        action='store_true',
                        default=False)
    parser.add_argument('--export-demangle', dest='export_demangle',
                        help='Demangle C++ symbols during export',
                        action='store_true',
                        default=False)
    parser.add_argument('--export-verbosity', dest='export_verbosity', metavar='[1-4]',
                        help='Export verbosity (default: 2)',
                        default=2,
                        type=int)
    parser.add_argument('-i', '--ikosview', dest='ikosview',
                        help='Show analysis results using ikosview GUI',
                        action='store_true',
                        default=False)
    parser.add_argument('--cpu', type=int, dest='cpu',
                        help='CPU time limit (seconds)', default=-1)
    parser.add_argument('--mem', type=int, dest='mem',
                        help='MEM limit (MB)', default=-1)

    # version argument
    class VersionAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            printf('ikos %s\n', settings.VERSION)
            printf('Copyright (c) 2011-2017 United States Government as represented by the\n')
            printf('Administrator of the National Aeronautics and Space Administration.\n')
            printf('All Rights Reserved.\n')
            parser.exit()

    parser.add_argument('--version', action=VersionAction, nargs=0,
                        help='show ikos version')

    # for debugging
    parser.add_argument('--print-command', dest='print_command',
                        help=argparse.SUPPRESS,
                        action='store_true',
                        default=False)

    # positional argument
    parser.add_argument('file', metavar='file[.c|.cpp|.bc]',
                        help='The file to analyze')

    opt = parser.parse_args(argv)

    if not opt.analyses:
        opt.analyses = default_analyses

    # by default, the entry point is main
    if not opt.entry_points:
        opt.entry_points = ('main',)
        opt.entry_points_init_gv = ('main',)

    # post process for global variable initialization
    if opt.gv_init_all:
        opt.gv_init = 'all'
    else:
        assert not(opt.gv_init_scalars_only and opt.gv_init_pointers_only), \
            'options --gv-init-scalars-only and --gv-init-pointers-only are mutually exclusive'
        if opt.gv_init_scalars_only:
            opt.gv_init = ['scalars']
        elif opt.gv_init_pointers_only:
            opt.gv_init = ['pointers']
        else:
            opt.gv_init = ['scalars', 'pointers']

    return opt


def printf(fmt, *args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    file.write(fmt % args if args else fmt)
    file.flush()


if hasattr(shlex, 'quote'):
    sh_quote = shlex.quote
else:
    sh_quote = pipes.quote


def path_ext(path):
    ''' Return the filename extension.

    >>> path_ext('test.c')
    '.c'
    >>> path_ext('/tmp/file.my.ext')
    '.ext'
    '''
    return os.path.splitext(path)[1]


def create_work_dir(wd=None, save=False):
    ''' Create a temporary working directory '''
    if wd is None:
        wd = tempfile.mkdtemp(prefix='ikos-')

    if not save:
        atexit.register(shutil.rmtree, path=wd)
    else:
        printf('Temporary files will be kept in directory: %s\n', wd)

    return wd


def namer(path, ext, wd):
    ''' Return the path to a file with the given extension, in the working directory.

    >>> namer('/home/me/test.c', '.bc', '/tmp/ikos-xxx')
    '/tmp/ikos-xxx/test.bc'
    '''
    base = os.path.basename(path)
    return os.path.join(wd, os.path.splitext(base)[0] + ext)


##################
# ikos toolchain #
##################


def clang(bc_path, cpp_path, colors=True):
    cmd = [settings.clang(),
           '-emit-llvm', '-c',
           '-g', '-D_FORTIFY_SOURCE=0', '-Wall',
           cpp_path, '-o', bc_path]

    if colors:
        cmd.append('-fcolor-diagnostics')
    else:
        cmd.append('-fno-color-diagnostics')

    if cpp_path.endswith('.cpp'):
        cmd.append('-std=c++14') # available because clang >= 4.0

    subprocess.check_call(cmd)


def ikos_pp(pp_path, bc_path, entry_points, full, inline, verify):
    cmd = [settings.ikos_pp()]

    if entry_points:
        cmd += ['-entry-points', ','.join(entry_points)]

    if full:
        cmd.append('-ikospp-level=full')
    else:
        cmd.append('-ikospp-level=simple')

    if inline:
        cmd.append('-ikospp-inline-all')

    if verify:
        cmd.append('-ikospp-verify')

    cmd += [bc_path, '-o', pp_path]
    subprocess.check_call(cmd)


def llvm_to_ar(ar_path, pp_path, translate_zero_initializers=False):
    cmd = [settings.opt(), '-load', settings.llvm_to_ar(), '-arbos', '-disable-output']

    if translate_zero_initializers:
        cmd.append('-enable-gvinit')

    with open(pp_path, 'rb') as pp_file:
        with open(ar_path, 'wb') as ar_file:
            subprocess.check_call(cmd, stdin=pp_file, stdout=ar_file)


class ArbosError(Exception):
    def __init__(self, message, cmd, returncode):
        super(ArbosError, self).__init__(message)
        self.cmd = cmd
        self.returncode = returncode


def arbos(ar_path, db_path,
          analyses, entry_points=None, entry_points_init_gv=None,
          arbos_optimize=True, arbos_optimize_cfg=True, dot_cfg=False,
          add_loop_counters=False, interprocedural=True, prec_level='mem',
          liveness=True, pointer=True,
          gv_init=('scalars', 'pointers'),
          summaries=False, pointer_summaries=False,
          display_invariants='off', display_checks='off',
          mem=-1, cpu=-1,
          print_command=False):
    # list of arbos passes
    passes = []

    if arbos_optimize:
        passes.append(('pointer-shift-opt', 'ps-opt'))

        if add_loop_counters:
            passes.append(('add-loop-counters', 'add-loop-counters'))

        if arbos_optimize_cfg:
            passes.append(('branching-opt', 'branching-opt'))

        if entry_points_init_gv:
            passes.append(('inline-init-gv', 'inline-init-gv'))

    passes.append(('unify-exit-nodes', 'unify-exit-nodes'))

    if dot_cfg:
        passes.append(('ar-to-dot', 'cfg-dot'))

    passes.append(('analyzer', 'analyzer'))

    loads = []
    pass_names = []
    options = []

    # build arbos parameters
    for lib_name, pass_name in passes:
        lib_path = settings.arbos_pass(lib_name)
        loads.append('-load=%s' % lib_path)
        pass_names.append('-%s' % pass_name)

    # libinline-init-gv options
    if entry_points_init_gv:
        for entry_point in entry_points_init_gv:
            options += ['--init-globals', entry_point]
        if arbos_optimize and gv_init != 'all':
            for init in gv_init:
                options.append('--only-%s' % init)

    # libanalyzer options
    for analysis in analyses:
        options += ['--analysis', analysis]

    if entry_points:
        for entry_point in entry_points:
            options += ['--entry-points', entry_point]

    if not interprocedural:
        options.append('--intra')
    if not liveness:
        options.append('--no-liveness')
    if not pointer:
        options.append('--no-pointer')

    options += ['--precision-level', prec_level]

    if summaries:
        options.append('--summaries')
    if pointer_summaries:
        options.append('--pointer-summaries')

    options += [
        '--display-invariants', display_invariants,
        '--display-checks', display_checks,
        '--output-db', db_path
    ]

    cmd = [settings.arbos()] + loads + pass_names + options

    if print_command:
        printf(' '.join(map(sh_quote, cmd)) + '\n')
        return passes

    # set resource limit
    def set_limits():
        if mem > 0:
            mem_bytes = mem * 1024 * 1024
            resource.setrlimit(resource.RLIMIT_AS, [mem_bytes, mem_bytes])

    # called after timeout
    def kill(p):
        try:
            printf('TIMEOUT\n')
            p.terminate()
            p.kill()
            p.wait()
        except OSError:
            pass

    p = subprocess.Popen(cmd, stdin=open(ar_path), preexec_fn=set_limits)
    timer = threading.Timer(cpu, kill, [p])

    if cpu > 0:
        timer.start()

    try:
        pid, returnstatus, ru_child = os.wait4(p.pid, 0)
    finally:
        # kill the timer if the process has terminated already
        if timer.isAlive():
            timer.cancel()

    # if it did not terminate properly, propagate this error code
    if os.WIFEXITED(returnstatus) and os.WEXITSTATUS(returnstatus) != 0:
        raise ArbosError('some run-time error occured', cmd, os.WEXITSTATUS(returnstatus))

    if os.WIFSIGNALED(returnstatus):
        raise ArbosError('exited with signal %d' % os.WTERMSIG(returnstatus), cmd, os.WTERMSIG(returnstatus))

    if os.WIFSTOPPED(returnstatus):
        raise ArbosError('exited with signal %d' % os.WSTOPSIG(returnstatus), cmd, os.WSTOPSIG(returnstatus))

    return passes


def save_settings(db, rows):
    ''' Save the analysis settings into the database '''
    c = db.cursor()
    c.executemany('INSERT INTO settings VALUES (?,?)', rows)
    db.commit()


def ikos_view(db_path, interprocedural):
    settings.ikos_view()


#################
# main for ikos #
#################

def main(argv):
    progname = os.path.basename(argv[0])

    start_date = datetime.datetime.now()

    # disable unix signals forwarding
    os.setpgrp()

    # parse arguments
    opt = parse_opt(argv[1:])

    # setup colors
    colors.setup(opt.colors)

    try:
        # create working directory
        wd = create_work_dir(opt.temp_dir, opt.save_temps)

        input_path = opt.file

        # compile c/c++ code
        if path_ext(input_path) in ('.c', '.cpp'):
            bc_path = namer(opt.file, '.bc', wd)

            try:
                with stats.timer('clang'):
                    clang(bc_path, input_path, colors.ENABLE)
            except subprocess.CalledProcessError as e:
                printf('%s: error while compiling %s, abort.\n', progname, input_path, file=sys.stderr)
                sys.exit(e.returncode)

            input_path = bc_path

        if path_ext(input_path) != '.bc':
            printf('%s: error: unexpected file extension.\n', progname, file=sys.stderr)
            sys.exit(1)

        # preprocess bitcode
        pp_path = namer(opt.file, '.pp.bc', wd)
        try:
            with stats.timer('ikos-pp'):
                ikos_pp(pp_path, input_path, opt.entry_points, opt.ikos_pp, opt.inline, opt.verify)
        except subprocess.CalledProcessError as e:
            printf('%s: error while preprocessing llvm bitcode, abort.\n', progname, file=sys.stderr)
            sys.exit(e.returncode)

        # translate bitcode to arbos ar
        ar_path = namer(opt.file, '.ar', wd)
        try:
            with stats.timer('llvm-to-ar'):
                llvm_to_ar(ar_path, pp_path, opt.gv_zero_initializers)
        except subprocess.CalledProcessError as e:
            printf('%s: error while running llvm-to-ar, abort.\n', progname, file=sys.stderr)
            sys.exit(e.returncode)

        # arbos (libanalyzer)
        try:
            with stats.timer('arbos'):
                passes = arbos(ar_path, opt.output_db,
                               opt.analyses, opt.entry_points, opt.entry_points_init_gv,
                               opt.arbos_optimize, opt.arbos_optimize_cfg, opt.dot_cfg,
                               settings.ABSTRACT_DOMAIN.startswith('GAUGE'),
                               not opt.intraprocedural, opt.precision_level,
                               not opt.no_liveness, not opt.no_pointer,
                               opt.gv_init,
                               opt.use_summaries, opt.use_pointer_summaries,
                               opt.display_invariants, opt.display_checks,
                               opt.mem, opt.cpu,
                               opt.print_command)
        except ArbosError as e:
            printf('%s: error: %s\n', progname, e, file=sys.stderr)
            sys.exit(e.returncode)

        if opt.print_command:
            return

        # open output database
        db = sqlite3.connect(opt.output_db)

        # insert timing results in the database
        stats.save_database(db)

        # insert settings in the database
        settings_rows = [
            ('version', settings.VERSION),
            ('start-date', start_date.isoformat(' ')),
            ('end-date', datetime.datetime.now().isoformat(' ')),
            ('entry-points-init-gv', json.dumps(opt.entry_points_init_gv)),
            ('working-directory', wd),
            ('input', opt.file),
            ('bc-file', input_path),
            ('pp-bc-file', pp_path),
            ('ar-file', ar_path),
            ('clang', settings.clang()),
            ('opt', settings.opt()),
            ('arbos', settings.arbos()),
            ('llvm-to-ar', settings.llvm_to_ar()),
            ('ikos-pp', settings.ikos_pp()),
            ('ikos-pp-level', 'full' if opt.ikos_pp else 'simple'),
            ('ikos-pp-inline-all', json.dumps(opt.inline)),
            ('global-variables-init', json.dumps(opt.gv_init)),
            ('arbos-passes', json.dumps(passes)),
        ]
        if opt.cpu > 0:
            settings_rows.append(('cpu-limit', opt.cpu))
        if opt.mem > 0:
            settings_rows.append(('mem-limit', opt.mem))
        save_settings(db, settings_rows)

        # display timing results
        if opt.display_timing_results != 'off':
            printf('\n')
            render.print_timing_results(db, opt.display_timing_results == 'full')

        # display summary
        if opt.display_summary != 'off':
            printf('\n')
            render.print_summary(db, opt.display_summary == 'full')

        # display raw checks
        if opt.show_raw_checks:
            printf('\n')
            render.print_raw_checks(db, not opt.intraprocedural)

        # export
        if opt.export:
            if opt.export_file is sys.stdout:
                printf('\n')
                printf(colors.bold('# Results') + '\n')

            # setup colors again (in case opt.colors = 'auto')
            colors.setup(opt.colors, file=opt.export_file)

            # export format
            formatter_class = render.formats[opt.export_format]
            formatter = formatter_class(opt.export_file, opt.export_verbosity, opt.export_demangle)

            render.export(db, formatter, opt.export_level, not opt.export_no_unreachable)

        # close database
        db.close()

        # ikos view
        if opt.ikosview:
            ikos_view(opt.output_db, not opt.intraprocedural)

        if opt.remove_db:
            os.remove(opt.output_db)
    except KeyboardInterrupt:
        pass
