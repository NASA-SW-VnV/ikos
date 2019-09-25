###############################################################################
#
# Library for ikos regression tests
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
import os
import shutil
import sqlite3
import subprocess
import sys
import tempfile

USE_COLORS = True
INTERACTIVE = True
VERBOSE = False
CLANG = 'clang'
IKOS_PP = 'ikos-pp'
IKOS_ANALYZER = 'ikos-analyzer'

# available ikos analyses
ANALYSES = (
    'boa',
    'dbz',
    'nullity',
    'prover',
    'upa',
    'uva',
    'sio',
    'uio',
    'shc',
    'poa',
    'pcmp',
    'sound',
    'fca',
    'dca',
    'dfa',
    'dbg',
    'watch',
)

# available colors
COLORS = {
    'grey': 0,
    'red': 1,
    'green': 2,
    'yellow': 3,
    'blue': 4,
    'magenta': 5,
    'cyan': 6,
    'white': 7
}
# available attributes
ATTRIBUTES = {
    'bold': 1,
    'dark': 2,
    'underline': 4,
    'blink': 5,
    'reverse': 7,
    'concealed': 8
}


def printf(fmt, *args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    file.write(fmt % args if args else fmt)
    file.flush()


def colorize(text, color=None, on_color=None, attrs=None):
    '''
    Colorize text.

    >>> colorize('Hello, World!', 'red', 'grey', ['bold', 'blink'])
    '\x1b[5m\x1b[1m\x1b[40m\x1b[31mHello, World!\x1b[0m'
    >>> colorize('Hello, World!', 'green')
    '\x1b[32mHello, World!\x1b[0m'
    '''
    if USE_COLORS:
        if color:
            text = '\033[%dm%s' % (30 + COLORS[color], text)

        if on_color:
            text = '\033[%dm%s' % (40 + COLORS[on_color], text)

        if attrs:
            for attr in attrs:
                text = '\033[%dm%s' % (ATTRIBUTES[attr], text)

        text += '\033[0m'

    return text


def bold(s): return colorize(s, attrs=['bold'])
def red(s): return colorize(s, 'red', attrs=['bold'])
def green(s): return colorize(s, 'green', attrs=['bold'])
def yellow(s): return colorize(s, 'yellow', attrs=['bold'])


def is_executable(fpath):
    return fpath and os.path.isfile(fpath) and os.access(fpath, os.X_OK)


def which(program):
    ''' Try to find program in the PATH, otherwise return None.

    >>> which('cat')
    '/bin/cat'
    '''
    fpath, fname = os.path.split(program)
    if fpath:
        if is_executable(program):
            return program
    else:
        for path in os.environ['PATH'].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if is_executable(exe_file):
                return exe_file

    return None


def find_clang():
    path = which(CLANG)
    assert is_executable(path), 'could not find clang'
    return path


def find_ikos_pp():
    path = which(IKOS_PP)
    assert is_executable(path), 'could not find ikos-pp'
    return path


def find_ikos_analyzer():
    path = which(IKOS_ANALYZER)
    assert is_executable(path), 'could not find ikos-analyzer'
    return path


def clang_emit_llvm_flags():
    ''' Clang flags to emit llvm bitcode '''
    # see analyzer.clang_emit_llvm_flags()
    return ['-c', '-emit-llvm']


def clang_ikos_flags():
    ''' Clang flags for ikos '''
    # see analyzer.clang_ikos_flags()
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


class Result:
    OK = 0
    WARNING = 1
    ERROR = 2
    UNREACHABLE = 3


class Database:
    def __init__(self, path):
        self.db = sqlite3.connect(path)
        self.cursor = self.db.cursor()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.db.close()

    def get_num_checks(self, result=None):
        where = ('WHERE status=%d' % result) if result else ''
        self.cursor.execute('SELECT COUNT(*) FROM checks %s' % where)
        return self.cursor.fetchone()[0]

    def get_line_status(self, line):
        self.cursor.execute('SELECT checks.status FROM checks INNER JOIN statements ON checks.statement_id = statements.id WHERE statements.line=%d' % line)
        return [row[0] for row in self.cursor.fetchall()]


class TestResult:
    def __init__(self, code, comments=None):
        assert code in ('PASS', 'PASS_IMPROVE', 'FAIL')
        self.code = code
        self.comments = comments or []

    def add_comment(self, comment):
        self.comments.append(comment)


class Test:
    def __init__(self,
                 filename,
                 description,
                 analyses,
                 result,
                 expected=None,
                 domain=None,
                 opt_level=None,
                 entry_points=None,
                 procedural=None,
                 options=None,
                 line_checks=None):
        if not isinstance(analyses, list):
            analyses = [analyses]

        assert all(a in ANALYSES for a in analyses)
        assert result in ('safe', 'unsafe', 'error')
        assert expected in (None, 'safe', 'unsafe', 'error')

        self.filename = filename
        self.description = description
        self.result = result
        self.expected = expected or result
        self.analyses = analyses
        self.domain = domain or 'interval'
        self.opt_level = opt_level or 'basic'
        self.entry_points = entry_points or ('main',)
        self.procedural = procedural or 'inter'
        self.options = options or []
        self.line_checks = line_checks or []

    def run(self, root, output_db):
        fullpath = os.path.join(root, self.filename)
        assert os.path.exists(fullpath)

        # create working directory
        wd = tempfile.mkdtemp(prefix='ikos-%s' % self.filename)
        atexit.register(shutil.rmtree, path=wd)

        # run clang
        bc_path = os.path.join(wd, '%s.bc' % self.filename)
        cmd = [find_clang()]
        cmd += clang_emit_llvm_flags()
        cmd += clang_ikos_flags()
        cmd += [fullpath, '-o', bc_path]
        if self.filename.endswith('.cpp'):
            cmd.append('-std=c++17')
        subprocess.check_call(cmd,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)

        # run ikos preprocessor
        pp_path = os.path.join(wd, '%s.pp.bc' % self.filename)
        cmd = [find_ikos_pp(),
               '-opt=%s' % self.opt_level,
               '-entry-points=%s' % ','.join(self.entry_points),
               bc_path,
               '-o', pp_path]
        subprocess.check_call(cmd,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)

        # run ikos analyzer
        cmd = [find_ikos_analyzer(),
               '-a=%s' % ','.join(self.analyses),
               '-d=%s' % self.domain,
               '-entry-points=%s' % ','.join(self.entry_points),
               '-proc=%s' % self.procedural]
        cmd.extend(self.options)
        if self.opt_level == 'aggressive':
            cmd.append('-allow-dbg-mismatch')
        if 'gauge' in self.domain:
            cmd.append('-add-loop-counters')
        cmd += [pp_path, '-o', output_db]
        subprocess.check_call(cmd,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)

        with Database(output_db) as db:
            # Get the global result
            errors = db.get_num_checks(Result.ERROR)
            warnings = db.get_num_checks(Result.WARNING)

            if errors == 0 and warnings == 0:
                result = 'safe'
            elif errors != 0:
                result = 'error'
            else:
                result = 'unsafe'

            # Set the test result (passed or failed)
            ret = TestResult('PASS')

            if result not in (self.result, self.expected):
                ret.code = 'FAIL'
                ret.add_comment('Got %d errors and %d warnings, was expecting "%s".'
                                % (errors, warnings, self.expected))
            elif result == self.result and result != self.expected:
                ret.code = 'PASS_IMPROVE'
                ret.add_comment('improvement: IKOS returned the right result '
                                '(%s) and not the expected one (%s).'
                                % (self.result, self.expected))

            # Line by line check
            for line_check in self.line_checks:
                if len(line_check) == 3:
                    line_num, line_result, line_expected = line_check
                elif len(line_check) == 2:
                    line_num, line_result = line_check
                    line_expected = line_result
                else:
                    assert False, 'Unexpected line_checks'

                assert line_result in ('ok', 'warning', 'error', 'unreachable')
                assert line_expected in ('ok', 'warning', 'error', 'unreachable')

                result = db.get_line_status(line_num)

                if Result.ERROR in result:
                    result = 'error'
                elif Result.WARNING in result:
                    result = 'warning'
                elif Result.OK in result and all(s in (Result.OK, Result.UNREACHABLE) for s in result):
                    result = 'ok'
                elif result and all(s == Result.UNREACHABLE for s in result):
                    result = 'unreachable'
                else:
                    result = 'unknown'

                if result not in (line_result, line_expected):
                    ret.code = 'FAIL'
                    ret.add_comment('Got status "%s" for line %d, was expecting "%s".'
                                    % (result, line_num, line_expected))
                elif result == line_result and line_result != line_expected:
                    if ret.code == 'PASS':
                        ret.code = 'PASS_IMPROVE'
                    ret.add_comment('improvement: IKOS returned the right result '
                                    '(%s) for line %d and not the expected one (%s).'
                                    % (line_result, line_num, line_expected))

            if ret.code == 'FAIL':
                ret.comments.insert(0, 'Running %r' % cmd)

            return ret


class TestManager:
    def __init__(self, root):
        self.root = root
        self.tests = []
        self.results = {'PASS': 0, 'PASS_IMPROVE': 0, 'FAIL': 0}

    def add(self, test):
        self.tests.append(test)

    def run(self):
        printf(bold('Running tests...\n'))

        # output database
        _, output_db = tempfile.mkstemp(suffix='.db', prefix='ikos-output-')
        atexit.register(lambda: os.unlink(output_db))

        for t in self.tests:
            printf('  %s ... ', t.description)

            if INTERACTIVE:
                printf('\n')
                self.print_progress()
                # Move the cursor right after the '...'
                printf('\r\033[A\033[%dC' % len('  %s ... ' % t.description))

            result = t.run(self.root, output_db)
            self.results[result.code] += 1

            if result.code == 'FAIL':
                printf(red(('Failed\n')))
            elif result.code == 'PASS':
                printf(green('Passed\n'))
            elif result.code == 'PASS_IMPROVE':
                printf(yellow('Passed with improvements!\n'))
            else:
                assert False, 'unknown result'

            if INTERACTIVE:
                # Clear everything down the cursor
                printf('\033[J')

            if VERBOSE:
                for line in result.comments:
                    printf('    %s\n' % line)

        self.print_result()
        exit(self.results['FAIL'])

    def get_num_tests(self):
        return len(self.tests)

    def get_num_done(self):
        return self.results['PASS'] + self.results['PASS_IMPROVE'] + self.results['FAIL']

    def print_result(self):
        printf(bold('Results:\n'))
        if self.results['FAIL'] == 0:
            if self.results['PASS_IMPROVE'] > 0:
                printf(green('  %d tests passed successfully (with %d improvement(s)).\n'
                             % (self.results['PASS'] + self.results['PASS_IMPROVE'],
                                self.results['PASS_IMPROVE'])))
            else:
                printf(green('  %d tests passed successfully.\n' % self.results['PASS']))
        else:
            printf(red('  %d/%d tests failed.\n' % (self.results['FAIL'], self.get_num_tests())))

    def print_progress(self):
        percent = 100.0 * self.get_num_done() / self.get_num_tests()
        full_width = 50
        width = int(full_width * percent / 100.0)

        progressbar = '[' + '#' * width + ' ' * (full_width - width) + '] %d%%' % int(percent)
        printf(progressbar)


def parse_args(description=None):
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--verbose', '-v', dest='verbose',
                        help='Verbose mode',
                        action='store_true', default=False)
    parser.add_argument('--no-colors', dest='no_colors',
                        help='Disable colors',
                        action='store_true', default=False)
    parser.add_argument('--no-interactive', dest='no_interactive',
                        help='Disable interactive mode',
                        action='store_true', default=False)
    parser.add_argument('--clang', dest='clang',
                        help='Path to the clang binary',
                        default='clang')
    parser.add_argument('--ikos-pp', dest='ikos_pp',
                        help='Path to the ikos-pp binary',
                        default='ikos-pp')
    parser.add_argument('--ikos-analyzer', dest='ikos_analyzer',
                        help='Path to the ikos-analyzer binary',
                        default='ikos-analyzer')

    args = parser.parse_args()

    global VERBOSE, USE_COLORS, INTERACTIVE, CLANG, IKOS_PP, IKOS_ANALYZER
    VERBOSE = args.verbose
    USE_COLORS = False if args.no_colors else os.isatty(sys.stdout.fileno())
    INTERACTIVE = False if args.no_interactive else os.isatty(sys.stdout.fileno())
    CLANG = args.clang
    IKOS_PP = args.ikos_pp
    IKOS_ANALYZER = args.ikos_analyzer
