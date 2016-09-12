#*******************************************************************************
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2016 United States Government as represented by the
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
#*****************************************************************************/

import atexit
import os
import sqlite3
import subprocess


# Get number of errors, warnings, and safe checks FROM the analysis
def get_analysis_raw_results(database_path, table):
    db = sqlite3.connect(database_path)
    cursor = db.cursor()
    table = table + '_results'
    results = {}

    cursor.execute('SELECT COUNT(*) FROM %s' % table)
    results['total'] = cursor.fetchone()[0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="ok"' % table)
    results['ok'] = cursor.fetchone()[0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="error"' % table)
    results['error'] = cursor.fetchone()[0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="warning"' % table)
    results['warning'] = cursor.fetchone()[0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="unreachable"' % table)
    results['unreachable'] = cursor.fetchone()[0]

    cursor.close()
    db.close()
    assert results['total'] == results['ok'] + results['error'] + results['warning'] + results['unreachable']
    return results


# Get number of errors, warnings, and safe checks FROM the analysis PER source code location
def get_analysis_results_per_location(database_path, table):
    db = sqlite3.connect(database_path)
    cursor = db.cursor()
    table = table + '_results'
    results = {}

    # count the number of checked source code locations (group by statement UID and type of check)
    cursor.execute('SELECT COUNT(*) FROM '
                   '(SELECT * FROM %s GROUP BY stmt_uid, safety_check)' % table)
    results['total'] = cursor.fetchone()[0]

    # count the number of source code locations that only have unreachable checks (ie. dead code)
    cursor.execute('SELECT COUNT(*) FROM '
                   '(SELECT * FROM %s r GROUP BY stmt_uid, safety_check'
                   ' HAVING COUNT(*)=(SELECT COUNT(*) FROM %s WHERE stmt_uid=r.stmt_uid AND safety_check=r.safety_check AND status="unreachable"))' % (table, table))
    results['unreachable'] = cursor.fetchone()[0]

    # ignoring unreachable checks, count the number of source code locations
    # that only have "ok" checks
    cursor.execute('SELECT COUNT(*) FROM '
                   '(SELECT * FROM %s r WHERE status != "unreachable" GROUP BY stmt_uid, safety_check'
                   ' HAVING COUNT(*)=(SELECT COUNT(*) FROM %s WHERE stmt_uid=r.stmt_uid AND safety_check=r.safety_check AND status="ok"))' % (table, table))
    results['ok'] = cursor.fetchone()[0]

    # count the number of source code locations that have at least one "error" check
    cursor.execute('SELECT COUNT(*) FROM '
                   '(SELECT * FROM %s WHERE status="error" GROUP BY stmt_uid, safety_check)' % table)
    results['error'] = cursor.fetchone()[0]

    # deduce the number of warnings
    results['warning'] = results['total'] - results['unreachable'] - results['ok'] - results['error']

    cursor.close()
    db.close()
    assert results['total'] == results['ok'] + results['error'] + results['warning'] + results['unreachable']
    return results


# Use c++filt (if available) to demangle symbol names

# is c++filt available?
_cppfilt_available = None

# current c++filt process
_cppfilt_proc = None

# cache of already demangled symbols
_cache_demangle = {}


# kill the current c++filt process
def _cppfilt_kill():
    global _cppfilt_proc

    if _cppfilt_proc and _cppfilt_proc.poll() is None:
        _cppfilt_proc.communicate()


def is_mangled(symbol):
    return len(symbol) >= 2 and symbol[0] == '_' and symbol[1] >= 'A' and symbol[1] <= 'Z'


# demangle a symbol name using c++filt if available
def demangle_symbol(symbol):
    global _cppfilt_available
    global _cppfilt_proc
    global _cache_demangle

    if not is_mangled(symbol):
        return symbol

    if _cppfilt_available is False:
        return symbol

    if symbol in _cache_demangle:
        return _cache_demangle[symbol]

    try:
        if not _cppfilt_proc:
            _cppfilt_proc = subprocess.Popen(['c++filt', '--no-strip-underscores', '--no-verbose'],
                                             stdin=subprocess.PIPE,
                                             stdout=subprocess.PIPE,
                                             stderr=subprocess.PIPE)
            _cppfilt_available = True
            atexit.register(_cppfilt_kill)

        if _cppfilt_proc.poll() is not None:
            return symbol

        _cppfilt_proc.stdin.write(symbol.encode('utf8') + b'\n')
        stdout = _cppfilt_proc.stdout.readline()
        result = stdout.decode('utf8').strip()
    except OSError:
        _cppfilt_available = False
        result = symbol

    _cache_demangle[symbol] = result
    return result


def format_context(context):
    ctx = []
    for callsite in context.split('/'):
        if '@' not in callsite:
            ctx.append(callsite)
        else:
            symbol, line, col = callsite.split('@')
            ctx.append('%s@%s@%s' % (demangle_symbol(symbol), line, col))

    return '/'.join(ctx)


def format_path(path):
    abs_path = os.path.realpath(path)
    rel_path = os.path.relpath(os.path.realpath(path), os.getcwd())
    return min(abs_path, rel_path, key=len)


# Print all analysis checks to standard output
def print_checks(database_path, inter_mode, table):
    if inter_mode:
        header = ('check', 'context', 'file', 'line', 'col', 'result')
        order = 'file, line, column, stmt_uid, context, safety_check'
    else:
        header = ('check', 'file', 'line', 'col', 'result')
        order = 'file, line, column, stmt_uid, safety_check'

    db = sqlite3.connect(database_path)
    cursor = db.cursor()

    cursor.execute('SELECT * FROM %s_results ORDER BY %s' % (table, order))
    all_rows = cursor.fetchall()

    # Demangle context info
    if inter_mode:
        for i, row in enumerate(all_rows):
            all_rows[i] = (row[0], format_context(row[1]), format_path(row[2]),
                           row[3], row[4], row[6])
    else:
        for i, row in enumerate(all_rows):
            all_rows[i] = (row[0], format_path(row[1]), row[2], row[3], row[5])

    # Reorganize data by columns
    cols = zip(*([header] + all_rows))

    # Compute column widths by taking maximum length of values per column
    col_widths = [max(len(str(value)) for value in col) for col in cols]

    # Create a suitable format string
    format = '|' + '|'.join([' %%-%ds ' % width for width in col_widths]) + '|'

    # Print each row using the computed format
    print(format % header)
    print('+' + '+'.join('-' * (width + 2) for width in col_widths) + '+')
    for row in all_rows:
        print(format % row)

    # Close the database
    cursor.close()
    db.close()
