################################################################################
#
# Extract and export information from a result database
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
import collections
import csv
import json
import operator
import os
import re
import sqlite3
import string
import sys

from ikos.colors import bold, blue_bold, green_bold, grey_bold, red_bold, yellow_bold
from ikos.demangle import demangle
from ikos import colors
from ikos import settings


# python 2 & 3 compatibility
PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3

if PY3:
    string_types = str,
    integer_types = int,
    ifilter = filter
    from functools import reduce
else:
    string_types = basestring,
    integer_types = (int, long)
    from itertools import ifilter


def printf(fmt, *args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    file.write(fmt % args if args else fmt)
    file.flush()


############
# settings #
############


def load_settings(db):
    ''' Load the settings from the database '''
    c = db.cursor()
    c.execute('SELECT * FROM settings')

    settings = {}
    for key, value in c.fetchall():
        try:
            settings[key] = json.loads(value)
        except ValueError:
            settings[key] = value

    c.close()
    return settings


##################
# timing results #
##################


def timing_results(db, full=True, sort=True):
    ''' Return the timing results from the database, as a list of tuples (pass, elapsed) '''
    c = db.cursor()
    where = "WHERE pass NOT LIKE '%arbos.%'" if not full else ''
    order_by = 'ORDER BY pass' if sort else ''
    c.execute('SELECT * FROM times %s %s' % (where, order_by))
    return c.fetchall()


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
    results = timing_results(db, full, sort)

    printf(bold('# Time stats:') + '\n')
    name_width = max(len(name) for name, _ in results)
    for name, elapsed in results:
        printf('%s: %s\n', name.ljust(name_width), format_time(elapsed))


###########
# summary #
###########


def summary(db):
    '''
    Return the analysis summary: number of errors, warnings, ok and
    unreachable per checked statements.
    '''
    c = db.cursor()
    results = {}

    # count the number of checked statements (group by statement UID and call context)
    c.execute('SELECT COUNT(*) FROM '
              '(SELECT * FROM results WHERE status IS NOT NULL GROUP BY stmt_uid, context)')
    results['total'] = c.fetchone()[0]

    # count the number of checked statements that only have unreachable checks (ie. dead code)
    c.execute('SELECT COUNT(*) FROM '
              '(SELECT * FROM results r WHERE status IS NOT NULL GROUP BY stmt_uid, context'
              ' HAVING COUNT(*)=(SELECT COUNT(*) FROM results WHERE stmt_uid = r.stmt_uid AND context = r.context AND status = "unreachable"))')
    results['unreachable'] = c.fetchone()[0]

    # ignoring unreachable checks, count the number of checked statements
    # that only have "ok" checks
    c.execute('SELECT COUNT(*) FROM '
              '(SELECT * FROM results r WHERE status IS NOT NULL AND status != "unreachable" GROUP BY stmt_uid, context'
              ' HAVING COUNT(*)=(SELECT COUNT(*) FROM results WHERE stmt_uid = r.stmt_uid AND context = r.context AND status = "ok"))')
    results['ok'] = c.fetchone()[0]

    # count the number of checked statements that have at least one "error" check
    c.execute('SELECT COUNT(*) FROM '
              '(SELECT * FROM results WHERE status = "error" GROUP BY stmt_uid, context)')
    results['error'] = c.fetchone()[0]

    # deduce the number of warnings
    results['warning'] = results['total'] - results['unreachable'] - results['ok'] - results['error']

    c.close()
    return results


def summary_opt(db):
    '''
    Return the analysis summary: number of errors, warnings, ok and
    unreachable per checked statements (optimized version).
    '''
    results = {
        'ok': 0,
        'error': 0,
        'warning': 0,
        'unreachable': 0,
    }

    def f(results, stmt_uid, context, rows):
        rows = set(row['status'] for row in rows if row['status'] is not None)

        if rows:
            if rows == {'unreachable'}:
                results['unreachable'] += 1
            elif 'error' in rows:
                results['error'] += 1
            elif 'warning' in rows:
                results['warning'] += 1
            elif rows == {'ok'}:
                results['ok'] += 1
            else:
                assert False, 'unreachable'

        return results

    results = group_reduce_checks(db, f, results)
    results['total'] = sum(results.values())
    return results


def print_summary(db, full=True):
    ''' Print the analysis summary from the database '''
    results = summary_opt(db)

    printf(bold('# Summary:') + '\n')

    if full:
        printf('Total number of checks                : %s\n',
               bold(results['total']))
        printf('Total number of unreachable checks    : %s\n',
               grey_bold(results['unreachable']) if results['unreachable'] else '0')
        printf('Total number of safe checks           : %s\n',
               green_bold(results['ok']) if results['ok'] else '0')
        printf('Total number of definite unsafe checks: %s\n',
               red_bold(results['error']) if results['error'] else '0')
        printf('Total number of warnings              : %s\n',
               yellow_bold(results['warning']) if results['warning'] else '0')
        printf('\n')

    if results['error'] == 0 and results['warning'] == 0:
        printf(green_bold('The program is SAFE') + '\n')
    else:
        if results['error'] != 0:
            printf(red_bold('The program is definitely UNSAFE') + '\n')
        else:
            printf(yellow_bold('The program is potentially UNSAFE') + '\n')


##########
# checks #
##########


def format_context(context):
    ''' Format a calling context.

    >>> format_context('./f@1@2@3/g@4@5@6/_Z14__ikos_unknownv@7@8@9')
    u'./f@1:2:3/g@4:5:6/__ikos_unknown()@7:8:9'
    '''
    ctx = []
    for callsite in context.split('/'):
        if '@' not in callsite:
            ctx.append(callsite)
        else:
            symbol, line, col, stmt_uid = callsite.split('@')
            ctx.append('%s@%s:%s:%s' % (demangle(symbol), line, col, stmt_uid))

    return '/'.join(ctx)


def format_path(path):
    ''' Format a path. Return the shortest between the absolute path or the relative path. '''
    abs_path = os.path.realpath(path)
    rel_path = os.path.relpath(os.path.realpath(path), os.getcwd())
    return min(abs_path, rel_path, key=len)


def format_status(status):
    ''' Add colors to a status '''
    if status == 'ok':
        return green_bold(status)
    elif status == 'error':
        return red_bold(status)
    elif status == 'warning':
        return yellow_bold(status)
    elif status == 'unreachable':
        return grey_bold(status)
    else:
        return status


def print_raw_checks(db, interprocedural):
    ''' Print all checks in the database, with very little processing '''
    db.row_factory = sqlite3.Row
    header = ['context', 'function', 'file', 'line', 'col', 'stmt', 'check', 'result', 'info']
    order = 'context, function, file, line, column, stmt_uid, safety_check'

    if not interprocedural:
        header.pop(0) # no context column if intraprocedural

    c = db.cursor()
    c.execute('SELECT * FROM results ORDER BY %s' % order)
    rows = c.fetchall()

    # Format all rows
    for i, row in enumerate(rows):
        rows[i] = [format_context(row['context']),
                   demangle(row['function'] or ' '),
                   format_path(row['file']),
                   str(row['line']),
                   str(row['column']),
                   str(row['stmt_uid']),
                   row['safety_check'],
                   row['status'] or ' ',
                   row['info'] or ' ']

        if not interprocedural: # no context column if intraprocedural
            rows[i].pop(0)

    # reorganize data by columns
    cols = zip(*([header] + rows))

    # compute column widths by taking maximum length of values per column
    col_widths = [max(len(value) for value in col) for col in cols]

    # print table
    printf(bold('# Checks') + '\n')
    printf('|' + '|'.join(' %s ' % head.ljust(width) for head, width in zip(header, col_widths)) + '|\n')
    printf('+' + '+'.join('-' * (width + 2) for width in col_widths) + '+\n')
    for row in rows:
        fmt = '|' + '|'.join(' %s ' + ' ' * (width - len(e)) for e, width in zip(row, col_widths)) + '|\n'
        row[-2] = format_status(row[-2]) # add colors for result column
        printf(fmt, *row)

    c.close()
    db.row_factory = None


##########
# export #
##########


class SourceLocation(object):
    ''' Represents a source code location '''

    def __init__(self, file, function, line, column):
        self.file = file or '?'
        self.function = function # None or mangled name
        self.line = line
        self.column = column

    def valid(self):
        return self.file != '?' and self.line >= 1 and self.column >= 1

    def __hash__(self):
        return hash((self.file, self.function, self.line, self.column))

    def __eq__(self, o):
        return self.file == o.file \
            and self.function == o.function \
            and self.line == o.line \
            and self.column == o.column

    def __repr__(self):
        return 'SourceLocation(%s, %s, %d, %d)' % (self.file, self.function, self.line, self.column)


class StatementLocation(SourceLocation):
    ''' Represents an AR statement location '''

    def __init__(self, file, function, line, column, stmt_uid):
        super(StatementLocation, self).__init__(file, function, line, column)
        self.stmt_uid = stmt_uid

    def source_location(self):
        return SourceLocation(self.file, self.function, self.line, self.column)

    def __hash__(self):
        return hash((self.file, self.function, self.line, self.column, self.stmt_uid))

    def __eq__(self, o):
        return super(StatementLocation, self).__eq__(o) and self.stmt_uid == o.stmt_uid

    def __repr__(self):
        return 'StatementLocation(%s, %s, %d, %d, %d)' % (self.file, self.function, self.line, self.column, self.stmt_uid)


class LocationRange:
    ''' Represents a location range '''

    def __init__(self, begin, end):
        self.begin = begin
        self.end = end

    def valid(self):
        return self.begin.valid() and self.end.valid()

    @property
    def function(self):
        assert self.begin.function == self.end.function
        return self.begin.function

    @property
    def file(self):
        assert self.begin.file == self.end.file
        return self.begin.file

    def __hash__(self):
        return hash((self.begin, self.end))

    def __eq__(self, o):
        return self.begin == o.begin and self.end == o.end

    def __repr__(self):
        return 'LocationRange(%r, %r)' % (self.begin, self.end)


class CallContext:
    ''' Represents a calling context '''

    def __init__(self, context):
        self.context = context
        self.stmt_locations = {}

    def empty(self):
        return self.context == '.'

    def __hash__(self):
        return hash(self.context)

    def __eq__(self, o):
        return self.context == o.context

    def __iter__(self):
        for s in self.context.split('/'):
            if '@' in s:
                function, line, column, stmt_uid = s.split('@')
                line, column, stmt_uid = int(line), int(column), int(stmt_uid)

                if stmt_uid in self.stmt_locations: # this should provide the correct file
                    yield self.stmt_locations[stmt_uid]
                else:
                    yield StatementLocation('?', function, line, column, stmt_uid)

    def __repr__(self):
        return 'CallContext(%s)' % self.context


class Report:
    ''' Represents a report for a source code location '''

    def __init__(self, location, contexts, status, checker,
                 short_msg, long_msg=None, full_msg=None,
                 extra_msgs=None):
        assert status in ('safe', 'note', 'warning', 'error', 'unreachable')
        self.location = location
        self.contexts = contexts
        self.status = status
        self.checker = checker
        self.short_msg = short_msg
        self.long_msg = long_msg or self.short_msg
        self.full_msg = full_msg or self.long_msg

        # additional messages unrelated to the status, with their checker.
        # For instance, [('variable x well initialized', 'uva'),]
        self.extra_msgs = extra_msgs or []

        # is this report valid for all contexts ? (None: unknown)
        self.all_contexts = None


def export(db, formatter, level='all', unreachable=True):
    ''' Export results from the database.

    Arguments:
        level(string): 'all', 'safe', 'note', 'warning', 'error'
        unreachable(bool): whether to export unreachable statements or not
    '''
    assert level in ('all', 'safe', 'note', 'warning', 'error')

    # load settings
    settings = load_settings(db)

    # generate reports for each statement and calling context
    stmt_reports = generate_reports(db)

    # map {stmt_uid => StatementLocation}
    stmt_locations = build_stmt_locations(stmt_reports)

    # merge reports having the same messages
    merge_same_reports_by_stmt(stmt_reports)

    if unreachable and 'uva' in settings['analyses']:
        # warning: this pass might return unsound results, because it assumes
        # that there is a report for each statement in the code (should be
        # almost true if the uninitialized variable analysis has been run).
        merge_unreachable_reports(stmt_reports)

    # reduce to a list of reports
    reports = reduce(operator.add, stmt_reports.values())

    # remove unwanted reports
    reports = [report for report in reports if filter_report(report, level, unreachable)]

    # attach stmt_locations to all contexts
    for report in reports:
        for context in report.contexts:
            context.stmt_locations = stmt_locations

    # sort by file, line, column
    def loc_key(loc):
        return (loc.file, loc.line, loc.column)

    def report_key(report):
        loc = report.location
        return loc_key(loc.begin if isinstance(loc, LocationRange) else loc)

    reports.sort(key=report_key)

    # export reports
    formatter.write(reports)


def group_reduce_checks(db, f, initializer=None):
    '''
    Get all checks from the database and group them by stmt_uid and context.

    For each group of checks with the same stmt_uid and context,
    apply accum_value = f(accum_value, stmt_uid, context, checks)
    '''
    db.row_factory = sqlite3.Row
    c = db.cursor()
    c.execute('SELECT * FROM results ORDER BY stmt_uid, context')

    # current state
    last_key = None
    rows = []
    accum_value = initializer

    for row in c.fetchall():
        row_key = (row['stmt_uid'], row['context'])

        if last_key == row_key:
            rows.append(row)
        else:
            if last_key and rows:
                accum_value = f(accum_value, last_key[0], last_key[1], rows)

            last_key = row_key
            rows = [row]

    if last_key and rows:
        accum_value = f(accum_value, last_key[0], last_key[1], rows)

    c.close()
    db.row_factory = None

    return accum_value


def generate_reports(db):
    '''
    Generate all reports.

    Returns a map statement uid => list of reports
    '''
    # map {stmt_uid => [reports[0], reports[1], ...]}
    stmt_reports = collections.defaultdict(list)

    def f(stmt_reports, stmt_uid, context, rows):
        stmt_reports[stmt_uid] += generate_stmt_reports(rows)
        return stmt_reports

    return group_reduce_checks(db, f, stmt_reports)


def merge_same_reports_by_stmt(stmt_reports):
    ''' Merge identical reports for a specific statement having different calling contexts '''
    for stmt_uid, reports in stmt_reports.items():
        if not reports:
            continue

        # map {(status, full_msg, extra_msgs) => report}
        merged_reports = {}

        # set of all calling contexts for this statement
        all_contexts = set()

        for report in reports:
            # key representing a unique message
            key = (report.status, report.full_msg, tuple(sorted(report.extra_msgs)))

            if key in merged_reports: # identical report, merge contexts
                merged_reports[key].contexts.extend(report.contexts)
            else: # new report
                merged_reports[key] = report

            all_contexts.update(report.contexts)

        reports = list(merged_reports.values())

        # set report.all_contexts
        for report in reports:
            report.all_contexts = (len(report.contexts) == len(all_contexts))

        stmt_reports[stmt_uid] = reports


def merge_unreachable_reports(stmt_reports):
    ''' Merge unreachable reports across following statements '''

    # map {location => [stmt_uid[0], stmt_uid[1], ...]}
    location_to_stmts = collections.defaultdict(list)

    # map {stmt_uid => {contexts_key => [reports[0], reports[1], ...]}}
    stmt_contexts = collections.defaultdict(lambda: collections.defaultdict(list))

    for stmt_uid, reports in stmt_reports.items():
        if not reports:
            continue

        loc = reports[0].location
        assert type(loc) is StatementLocation
        loc = loc.source_location()
        location_to_stmts[loc].append(stmt_uid)

        for report in reports:
            # key representing a unique set of calling contexts
            contexts_key = tuple(sorted(c.context for c in report.contexts))

            stmt_contexts[stmt_uid][contexts_key].append(report)

    # sorted locations
    locations = sorted(location_to_stmts.keys(), key=lambda l: (l.file, l.function, l.line, l.column))

    # map {contexts_key => report} of unreachable reports for the last location
    last_unreachable_reports = {}
    last_loc = None

    for loc in locations:
        unreachable_reports = {}

        if last_loc and last_loc.file == loc.file and last_loc.function == loc.function:
            for contexts_key, unreachable_report in last_unreachable_reports.items():
                if all(contexts_key in stmt_contexts[stmt_uid] and
                       any(report.status == 'unreachable' and
                           report.all_contexts == unreachable_report.all_contexts
                           for report in stmt_contexts[stmt_uid][contexts_key])
                       for stmt_uid in location_to_stmts[loc]):
                    # merge unreachable statements
                    unreachable_report.location = create_location_range(unreachable_report.location, loc)

                    # set message
                    unreachable_report.short_msg = unreachable_report.long_msg = unreachable_report.full_msg = 'unreachable statements'

                    # keep grabbing similar unreachable reports
                    unreachable_reports[contexts_key] = unreachable_report

        for stmt_uid in location_to_stmts[loc]:
            for contexts_key in list(stmt_contexts[stmt_uid].keys()):
                if any(report.status == 'unreachable' for report in stmt_contexts[stmt_uid][contexts_key]):
                    if contexts_key in unreachable_reports:
                        # if location is a StatementLocation, downcast to a SourceLocation
                        report = unreachable_reports[contexts_key]
                        if type(report.location) is StatementLocation:
                            report.location = report.location.source_location()

                        # remove unreachable reports, already in unreachable_reports
                        stmt_contexts[stmt_uid][contexts_key] = list(filter(lambda r: r.status != 'unreachable',
                                                                            stmt_contexts[stmt_uid][contexts_key]))
                    else:
                        unreachable_reports[contexts_key] = next(ifilter(lambda r: r.status == 'unreachable',
                                                                         stmt_contexts[stmt_uid][contexts_key]))

        last_loc = loc
        last_unreachable_reports = unreachable_reports

    # update stmt_reports according to stmt_contexts
    for stmt_uid in stmt_reports.keys():
        stmt_reports[stmt_uid] = reduce(operator.add, stmt_contexts[stmt_uid].values())


def create_location_range(begin, end):
    if isinstance(begin, LocationRange):
        begin = begin.begin
    if type(begin) is StatementLocation:
        begin = begin.source_location()

    if isinstance(end, LocationRange):
        end = end.end
    if type(end) is StatementLocation:
        end = end.source_location()

    assert type(begin) is SourceLocation
    assert type(end) is SourceLocation
    return LocationRange(begin, end)


def build_stmt_locations(stmt_reports):
    ''' Return a map {stmt_uid => StatementLocation} '''
    stmt_locations = {}

    for stmt_uid, reports in stmt_reports.items():
        if not reports:
            continue

        loc = reports[0].location
        assert type(loc) is StatementLocation
        stmt_locations[stmt_uid] = loc

    return stmt_locations


def filter_report(report, level, unreachable):
    ''' Return True iif the report should be exported, according to level an unreachable '''
    if report.status == 'unreachable':
        return unreachable
    elif level in ('all', 'safe'):
        return True
    elif level == 'note':
        return report.status in ('note', 'warning', 'error')
    elif level == 'warning':
        return report.status in ('warning', 'error')
    elif level == 'error':
        return report.status == 'error'
    else:
        assert False, 'unexpected level'


##################
# export formats #
##################


class Formatter(object):
    ''' Base class for formatters '''

    def __init__(self, output, verbosity, demangle):
        '''
        Arguments:
            output(file): output file
            verbosity(int): verbosity level
            demangle(bool): demangle C++ symbols
        '''
        self.output = output
        self.verbosity = verbosity
        self.demangle = demangle


class GCCFormatter(Formatter):
    ''' GCC output formatter '''

    def __init__(self, output, verbosity, demangle):
        super(GCCFormatter, self).__init__(output, verbosity, demangle)
        self.sources = {}

    def load_source_code(self, path):
        if path not in self.sources:
            try:
                with open(path, 'r') as f:
                    self.sources[path] = list(f.readlines())
            except IOError:
                printf('warning: could not open file %s\n', path, file=sys.stderr)
                self.sources[path] = []

        return self.sources[path]

    def format_status(self, status):
        if status == 'safe':
            return green_bold('%s')
        elif status == 'note':
            return blue_bold('%s')
        elif status == 'warning':
            return yellow_bold('%s')
        elif status == 'error':
            return red_bold('%s')
        elif status == 'unreachable':
            return grey_bold('%s')
        else:
            return '%s'

    def write_short_prefix(self, loc):
        printf(bold('%s: '), format_path(loc.file), file=self.output)

    def write_long_prefix(self, loc):
        printf(bold('%s:%d:%d: '), format_path(loc.file), loc.line, loc.column, file=self.output)

    def long_indent(self, loc):
        return ' ' * len('%s:%d:%d: ' % (format_path(loc.file), loc.line, loc.column))

    def status_indent(self, status):
        return ' ' * len('%s: ' % status)

    def write_long_indent(self, loc):
        printf(self.long_indent(loc), file=self.output)

    def write_status_message(self, loc, status, message):
        # assume write_long_prefix or write_long_index has been called
        message = message.replace('\n', '\n' + self.long_indent(loc) + self.status_indent(status))
        printf(self.format_status(status) + ': %s\n', status, message, file=self.output)

    def write_source_line(self, loc, pointer_color=green_bold):
        assert isinstance(loc, SourceLocation)

        if loc.valid():
            source_code = self.load_source_code(loc.file)

            if loc.line <= len(source_code):
                line = source_code[loc.line - 1].rstrip()
                printf(line + '\n', file=self.output)
                footer = ''.join('\t' if c == '\t' else ' ' for c in line[:loc.column - 1])
                footer += pointer_color('^')
                printf(footer + '\n', file=self.output)

    def write_source_lines(self, loc, pointer_color=green_bold, max_next=2, max_prev=2):
        assert isinstance(loc, LocationRange)

        if loc.valid():
            source_code = self.load_source_code(loc.begin.file)

            if loc.end.line <= len(source_code):
                # first line
                line = source_code[loc.begin.line - 1].rstrip()
                printf(line + '\n', file=self.output)
                footer = ''.join('\t' if c == '\t' else ' ' for c in line[:loc.begin.column - 1])
                footer += pointer_color('^')
                if loc.begin.line == loc.end.line:
                    footer += pointer_color('~' * len(line[loc.begin.column:loc.end.column - 1]))
                    footer += pointer_color('^')
                else:
                    footer += pointer_color('~' * len(line[loc.begin.column:]))
                printf(footer + '\n', file=self.output)

                for i in range(max_next):
                    if loc.begin.line + i + 1 < loc.end.line - max_prev:
                        line = source_code[loc.begin.line + i].rstrip()
                        printf(line + '\n', file=self.output)
                        footer = pointer_color(''.join('~~~~' if c == '\t' else '~' for c in line))
                        printf(footer + '\n', file=self.output)

                if loc.end.line - loc.begin.line - 1 > max_prev + max_next:
                    printf('...\n', file=self.output)

                for i in reversed(range(max_prev)):
                    if loc.end.line - i - 1 > loc.begin.line:
                        line = source_code[loc.end.line - i - 2].rstrip()
                        printf(line + '\n', file=self.output)
                        footer = pointer_color(''.join('~~~~' if c == '\t' else '~' for c in line))
                        printf(footer + '\n', file=self.output)

                if loc.begin.line != loc.end.line:
                    line = source_code[loc.end.line - 1].rstrip()
                    printf(line + '\n', file=self.output)
                    footer = pointer_color(''.join('~~~~' if c == '\t' else '~' for c in line[:loc.end.column - 1]))
                    footer += pointer_color('^')
                    printf(footer + '\n', file=self.output)

    def write_contexts(self, loc, contexts, max_call_sites=3):
        for context in contexts:
            if context.empty() and len(contexts) > 1:
                self.write_long_prefix(loc)
                self.write_status_message(loc, 'note', "called from entry point '%s'" % demangle(loc.function))
            elif not context.empty():
                self.write_long_prefix(loc)
                call_sites = list(context)
                call_sites.reverse()

                if self.verbosity <= 3:
                    call_sites = call_sites[:max_call_sites]

                self.write_status_message(loc, 'note', 'called from:')
                for call_site in call_sites:
                    self.write_long_prefix(call_site)
                    printf("function '%s'\n", demangle(call_site.function), file=self.output)
                    self.write_source_line(call_site)

    def write(self, reports):
        for report in reports:
            loc = report.location

            if isinstance(loc, LocationRange) and report.status == 'unreachable':
                # print current function
                self.write_short_prefix(loc.begin)
                printf("In function '%s':\n", demangle(loc.begin.function), file=self.output)

                # print status message
                self.write_long_prefix(loc.begin)
                self.write_status_message(loc.begin, 'unreachable',
                                          'unreachable statements (from %d:%d to %d:%d)' % (loc.begin.line, loc.begin.column, loc.end.line, loc.end.column))

                # print line and pointer on column
                self.write_source_lines(loc, pointer_color=grey_bold)

                # print contexts
                if (not report.all_contexts and self.verbosity >= 2) or self.verbosity >= 4:
                    self.write_contexts(loc.begin, report.contexts)
            elif isinstance(loc, SourceLocation):
                # print current function
                self.write_short_prefix(loc)
                printf("In function '%s':\n", demangle(loc.function), file=self.output)

                # print status message
                if self.verbosity <= 1:
                    message = report.short_msg
                elif self.verbosity == 2:
                    message = report.long_msg
                else:
                    message = report.full_msg

                self.write_long_prefix(loc)
                self.write_status_message(loc, report.status, message)

                # print additional info
                if self.verbosity >= 4 and report.extra_msgs:
                    for extra_msg, checker in report.extra_msgs:
                        self.write_long_indent(loc)
                        self.write_status_message(loc, 'note', extra_msg)

                # print line and pointer on column
                self.write_source_line(loc)

                # print contexts
                if (not report.all_contexts and self.verbosity >= 2) or self.verbosity >= 4:
                    self.write_contexts(loc, report.contexts)
            else:
                assert False, 'unreachable'


class JSONEncoder(json.JSONEncoder):
    _demangle = False

    def default(self, obj):
        if isinstance(obj, Report):
            return self.encode_report(obj)
        elif isinstance(obj, StatementLocation):
            return self.encode_stmt_location(obj)
        elif isinstance(obj, SourceLocation):
            return self.encode_source_location(obj)
        elif isinstance(obj, LocationRange):
            return self.encode_location_range(obj)
        elif isinstance(obj, CallContext):
            return self.encode_call_context(obj)

        return json.JSONEncoder.default(self, obj)

    def encode_report(self, report):
        return {
            'location': report.location,
            'contexts': report.contexts,
            'status': report.status,
            'checker': report.checker,
            'short_msg': report.short_msg,
            'long_msg': report.long_msg,
            'full_msg': report.full_msg,
            'extra_msgs': report.extra_msgs,
            'all_contexts': report.all_contexts,
        }

    def encode_stmt_location(self, loc):
        return {
            'type': 'stmt',
            'file': loc.file,
            'function': demangle(loc.function) if self._demangle else loc.function,
            'line': loc.line,
            'column': loc.column,
            'stmt_uid': loc.stmt_uid,
        }

    def encode_source_location(self, loc):
        return {
            'type': 'source',
            'file': loc.file,
            'function': demangle(loc.function) if self._demangle else loc.function,
            'line': loc.line,
            'column': loc.column,
        }

    def encode_location_range(self, loc):
        return {
            'type': 'range',
            'begin': loc.begin,
            'end': loc.end,
        }

    def encode_call_context(self, context):
        return list(context)


class JSONFormatter(Formatter):
    ''' JSON output formatter '''

    def write(self, reports):
        JSONEncoder._demangle = self.demangle
        json.dump(reports, self.output, cls=JSONEncoder)
        self.output.write('\n')


class CSVFormatter(Formatter):
    ''' CSV output formatter '''

    def __init__(self, output, verbosity, demangle):
        super(CSVFormatter, self).__init__(output, verbosity, demangle)
        self.sources = {}

    def load_source_code(self, path):
        if path not in self.sources:
            try:
                with open(path, 'r') as f:
                    self.sources[path] = list(f.readlines())
            except IOError:
                self.sources[path] = []

        return self.sources[path]

    def write(self, reports):
        writer = csv.writer(self.output, quoting=csv.QUOTE_MINIMAL)
        writer.writerow(['file', 'function',
                         'line_start', 'column_start',
                         'line_end', 'column_end',
                         'stmt_uid', 'contexts',
                         'code', 'status', 'checker',
                         'message', 'extra_messages'])

        for report in reports:
            loc = report.location

            if isinstance(loc, LocationRange):
                file = loc.begin.file
                function = demangle(loc.begin.function)
                line_start = loc.begin.line
                column_start = loc.begin.column
                line_end = loc.end.line
                column_end = loc.end.column
                stmt_uid = ''
            else:
                file = loc.file
                function = demangle(loc.function)
                line_start = loc.line
                column_start = loc.column
                line_end = ''
                column_end = ''
                stmt_uid = loc.stmt_uid if isinstance(loc, StatementLocation) else ''

            if report.all_contexts:
                contexts = 'ALL'
            else:
                contexts = []

                for context in report.contexts:
                    contexts.append('/'.join('%s@%d@%d' % (demangle(cs.function), cs.line, cs.column) for cs in context))

                contexts = ' | '.join(contexts)

            code = ''
            if not isinstance(loc, LocationRange) and loc.valid():
                source_code = self.load_source_code(loc.file)

                if loc.line <= len(source_code):
                    code = source_code[loc.line - 1].strip()

            if self.verbosity <= 1:
                message = report.short_msg
            elif self.verbosity == 2:
                message = report.long_msg
            else:
                message = report.full_msg

            message = message.replace('\n', '. ')

            extra_messages = ' | '.join(msg.replace('\n', '. ') for msg, checker in report.extra_msgs)

            writer.writerow([file, function,
                             line_start, column_start,
                             line_end, column_end,
                             stmt_uid, contexts,
                             code, report.status, report.checker,
                             message, extra_messages])


# list of available formats
formats = {
    'gcc': GCCFormatter,
    'json': JSONFormatter,
    'csv': CSVFormatter,
}


###################################
# extract information from checks #
###################################


ar_temporary_variable = re.compile(r'^__v:(init\-gv\-)?\d+$')
llvm_temporary_variable = re.compile(r'^([^\.]+)\._\d+$')
ikos_special_variable = re.compile(r'^shadow\.(.*)$')
llvm_names = {'add', 'sub', 'div', 'mul', 'inc', 'dec', 'incdec', 'rem',
              'shl', 'shr', 'and', 'or', 'land', 'xor', 'neg',
              'cmp', 'conv', 'cond', 'bf', 'retval', 'tobool',
              'Pivot', 'SwitchLeaf', 'PHILowerSelect', 'phitmp',
              'arrayidx', 'arraydecay', 'idx', 'idxprom',
              'vaarg', 'vla', 'vfn', 'call',
              'gp_offset', 'gp_offset_p', 'fits_in_gp',
              'fp_offset', 'fp_offset_p', 'fits_in_fp',
              'reg_save_area', 'reg_save_area_p',
              'overflow_arg_area', 'overflow_arg_area_p',
              '__value_'}


def source_var(name):
    ''' Return the source level variable name, or otherwise None '''
    if re.match(ar_temporary_variable, name): # __v:1
        return None
    elif re.match(llvm_temporary_variable, name): # f._1
        return None
    elif re.match(ikos_special_variable, name): # shadow.<...>
        return None
    elif name[0] == '.': # .str
        return None
    elif '.' not in name: # global variable
        return name
    else:
        name = name.split('.')[1].rstrip(string.digits)
        if name in llvm_names: # f.add, f.add2
            return None
        else:
            return name


def is_local_var(name):
    ''' Return True iif the variable is local to a function (AR_Local_Variable or AR_Internal_Variable) '''
    return '.' in name and name[0] != '.' and not re.match(ikos_special_variable, name)


def is_global_var(name):
    ''' Return True iif the variable is a global variable (AR_Global_Variable) '''
    return '.' not in name and name[0] != '.'


def local_var_function(name):
    if '.' in name and name[0] != '.':
        return name.split('.')[0]
    else:
        return None


def quote_var(name, prefix=''):
    name = demangle(name)
    if name.startswith('vtable for ') or name.startswith('typeinfo for '):
        return name
    else:
        return prefix + "'%s'" % name


def collect_variable_names(rows):
    variables = set()

    for row in rows:
        assert row['safety_check'] in ('nullity', 'uva', 'upa')
        info = json.loads(row['info'])
        if 'name' in info and source_var(info['name']):
            variables.add(quote_var(source_var(info['name'])))

    return sorted(variables)


def row_match(row, **kwargs):
    ''' Return True if a given row matches some criteria '''
    row_info = json.loads(row['info'])
    return all(key in row_info and row_info[key] == value for key, value in kwargs.items())


def extract_row(rows, **kwargs):
    ''' Extract a specific row based on some criteria '''
    result_row, result_others = None, []

    for row in rows:
        if not result_row and row_match(row, **kwargs):
            result_row = row
        else:
            result_others.append(row)

    assert result_row and len(rows) == len(result_others) + 1
    return result_row, result_others


def extract_mem_access_pointer_underflow(overflow_info, underflow_rows):
    ''' Extract the underflow check corresponding to the given overflow check '''
    assert underflow_rows

    query = {'type': overflow_info['type']}
    for key in ('pointer', 'access-size'):
        if key in overflow_info:
            query[key] = overflow_info[key]

    return extract_row(underflow_rows, **query)


def extract_mem_access_strcpy(overflow_rows):
    ''' Extract the strcpy check '''
    assert len(overflow_rows) == 3
    result_row, result_others = None, []

    for row in overflow_rows:
        row_info = json.loads(row['info'])
        if row_info['type'] == 'strcpy':
            result_row = row
        else:
            result_others.append(row)

    if not result_row:
        # in this case, we have to remove the duplicate row
        keys = set()
        result_others = []

        for row in overflow_rows:
            row_info = json.loads(row['info'])
            if row_info['type'] == 'var': # strcpy does not produce 'var' type
                result_others.append(row)
            else:
                if 'pointer' in row_info:
                    key = (row_info['type'], row_info['pointer'])
                else:
                    key = (row_info['type'],)

                if key not in keys:
                    result_others.append(row)
                    keys.add(key)

    assert len(result_others) == 2
    return result_row, result_others


def extract_mem_access_pointer_upa(overflow_info, upa_rows):
    ''' Extract the upa check on the pointer of the given overflow/underflow check '''
    if not upa_rows:
        return None, []

    if overflow_info['type'] == 'undefined-constant-pointer':
        return extract_row(upa_rows, type='undefined-constant')
    elif overflow_info['type'] == 'null-constant-pointer':
        return extract_row(upa_rows, type='null-constant')
    elif overflow_info['type'] == 'null-var-pointer':
        return extract_row(upa_rows, type='null-pointer', name=overflow_info['pointer'])
    elif overflow_info['type'] == 'undefined-var-pointer':
        return extract_row(upa_rows, type='undefined-pointer', name=overflow_info['pointer'])
    elif overflow_info['type'] in ('global-var', 'function-pointer', 'undefined-constant-access-size', 'undefined-var-access-size', 'no-points-to', 'var'):
        return extract_row(upa_rows, name=overflow_info['pointer'])
    else:
        assert False, 'unexpected overflow type'


def extract_mem_access_pointer_nullity(overflow_info, nullity_rows):
    ''' Extract the nullity check on the pointer of the given overflow/underflow check '''
    if not nullity_rows:
        return None, []

    if overflow_info['type'] in ('global-var', 'function-pointer'):
        return extract_row(nullity_rows, type=overflow_info['type'], name=overflow_info['pointer'])
    elif overflow_info['type'] == 'undefined-constant-pointer':
        return extract_row(nullity_rows, type='undefined-constant')
    elif overflow_info['type'] == 'null-constant-pointer':
        return extract_row(nullity_rows, type='null-constant')
    elif overflow_info['type'] in ('null-var-pointer', 'undefined-var-pointer', 'undefined-constant-access-size', 'undefined-var-access-size', 'no-points-to', 'var'):
        return extract_row(nullity_rows, name=overflow_info['pointer'])
    else:
        assert False, 'unexpected overflow type'


def extract_mem_access_pointer_uva(overflow_info, uva_rows):
    ''' Extract the uva check on the pointer of the given overflow/underflow check '''
    if not uva_rows:
        return None, []

    if overflow_info['type'] in ('global-var', 'function-pointer'):
        return extract_row(uva_rows, type=overflow_info['type'], name=overflow_info['pointer'])
    elif overflow_info['type'] == 'undefined-constant-pointer':
        return extract_row(uva_rows, type='undefined-constant')
    elif overflow_info['type'] == 'null-constant-pointer':
        return extract_row(uva_rows, type='constant')
    elif overflow_info['type'] in ('null-var-pointer', 'undefined-var-pointer', 'undefined-constant-access-size', 'undefined-var-access-size', 'no-points-to', 'var'):
        return extract_row(uva_rows, name=overflow_info['pointer'])
    else:
        assert False, 'unexpected overfow type'


def extract_pointer_uva_from_upa(upa_info, uva_rows):
    ''' Extract the uva check on the pointer of the given upa check'''
    if not uva_rows:
        return None, []

    if upa_info['type'] == 'null-constant':
        return extract_row(uva_rows, type='constant')
    elif upa_info['type'] == 'undefined-constant':
        return extract_row(uva_rows, type=upa_info['type'])
    elif upa_info['type'] in ('null-pointer', 'undefined-pointer', 'no-points-to', 'no-requirement', 'var'):
        return extract_row(nullity_rows, name=upa_info['name'])
    else:
        assert False, 'unexpected upa type'

def extract_pointer_nullity_from_upa(upa_info, nullity_rows):
    ''' Extract the nullity check on the pointer of the given upa check '''
    if not nullity_rows:
        return None, []

    if upa_info['type'] in ('null-constant', 'undefined-constant'):
        return extract_row(nullity_rows, type=upa_info['type'])
    elif upa_info['type'] in ('null-pointer', 'undefined-pointer'):
        return extract_row(nullity_rows, type='var', name=upa_info['name'])
    elif upa_info['type'] in ('no-points-to', 'no-requirement', 'var'):
        return extract_row(nullity_rows, name=upa_info['name'])
    else:
        assert False, 'unexpected upa type'


def extract_pointer_uva_from_nullity(nullity_info, uva_rows):
    ''' Extract the uva check on the pointer of the given nullity check '''
    if not uva_rows:
        return None, []

    if nullity_info['type'] in ('local-var', 'global-var', 'function-pointer', 'var'):
        return extract_row(uva_rows, type=nullity_info['type'], name=nullity_info['name'])
    elif nullity_info['type'] == 'null-constant':
        return extract_row(uva_rows, type='constant')
    elif nullity_info['type'] == 'undefined-constant':
        return extract_row(uva_rows, type='undefined-constant')
    else:
        assert False, 'unexpected nullity type'


def extract_mem_access_size_uva(overflow_info, uva_rows):
    ''' Extract the uva check on the acces-size (length) of the given overflow/underflow check '''
    if not uva_rows:
        return None, []

    if overflow_info['type'] == 'undefined-constant-access-size':
        return extract_row(uva_rows, type='undefined-constant')
    elif isinstance(overflow_info['access-size'], integer_types):
        return extract_row(uva_rows, type='constant')
    elif isinstance(overflow_info['access-size'], string_types):
        return extract_row(uva_rows, type='var', name=overflow_info['access-size'])
    else:
        assert False, 'unexpected access-size'


def is_array_access(underflow_info, overflow_info):
    ''' Try to guess whether it is an array access or not '''
    assert underflow_info['type'] == 'var' and overflow_info['type'] == 'var'
    access_size = underflow_info['access-size']

    def is_multiple_size(b):
        return b in ('-oo', '+oo') or b % access_size == 0

    def is_finite_bound(b):
        return b not in ('-oo', '+oo')

    return not source_var(underflow_info['pointer']) and \
        isinstance(access_size, integer_types) and \
        access_size > 1 and \
        (is_multiple_size(underflow_info['offset_min']) and
         is_multiple_size(underflow_info['offset_max']) and
         all(is_multiple_size(mem['size_min']) and
             is_multiple_size(mem['size_max']) and
             is_multiple_size(mem['diff_min']) and
             is_multiple_size(mem['diff_max'])
             for mem in overflow_info['points-to'])) and \
        (is_finite_bound(underflow_info['offset_min']) or
         is_finite_bound(underflow_info['offset_max']) or
         any(is_finite_bound(mem['size_min']) or
             is_finite_bound(mem['size_max']) or
             is_finite_bound(mem['diff_min']) or
             is_finite_bound(mem['diff_max'])
             for mem in overflow_info['points-to']))


def bound_idiv(bound, divisor):
    if bound in ('-oo', '+oo'):
        return bound
    else:
        return bound // divisor


####################################
# generate reports for a statement #
####################################


def generate_stmt_reports(rows):
    ''' Generate reports for a specific statement '''

    if not rows:
        return []

    reports = []
    context = CallContext(rows[0]['context'])
    location = StatementLocation(rows[0]['file'], rows[0]['function'], rows[0]['line'], rows[0]['column'], rows[0]['stmt_uid'])

    # check that they all have the same context and statement location
    assert all(CallContext(row['context']) == context for row in rows)
    assert all(StatementLocation(row['file'], row['function'], row['line'], row['column'], row['stmt_uid']) == location for row in rows)

    ######################################################################
    # special warnings (e.g. cast int-to-ptr or note for function calls) #
    ######################################################################

    for row in filter(lambda row: row['status'] is None, rows):
        reports.append(generate_special_check_report(row, location, context))

    rows = list(filter(lambda row: row['status'] is not None, rows))

    if not rows:
        return reports

    ###############
    # unreachable #
    ###############

    if all(row['status'] == 'unreachable' for row in rows):
        reports.append(Report(location, [context], 'unreachable', 'unreachable',
                              'unreachable statement'))
        return reports

    assert all(row['status'] != 'unreachable' for row in rows)

    ####################################
    # build a map safety_check => rows #
    ####################################

    checks = {}

    for row in rows:
        safety_check = row['safety_check']
        if safety_check not in checks:
            checks[safety_check] = []

        checks[safety_check].append(row)

    ##########
    # prover #
    ##########

    if 'prover' in checks:
        assert len(rows) <= 2
        assert len(checks['prover']) == 1
        assert 'uva' not in checks or len(checks['uva']) == 1
        prover_row = checks['prover'][0]
        uva_row = checks['uva'][0] if 'uva' in checks else None
        reports.append(generate_prover_report(prover_row, uva_row, location, context))
        return reports

    ####################
    # division by zero #
    ####################

    if 'dbz' in checks:
        assert len(rows) <= 3
        assert len(checks['dbz']) == 1
        assert 'uva' not in checks or len(checks['uva']) == 2
        dbz_row = checks['dbz'][0]
        uva_rows = checks['uva'] if 'uva' in checks else []
        reports.append(generate_dbz_report(dbz_row, uva_rows, location, context))
        return reports

    #################
    # memory access #
    #################

    if 'overflow' in checks and 'underflow' in checks:
        overflow_rows = checks['overflow']
        underflow_rows = checks['underflow']
        upa_rows = checks['upa'] if 'upa' in checks else []
        nullity_rows = checks['nullity'] if 'nullity' in checks else []
        uva_rows = checks['uva'] if 'uva' in checks else []
        reports += generate_mem_access_reports(overflow_rows, underflow_rows, upa_rows, nullity_rows, uva_rows, location, context)
        return reports

    #####################
    # pointer alignment #
    #####################

    if 'upa' in checks:
        upa_rows = checks['upa']
        nullity_rows = checks['uva'] if 'uva' in checks else []
        uva_rows = checks['uva'] if 'uva' in checks else []
        reports.append(generate_upa_report(upa_rows, nullity_rows, uva_rows, location, context))
        return reports

    #################
    # nullity check #
    #################

    if 'nullity' in checks:
        nullity_rows = checks['nullity']
        uva_rows = checks['uva'] if 'uva' in checks else []
        reports.append(generate_nullity_report(nullity_rows, uva_rows, location, context))
        return reports

    ###########################
    # uninitialized variables #
    ###########################

    if 'uva' in checks:
        uva_rows = checks['uva']
        reports.append(generate_uva_report(uva_rows, location, context))
        return reports

    assert False, 'unreachable'


###############################
# generate reports for checks #
###############################


def generate_special_check_report(row, location, context):
    ''' Generate a report for a special check (empty status) '''
    info = json.loads(row['info'])

    if row['safety_check'] == 'cast-int-ptr':
        short_msg = long_msg = full_msg = 'cast from integer to pointer, analysis might be unsound'
        if source_var(info['dest']) and source_var(info['src']):
            full_msg = "cast from integer variable '%s' to pointer variable '%s', analysis might be unsound" % (demangle(source_var(info['src'])), demangle(source_var(info['dest'])))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'ignored-mem-write':
        short_msg = 'ignored memory write, analysis might be unsound'
        long_msg = full_msg = 'ignored memory write because the analysis does not have enough information about the destination, analysis might be unsound'
        if source_var(info['pointer']):
            full_msg = "ignored memory write because the analysis does not have enough information about pointer '%s', analysis might be unsound" % demangle(source_var(info['pointer']))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'ignored-mem-copy':
        short_msg = 'ignored memcpy, analysis might be unsound'
        long_msg = full_msg = 'ignored memcpy because the analysis does not have enough information about the destination, analysis might be unsound'
        if source_var(info['dest']):
            full_msg = "ignored memcpy because the analysis does not have enough information about pointer '%s', analysis might be unsound" % demangle(source_var(info['dest']))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'ignored-mem-set':
        short_msg = 'ignored memset, analysis might be unsound'
        long_msg = full_msg = 'ignored memset because the analysis does not have enough information about the destination, analysis might be unsound'
        if source_var(info['pointer']):
            full_msg = "ignored memset because the analysis does not have enough information about pointer '%s', analysis might be unsound" % demangle(source_var(info['pointer']))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'ignored-va-start':
        short_msg = 'ignored va_start(), analysis might be unsound'
        long_msg = full_msg = 'ignored va_start() because the analysis does not have enough information about the parameter, analysis might be unsound'
        if source_var(info['pointer']):
            full_msg = "ignored va_start() because the analysis does not have enough information about pointer '%s', analysis might be unsound" % demangle(source_var(info['pointer']))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'ignored-va-end':
        short_msg = 'ignored va_end(), analysis might be unsound'
        long_msg = full_msg = 'ignored va_end() because the analysis does not have enough information about the parameter, analysis might be unsound'
        if source_var(info['pointer']):
            full_msg = "ignored va_end() because the analysis does not have enough information about pointer '%s', analysis might be unsound" % demangle(source_var(info['pointer']))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'ignored-va-copy':
        short_msg = 'ignored va_copy(), analysis might be unsound'
        long_msg = full_msg = 'ignored va_copy() because the analysis does not have enough information about the destination, analysis might be unsound'
        if source_var(info['dest']):
            full_msg = "ignored va_copy() because the analysis does not have enough information about pointer '%s', analysis might be unsound" % demangle(source_var(info['dest']))
        return Report(location, [context], 'warning', 'special', short_msg, long_msg, full_msg)
    elif row['safety_check'] == 'call':
        if info['type'] == 'unreachable':
            return Report(location, [context], 'unreachable', 'unreachable', 'unreachable function call')
        elif info['type'] == 'no-points-to':
            short_msg = 'ignored function call, analysis might be unsound'
            long_msg = full_msg = 'ignored function call because the analysis does not have enough information about the function pointer, analysis might be unsound'
            if source_var(info['pointer']):
                full_msg = "ignored function call because the analysis does not have enough information about function pointer '%s', analysis might be unsound" % demangle(source_var(info['pointer']))
            return Report(location, [context], 'warning', 'call', short_msg, long_msg, full_msg)
        elif info['type'] == 'recursive':
            short_msg = 'ignored recursive function call, analysis might be unsound'
            long_msg = "ignored recursive function call to '%s', analysis might be unsound" % demangle(info['function'])
            return Report(location, [context], 'warning', 'call', short_msg, long_msg)
        elif info['type'] == 'no-match':
            points_to = info['points-to']
            short_msg = 'invalid call to function pointer'
            if len(points_to) == 1:
                long_msg = "invalid call to function pointer, wrong signature for function '%s'" % demangle(points_to[0]['function'])
            else:
                long_msg = 'invalid call to function pointer, all possible callees have a wrong signature.\n'
                long_msg += 'possible callees are: %s' % (', '.join("'%s'" % demangle(p['function']) for p in points_to))
            return Report(location, [context], 'error', 'call', short_msg, long_msg)
        elif info['type'] == 'ok':
            points_to = info['points-to']

            short_msg = 'valid function call'
            if len(points_to) == 1:
                long_msg = "valid function call to %s function '%s'" % (points_to[0]['type'], demangle(points_to[0]['function']))
            else:
                long_msg = 'valid call to function pointer.\n'
                long_msg += 'possible calles are:'
                for p in points_to:
                    if p['type'] == 'extern':
                        long_msg += "\n\textern '%s'" % demangle(p['function'])
                    elif p['type'] == 'internal':
                        long_msg += "\n\tinternal '%s'" % demangle(p['function'])
                    elif p['type'] == 'wrong-signature':
                        long_msg += "\n\tinternal '%s' (ignored, wrong signature)" % demangle(p['function'])
                    elif p['type'] == 'not-function':
                        long_msg += "\n\tnot a function '%s'" % demangle(p['function'])
                    else:
                        assert False, 'unexpected points-to type %s' % p['type']

            return Report(location, [context], 'note', 'call', short_msg, long_msg)
        else:
            assert False, 'unexpected type %s for call check' % info['type']
    else:
        assert False, 'unexpected safety check: %s' % row['safety_check']


def generate_prover_report(prover_row, uva_row, location, context):
    ''' Generate a report for a __ikos_assert() statement.

    Arguments:
        prover_row(sqlite3.Row): the prover check
        uva_row(sqlite3.Row): the uva check, or None
    '''
    if uva_row:
        if uva_row['status'] == 'error':
            short_msg = 'assertion on %s' % generate_uva_msg(uva_row)
            return Report(location, [context], 'error', 'uva', short_msg)
        elif uva_row['status'] == 'warning':
            assert prover_row['status'] != 'error'
            short_msg = 'assertion could not be proven, %s' % generate_uva_msg(uva_row)
            return Report(location, [context], 'warning', 'uva', short_msg)

    if prover_row['status'] == 'error':
        return Report(location, [context], 'error', 'prover', 'assertion never holds',
                      extra_msgs=generate_messages([uva_row]))
    elif prover_row['status'] == 'warning':
        return Report(location, [context], 'warning', 'prover', 'assertion could not be proven',
                      extra_msgs=generate_messages([uva_row]))
    else:
        assert prover_row['status'] == 'ok'
        return Report(location, [context], 'safe', 'prover', 'assertion holds',
                      extra_msgs=generate_messages([uva_row]))


def generate_dbz_report(dbz_row, uva_rows, location, context):
    ''' Generate a report for a division statement.

    Arguments:
        dbz_row(sqlite3.Row): the dbz check
        uva_rows(list): a list of uva checks, or the empty list
    '''
    dbz_info = json.loads(dbz_row['info'])

    # guess the numerator/divisor uva check
    if not uva_rows:
        num_uva_row, div_uva_row = None, None
    else:
        info = json.loads(uva_rows[0]['info'])
        if (dbz_info['type'] == 'constant' and info['type'] == 'constant') \
                or (dbz_info['type'] == 'var' and info['type'] == 'var' and dbz_info['name'] == info['name']):
            num_uva_row, div_uva_row = uva_rows[1], uva_rows[0]
        else:
            num_uva_row, div_uva_row = uva_rows[0], uva_rows[1]

    # errors
    if div_uva_row and div_uva_row['status'] == 'error':
        return Report(location, [context], 'error', 'uva', 'divisor is undefined (undefined behavior)',
                      extra_msgs=generate_messages([num_uva_row]))
    elif dbz_row['status'] == 'error':
        return Report(location, [context], 'error', 'dbz', 'division by zero',
                      extra_msgs=generate_messages(uva_rows))
    elif num_uva_row and num_uva_row['status'] == 'error':
        return Report(location, [context], 'error', 'uva', 'numerator is undefined (undefined behavior)',
                      extra_msgs=generate_messages([div_uva_row, dbz_row]))
    # warnings
    elif div_uva_row and div_uva_row['status'] == 'warning':
        return Report(location, [context], 'warning', 'uva', 'divisor might be undefined',
                      extra_msgs=generate_messages([num_uva_row]))
    elif dbz_row['status'] == 'warning':
        short_msg = 'divisor might be 0'
        long_msg = generate_dbz_msg(dbz_row)
        return Report(location, [context], 'warning', 'dbz', short_msg, long_msg,
                      extra_msgs=generate_messages(uva_rows))
    elif num_uva_row and num_uva_row['status'] == 'warning':
        return Report(location, [context], 'warning', 'uva', 'numerator might be undefined',
                      extra_msgs=generate_messages([div_uva_row, dbz_row]))
    # safe
    else:
        assert dbz_row['status'] == 'ok' and all(row['status'] == 'ok' for row in uva_rows)
        return Report(location, [context], 'safe', 'dbz', 'safe division',
                      extra_msgs=generate_messages(uva_rows + [dbz_row]))


def generate_upa_report(upa_rows, nullity_rows, uva_rows, location, context):
    ''' Generate a report for a statement that has upa checks '''
    checks = {
        'upa_error': [],
        'upa_warning': [],
        'nullity_error': [],
        'nullity_warning': [],
        'uva_error': [],
        'uva_warning': [],
        'ok': []
    }

    for upa_row in upa_rows:
        upa_info = json.loads(upa_row['info'])
        # Get corresponding nullity check from upa check
        nullity_row, nullity_rows = extract_pointer_nullity_from_upa(upa_info, nullity_rows)

        # Get corresponding uva check from upa check
        uva_row, uva_rows = extract_pointer_uva_from_upa(upa_info, uva_rows)

        # compute status
        if uva_row and uva_row['status'] in ('error', 'warning'):
            assert nullity_row['status'] != 'error'
            checks['uva_%s' % uva_row['status']].append(uva_row)
        else:
            if uva_row:
                checks['ok'].append(uva_row)
            if nullity_row and nullity_row['status'] in ('error', 'warning'):
                checks['nullity_%s' % nullity_row['status']].append(nullity_row)
            else:
                if nullity_row:
                    checks['ok'].append(nullity_row)
                if upa_row['status'] in ('error', 'warning'):
                    checks['upa_%s' % upa_row['status']].append(upa_row)
                else:
                    checks['ok'].append(upa_row)

    for nullity_row in nullity_rows:
        # nullity checks for other parameters
        if nullity_row['status'] in ('error', 'warning'):
            checks['nullity_%s' % nullity_row['status']].append(nullity_row)
        else:
            checks['ok'].append(nullity_row)

    for uva_row in uva_rows:
        # uva checks for other parameters
        if uva_row['status'] in ('error', 'warning'):
            checks['uva_%s' % uva_row['status']].append(uva_row)
        else:
            checks['ok'].append(uva_row)

    # errors
    if checks['nullity_error']:
        variables = collect_variable_names(checks['nullity_error'])
        short_msg = long_msg = 'null pointer dereference'
        if variables and len(variables) == 1:
            long_msg = 'null pointer dereference of pointer %s' % variables[0]
        elif variables:
            long_msg = 'null pointer dereference of pointers %s' % (', '.join(variables))

        return Report(location, [context], 'error', 'nullity', short_msg, long_msg,
                      extra_msgs=generate_messages(checks['uva_error'] + checks['upa_error'] + checks['uva_warning'] + checks['nullity_warning'] + checks['upa_warning'] + checks['ok']))
    elif checks['uva_error']:
        variables = collect_variable_names(checks['uva_error'])
        if variables and len(variables) == 1:
            short_msg = 'uninitialized variable %s (undefined behavior)' % variables[0]
        elif variables:
            short_msg = 'uninitialized variables %s (undefined behavior)' % (', '.join(variables))
        else:
            short_msg = 'undefined expression (undefined behavior)'

        return Report(location, [context], 'error', 'uva', short_msg,
                      extra_msgs=generate_messages(checks['upa_error'] + checks['uva_warning'] + checks['nullity_warning'] + checks['upa_warning'] + checks['ok']))
    elif checks['upa_error']:
        variables = collect_variable_names(checks['upa_error'])
        if variables and len(variables) == 1:
            short_msg = 'unaligned pointer %s (undefined behavior)' % variables[0]
        elif variables:
            short_msg = 'unaligned pointers %s (undefined behavior)' % (', '.join(variables))
        else:
            short_msg = 'unaligned pointer (undefined behavior)'

        return Report(location, [context], 'error', 'upa', short_msg,
                      extra_msgs=generate_messages(checks['uva_warning'] + checks['nullity_warning'] + checks['upa_warning'] + checks['ok']))
    # warnings
    elif checks['nullity_warning']:
        variables = collect_variable_names(checks['nullity_warning'])
        if variables:
            short_msg = '%s might be null' % (', '.join(variables))
        else:
            short_msg = 'pointer might be null'

        return Report(location, [context], 'warning', 'nullity', short_msg,
                      extra_msgs=generate_messages(checks['uva_warning'] + checks['upa_warning'] + checks['ok']))
    elif checks['uva_warning']:
        variables = collect_variable_names(checks['uva_warning'])
        if variables and len(variables) == 1:
            short_msg = 'variable %s might be uninitialized' % variables[0]
        elif variables:
            short_msg = 'variables %s might be uninitialized' % (', '.join(variables))
        else:
            short_msg = 'expression might be undefined'

        return Report(location, [context], 'warning', 'uva', short_msg,
                      extra_msgs=generate_messages(checks['upa_warning'] + checks['ok']))
    elif checks['upa_warning']:
        variables = collect_variable_names(checks['upa_warning'])
        upa_info = json.loads(checks['upa_warning'][0]['info'])
        if upa_info['type'] == 'no-points-to':
            short_msg = 'memory access might be unaligned, the analysis could not detect the memory location pointed'
            if variables and len(variables) == 1:
                short_msg += ' by %s' % variables[0]
            elif variables:
                short_msg += 'by %s' % (', '.join(variables))
        else:
            if variables and len(variables) == 1:
                short_msg = 'pointer %s might be unaligned' % variables[0]
            elif variables:
                short_msg = 'pointers %s might be unaligned' % (', '.join(variables))
            else:
                short_msg = 'pointer might be unaligned'

        return Report(location, [context], 'warning', 'upa', short_msg,
                      extra_msgs=generate_messages(checks['ok']))
    # safe
    else:
        return Report(location, [context], 'safe', 'upa', 'safe statement',
                      extra_msgs=generate_messages(checks['ok']))


def generate_nullity_report(nullity_rows, uva_rows, location, context):
    ''' Generate a report for a statement that has nullity checks '''
    checks = {
        'uva_error': [],
        'uva_warning': [],
        'nullity_error': [],
        'nullity_warning': [],
        'ok': []
    }

    for nullity_row in nullity_rows:
        # for each nullity check, there might be a uva check
        nullity_info = json.loads(nullity_row['info'])
        uva_row, uva_rows = extract_pointer_uva_from_nullity(nullity_info, uva_rows)
        # compute status
        if uva_row and uva_row['status'] in ('error', 'warning'):
            assert nullity_row['status'] != 'error'
            checks['uva_%s' % uva_row['status']].append(uva_row)
        else:
            if uva_row:
                checks['ok'].append(uva_row)
            if nullity_row['status'] in ('error', 'warning'):
                checks['nullity_%s' % nullity_row['status']].append(nullity_row)
            else:
                checks['ok'].append(nullity_row)

    for uva_row in uva_rows:
        # uva checks for other parameters
        if uva_row['status'] in ('error', 'warning'):
            checks['uva_%s' % uva_row['status']].append(uva_row)
        else:
            checks['ok'].append(uva_row)

    # errors
    if checks['nullity_error']:
        variables = collect_variable_names(checks['nullity_error'])
        short_msg = long_msg = 'null pointer dereference'
        if variables and len(variables) == 1:
            long_msg = 'null pointer dereference of pointer %s' % variables[0]
        elif variables:
            long_msg = 'null pointer dereference of pointers %s' % (', '.join(variables))

        return Report(location, [context], 'error', 'nullity', short_msg, long_msg,
                      extra_msgs=generate_messages(checks['uva_error'] + checks['uva_warning'] + checks['nullity_warning'] + checks['ok']))
    elif checks['uva_error']:
        variables = collect_variable_names(checks['uva_error'])
        if variables and len(variables) == 1:
            short_msg = 'uninitialized variable %s (undefined behavior)' % variables[0]
        elif variables:
            short_msg = 'uninitialized variables %s (undefined behavior)' % (', '.join(variables))
        else:
            short_msg = 'undefined expression (undefined behavior)'

        return Report(location, [context], 'error', 'uva', short_msg,
                      extra_msgs=generate_messages(checks['uva_warning'] + checks['nullity_warning'] + checks['ok']))
    # warnings
    elif checks['nullity_warning']:
        variables = collect_variable_names(checks['nullity_warning'])
        if variables:
            short_msg = '%s might be null' % (', '.join(variables))
        else:
            short_msg = 'pointer might be null'

        return Report(location, [context], 'warning', 'nullity', short_msg,
                      extra_msgs=generate_messages(checks['uva_warning'] + checks['ok']))
    elif checks['uva_warning']:
        variables = collect_variable_names(checks['uva_warning'])
        if variables and len(variables) == 1:
            short_msg = 'variable %s might be uninitialized' % variables[0]
        elif variables:
            short_msg = 'variables %s might be uninitialized' % (', '.join(variables))
        else:
            short_msg = 'expression might be undefined'

        return Report(location, [context], 'warning', 'uva', short_msg,
                      extra_msgs=generate_messages(checks['ok']))
    # safe
    else:
        return Report(location, [context], 'safe', 'nullity', 'safe statement',
                      extra_msgs=generate_messages(checks['ok']))


def generate_uva_report(rows, location, context):
    ''' Generate a report for a statement that only has uva checks '''
    assert all(row['safety_check'] == 'uva' for row in rows)

    checks = {'error': [], 'warning': [], 'ok': []}

    for row in rows:
        checks[row['status']].append(row)

    if checks['error']:
        variables = collect_variable_names(checks['error'])
        if variables and len(variables) == 1:
            short_msg = 'uninitialized variable %s (undefined behavior)' % variables[0]
        elif variables:
            short_msg = 'uninitialized variables %s (undefined behavior)' % (', '.join(variables))
        else:
            short_msg = 'undefined expression (undefined behavior)'

        return Report(location, [context], 'error', 'uva', short_msg,
                      extra_msgs=generate_messages(checks['warning'] + checks['ok']))
    elif checks['warning']:
        variables = collect_variable_names(checks['warning'])
        if variables and len(variables) == 1:
            short_msg = 'variable %s might be uninitialized' % variables[0]
        elif variables:
            short_msg = 'variables %s might be uninitialized' % (', '.join(variables))
        else:
            short_msg = 'expression might be undefined'

        return Report(location, [context], 'warning', 'uva', short_msg,
                      extra_msgs=generate_messages(checks['ok']))
    else:
        return Report(location, [context], 'safe', 'uva', 'safe statement',
                      extra_msgs=generate_messages(rows))


def generate_mem_access_reports(overflow_rows, underflow_rows, upa_rows, nullity_rows, uva_rows, location, context):
    ''' Generate reports for a statement manipulating memory '''
    reports = []

    if len(overflow_rows) == 1 and len(underflow_rows) == 1:
        # load, store, memset, strlen, strnlen
        assert len(upa_rows) <= 1
        assert len(nullity_rows) <= 1
        assert len(uva_rows) <= 3

        reports.append(generate_mem_access_report(overflow_rows[0],
                                                  underflow_rows[0],
                                                  upa_rows[0] if upa_rows else None,
                                                  nullity_rows[0] if nullity_rows else None,
                                                  uva_rows,
                                                  location,
                                                  context))
    elif len(overflow_rows) >= 2 and len(overflow_rows) <= 3 and len(underflow_rows) == 2:
        # memcpy, memmove, strncpy, strcat, strncat, strcpy
        assert len(upa_rows) <= 2
        assert len(nullity_rows) <= 2
        assert len(uva_rows) <= 3

        if len(overflow_rows) == 3:
            # find strcpy check and remove it from overflow_rows
            strcpy_row, overflow_rows = extract_mem_access_strcpy(overflow_rows)

            if strcpy_row:
                reports.append(generate_strcpy_report(strcpy_row,
                                                      location,
                                                      context))

        assert len(overflow_rows) == 2

        # find matching checks: overflow, underflow
        fst_overflow_row = overflow_rows.pop()
        snd_overflow_row = overflow_rows.pop()
        fst_overflow_info = json.loads(fst_overflow_row['info'])
        snd_overflow_info = json.loads(snd_overflow_row['info'])
        fst_underflow_row, underflow_rows = extract_mem_access_pointer_underflow(fst_overflow_info, underflow_rows)
        snd_underflow_row = underflow_rows.pop()

        # find matching upa checks
        if upa_rows:
            fst_upa_row, upa_rows = extract_mem_access_pointer_upa(fst_overflow_info, upa_rows)
            snd_upa_row = upa_rows.pop()
        else:
            fst_upa_row, snd_upa_row = None, None

        # find matching nullity checks
        if nullity_rows:
            fst_nullity_row, nullity_rows = extract_mem_access_pointer_nullity(fst_overflow_info, nullity_rows)
            snd_nullity_row = nullity_rows.pop()
        else:
            fst_nullity_row, snd_nullity_row = None, None

        # find matching uva checks
        if uva_rows:
            fst_pointer_uva, uva_rows = extract_mem_access_pointer_uva(fst_overflow_info, uva_rows)
            snd_pointer_uva, uva_rows = extract_mem_access_pointer_uva(snd_overflow_info, uva_rows)

            fst_uva_rows = [fst_pointer_uva]
            snd_uva_rows = [snd_pointer_uva]

            if isinstance(fst_overflow_info['access-size'], integer_types):
                if any(row_match(row, type='constant') for row in uva_rows):
                    fst_access_size_uva, uva_rows = extract_mem_access_size_uva(fst_overflow_info, uva_rows)
                    fst_uva_rows.append(fst_access_size_uva)
                if any(row_match(row, type='constant') for row in uva_rows):
                    snd_access_size_uva, uva_rows = extract_mem_access_size_uva(fst_overflow_info, uva_rows)
                    snd_uva_rows.append(snd_access_size_uva)
            elif fst_overflow_info['access-size'] == snd_overflow_info['access-size']:
                access_size, uva_rows = extract_mem_access_size_uva(fst_overflow_info, uva_rows)
                fst_uva_rows.append(access_size)
                snd_uva_rows.append(access_size)
            else:
                assert False, 'unreachable'
        else:
            fst_uva_rows, snd_uva_rows = [], []

        reports.append(generate_mem_access_report(fst_overflow_row,
                                                  fst_underflow_row,
                                                  fst_upa_row,
                                                  fst_nullity_row,
                                                  fst_uva_rows,
                                                  location,
                                                  context))
        reports.append(generate_mem_access_report(snd_overflow_row,
                                                  snd_underflow_row,
                                                  snd_upa_row,
                                                  snd_nullity_row,
                                                  snd_uva_rows,
                                                  location,
                                                  context))
        # Third operand (integer)
        if uva_rows:
            reports.append(generate_uva_report(uva_rows, location, context))
    else:
        assert False, 'unreachable'

    return reports


def generate_mem_access_report(overflow_row, underflow_row, upa_row, nullity_row, uva_rows, location, context):
    ''' Generate a report for a memory access '''
    overflow_info = json.loads(overflow_row['info'])

    all_uva_rows = uva_rows

    # try to guess the uva check for the pointer
    pointer_uva, uva_rows = extract_mem_access_pointer_uva(overflow_info, uva_rows)

    # try to guess the uva check for the access size
    size_uva, uva_rows = extract_mem_access_size_uva(overflow_info, uva_rows)

    # errors
    # undefined pointer
    if overflow_row['status'] == 'error' and overflow_info['type'] == 'undefined-constant-pointer':
        assert not pointer_uva or pointer_uva['status'] == 'error'
        return Report(location, [context], 'error', 'uva', 'memory access on an undefined pointer (undefined behavior)',
                      extra_msgs=generate_messages([size_uva] + uva_rows))
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'undefined-var-pointer':
        assert not pointer_uva or pointer_uva['status'] == 'error'
        short_msg = long_msg = 'memory access on an undefined pointer (undefined behavior)'
        if source_var(overflow_info['pointer']):
            long_msg = "memory access on undefined pointer '%s' (undefined behavior)" % demangle(source_var(overflow_info['pointer']))
        return Report(location, [context], 'error', 'uva', short_msg, long_msg,
                      extra_msgs=generate_messages([size_uva] + uva_rows))
    # nullity pointer
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'null-constant-pointer':
        assert not nullity_row or nullity_row['status'] == 'error'
        return Report(location, [context], 'error', 'nullity', 'null pointer dereference',
                      extra_msgs=generate_messages(all_uva_rows))
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'null-var-pointer':
        assert not nullity_row or nullity_row['status'] == 'error'
        short_msg = long_msg = 'null pointer dereference'
        if source_var(overflow_info['pointer']):
            long_msg = "null pointer dereference of pointer '%s'" % demangle(source_var(overflow_info['pointer']))
        return Report(location, [context], 'error', 'nullity', short_msg, long_msg,
                      extra_msgs=generate_messages(all_uva_rows))
    # undefined access size
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'undefined-constant-access-size':
        assert not size_uva or size_uva['status'] == 'error'
        return Report(location, [context], 'error', 'uva', 'undefined length parameter (undefined behavior)',
                      extra_msgs=generate_messages(uva_rows + [pointer_uva, nullity_row, upa_row]))
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'undefined-var-access-size':
        assert not size_uva or size_uva['status'] == 'error'
        short_msg = long_msg = 'undefined length parameter (undefined behavior)'
        if source_var(overflow_info['access-size']):
            long_msg = "uninitialized variable '%s'" % demangle(source_var(overflow_info['access-size']))
        return Report(location, [context], 'error', 'uva', short_msg, long_msg,
                      extra_msgs=generate_messages(uva_rows + [pointer_uva, nullity_row, upa_row]))
    # dereferencing a function pointer
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'function-pointer':
        short_msg = 'dereferencing a function pointer'
        long_msg = "dereferencing function pointer '%s'" % demangle(overflow_info['pointer'])
        return Report(location, [context], 'error', 'boa', short_msg, long_msg,
                      extra_msgs=generate_messages(all_uva_rows + [nullity_row]))
    # unaligned access
    elif upa_row and upa_row['status'] == 'error':
        short_msg = long_msg = 'unaligned pointer'
        if source_var(overflow_info['pointer']):
            long_msg += ": '%s'" % demangle(source_var(overflow_info['pointer']))
        return Report(location, [context], 'error', 'upa', short_msg, long_msg,
                      extra_msgs=generate_messages(all_uva_rows + [nullity_row]))
    # definite underflow
    elif underflow_row['status'] == 'error' and overflow_info['type'] == 'var':
        return generate_definite_underflow_report(underflow_row, overflow_row,
                                                  upa_row, nullity_row, all_uva_rows,
                                                  location, context)
    # definite overflow
    elif overflow_row['status'] == 'error' and overflow_info['type'] == 'var':
        return generate_definite_overflow_report(underflow_row, overflow_row,
                                                 upa_row, nullity_row, all_uva_rows,
                                                 location, context)
    # warnings
    # undefined pointer
    elif pointer_uva and pointer_uva['status'] == 'warning':
        short_msg = long_msg = 'pointer might be undefined'
        if source_var(overflow_info['pointer']):
            long_msg = "pointer '%s' might be uninitialized" % demangle(source_var(overflow_info['pointer']))
        return Report(location, [context], 'warning', 'uva', short_msg, long_msg,
                      extra_msgs=generate_messages([size_uva] + uva_rows))
    # nullity pointer
    elif nullity_row and nullity_row['status'] == 'warning':
        short_msg = long_msg = 'pointer might be null'
        if source_var(overflow_info['pointer']):
            long_msg = "pointer '%s' might be null" % demangle(source_var(overflow_info['pointer']))
        return Report(location, [context], 'warning', 'nullity', short_msg, long_msg,
                      extra_msgs=generate_messages(all_uva_rows))
    # undefined access size
    elif size_uva and size_uva['status'] == 'warning':
        # (only for memcpy, memmove and memset)
        short_msg = long_msg = 'length parameter might be undefined'
        if source_var(overflow_info['access-size']):
            long_msg = "variable '%s' might be uninitialized" % demangle(source_var(overflow_info['access-size']))
        return Report(location, [context], 'warning', 'uva', short_msg, long_msg,
                      extra_msgs=generate_messages(uva_rows + [pointer_uva, nullity_row, upa_row]))
    # no points-to
    elif overflow_row['status'] == 'warning' and overflow_info['type'] == 'no-points-to':
        short_msg = 'memory access might be unsafe'
        if source_var(overflow_info['pointer']):
            long_msg = "memory access might be unsafe, the analysis could not detect the memory location pointed by '%s'" % demangle(source_var(overflow_info['pointer']))
        else:
            long_msg = 'memory access might be unsafe, the analysis could not detect the accessed memory location'
        return Report(location, [context], 'warning', 'boa', short_msg, long_msg,
                      extra_msgs=generate_messages(all_uva_rows + [nullity_row]))
    # unaligned access
    elif upa_row and upa_row['status'] == 'warning':
        short_msg = long_msg = 'pointer might be unaligned'
        if source_var(overflow_info['pointer']):
            long_msg += ": '%s'" % demangle(source_var(overflow_info['pointer']))
        return Report(location, [context], 'warning', 'upa', short_msg, long_msg,
                      extra_msgs=generate_messages(all_uva_rows + [nullity_row]))
    # warning
    elif (underflow_row['status'] == 'warning' or overflow_row['status'] == 'warning') and overflow_info['type'] == 'var':
        return generate_mem_access_warning_report(underflow_row, overflow_row,
                                                  upa_row, nullity_row, all_uva_rows,
                                                  location, context)
    # safe
    else:
        assert underflow_row['status'] == 'ok' and overflow_row['status'] == 'ok'
        assert overflow_info['type'] in ('var', 'global-var')
        return Report(location, [context], 'safe', 'boa', 'safe memory access',
                      extra_msgs=generate_messages(all_uva_rows + [nullity_row, upa_row, overflow_row, underflow_row]))


def generate_definite_underflow_report(underflow_row, overflow_row, upa_row, nullity_row, uva_rows, location, context):
    ''' Generate a report for a definite buffer underflow '''
    overflow_info = json.loads(overflow_row['info'])
    underflow_info = json.loads(underflow_row['info'])
    ptr = overflow_info['pointer']
    access_size = overflow_info['access-size']
    offset_min = underflow_info['offset_min']
    offset_max = underflow_info['offset_max']

    short_msg = 'buffer underflow'

    if is_array_access(underflow_info, overflow_info):
        index_min = bound_idiv(offset_min, access_size)
        index_max = bound_idiv(offset_max, access_size)

        if index_min == index_max:
            long_msg = 'buffer underflow, trying to access index %d' % index_max
        else:
            long_msg = 'buffer underflow, trying to access a negative index (%s)' % var_within_interval('index', index_min, index_max)
    else:
        if source_var(ptr):
            if offset_min == offset_max:
                long_msg = "buffer underflow, pointer '%s' has a negative offset %d (bytes)" % (demangle(source_var(ptr)), offset_max)
            else:
                long_msg = "buffer underflow, pointer '%s' has a negative offset (%s) (bytes)" % (demangle(source_var(ptr)), var_within_interval('offset', offset_min, offset_max))
        else:
            if offset_min == offset_max:
                long_msg = 'buffer underflow, trying to access offset %d (bytes)' % offset_max
            else:
                long_msg = 'buffer underflow, trying to access a negative offset (%s) (bytes)' % var_within_interval('offset', offset_min, offset_max)

    return Report(location, [context], 'error', 'boa', short_msg, long_msg,
                  extra_msgs=generate_messages(uva_rows + [nullity_row, upa_row]))


def generate_definite_overflow_report(underflow_row, overflow_row, upa_row, nullity_row, uva_rows, location, context):
    ''' Generate a report for a definite buffer overflow '''
    overflow_info = json.loads(overflow_row['info'])
    underflow_info = json.loads(underflow_row['info'])
    access_size = overflow_info['access-size']
    offset_min = underflow_info['offset_min']
    offset_max = underflow_info['offset_max']
    points_to = overflow_info['points-to']

    short_msg = 'buffer overflow'

    if is_array_access(underflow_info, overflow_info):
        index_min = bound_idiv(offset_min, access_size)
        index_max = bound_idiv(offset_max, access_size)

        if index_min == index_max:
            # array access on a specific index
            long_msg = 'buffer overflow, trying to access index %d' % index_max
            memory_locations = [array_str(mem, access_size, overflow_row['function']) for mem in points_to]

            if len(memory_locations) == 1:
                full_msg = 'buffer overflow, trying to access index %d of %s' % (index_max, memory_locations[0])
            else:
                full_msg = 'buffer overflow, trying to access index %d of:\n   ' % index_max + '\nor '.join(memory_locations)
        else:
            # array access on an unknown index
            long_msg = short_msg

            memory_locations = []
            for mem in points_to: # all invalid
                diff_min = bound_idiv(mem['diff_min'], access_size)
                diff_max = bound_idiv(mem['diff_max'], access_size)

                sign = '=' if diff_min == diff_max else '>='
                op = ' + %d' % (diff_min - 1) if diff_min > 1 else ''
                s = 'trying to access index %s size%s of ' % (sign, op)
                s += array_str(mem, access_size, overflow_row['function'])
                memory_locations.append(s)

            if len(memory_locations) == 1:
                full_msg = 'buffer overflow, %s' % memory_locations[0]
            else:
                full_msg = 'buffer overflow:\n   ' + '\nor '.join(memory_locations)
    else:
        # not is_array_access(underflow_info, overflow_info):
        if offset_min == offset_max and isinstance(access_size, integer_types):
            # access on a specific offset
            offset = offset_max + access_size - 1
            long_msg = 'buffer overflow, trying to access offset %d (bytes)' % offset
            memory_locations = [memory_str(mem, overflow_row['function']) for mem in points_to]

            if len(memory_locations) == 1:
                full_msg = 'buffer overflow, trying to access offset %d (bytes) of %s' % (offset, memory_locations[0])
            else:
                full_msg = 'buffer overflow, trying to access offset %d (bytes) of:\n   ' % offset + '\nor '.join(memory_locations)
        else:
            # access on unknown offset
            long_msg = short_msg

            memory_locations = []
            for mem in points_to: # all invalid
                sign = '=' if mem['diff_min'] == mem['diff_max'] else '>='
                op = ' + %d' % (mem['diff_min'] - 1) if mem['diff_min'] > 1 else ''
                s = 'trying to access offset %s size%s (bytes) of ' % (sign, op)
                s += memory_str(mem, overflow_row['function'])
                memory_locations.append(s)

            if len(memory_locations) == 1:
                full_msg = 'buffer overflow, %s' % memory_locations[0]
            else:
                full_msg = 'buffer overflow:\n   ' + '\nor '.join(memory_locations)

    return Report(location, [context], 'error', 'boa', short_msg, long_msg, full_msg,
                  extra_msgs=generate_messages(uva_rows + [nullity_row, upa_row]))


def generate_mem_access_warning_report(underflow_row, overflow_row, upa_row, nullity_row, uva_rows, location, context):
    ''' Generate a report for a possible buffer underflow/overflow '''
    overflow_info = json.loads(overflow_row['info'])
    underflow_info = json.loads(underflow_row['info'])
    access_size = overflow_info['access-size']
    offset_min = underflow_info['offset_min']
    offset_max = underflow_info['offset_max']
    points_to = overflow_info['points-to']

    if is_array_access(underflow_info, overflow_info):
        short_msg = 'array access might be unsafe'
    else:
        short_msg = 'memory access might be unsafe'

    if underflow_row['status'] == 'warning' and overflow_row['status'] == 'warning':
        long_msg = short_msg + ', possible underflow/overflow'
        safe_mem_rows = []
    elif underflow_row['status'] == 'warning':
        long_msg = short_msg + ', possible underflow'
        safe_mem_rows = [overflow_row]
    elif overflow_row['status'] == 'warning':
        long_msg = short_msg + ', possible overflow'
        safe_mem_rows = [underflow_row]
    else:
        assert False, 'unreachable'

    if is_array_access(underflow_info, overflow_info):
        index_min = bound_idiv(offset_min, access_size)
        index_max = bound_idiv(offset_max, access_size)
        long_msg += ' (%s)' % var_within_interval('index', index_min, index_max)

        memory_locations = []
        for mem in points_to:
            diff_max = bound_idiv(mem['diff_max'], access_size)

            if diff_max != '+oo' and int(diff_max) <= 0: # safe
                continue

            if diff_max != '+oo':
                s = 'index might be up to size + %d of ' % (diff_max - 1)
            else:
                s = 'index might be >= size of '

            s += array_str(mem, access_size, overflow_row['function'])
            memory_locations.append(s)

        full_msg = long_msg
        if underflow_row['status'] == 'warning':
            full_msg += '\nindex might be < 0'
        if overflow_row['status'] == 'warning':
            assert memory_locations
            full_msg += '\n' + '\n'.join(memory_locations)
    else:
        long_msg += ' (%s)' % var_within_interval('offset', offset_min, offset_max)

        memory_locations = []
        for mem in points_to:
            if mem['diff_max'] != '+oo' and int(mem['diff_max']) <= 0: # safe
                continue

            if mem['diff_max'] != '+oo':
                s = 'offset might be up to size + %d (bytes) of ' % (mem['diff_max'] - 1)
            else:
                s = 'offset might be >= size (bytes) of '

            s += memory_str(mem, overflow_row['function'])
            memory_locations.append(s)

        full_msg = long_msg
        if underflow_row['status'] == 'warning':
            full_msg += '\noffset might be < 0'
        if overflow_row['status'] == 'warning':
            assert memory_locations
            full_msg += '\n' + '\n'.join(memory_locations)

    return Report(location, [context], 'warning', 'boa', short_msg, long_msg, full_msg,
                  extra_msgs=generate_messages(uva_rows + [nullity_row, upa_row] + safe_mem_rows))


def generate_strcpy_report(overflow_row, location, context):
    ''' Generate a report for a strcpy() check '''
    overflow_info = json.loads(overflow_row['info'])
    assert overflow_info['type'] == 'strcpy'

    if overflow_row['status'] == 'ok':
        short_msg = 'safe strcpy call'
        return Report(location, [context], 'safe', 'boa', short_msg)
    elif overflow_row['status'] == 'warning':
        short_msg = 'strcpy call might be unsafe'
        return Report(location, [context], 'warning', 'boa', short_msg)
    else:
        assert False, 'unreachable'


#####################
# generate messages #
#####################


def operand_str(info):
    ''' Return the operand as a string '''
    if info['type'] == 'local-var':
        if source_var(info['name']):
            return quote_var(source_var(info['name']), prefix='local variable ')
        else:
            return 'local variable'
    elif info['type'] == 'global-var':
        if source_var(info['name']):
            return quote_var(source_var(info['name']), prefix='global variable ')
        else:
            return 'static variable'
    elif info['type'] == 'function-pointer':
        return "function pointer to '%s'" % demangle(info['name'])
    elif info['type'] == 'null-constant':
        return 'null'
    elif info['type'] == 'undefined-constant':
        return 'undefined constant'
    elif info['type'] == 'constant':
        return 'constant'
    elif info['type'] == 'var':
        if source_var(info['name']):
            return quote_var(source_var(info['name']), prefix='variable ')
        else:
            return 'expression'
    else:
        assert False, 'unexpected operand'


def var_within_interval(name, min='-oo', max='+oo', suffix='', default=None):
    if min == max:
        return '%s = %d%s' % (name, min, suffix)
    elif min != '-oo' and max != '+oo':
        return '%d%s <= %s <= %d%s' % (min, suffix, name, max, suffix)
    elif min != '-oo':
        return '%s >= %d%s' % (name, min, suffix)
    elif max != '+oo':
        return '%s <= %d%s' % (name, max, suffix)
    elif default:
        return default
    else:
        return 'could not bound %s' % name


def array_str(mem, element_size, current_function=None):
    if source_var(mem['addr']):
        s = quote_var(source_var(mem['addr']), prefix='array ')
    else:
        s = 'array'

    if is_local_var(mem['addr']) and local_var_function(mem['addr']) != current_function:
        s += " (in function '%s')" % demangle(local_var_function(mem['addr']))

    elements_min = bound_idiv(mem['size_min'], element_size)
    elements_max = bound_idiv(mem['size_max'], element_size)
    if elements_min == elements_max:
        s += ' containing %d elements' % elements_max
    elif elements_max != '+oo':
        s += ' containing at most %d elements' % elements_max

    return s


def memory_str(mem, current_function=None):
    if is_global_var(mem['addr']) and source_var(mem['addr']):
        s = quote_var(source_var(mem['addr']), prefix='global variable ')
    elif source_var(mem['addr']):
        s = quote_var(source_var(mem['addr']), prefix='buffer ')
    else:
        s = 'buffer'

    if is_local_var(mem['addr']) and local_var_function(mem['addr']) != current_function:
        s += " (in function '%s')" % demangle(local_var_function(mem['addr']))

    if mem['size_min'] == mem['size_max']:
        s += ' of size %d bytes' % mem['size_max']
    elif mem['size_max'] != '+oo':
        s += ' of size <= %d bytes' % mem['size_max']

    return s


def generate_dbz_msg(row):
    ''' Generate a message for a dbz check '''
    assert row['safety_check'] == 'dbz'
    info = json.loads(row['info'])

    if row['status'] == 'error':
        return 'division by zero'
    elif row['status'] == 'warning':
        return 'divisor might be 0 (%s)' % var_within_interval('divisor', info['min'], info['max'])
    elif row['status'] == 'ok':
        return 'safe division (%s)' % var_within_interval('divisor', info['min'], info['max'])
    else:
        assert False, 'unexpected dbz check'


def generate_nullity_msg(row):
    ''' Generate a message for a nullity check '''
    assert row['safety_check'] == 'nullity'
    info = json.loads(row['info'])

    if row['status'] == 'ok' and info['type'] in ('local-var', 'global-var'):
        return 'non-null %s' % operand_str(info)
    elif row['status'] == 'error' and info['type'] == 'function-pointer':
        return 'dereferencing %s' % operand_str(info)
    elif row['status'] == 'error' and info['type'] == 'null-constant':
        return 'null dereference'
    elif info['type'] == 'undefined-constant':
        return None
    elif info['type'] == 'var':
        if row['status'] == 'ok':
            return '%s is non-null' % operand_str(info)
        elif row['status'] == 'warning':
            return '%s might be null' % operand_str(info)
        elif row['status'] == 'error':
            return '%s is null' % operand_str(info)
        else:
            assert False, 'unreachable'
    else:
        assert False, 'unexpected nullity check'


def generate_overflow_underflow_msg(row):
    ''' Generate a message for a overflow/underflow check '''
    assert row['safety_check'] in ('overflow', 'underflow')
    info = json.loads(row['info'])

    if row['status'] == 'error' and info['type'] == 'undefined-constant-pointer':
        return 'undefined expression (undefined behavior)'
    elif row['status'] == 'error' and info['type'] == 'undefined-var-pointer':
        if source_var(info['pointer']):
            return "uninitialized variable '%s' (undefined behavior)" % demangle(source_var(info['pointer']))
        else:
            return 'undefined expression (undefined behavior)'
    elif row['status'] == 'error' and info['type'] == 'null-constant-pointer':
        return 'null pointer dereference'
    elif row['status'] == 'error' and info['type'] == 'null-var-pointer':
        if source_var(info['pointer']):
            return "null pointer dereference of pointer '%s'" % demangle(source_var(info['pointer']))
        else:
            return 'null pointer dereference'
    elif row['status'] == 'error' and info['type'] == 'undefined-constant-access-size':
        return 'undefined length parameter (undefined behavior)'
    elif row['status'] == 'error' and info['type'] == 'undefined-var-access-size':
        if source_var(info['access-size']):
            return "uninitialized variable '%s'" % demangle(source_var(info['access-size']))
        else:
            return 'undefined length parameter (undefined behavior)'
    elif row['status'] == 'error' and info['type'] == 'function-pointer':
        return "dereferencing function pointer '%s'" % demangle(info['pointer'])
    elif row['status'] == 'ok' and info['type'] == 'global-var':
        if source_var(info['pointer']):
            return 'safe memory access on %s' % quote_var(source_var(info['pointer']), prefix='global variable ')
        else:
            return 'safe memory access on global variable'
    elif row['status'] == 'warning' and info['type'] == 'no-points-to':
        if source_var(info['pointer']):
            return "memory access might be unsafe, the analysis could not detect the memory location pointed by '%s'" % demangle(source_var(info['pointer']))
        else:
            return 'memory access might be unsafe, the analysis could not detect the accessed memory location'
    elif info['type'] == 'strcpy':
        return _generate_overflow_msg(row)
    elif info['type'] == 'var':
        if 'points-to' in info:
            return _generate_overflow_msg(row)
        elif 'offset_min' in info:
            return _generate_underflow_msg(row)
        else:
            assert False, 'unreachable'
    else:
        assert False, 'unreachable'


def _generate_overflow_msg(row):
    ''' Generate a message for a overflow check '''
    assert row['safety_check'] == 'overflow'
    info = json.loads(row['info'])

    if info['type'] == 'strcpy':
        if row['status'] == 'ok':
            return 'safe strcpy call'
        elif row['status'] == 'warning':
            return 'strcpy call might be unsafe'
        else:
            assert False, 'unreachable'
    elif info['type'] == 'var':
        assert row['status'] == 'ok', '_generate_overflow_msg() only supports safe checks'
        points_to = info['points-to']

        memory_locations = []
        for mem in points_to:
            s = 'accessing offsets up to size - %d (bytes) of ' % abs(mem['diff_max'] - 1)
            s += memory_str(mem, row['function'])
            memory_locations.append(s)

        if len(memory_locations) == 1:
            return 'no buffer overflow, %s' % memory_locations[0]
        else:
            return 'no buffer overflow:\n   ' + '\nor '.join(memory_locations)
    else:
        assert False, 'unexpected overflow check'


def _generate_underflow_msg(row):
    ''' Generate a message for a underflow check '''
    assert row['safety_check'] == 'underflow'
    info = json.loads(row['info'])

    if info['type'] == 'var':
        assert row['status'] == 'ok', '_generate_underflow_msg() only supports safe checks'
        return 'no buffer underflow, accessing offsets >= %d (bytes)' % info['offset_min']
    else:
        assert False, 'unexpected underflow check'


def generate_uva_msg(row):
    ''' Generate a message for a uva check '''
    assert row['safety_check'] == 'uva'
    info = json.loads(row['info'])

    if row['status'] == 'error':
        assert info['type'] in ('undefined-constant', 'var')
        if info['type'] == 'var' and source_var(info['name']):
            return "uninitialized variable '%s' (undefined behavior)" % demangle(source_var(info['name']))
        else:
            return 'undefined expression (undefined behavior)'
    elif row['status'] == 'warning':
        assert info['type'] == 'var'
        if source_var(info['name']):
            return "variable '%s' might be uninitialized" % demangle(source_var(info['name']))
        else:
            return 'expression might be undefined'
    elif row['status'] == 'ok':
        if info['type'] == 'constant':
            return None
        elif info['type'] == 'var' and not source_var(info['name']):
            return 'expression well defined'
        else:
            return '%s well initialized' % operand_str(info)
    else:
        assert False, 'unexpected uva check'


def generate_upa_msg(row):
    ''' Generate a message for a upa check '''
    assert row['safety_check'] == 'upa'
    info = json.loads(row['info'])

    if row['status'] == 'error':
        assert info['type'] in ('null-constant', 'undefined-constant', 'null-pointer', 'undefined-pointer', 'var')
        if info['type'] == 'null-constant':
            return 'null dereference'
        elif info['type'] == 'undefined-constant':
            return 'undefined constant'
        elif info['type'] == 'null-pointer':
            if source_var(info['name']):
                return "null pointer dereference of pointer '%s'" % demangle(source_var(info['name']))
            else:
                return 'null pointer dereference'
        elif info['type'] == 'undefined-pointer':
            if source_var(info['name']):
                return "undefined pointer '%s'" % demangle(source_var(info['name']))
            else:
                return 'undefined pointer'
        elif info['type'] == 'var':
            if source_var(info['name']):
                return "unaligned pointer '%s'" % demangle(source_var(info['name']))
            else:
                return 'unaligned pointer'
    elif row['status'] == 'warning':
        assert info['type'] in ('no-points-to', 'var')
        if info['type'] == 'no-points-to':
            if source_var(info['name']):
                return "memory access might be unaligned, the analysis could not detect the memory location pointed by '%s'" % demangle(source_var(info['name']))
            else:
                return 'memory access might be unaligned, the analysis could not detect the accessed memory location'
        elif info['type'] == 'var':
            if source_var(info['name']):
                return "pointer might be unaligned: '%s'" % demangle(source_var(info['name']))
            else:
                return 'pointer might be unaligned'
    elif row['status'] == 'ok':
        assert info['type'] in ('no-requirement', 'var')
        if info['type'] == 'no-requirement':
                return 'no requirement needed for pointer'
        elif info['type'] == 'var':
            if source_var(info['name']):
                return "pointer aligned to requirement (%s)" % demangle(source_var(info['name']))
            else:
                return 'pointer aligned to requirement'


def generate_messages(rows):
    ''' Generate a list of messages for a list of checks '''
    messages = []

    for row in rows:
        if not row:
            msg, checker = None, None
        elif row['safety_check'] == 'uva':
            msg, checker = generate_uva_msg(row), 'uva'
        elif row['safety_check'] == 'upa':
            msg, checker = generate_upa_msg(row), 'upa'
        elif row['safety_check'] == 'dbz':
            msg, checker = generate_dbz_msg(row), 'dbz'
        elif row['safety_check'] == 'nullity':
            msg, checker = generate_nullity_msg(row), 'nullity'
        elif row['safety_check'] in ('overflow', 'underflow'):
            msg, checker = generate_overflow_underflow_msg(row), 'boa'
        else:
            assert False, 'unreachable'

        if msg and (msg, checker) not in messages: # remove duplicates
            messages.append((msg, checker))

    return messages


##########################
# command line interface #
##########################


def parse_opt(argv):
    usage = '%(prog)s [options] file.db'
    parser = argparse.ArgumentParser(usage=usage)

    parser.add_argument('--colors', dest='colors',
                        help='Enable colors (default: auto)',
                        default='auto',
                        choices=('off', 'on', 'no', 'yes', 'auto'))
    parser.add_argument('-t', '--times', dest='display_timing_results',
                        help='Display timing results (default: off)',
                        default='off',
                        choices=('off', 'short', 'full'))
    parser.add_argument('-s', '--summary', dest='display_summary',
                        help='Display the analysis summary (default: off)',
                        default='off',
                        choices=('off', 'short', 'full'))
    parser.add_argument('--show-raw-checks', dest='show_raw_checks',
                        help='Print analysis raw checks',
                        action='store_true',
                        default=False)
    parser.add_argument('-e', '--export', dest='export',
                        help='Export analysis results into a specific format',
                        action='store_true',
                        default=False)
    parser.add_argument('--export-format', dest='export_format',
                        help='Export format (default: gcc)',
                        default='gcc',
                        choices=formats.keys())
    parser.add_argument('-o', '--export-file', dest='export_file', metavar='FILE',
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
    parser.add_argument('-v', '--export-verbosity', dest='export_verbosity', metavar='[1-4]',
                        help='Export verbosity (default: 2)',
                        default=2,
                        type=int)

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

    # positional argument
    parser.add_argument('file', metavar='file.db',
                        help='The result database')

    return parser.parse_args(argv)


########################
# main for ikos-render #
########################

def main(argv):
    progname = os.path.basename(argv[0])

    # parse arguments
    opt = parse_opt(argv[1:])

    # setup colors
    colors.setup(opt.colors)

    if not os.path.isfile(opt.file):
        printf("%s: error: no such file: \'%s\'\n", progname, opt.file, file=sys.stderr)
        sys.exit(1)

    try:
        # open result database
        db = sqlite3.connect(opt.file)

        first = True

        # load settings
        settings = load_settings(db)

        # display timing results
        if opt.display_timing_results != 'off':
            if not first: printf('\n')
            print_timing_results(db, opt.display_timing_results == 'full')
            first = False

        # display summary
        if opt.display_summary != 'off':
            if not first: printf('\n')
            print_summary(db, opt.display_summary == 'full')
            first = False

        # display raw checks
        if opt.show_raw_checks:
            if not first: printf('\n')
            print_raw_checks(db, settings['procedural'] == 'inter')
            first = False

        # export
        if opt.export:
            if not first and opt.export_file is sys.stdout: printf('\n')

            # setup colors again (in case opt.colors = 'auto')
            colors.setup(opt.colors, file=opt.export_file)

            # export format
            formatter_class = formats[opt.export_format]
            formatter = formatter_class(opt.export_file, opt.export_verbosity, opt.export_demangle)

            export(db, formatter, opt.export_level, not opt.export_no_unreachable)

        # close database
        db.close()
    except sqlite3.DatabaseError as e:
        printf('%s: error: %s\n', progname, e, file=sys.stderr)
        sys.exit(1)
