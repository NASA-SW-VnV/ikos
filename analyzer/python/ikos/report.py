##############################################################################
#
# Generate an analysis report from a result database
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2023 United States Government as represented by the
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
import collections
import csv
import functools
import io
import itertools
import json
import operator
import os
import os.path
import sqlite3
import sys
from xml.etree import ElementTree
from xml.sax.saxutils import escape
from xml.sax.saxutils import quoteattr

from ikos import args
from ikos import colors
from ikos import settings
from ikos.abs_int import Signedness, MachineInt, Interval, Congruence
from ikos.colors import bold, bold_blue, bold_green, bold_magenta, bold_red, \
    bold_yellow
from ikos.enums import Result, CheckKind, CheckerName, ValueKind, \
    StatementKind, MemoryLocationKind, FunctionCallCheckKind, \
    BufferOverflowCheckKind, ChecksTable
from ikos.log import printf
from ikos.output_db import OutputDatabase, File, Function, Statement, \
    CallContext, Operand, NumOperandPair, MemoryLocation, Check


##################
# timing results #
##################


def format_time(elapsed):
    ''' Format an elapsed time.

    >>> format_time(3.1415)
    '3.142 sec'
    >>> format_time(2 * 3600 * 24 + 23 * 3600 + 42 * 60 + 3.20498)
    '2 day 23 hour 42 min 3.205 sec'
    '''
    s = []

    if elapsed >= 3600 * 24:
        s.append('%d day' % int(elapsed // (3600 * 24)))
        elapsed %= 3600 * 24

    if elapsed >= 3600:
        s.append('%d hour' % int(elapsed // 3600))
        elapsed %= 3600

    if elapsed >= 60:
        s.append('%d min' % int(elapsed // 60))
        elapsed %= 60

    s.append('%.3f sec' % elapsed)
    return ' '.join(s)


def print_timing_results(db, full=True, sort=True):
    ''' Print the timing results from the database '''
    results = db.load_timing_results(full, sort)

    printf(bold('# Time stats:') + '\n')
    name_width = max(len(name) for name, _ in results)
    for name, elapsed in results:
        printf('%s: %s\n', name.ljust(name_width), format_time(elapsed))


###########
# summary #
###########


# Represents a check, as a hashable type
CheckTuple = collections.namedtuple('CheckTuple', ('kind', 'operands', 'info'))


def generate_statement_result(checks, keep_oks=True, keep_checkers=True):
    '''
    Generate a result for a list of checks on a specific statement
    and call context
    '''
    result = Result.OK
    errors = set()
    warnings = set()
    oks = set() if keep_oks else None
    checkers = set() if keep_checkers else None

    for check in checks:
        if keep_checkers:
            checkers.add(check[ChecksTable.CHECKER])

        status = check[ChecksTable.STATUS]

        if status == Result.UNREACHABLE:
            result = Result.UNREACHABLE

            if keep_checkers:
                continue  # keep iterating
            else:
                break  # exit early

        check = CheckTuple(check[ChecksTable.KIND],
                           check[ChecksTable.OPERANDS],
                           check[ChecksTable.INFO])

        if status == Result.ERROR:
            result = Result.ERROR
            errors.add(check)
        elif status == Result.WARNING:
            if result == Result.OK:
                result = Result.WARNING
            warnings.add(check)
        elif status == Result.OK:
            if keep_oks:
                oks.add(check)
        else:
            assert False, 'unexpected status'

    return result, errors, warnings, oks, checkers


class Summary:
    ''' Represents a summary '''

    def __init__(self, ok, error, warning, unreachable):
        self.ok = ok
        self.error = error
        self.warning = warning
        self.unreachable = unreachable

    @property
    def total(self):
        return self.ok + self.error + self.warning + self.unreachable


def generate_summary(db):
    '''
    Return the analysis summary: number of errors, warnings, ok and
    unreachable per checked statements.
    '''
    summary = Summary(ok=0, error=0, warning=0, unreachable=0)

    c = db.con.cursor()
    order_by = 'statement_id, call_context_id'
    c.execute('SELECT * FROM checks ORDER BY %s' % order_by)

    stmt_id_key = operator.itemgetter(ChecksTable.STATEMENT_ID)
    context_id_key = operator.itemgetter(ChecksTable.CALL_CONTEXT_ID)

    for statement_id, statement_checks in itertools.groupby(c,
                                                            key=stmt_id_key):
        # Iterate over the checks for statement = statement_id
        statement_results = set()
        statement_checkers = set()
        statement_errors = set()
        statement_warnings = set()

        for context_id, checks in itertools.groupby(statement_checks,
                                                    key=context_id_key):
            # Iterate over the checks for statement = statement_id
            # and context = context_id
            result, errors, warnings, _, checkers = \
                generate_statement_result(checks,
                                          keep_oks=False,
                                          keep_checkers=True)
            statement_results.add(result)
            statement_checkers.update(checkers)
            statement_errors.update(errors)
            statement_warnings.update(warnings)

        if statement_results == {Result.UNREACHABLE}:
            # Statement is unreachable for all the calling contexts,
            # check that it comes from dca
            if CheckerName.DEAD_CODE in statement_checkers:
                # Statement is never reachable
                summary.unreachable += 1
        else:
            if Result.OK in statement_results:
                # Some paths are safe
                summary.ok += 1

            summary.error += len(statement_errors)
            summary.warning += len(statement_warnings)

    c.close()
    return summary


def print_summary(db, full=True):
    ''' Print the analysis summary from the database '''
    summary = generate_summary(db)

    printf(bold('# Summary:') + '\n')

    if full:
        printf('Total number of checks                : %s\n',
               bold(summary.total))
        printf('Total number of unreachable checks    : %s\n',
               bold_magenta(summary.unreachable)
               if summary.unreachable
               else '0')
        printf('Total number of safe checks           : %s\n',
               bold_green(summary.ok)
               if summary.ok
               else '0')
        printf('Total number of definite unsafe checks: %s\n',
               bold_red(summary.error)
               if summary.error
               else '0')
        printf('Total number of warnings              : %s\n',
               bold_yellow(summary.warning)
               if summary.warning
               else '0')
        printf('\n')

    if summary.error == 0 and summary.warning == 0:
        printf(bold_green('The program is SAFE') + '\n')
    else:
        if summary.error != 0:
            printf(bold_red('The program is definitely UNSAFE') + '\n')
        else:
            printf(bold_yellow('The program is potentially UNSAFE') + '\n')


######################
# display raw checks #
######################


def memoize(f):
    ''' Decorator to wrap a function with memoizing '''
    cache = {}

    def helper(*args):
        if args not in cache:
            cache[args] = f(*args)
        return cache[args]

    return helper


@memoize
def format_path(path):
    '''
    Format a path.

    Return the shortest between the absolute path or the relative path.
    '''
    if not path:
        return None

    abs_path = os.path.realpath(path)

    # Check the drives to make sure a relpath can be obtained
    path_drive_split = os.path.splitdrive(abs_path)
    cur_drive_split  = os.path.splitdrive(os.getcwd())

    if path_drive_split and (path_drive_split[0] != cur_drive_split[0]):
        return abs_path
    else:
        rel_path = os.path.relpath(os.path.realpath(path), os.getcwd())
        return min(abs_path, rel_path, key=len)


def format_status(status):
    ''' Add colors to a status '''
    if status == 'ok':
        return bold_green(status)
    elif status == 'error':
        return bold_red(status)
    elif status == 'warning':
        return bold_yellow(status)
    elif status == 'unreachable':
        return bold_magenta(status)
    else:
        assert False, 'unexpected status'


def format_operands(operands):
    if not operands:
        return None

    operands = ('%d: %s' % (p.num, p.operand.repr) for p in operands)
    return ', '.join(operands)


def print_raw_checks(db, interprocedural):
    ''' Print all checks in the database, with very little processing '''
    header = [
        'context',
        'function',
        'file',
        'line',
        'col',
        'stmt',
        'check',
        'checker',
        'result',
        'operands',
        'info'
    ]
    order_by = 'call_context_id, statement_id, kind'

    if not interprocedural:
        header.pop(0)  # no context column if intraprocedural

    c = db.con.cursor()
    c.execute('SELECT * FROM checks ORDER BY %s' % order_by)
    rows = c.fetchall()

    # Format all rows
    for i, row in enumerate(rows):
        check = Check(row, db)
        statement = check.statement()
        function = statement.function()
        call_context = check.call_context()

        rows[i] = [call_context.str(),
                   function.pretty_name(),
                   format_path(statement.file_path()) or ' ',
                   str(statement.line_or(' ')),
                   str(statement.column_or(' ')),
                   str(statement.id),
                   CheckKind.short_name(check.kind),
                   CheckerName.short_name(check.checker),
                   Result.str(check.status),
                   format_operands(check.load_operands()) or ' ',
                   check.info or ' ']

        if not interprocedural:
            rows[i].pop(0)  # no context column if intraprocedural

    # Reorganize data by columns
    cols = zip(*([header] + rows))

    # Compute column widths by taking maximum length of values per column
    col_widths = [max(len(value) for value in col) for col in cols]

    # Print table
    printf(bold('# Checks') + '\n')
    printf('|' +
           '|'.join(' %s ' % head.ljust(width)
                    for head, width in zip(header, col_widths)) +
           '|\n')
    printf('+' + '+'.join('-' * (width + 2) for width in col_widths) + '+\n')
    for row in rows:
        fmt = ('|' +
               '|'.join(' %s ' + ' ' * (width - len(e))
                        for e, width in zip(row, col_widths)) +
               '|\n')
        row[-3] = format_status(row[-3])  # add colors for result column
        printf(fmt, *row)

    c.close()


##########
# report #
##########


class Report:
    ''' Represents an analysis report '''

    def __init__(self, db):
        self.db = db
        self.statement_reports = []

    def append(self, statement_report):
        self.statement_reports.append(statement_report)

    def __repr__(self):
        if not self.statement_reports:
            return 'Report([])'

        lines = ',\n'.join(map(repr, self.statement_reports))
        return 'Report([\n%s\n])' % lines


class StatementReport:
    ''' Represents a report on a statement '''

    def __init__(self,
                 db,
                 kind,
                 status,
                 statement_id,
                 call_context_ids,
                 operands=None,
                 info=None):
        self.db = db
        self.kind = kind
        self.status = status
        self.statement_id = statement_id
        self.call_context_ids = call_context_ids
        self.operands = operands
        self.info = info

    def statement(self):
        return self.db.statements[self.statement_id]

    def call_contexts(self):
        return (self.db.call_contexts[id] for id in self.call_context_ids)

    def load_operands(self):
        ''' Return the operands, or None '''
        if not self.operands:
            return None

        operands = json.loads(self.operands)
        return [NumOperandPair(no, self.db.operands[id])
                for no, id in operands]

    def load_info(self):
        ''' Return the info, or None '''
        if not self.info:
            return None

        return json.loads(self.info)

    def __repr__(self):
        s = ('StatementReport('
             'kind=%s, '
             'status=%s, '
             'statement_id=%d, '
             'context_ids=%r, '
             'operands=%s, '
             'info=%s)')
        return s % (CheckKind.short_name(self.kind),
                    Result.str(self.status),
                    self.statement_id,
                    self.call_context_ids,
                    self.operands,
                    self.info)


def generate_report(db, status_filter=None, analyses_filter=None):
    '''
    Generate an analysis report.

    Arguments:
        status_filter(list): List of status, or None
        analyses_filter(list): List of checkers, or None
    '''
    report = Report(db)

    # Parse filters
    if status_filter is not None:
        status_filter = tuple(map(Result.from_str, status_filter))

    if analyses_filter is not None:
        analyses_filter = tuple(map(CheckerName.from_short_name,
                                    analyses_filter))

    display_oks = (status_filter is None or Result.OK in status_filter)
    display_unreachables = ((status_filter is None or
                             Result.UNREACHABLE in status_filter) and
                            (analyses_filter is None or
                             CheckerName.DEAD_CODE in analyses_filter))

    # Generate where clause
    where = []

    if status_filter is not None:
        if not status_filter:
            where = ['0=1']  # status_filter=[], filter everything
        else:
            where.append(' OR '.join('(status=%d)' % status
                                     for status in status_filter))

    if analyses_filter is not None:
        if not analyses_filter:
            where = ['0=1']  # analyses_filter=[], filter everything
        elif len(analyses_filter) == len(args.analyses):
            pass  # nothing to filter
        else:
            where.append(' OR '.join('(checker=%d)' % checker
                                     for checker in analyses_filter))

    where = ' AND '.join('(%s)' % clause for clause in where)

    if display_unreachables and not display_oks:
        # Only show unreachable statements if the statement is unreachable for
        # all calling contexts. To detect this, we need to make sure to get all
        # checks from the DeadCodeChecker, especially 'ok' checks.
        where = '(%s) OR (checker=%d)' % (where, CheckerName.DEAD_CODE)

    if where:
        where = 'WHERE %s' % where

    order_by = 'ORDER BY statement_id, call_context_id'

    # Execute query
    c = db.con.cursor()
    c.execute('SELECT * FROM checks %s %s' % (where, order_by))

    stmt_id_key = operator.itemgetter(ChecksTable.STATEMENT_ID)
    context_id_key = operator.itemgetter(ChecksTable.CALL_CONTEXT_ID)

    for statement_id, statement_checks in itertools.groupby(c,
                                                            key=stmt_id_key):
        # Iterate over the checks for statement = statement_id
        statement_results = set()
        statement_context_ids = list()
        statement_checkers = set()
        statement_errors = collections.defaultdict(list)
        statement_warnings = collections.defaultdict(list)
        statement_oks = collections.defaultdict(list)

        for context_id, checks in itertools.groupby(statement_checks,
                                                    key=context_id_key):
            # Iterate over the checks for statement = statement_id
            # and context = context_id
            result, errors, warnings, oks, checkers = \
                generate_statement_result(checks,
                                          keep_oks=display_oks,
                                          keep_checkers=display_unreachables)
            statement_results.add(result)
            statement_context_ids.append(context_id)
            if display_unreachables:
                statement_checkers.union(checkers)
            for error in errors:
                statement_errors[error].append(context_id)
            for warning in warnings:
                statement_warnings[warning].append(context_id)
            if display_oks:
                for ok in oks:
                    statement_oks[ok].append(context_id)

        if statement_results == {Result.UNREACHABLE}:
            # Statement is unreachable for all the calling contexts,
            # check that we want unreachable checks and that it comes from dca
            if display_unreachables and CheckerName.DEAD_CODE in checkers:
                report.append(StatementReport(
                    db=db,
                    kind=CheckKind.UNREACHABLE,
                    status=Result.UNREACHABLE,
                    statement_id=statement_id,
                    call_context_ids=statement_context_ids
                ))
        else:
            for check, context_ids in statement_errors.items():
                report.append(StatementReport(db=db,
                                              kind=check.kind,
                                              status=Result.ERROR,
                                              statement_id=statement_id,
                                              call_context_ids=context_ids,
                                              operands=check.operands,
                                              info=check.info))
            for check, context_ids in statement_warnings.items():
                report.append(StatementReport(db=db,
                                              kind=check.kind,
                                              status=Result.WARNING,
                                              statement_id=statement_id,
                                              call_context_ids=context_ids,
                                              operands=check.operands,
                                              info=check.info))
            for check, context_ids in statement_oks.items():
                report.append(StatementReport(db=db,
                                              kind=check.kind,
                                              status=Result.OK,
                                              statement_id=statement_id,
                                              call_context_ids=context_ids,
                                              operands=check.operands,
                                              info=check.info))

    c.close()
    return report


##################
# report formats #
##################


class Formatter(object):
    ''' Base class for formatters '''

    def __init__(self, output, verbosity):
        '''
        Arguments:
            output(file): output file
            verbosity(int): verbosity level
        '''
        self.output = output
        self.verbosity = verbosity

    def format(self, report):
        raise NotImplementedError


class TextFormatter(Formatter):
    ''' Text output formatter (similar to clang compilation warnings) '''

    def __init__(self, output, verbosity):
        super(TextFormatter, self).__init__(output, verbosity)
        self.sources = {}

    # error, warning, unreachable, ok
    RESULT_ORDER = [3, 1, 0, 2]

    @classmethod
    def sorting_key(cls, report):
        statement = report.statement()
        return (cls.RESULT_ORDER[report.status],
                statement.file_id_or(-1),
                statement.line_or(-1),
                statement.column_or(-1),
                report.kind)

    def write_path(self, file):
        printf(bold('%s: '), format_path(file.path) if file else '?',
               file=self.output)

    def write_in_function(self, function):
        printf("In function '%s':", function.pretty_name(), file=self.output)

    def write_newline(self):
        printf('\n', file=self.output)

    def write_source_location(self, statement):
        printf(bold('%s:%s:%s: '),
               format_path(statement.file_path()) or '?',
               statement.line_or('?'),
               statement.column_or('?'),
               file=self.output)

    RESULT_STR = ['safe', 'warning', 'error', 'unreachable']
    RESULT_FORMATTER = [bold_green, bold_yellow, bold_red, bold_magenta]

    def write_status(self, result):
        formatter = TextFormatter.RESULT_FORMATTER[result]
        printf('%s: ' % formatter(TextFormatter.RESULT_STR[result]),
               file=self.output)

    def write_note(self):
        printf('%s: ' % bold_blue('note'), file=self.output)

    def source_location_indent(self, statement):
        return len('%s:%s:%s: ' % (format_path(statement.file_path()) or '?',
                                   statement.line_or('?'),
                                   statement.column_or('?')))

    def status_indent(self, result):
        return len('%s: ' % TextFormatter.RESULT_STR[result])

    def write_message(self, statement, result, message):
        if '\n' in message:
            # Assume write_source_location and write_status have been called
            n = self.source_location_indent(statement)
            n += self.status_indent(result)
            message = message.replace('\n', '\n' + ' ' * n)

        printf('%s\n', message, file=self.output)

    def load_source_code(self, path):
        if path not in self.sources:
            try:
                with io.open(path, 'r',
                             encoding='utf-8',
                             errors='ignore') as f:
                    self.sources[path] = tuple(f.readlines())
            except IOError:
                printf('warning: could not open file %s\n', path,
                       file=sys.stderr)
                self.sources[path] = []

        return self.sources[path]

    def write_source_code(self, statement, pointer_color=bold_green):
        if statement.file_id is not None and statement.line is not None:
            source_code = self.load_source_code(statement.file_path())

            if statement.line <= len(source_code):
                line = source_code[statement.line - 1].rstrip()
                printf(line + '\n', file=self.output)
                if statement.column is not None:
                    footer = ''.join('\t' if c == '\t' else ' '
                                     for c in line[:statement.column - 1])
                    footer += pointer_color('^')
                    printf(footer + '\n', file=self.output)

    def write_call_context(self, statement, call_context, max_depth=3):
        self.write_source_location(statement)
        self.write_note()

        if call_context.empty():
            function = statement.function()
            printf("called from entry point '%s'\n",
                   function.pretty_name(), file=self.output)
            return

        printf('called from:\n', file=self.output)
        for _ in range(max_depth):
            if call_context.empty():
                return

            call_statement = call_context.call()
            function = call_context.function()

            self.write_source_location(call_statement)
            printf("function '%s'\n", function.pretty_name(), file=self.output)

            self.write_source_code(call_statement)

            call_context = call_context.parent()

    def format(self, report):
        statement_reports = report.statement_reports

        # Sort reports
        statement_reports.sort(key=self.sorting_key)

        for statement_report in statement_reports:
            statement = statement_report.statement()
            function = statement.function()
            message = generate_message(statement_report, self.verbosity)

            self.write_path(function.file())
            self.write_in_function(function)
            self.write_newline()

            self.write_source_location(statement)
            self.write_status(statement_report.status)
            self.write_message(statement, statement_report.status, message)

            self.write_source_code(statement)

            if self.verbosity >= 3:
                for call_context in statement_report.call_contexts():
                    self.write_call_context(statement, call_context)


class JSONEncoder(json.JSONEncoder):
    ''' JSON encoder '''

    def default(self, obj):
        if isinstance(obj, Report):
            return self.encode_report(obj)
        elif isinstance(obj, StatementReport):
            return self.encode_statement_report(obj)
        elif isinstance(obj, File):
            return self.encode_file(obj)
        elif isinstance(obj, Function):
            return self.encode_function(obj)
        elif isinstance(obj, Statement):
            return self.encode_statement(obj)
        elif isinstance(obj, Operand):
            return self.encode_operand(obj)
        elif isinstance(obj, CallContext):
            return self.encode_call_context(obj)
        elif isinstance(obj, MemoryLocation):
            return self.encode_memory_location(obj)

        return json.JSONEncoder.default(self, obj)

    @staticmethod
    def encode_report(report):
        return {
            'files': report.db.files,
            'functions': report.db.functions,
            'statements': report.db.statements,
            'operands': report.db.operands,
            'call_contexts': report.db.call_contexts,
            'memory_locations': report.db.memory_locations,
            'reports': report.statement_reports,
        }

    @staticmethod
    def encode_statement_report(report):
        operands = None
        if report.operands is not None:
            operands = [(p.num, p.operand.id) for p in report.load_operands()]

        return {
            'kind': report.kind,
            'status': report.status,
            'statement_id': report.statement_id,
            'call_context_ids': report.call_context_ids,
            'operands': operands,
            'info': report.load_info(),
        }

    @staticmethod
    def encode_file(file):
        return {
            'id': file.id,
            'path': file.path,
        }

    @staticmethod
    def encode_function(function):
        return {
            'id': function.id,
            'name': function.name,
            'demangled': function.demangled,
            'definition': function.definition,
            'file_id': function.file_id,
            'line': function.line,
        }

    @staticmethod
    def encode_statement(statement):
        return {
            'id': statement.id,
            'kind': statement.kind,
            'function_id': statement.function_id,
            'file_id': statement.file_id,
            'line': statement.line,
            'column': statement.column,
        }

    @staticmethod
    def encode_operand(operand):
        return {
            'id': operand.id,
            'kind': operand.kind,
            'repr': operand.repr,
        }

    @staticmethod
    def encode_call_context(call_context):
        return {
            'id': call_context.id,
            'call_id': call_context.call_id,
            'function_id': call_context.function_id,
            'parent_id': call_context.parent_id,
        }

    @staticmethod
    def encode_memory_location(memory_location):
        return {
            'id': memory_location.id,
            'kind': memory_location.kind,
            'info': memory_location.load_info()
        }


class JSONFormatter(Formatter):
    ''' JSON output formatter '''

    def format(self, report):
        json.dump(report, self.output, cls=JSONEncoder)
        self.output.write('\n')


class SARIFFormatter(Formatter):
    ''' SARIF output formatter '''

    checks = [
        ('buffer_overflow_analysis', 'checks for buffer overflows and out-of-bound array accesses.'),
        ('division_by_zero_analysis', 'checks for integer divisions by zero.'),
        ('null_pointer_analysis', 'checks for null pointer dereferences.'),
        ('assertion_prover', 'prove user-defined properties, using __ikos_assert(condition).'),
        ('unaligned_pointer_analysis', 'checks for unaligned pointer dereferences.'),
        ('uninitialized_variable_analysis', 'checks for read of uninitialized variables.'),
        ('signed_integer_overflow_analysis', 'checks for signed integer overflows.'),
        ('unsigned_integer_overflow_analysis', 'checks for unsigned integer overflows.'),
        ('shift_count_analysis', 'checks for invalid shifts, where the amount shifted is greater or equal to the bit-width of the left operand, or less than zero.'),
        ('pointer_overflow_analysis', 'checks for pointer arithmetic overflows.'),
        ('pointer_comparison_analysis', 'checks for pointer comparisons between pointers referring to different objects.'),
        ('soundness_analysis', 'checks for instructions that could make the analysis unsound, i.e miss bugs.'),
        ('function_call_analysis', 'checks for function calls through function pointers of the wrong type.'),
        ('dead_code_analysis', 'checks for unreachable statements.'),
        ('double_free_analysis', 'checks for double free, invalid free, use after free and use after return.'),
        ('debugger', 'prints debug information, using __ikos_print_values("desc", x) and __ikos_print_invariant().'),
        ('memory_watcher', 'prints memory writes at a given memory location, using __ikos_watch_mem(ptr, size)')
    ]

    rules = [
        {
            'id': name,
            'shortDescription': {'text': text},
            'helpUri': 'https://github.com/NASA-SW-VnV/ikos/blob/master/analyzer/README.md#checks'
        }
        for (name, text) in checks
    ]

    tool = {
        'driver': {
            'name': 'IKOS',
            'version': settings.VERSION,
            'informationUri': 'https://github.com/NASA-SW-VnV/ikos',
            'rules': rules,
        },
    }

    def format_artifacts(self, report):
        files = set()
        for statement_report in report.statement_reports:
            statement = statement_report.statement()
            name = statement.file_path()
            if name:
                files.add(name)
        return [{'location': {'uri': format_path(file)}} for file in files]

    # the only levels in SARIF are error, warning, and note => change unreachable into note
    def format_level(self, level):
        if level == "unreachable":
            return "note"
        else:
            return level

    # some messages provided by IKOS are multi-line but it's not possible under the SARIF format:
    # change it into some single line message
    def single_line(self, text):
        # bundle multi-line messages into single-line messages
        if '\n\t*' in text:
            return text.replace('\n\t*', '  +--> ')
        else:
            return text

    def format_location(self, statement, message=None):
        path = format_path(statement.file_path())
        artifact = {'uri': path}
        region = {'startLine': statement.line, 'startColumn': statement.column}
        physical = {'artifactLocation': artifact, 'region': region}
        location = {'physicalLocation': physical}
        if message:
            location['message'] = {'text': message}
        return location

    def format_stacks(self, statement_report):
        call_contexts = statement_report.call_contexts()
        stacks = []
        for context in call_contexts:
            frames = []
            while not context.empty():
                call = context.call()
                func = call.function().pretty_name()
                location = self.format_location(call, message="Call from %s" % func)
                frames.append({'location': location})
                context = context.parent()
            if frames:
                # The Sarif Viewer extension for VS Code ignores stacks without
                # a message for some reason
                message = {'text': str(context)}
                stack = {'message': message, 'frames': frames}
                stacks.append(stack)
        return stacks

    def format_results(self, report):
        results = []
        for statement_report in report.statement_reports:
            statement = statement_report.statement()
            path = statement.file_path()
            # check if it's not a garbage result by checking if a filename path is provided
            # and if the line and column numbers are not zero
            if path and statement.line > 0 and statement.column > 0:
                result = {
                    'ruleId': CheckKind.short_name(statement_report.kind),
                    'level': self.format_level(Result.str(statement_report.status)),
                    'message': {
                        'text': quoteattr(self.single_line(generate_message(statement_report, self.verbosity))),
                    },
                    'locations': [self.format_location(statement)],
                }
                stacks = self.format_stacks(statement_report)
                if stacks:
                    result['stacks'] = stacks
                results.append(result)
        return results

    # main method to format an IKOS report into a SARIF formatted report
    def format(self, report):
        run = {
            'tool': self.tool,
            'artifacts': self.format_artifacts(report),
            'results': self.format_results(report),
        }
        log = {
            'version': '2.1.0',
            '$schema': 'http://json.schemastore.org/sarif-2.1.0',
            'runs': [run],
        }
        json.dump(log, self.output, indent = 3)
        self.output.write('\n')


class CSVFormatter(Formatter):
    ''' CSV output formatter '''

    def format(self, report):
        writer = csv.writer(self.output, quoting=csv.QUOTE_MINIMAL)
        writer.writerow([
            'file',
            'function',
            'line',
            'column',
            'statement_id',
            'contexts',
            'status',
            'check',
            'message'
        ])

        for statement_report in report.statement_reports:
            statement = statement_report.statement()
            function = statement.function()

            writer.writerow([
                statement.file_path(),
                function.pretty_name(),
                statement.line,
                statement.column,
                statement.id,
                ' | '.join(call_context.str()
                           for call_context
                           in statement_report.call_contexts()),
                Result.str(statement_report.status),
                CheckKind.short_name(statement_report.kind),
                generate_message(statement_report, self.verbosity),
            ])


class JUnitFormatter(Formatter):
    ''' JUnit.xml formatter '''

    def get_timing_result(self, report):
        ''' get the timing results for the analysis from the database '''
        db = report.db
        elapsed = 0.0
        if db:
            results = db.load_timing_results(True, True)
            for result in results:
                if result[0] == 'ikos-analyzer':
                    return result[1]
        return elapsed

    # some messages provided by IKOS are multi-line but 
    # we want to change it into some single line message
    def single_line (self, text):
        # 
        if '\n\t*' in text:
            return text.replace('\n\t*', '  +--> ')
        else:
            return text

    def format(self, report):
        testname = '.ikos-analysis-results'
        # get the time spent on analysis
        elapsed = self.get_timing_result (report)
        # get the summary report
        summary = generate_summary (report.db)
        test_count = summary.ok + summary.error + summary.warning + summary.unreachable
        error_count = summary.error + summary.warning
        # build the data structure to report the analysis summary
        data = {
            'testname': testname,
            'test_count': test_count,
            'error_count': error_count,
            'time': '%.3f' % round(elapsed, 3),
            'skip': 0,
        }
        xml = """<?xml version="1.0" encoding="UTF-8"?>
<testsuite
  name="%(testname)s"
  tests="%(test_count)d"
  errors="0"
  failures="%(error_count)d"
  time="%(time)s"
  skipped="%(skip)d"
>
""" % data

        for statement_report in report.statement_reports:
            statement = statement_report.statement()
            
            path = statement.file_path()
            # make sure that it's not one of those unreachable statements without any info
            if path and statement.line > 0 and statement.column > 0:
                # report each error/warning as a failing testcase
                data = {
                    'quoted_name': quoteattr(
                        '%s: %s (%s:%d)' % (
                            Result.str(statement_report.status), 
                            CheckKind.short_name(statement_report.kind),
                            path, statement.line)),
                    'testname': testname,
                    'quoted_message': quoteattr(self.single_line(generate_message(statement_report, self.verbosity))),
                }
                xml += """  <testcase
    name=%(quoted_name)s
    classname="%(testname)s"
  >
      <failure message=%(quoted_message)s/>
  </testcase>
""" % data

        xml += '</testsuite>'
        self.output.write(xml)


class AutoFormatter(TextFormatter):
    '''
    Automatic output formatter

    If the number of report is too big, it recommends to use ikos-report.
    Otherwise, it uses the TextFormatter.
    '''

    MAX_NUM_REPORT = 15

    def format(self, report):
        if len(report.statement_reports) == 0:
            printf('No entries.\n', file=self.output)
        elif len(report.statement_reports) > AutoFormatter.MAX_NUM_REPORT:
            printf('Report is too big (> %d entries)\n\n'
                   'Use `ikos-report %s` to examine the report'
                   ' in your terminal.\n'
                   'Use `ikos-view %s` to examine the report'
                   ' in a web interface.\n',
                   AutoFormatter.MAX_NUM_REPORT,
                   report.db.path,
                   report.db.path,
                   file=self.output)
        else:
            super(AutoFormatter, self).format(report)


# available formats
formats = {
    'text': TextFormatter,
    'json': JSONFormatter,
    'sarif': SARIFFormatter,
    'csv': CSVFormatter,
    'auto': AutoFormatter,
    'junit': JUnitFormatter
}


###################
# report messages #
###################


def generate_message(report, verbosity):
    ''' Generate a message for the given statement report and verbosity '''
    return GENERATE_MESSAGE_MAP[report.kind](report, verbosity)


def is_variable_name(s):
    ''' Return true if the given string is a variable name '''
    # First letter is alpha or underscore
    # Everything else is alphanumeric or underscore
    return (s and
            (s[0].isalpha() or s[0] == '_') and
            all(c.isalnum() or c == '_' for c in s))


def ordinal_str(num):
    assert num >= 1

    if num == 1:
        return 'first'
    elif num == 2:
        return 'second'
    elif num == 3:
        return 'third'

    if num > 9:
        if str(num)[-2] == '1':
            return '%dth' % num

    last_digit = num % 10
    if last_digit == 1:
        return '%dst' % num
    elif last_digit == 2:
        return '%dnd' % num
    elif last_digit == 3:
        return '%drd' % num
    else:
        return '%dth' % num


def statement_operand_str(statement, num):
    '''
    Return a string to represent the num-th operand of the given statement
    '''
    if (statement.kind == StatementKind.ASSIGNMENT or
            statement.kind == StatementKind.UNARY_OPERATION):
        return 'right hand side'
    elif (statement.kind == StatementKind.BINARY_OPERATION or
            statement.kind == StatementKind.COMPARISON):
        if num == 0:
            return 'left operand'
        else:
            return 'right operand'
    elif statement.kind == StatementKind.RETURN:
        return 'returned value'
    elif statement.kind == StatementKind.ALLOCATE:
        return 'size operand'
    elif statement.kind == StatementKind.POINTER_SHIFT:
        if num == 0:
            return 'base operand'
        else:
            return '%s operand' % ordinal_str(num)
    elif statement.kind == StatementKind.LOAD:
        return 'pointer'
    elif statement.kind == StatementKind.STORE:
        if num == 0:
            return 'pointer'
        else:
            return 'stored value'
    elif (statement.kind == StatementKind.EXTRACT_ELEMENT or
            statement.kind == StatementKind.INSERT_ELEMENT):
        return '%s operand' % ordinal_str(num + 1)
    elif (statement.kind == StatementKind.CALL or
            statement.kind == StatementKind.INVOKE):
        if num == 0:
            return 'function pointer'
        else:
            return '%s argument' % ordinal_str(num)
    elif (statement.kind == StatementKind.LANDING_PAD or
            statement.kind == StatementKind.RESUME):
        return 'operand'
    else:
        assert False, 'unexpected kind'


def memory_location_str(mem_loc):
    ''' Return a string to represent the given memory location '''
    info = mem_loc.load_info()

    if mem_loc.kind == MemoryLocationKind.LOCAL:
        if info and 'name' in info:
            return "local variable '%s'" % info['name']
        else:
            return 'unnamed local variable'
    elif mem_loc.kind == MemoryLocationKind.GLOBAL:
        if 'demangle' in info:
            return "global variable '%s'" % info['demangle']
        elif 'name' in info:
            return "global variable '%s'" % info['name']
        elif 'cst' in info:
            return "constant %s" % info['cst']
        else:
            return 'unnamed global variable'
    elif mem_loc.kind == MemoryLocationKind.FUNCTION:
        function = mem_loc.db.functions[info['id']]
        return "function '%s'" % function.pretty_name()
    elif mem_loc.kind == MemoryLocationKind.AGGREGATE:
        return 'aggregate variable'
    elif mem_loc.kind == MemoryLocationKind.ABSOLUTE_ZERO:
        return 'zero'
    elif mem_loc.kind == MemoryLocationKind.ARGV:
        return "'argv'"
    elif mem_loc.kind == MemoryLocationKind.LIBC_ERRNO:
        return "'errno'"
    elif mem_loc.kind == MemoryLocationKind.DYN_ALLOC:
        call = mem_loc.db.statements[info['call_id']]
        function = call.function()

        if call.line is not None and call.column is not None:
            return "dynamic memory allocated at '%s:%d:%d'" % (
                function.pretty_name(),
                call.line,
                call.column
            )
        else:
            return "dynamic memory allocated in '%s'" % function.pretty_name()
    else:
        assert False, 'unexpected kind'


def generate_unreachable_message(report, verbosity):
    if report.status == Result.OK:
        return 'statement is reachable'
    elif report.status == Result.UNREACHABLE:
        return 'code is dead'
    else:
        assert False, 'unexpected status'


def generate_unexpected_operand_message(report, verbosity):
    assert report.status == Result.ERROR
    return 'unexpected operand'


def generate_uninitialized_variable_message(report, verbosity):
    if report.status == Result.OK:
        return 'operand is well initialized'

    (num, operand), = report.load_operands()
    if ValueKind.BEGIN_CONSTANT <= operand.kind <= ValueKind.END_CONSTANT:
        s = statement_operand_str(report.statement(), num)
    elif ValueKind.BEGIN_VARIABLE <= operand.kind <= ValueKind.END_VARIABLE:
        if is_variable_name(operand.repr):
            s = "variable '%s'" % operand.repr
        else:
            s = "expression '%s'" % operand.repr
    else:
        assert False, 'unexpected operand'

    if report.status == Result.WARNING:
        s += ' might be uninitialized'
    elif report.status == Result.ERROR:
        s += ' is uninitialized'
    else:
        assert False, 'unexpected status'

    return s


def generate_assert_message(report, verbosity):
    if report.status == Result.OK:
        return 'assertion holds'
    elif report.status == Result.WARNING:
        return 'assertion could not be proven'
    elif report.status == Result.ERROR:
        return 'assertion never holds'
    else:
        assert False, 'unexpected status'


def generate_division_by_zero_message(report, verbosity):
    if report.status == Result.OK:
        return 'divisor is not zero'
    elif report.status == Result.ERROR:
        return 'division by zero'

    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    interval = Interval.from_dict(report.load_info())

    s = 'divisor might be zero'
    if verbosity >= 2 or not interval.is_top():
        if is_variable_name(operand.repr):
            s += ' (%s)' % interval.to_constraints(operand.repr)
        else:
            s += ' (%s)' % interval.to_constraints('divisor')

    return s


def generate_shift_count_message(report, verbosity):
    if report.status == Result.OK:
        return 'shift count is valid'

    if report.status == Result.ERROR:
        s = 'invalid shift count'
    elif report.status == Result.WARNING:
        s = 'shift count might be invalid'
    else:
        assert False, 'unexpected status'

    (_, operand), = report.load_operands()
    interval = Interval.from_dict(report.load_info())
    if ((verbosity >= 2 or not interval.is_top()) and
            operand.kind != ValueKind.INTEGER_CONSTANT):
        if is_variable_name(operand.repr):
            s += ' (%s)' % interval.to_constraints(operand.repr)
        else:
            s += ' (%s)' % interval.to_constraints('count')

    if verbosity >= 2:
        n = interval.bit_width - 1
        s += '\nshift count is required to be between 0 and %d' % n

    return s


def generate_integer_overflow_message(report, verbosity, signedness, kind):
    if report.status == Result.OK:
        return 'safe from %s integer %s' % (signedness, kind)

    if report.status == Result.ERROR:
        s = '%s integer %s' % (signedness, kind)
    elif report.status == Result.WARNING:
        s = 'possible %s integer %s' % (signedness, kind)
    else:
        assert False, 'unexpected status'

    (_, left_operand), (_, right_operand) = report.load_operands()
    info = report.load_info()
    left_interval = Interval.from_dict(info['left'])
    right_interval = Interval.from_dict(info['right'])

    ops = []
    if ((verbosity >= 2 or not left_interval.is_top()) and
            left_operand.kind != ValueKind.INTEGER_CONSTANT):
        if is_variable_name(left_operand.repr):
            ops.append(left_interval.to_constraints(left_operand.repr))
        else:
            ops.append(left_interval.to_constraints('left'))
    if ((verbosity >= 2 or not right_interval.is_top()) and
            right_operand.kind != ValueKind.INTEGER_CONSTANT):
        if is_variable_name(right_operand.repr):
            ops.append(right_interval.to_constraints(right_operand.repr))
        else:
            ops.append(right_interval.to_constraints('right'))

    if ops:
        s += ' (%s)' % ', '.join(ops)

    if verbosity >= 2:
        s += "\nbetween operands '%s' and '%s'" % (
            left_operand.repr,
            right_operand.repr
        )

    return s


def generate_null_pointer_deref_message(report, verbosity):
    if report.status == Result.OK:
        return 'pointer is non-null'

    (num, operand), = report.load_operands()
    if ValueKind.BEGIN_CONSTANT <= operand.kind <= ValueKind.END_CONSTANT:
        s = statement_operand_str(report.statement(), num)
    elif ValueKind.BEGIN_VARIABLE <= operand.kind <= ValueKind.END_VARIABLE:
        s = "pointer '%s'" % operand.repr
    else:
        assert False, 'unexpected operand'

    if report.status == Result.WARNING:
        s += ' might be null'
    elif report.status == Result.ERROR:
        s += ' is null'
    else:
        assert False, 'unexpected status'

    return s


def generate_null_pointer_cmp_message(report, verbosity):
    assert report.status == Result.ERROR
    (num, _), = report.load_operands()
    if num == 0:
        return 'invalid comparison with null as left operand'
    else:
        return 'invalid comparison with null as right operand'


def generate_invalid_pointer_cmp_message(report, verbosity):
    assert report.status == Result.ERROR
    (_, operand), = report.load_operands()
    return "pointer '%s' is invalid" % operand.repr


def generate_pointer_cmp_message(report, verbosity):
    if report.status == Result.OK:
        return 'safe pointer comparison'
    elif report.status == Result.ERROR:
        return 'comparison of pointers referring to different objects'
    elif report.status == Result.WARNING:
        return 'comparison of pointers that might refer to different objects'
    else:
        assert False, 'unexpected status'


def generate_pointer_overflow_message(report, verbosity):
    if report.status == Result.OK:
        return 'safe pointer arithmetic'
    elif report.status == Result.WARNING:
        return 'pointer arithmetic might overflow'
    elif report.status == Result.ERROR:
        return 'pointer arithmetic overflow'
    else:
        assert False, 'unexpected status'


def generate_invalid_pointer_deref_message(report, verbosity):
    assert report.status == Result.ERROR
    (_, operand), = report.load_operands()
    s = "pointer '%s' is invalid" % operand.repr
    return s


def generate_unknown_memory_access_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'memory access might be invalid'
    s += ", could not infer information about pointer '%s'" % operand.repr
    return s


def generate_unaligned_pointer_message(report, verbosity):
    if report.status == Result.OK:
        return 'memory access is well aligned'

    if report.status == Result.WARNING:
        s = 'memory access might be unaligned'
    elif report.status == Result.ERROR:
        s = 'memory access is unaligned'
    else:
        assert False, 'unexpected status'

    info = report.load_info()
    requirement = Congruence.from_dict(info['requirement'])
    s += ', access requires %d bytes alignment' % requirement.a

    if verbosity <= 1:
        return s

    offset = Congruence.from_dict(info['offset'])
    if offset.is_top():
        s += '\npointer offset is unknown'
    elif offset.a == 0:
        s += '\npointer offset is %d bytes' % offset.b
    elif offset.b == 0:
        s += '\npointer offset is a multiple of %d bytes' % offset.a,
    else:
        s += '\npointer offset is equal to (%d modulo %d) bytes' % (
            offset.b,
            offset.a
        )

    points_to = []
    for block_info in info['points_to']:
        mem_loc_id = block_info['id']
        mem_loc = report.db.memory_locations[mem_loc_id]

        if mem_loc.kind == MemoryLocationKind.ABSOLUTE_ZERO:
            continue

        line = memory_location_str(mem_loc)

        if 'congruence' in block_info:
            congruence = Congruence.from_dict(block_info['congruence'])
            if not congruence.is_top():
                line += ' with alignment of %d bytes' % congruence.a

        points_to.append(line)

    # make the output deterministic
    points_to.sort()

    if len(points_to) == 1:
        s += '\npointer points to ' + points_to[0]
    elif len(points_to) > 1:
        lines = ''.join('\n\t* %s' % p for p in points_to)
        s += '\npointer points to:' + lines

    return s


def generate_buffer_overflow_gets_message(report, verbosity):
    assert report.status == Result.ERROR
    return "call to unsafe function 'gets'"


def generate_buffer_overflow_message(report, verbosity):
    if report.status == Result.OK:
        return 'safe memory access'

    info = report.load_info()
    kinds = set(block_info['kind'] for block_info in info['points_to'])

    if len(kinds) > 1:
        # different kind of errors, use a generic message
        if report.status == Result.WARNING:
            s = 'memory access might be invalid'
        elif report.status == Result.ERROR:
            s = 'invalid memory access'
        else:
            assert False, 'unexpected status'
    elif kinds == {BufferOverflowCheckKind.FUNCTION}:
        assert report.status == Result.ERROR
        s = 'dereferencing a function pointer'
    elif kinds == {BufferOverflowCheckKind.USE_AFTER_FREE}:
        if report.status == Result.WARNING:
            s = 'possible use after free'
        elif report.status == Result.ERROR:
            s = 'use after free'
        else:
            assert False, 'unexpected status'
    elif kinds == {BufferOverflowCheckKind.USE_AFTER_RETURN}:
        if report.status == Result.WARNING:
            s = 'possible use after return'
        elif report.status == Result.ERROR:
            s = 'use after return'
        else:
            assert False, 'unexpected status'
    elif kinds == {BufferOverflowCheckKind.HARDWARE_ADDRESSES}:
        if report.status == Result.WARNING:
            s = 'memory access might be invalid'
        elif report.status == Result.ERROR:
            s = 'invalid memory access'
        else:
            assert False, 'unexpected status'

        access_size = Interval.from_dict(info['access_size'])
        offset = Interval.from_dict(info['offset'])

        if access_size.ub.is_max():
            s += ', could not bound access size'
            return s
        if offset.ub.is_max():
            s += ', could not bound offset'
            return s

        if access_size.is_constant():
            s += ', accessing %d bytes' % access_size.ub.n
        elif not access_size.lb.is_min():
            s += ', accessing between %d and %d bytes' % (
                access_size.lb.n,
                access_size.ub.n
            )
        else:
            s += ', accessing up to %d bytes' % access_size.ub.n

        if offset.is_constant():
            s += ' at address 0x%x' % offset.ub.n
        else:
            s += ' at address between 0x%x and 0x%x' % (
                offset.lb.n,
                offset.ub.n
            )

        return s
    elif kinds == {BufferOverflowCheckKind.OUT_OF_BOUND}:
        if report.status == Result.WARNING:
            s = 'possible buffer overflow'
        elif report.status == Result.ERROR:
            s = 'buffer overflow'
        else:
            assert False, 'unexpected status'
    else:
        assert False, 'unexpected kind'

    # Size of an array element, or None if it's not an array access
    array_element_size = info.get('array_element_size', None)

    points_to = []
    for block_info in info['points_to']:
        status = block_info['status']
        kind = block_info['kind']
        mem_loc_id = block_info['id']
        mem_loc = report.db.memory_locations[mem_loc_id]

        line = memory_location_str(mem_loc)

        if kind == BufferOverflowCheckKind.OUT_OF_BOUND:
            size = Interval.from_dict(block_info['size'])

            if array_element_size:
                if size.is_constant():
                    n = size.ub.n // array_element_size
                    line += ' of %d elements' % n
                elif not size.ub.is_max():
                    ub = size.ub.n // array_element_size
                    line += ' of at most %d elements' % ub
            else:
                if size.is_constant():
                    line += ' of size %d bytes' % size.ub.n
                elif not size.ub.is_max():
                    line += ' of size at most %d bytes' % size.ub.n

        if len(info['points_to']) == 1:
            # no need for further explanation
            points_to.append(line)
            break

        if kind == BufferOverflowCheckKind.FUNCTION:
            assert status == Result.ERROR
            line += ', which is a function'
        elif kind == BufferOverflowCheckKind.USE_AFTER_FREE:
            if status == Result.WARNING:
                line += ', which might be released'
            elif status == Result.ERROR:
                line += ', which is released'
            else:
                assert False, 'unexpected status'
        elif kind == BufferOverflowCheckKind.USE_AFTER_RETURN:
            if status == Result.WARNING:
                line += ', which might be out of scope'
            elif status == Result.ERROR:
                line += ', which is out of scope'
            else:
                assert False, 'unexpected status'
        elif kind == BufferOverflowCheckKind.HARDWARE_ADDRESSES:
            if status == Result.WARNING:
                line += ', which might not be a valid hardware address'
            elif status == Result.ERROR:
                line += ', which is an invalid hardware address'
            else:
                assert False, 'unexpected status'
        elif kind == BufferOverflowCheckKind.OUT_OF_BOUND:
            if status == Result.OK:
                line += ', which is valid'
            elif status == Result.WARNING:
                line += ', which might be out of bounds'
            elif status == Result.ERROR:
                line += ', which is out of bounds'
            else:
                assert False, 'unexpected status'
        else:
            assert False, 'unexpected kind'

        points_to.append(line)

    # make the output deterministic
    points_to.sort()

    if kinds == {BufferOverflowCheckKind.OUT_OF_BOUND} and array_element_size:
        # Out of bound array access
        offset = Interval.from_dict(info['offset'])
        offset = offset.sign_cast(Signedness.SIGNED)

        if offset.is_constant():
            n = offset.ub.n // array_element_size
            s += ', accessing index %d' % n
        elif not offset.ub.is_max():
            ub = offset.ub.n // array_element_size
            if not offset.lb.is_min():
                lb = offset.lb.n // array_element_size
                s += ', accessing index between %d and %d' % (lb, ub)
            else:
                s += ', accessing index up to %d' % ub
        else:
            s += ', could not bound index for access'

        if len(points_to) == 1:
            s += ' of %s' % points_to[0]
        else:
            s += ' of:%s' % ''.join('\n\t* %s' % p for p in points_to)

        return s

    (_, operand), _ = report.load_operands()
    s += ", pointer '%s'" % operand.repr
    points_to_prep = ' points to'

    if BufferOverflowCheckKind.OUT_OF_BOUND in kinds:
        offset_prep = ' at'
        points_to_prep = ' of'

        access_size = Interval.from_dict(info['access_size'])
        if access_size.is_constant():
            s += ' accesses %d bytes' % access_size.ub.n
        elif not access_size.ub.is_max():
            if not access_size.lb.is_min():
                s += ' accesses between %d and %d bytes' % (
                    access_size.lb.n,
                    access_size.ub.n
                )
            else:
                s += ' accesses up to %d bytes' % access_size.ub.n
        else:
            offset_prep = ' with'
            points_to_prep = ' points to'

        offset = Interval.from_dict(info['offset'])
        offset = offset.sign_cast(Signedness.SIGNED)
        if offset.is_constant():
            s += offset_prep + ' offset %d bytes' % offset.ub.n
        elif not offset.ub.is_max():
            if not offset.lb.is_min():
                s += offset_prep + ' offset between %d and %d bytes' % (
                    offset.lb.n,
                    offset.ub.n
                )
            else:
                s += offset_prep + ' offset up to %d bytes' % offset.ub.n

    s += points_to_prep
    if len(points_to) == 1:
        s += ' %s' % points_to[0]
    else:
        s += ':%s' % ''.join('\n\t* %s' % p for p in points_to)

    return s


def generate_ignored_store_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'ignored memory write'
    s += ", could not infer information about pointer '%s'." % operand.repr
    s += ' Analysis might be unsound.'
    return s


def generate_ignored_memcpy_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'ignored memcpy()'
    s += ", could not infer information about pointer '%s'." % operand.repr
    s += ' Analysis might be unsound.'
    return s


def generate_ignored_memmove_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'ignored memmove()'
    s += ", could not infer information about pointer '%s'." % operand.repr
    s += ' Analysis might be unsound.'
    return s


def generate_ignored_memset_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'ignored memset()'
    s += ", could not infer information about pointer '%s'." % operand.repr
    s += ' Analysis might be unsound.'
    return s


def generate_ignored_free_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'ignored memory deallocation'
    s += ", could not infer information about pointer '%s'." % operand.repr
    s += ' Analysis might be unsound.'
    return s


def generate_ignored_call_side_effect_on_pointer_param_message(report,
                                                               verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    info = report.load_info()
    function_id = info['fun_id']
    function = report.db.functions[function_id]
    s = "ignored side effect of call to function '%s'" % function.pretty_name()
    s += ", could not infer information about pointer '%s'." % operand.repr
    s += ' Analysis might be unsound.'
    return s


def generate_ignored_call_side_effect_message(report, verbosity):
    assert report.status == Result.WARNING
    info = report.load_info()
    function_id = info['fun_id']
    function = report.db.functions[function_id]
    s = "ignored side effect of call to extern function '%s'."
    s = s % function.pretty_name()
    s += ' Analysis might be unsound.'
    return s


def generate_recursive_function_call_message(report, verbosity):
    assert report.status == Result.WARNING
    info = report.load_info()
    function_id = info['fun_id']
    function = report.db.functions[function_id]
    s = "function call to '%s' is recursive."
    s = s % function.pretty_name()
    s += ' Analysis might be unsound.'
    return s


def generate_call_inline_asm_message(report, verbosity):
    assert report.status == Result.OK
    return 'safe call to inline assembly code'


def generate_unknown_function_call_message(report, verbosity):
    assert report.status == Result.WARNING
    (_, operand), = report.load_operands()
    s = 'function call might be unsafe'
    s += ", could not infer information about pointer '%s'" % operand.repr
    return s


def generate_function_call_message(report, verbosity):
    info = report.load_info()

    if report.status == Result.OK:
        s = 'safe function call'

        callees = []
        for block_info in info['points_to']:
            function_id = block_info['fun_id']
            function = report.db.functions[function_id]
            callees.append("'%s'" % function.pretty_name())

        # make the output deterministic
        callees.sort()

        if len(callees) == 1:
            s += ' to %s' % callees[0]
        else:
            lines = ''.join('\n\t* %s' % c for c in callees)
            s += ', called functions are:' + lines

        return s

    if report.status == Result.WARNING:
        s = 'function call might be invalid'
    elif report.status == Result.ERROR:
        s = 'invalid function call'
    else:
        assert False, 'unexpected status'

    (_, operand), = report.load_operands()
    s += ", pointer '%s' points to" % operand.repr

    callees = []
    for block_info in info['points_to']:
        status = block_info['kind']
        mem_loc_id = block_info['id']
        mem_loc = report.db.memory_locations[mem_loc_id]

        if status == FunctionCallCheckKind.NOT_FUNCTION:
            line = '%s, which is not a function' % memory_location_str(mem_loc)
        else:
            function_id = block_info['fun_id']
            function = report.db.functions[function_id]
            line = "function '%s'" % function.pretty_name()

            if status == FunctionCallCheckKind.WRONG_SIGNATURE:
                line += ', which has a mismatching type'
            elif status == FunctionCallCheckKind.OK:
                line += ', which is valid'
            else:
                assert False, 'unexpected kind'

        callees.append(line)

    # make the output deterministic
    callees.sort()

    if len(callees) == 1:
        s += ' %s' % callees[0]
    else:
        s += ':%s' % ''.join('\n\t* %s' % c for c in callees)

    return s


def generate_double_free_message(report, verbosity):
    if report.status == Result.OK:
        return 'safe memory deallocation'

    info = report.load_info()

    points_to = []
    all_dyn_alloc = True
    for block_info in info['points_to']:
        mem_loc_id = block_info['id']
        status = block_info['status']
        mem_loc = report.db.memory_locations[mem_loc_id]

        line = memory_location_str(mem_loc)
        if mem_loc.kind != MemoryLocationKind.DYN_ALLOC:
            assert status == Result.ERROR
            all_dyn_alloc = False
            line += ', which is not dynamically allocated'
        elif status == Result.ERROR:
            line += ', which is already released'
        elif status == Result.WARNING:
            line += ', which might be already released'
        elif status == Result.OK:
            line += ', which is valid'
        else:
            assert False, 'unexpected status'

        points_to.append(line)

    # make the output deterministic
    points_to.sort()

    if all_dyn_alloc:
        if report.status == Result.WARNING:
            s = 'possible double free'
        elif report.status == Result.ERROR:
            s = 'double free'
        else:
            assert False, 'unexpected status'
    else:
        if report.status == Result.WARNING:
            s = 'memory deallocation might be invalid'
        elif report.status == Result.ERROR:
            s = 'invalid memory deallocation'
        else:
            assert False, 'unexpected status'

    (_, operand), = report.load_operands()
    s += ", pointer '%s' points to" % operand.repr

    if len(points_to) == 1:
        s += ' %s' % points_to[0]
    else:
        s += ':%s' % ''.join('\n\t* %s' % p for p in points_to)

    return s


GENERATE_MESSAGE_MAP = {
    CheckKind.UNREACHABLE: generate_unreachable_message,
    CheckKind.UNEXPECTED_OPERAND: generate_unexpected_operand_message,
    CheckKind.UNINITIALIZED_VARIABLE: generate_uninitialized_variable_message,
    CheckKind.ASSERT: generate_assert_message,
    CheckKind.DIVISION_BY_ZERO: generate_division_by_zero_message,
    CheckKind.SHIFT_COUNT: generate_shift_count_message,
    CheckKind.SIGNED_INT_UNDERFLOW: functools.partial(
        generate_integer_overflow_message,
        signedness='signed',
        kind='underflow'),
    CheckKind.SIGNED_INT_OVERFLOW: functools.partial(
        generate_integer_overflow_message,
        signedness='signed',
        kind='overflow'),
    CheckKind.UNSIGNED_INT_UNDERFLOW: functools.partial(
        generate_integer_overflow_message,
        signedness='unsigned',
        kind='underflow'),
    CheckKind.UNSIGNED_INT_OVERFLOW: functools.partial(
        generate_integer_overflow_message,
        signedness='unsigned',
        kind='overflow'),
    CheckKind.NULL_POINTER_DEREF: generate_null_pointer_deref_message,
    CheckKind.NULL_POINTER_COMPARISON: generate_null_pointer_cmp_message,
    CheckKind.INVALID_POINTER_COMPARISON: generate_invalid_pointer_cmp_message,
    CheckKind.POINTER_COMPARISON: generate_pointer_cmp_message,
    CheckKind.POINTER_OVERFLOW: generate_pointer_overflow_message,
    CheckKind.INVALID_POINTER_DEREF: generate_invalid_pointer_deref_message,
    CheckKind.UNKNOWN_MEMORY_ACCESS: generate_unknown_memory_access_message,
    CheckKind.UNALIGNED_POINTER: generate_unaligned_pointer_message,
    CheckKind.BUFFER_OVERFLOW_GETS: generate_buffer_overflow_gets_message,
    CheckKind.BUFFER_OVERFLOW: generate_buffer_overflow_message,
    CheckKind.IGNORED_STORE: generate_ignored_store_message,
    CheckKind.IGNORED_MEMORY_COPY: generate_ignored_memcpy_message,
    CheckKind.IGNORED_MEMORY_MOVE: generate_ignored_memmove_message,
    CheckKind.IGNORED_MEMORY_SET: generate_ignored_memset_message,
    CheckKind.IGNORED_FREE: generate_ignored_free_message,
    CheckKind.IGNORED_CALL_SIDE_EFFECT_ON_POINTER_PARAM:
        generate_ignored_call_side_effect_on_pointer_param_message,
    CheckKind.IGNORED_CALL_SIDE_EFFECT:
        generate_ignored_call_side_effect_message,
    CheckKind.RECURSIVE_FUNCTION_CALL:
        generate_recursive_function_call_message,
    CheckKind.FUNCTION_CALL_INLINE_ASSEMBLY: generate_call_inline_asm_message,
    CheckKind.UNKNOWN_FUNCTION_CALL_POINTER:
        generate_unknown_function_call_message,
    CheckKind.FUNCTION_CALL: generate_function_call_message,
    CheckKind.FREE: generate_double_free_message,
}


##########################
# command line interface #
##########################


def parse_arguments(argv):
    usage = '%(prog)s [options] file.db'
    description = 'Generate an analysis report from an output database'
    formatter_class = argparse.RawTextHelpFormatter
    parser = argparse.ArgumentParser(usage=usage,
                                     description=description,
                                     formatter_class=formatter_class)

    # Positional arguments
    parser.add_argument('file',
                        metavar='file.db',
                        help='Result database')

    # Optional arguments
    parser.add_argument('--version',
                        action=args.VersionAction,
                        nargs=0,
                        help='Show ikos version')
    parser.add_argument('--color',
                        dest='color',
                        metavar='',
                        help=args.help('Enable terminal colors:',
                                       args.color_choices,
                                       args.default_color),
                        choices=args.choices(args.color_choices),
                        default=args.default_color)
    parser.add_argument('-t', '--times',
                        dest='display_times',
                        metavar='',
                        help=args.help('Display timing results',
                                       args.display_times_choices,
                                       'no'),
                        choices=args.choices(args.display_times_choices),
                        default='no')
    parser.add_argument('-s', '--summary',
                        dest='display_summary',
                        metavar='',
                        help=args.help('Display the analysis summary',
                                       args.display_summary_choices,
                                       'no'),
                        choices=args.choices(args.display_summary_choices),
                        default='no')
    parser.add_argument('--display-raw-checks',
                        dest='display_raw_checks',
                        help='Display analysis raw checks',
                        action='store_true',
                        default=False)
    parser.add_argument('-f', '--format',
                        dest='format',
                        metavar='',
                        help=args.help('Available report formats:',
                                       args.report_formats,
                                       'text'),
                        choices=args.choices(args.report_formats),
                        default='text')
    parser.add_argument('-o', '--report-file',
                        dest='report_file',
                        metavar='<file>',
                        help='Write the report into a file (default: stdout)',
                        default=sys.stdout,
                        type=argparse.FileType('w'))
    parser.add_argument('--web-port',
                        dest='web_port',
                        metavar='',
                        help='Listening port for ikos view',
                        default=8080,
                        type=int)
    parser.add_argument('--status-filter',
                        dest='status_filter',
                        metavar='',
                        help=args.help('Available status filters:',
                                       args.status_filters,
                                       args.default_status_filter),
                        action='append')
    parser.add_argument('--analyses-filter',
                        dest='analyses_filter',
                        metavar='',
                        help=args.help('Available analyses filters:',
                                       args.analyses,
                                       '*'),
                        action='append')
    parser.add_argument('-v', '--report-verbosity',
                        dest='report_verbosity',
                        metavar='[1-4]',
                        help='Report verbosity (default: 1)',
                        default=1,
                        type=int)

    opt = parser.parse_args(argv)

    # parse --status-filter
    opt.status_filter = args.parse_argument(parser,
                                            'status-filter',
                                            choices=args.status_filters,
                                            groups=None,
                                            default=args.default_status_filter,
                                            value=opt.status_filter)

    # parse --analyses-filter
    opt.analyses_filter = args.parse_argument(parser,
                                              'analyses-filter',
                                              choices=args.analyses,
                                              groups=None,
                                              default='*',
                                              value=opt.analyses_filter)

    # check for consistency between --web-port and -f=web
    if opt.web_port != 8080 and opt.format != 'web':
        parser.error('cannot use --web-port without --format=web')

    return opt


def ikos_view(opt, db):
    from ikos import view
    from ikos import log

    log.setup('info')
    v = view.View(db, port=opt.web_port)
    v.serve()


########################
# main for ikos-report #
########################

def main(argv):
    progname = os.path.basename(argv[0])

    # parse arguments
    opt = parse_arguments(argv[1:])

    # setup colors
    colors.setup(opt.color)

    if not os.path.isfile(opt.file):
        printf("%s: error: no such file: \'%s\'\n",
               progname, opt.file, file=sys.stderr)
        sys.exit(1)

    try:
        # open result database
        db = OutputDatabase(opt.file)

        first = True

        # load settings
        settings = db.load_settings()

        # display timing results
        if opt.display_times != 'no':
            if not first:
                printf('\n')
            print_timing_results(db, opt.display_times == 'full')
            first = False

        # display summary
        if opt.display_summary != 'no':
            if not first:
                printf('\n')
            print_summary(db, opt.display_summary == 'full')
            first = False

        # display raw checks
        if opt.display_raw_checks:
            if not first:
                printf('\n')
            print_raw_checks(db, settings['procedural'] == 'interprocedural')
            first = False

        # start ikos-view
        if opt.format == 'web':
            ikos_view(opt, db)
            return

        # report
        if opt.format != 'no':
            if not first and opt.report_file is sys.stdout:
                printf('\n')

            # setup colors again (in case opt.color = 'auto')
            colors.setup(opt.color, file=opt.report_file)

            # generate report
            rep = generate_report(db,
                                  status_filter=opt.status_filter,
                                  analyses_filter=opt.analyses_filter)

            # format report
            formatter_class = formats[opt.format]
            formatter = formatter_class(opt.report_file, opt.report_verbosity)
            formatter.format(rep)

        # close database
        db.close()
    except sqlite3.DatabaseError as e:
        printf('%s: error: %s\n', progname, e, file=sys.stderr)
        sys.exit(1)
