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


# Use c++filt (if available) to demangle a set of symbol names
# Returns a map from symbol names to demangled names
def demangle_symbols(symbols):
    symbols = list(symbols)

    try:
        p = subprocess.Popen(['c++filt', '--no-verbose'],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        stdout, _ = p.communicate('\n'.join(symbols).encode('utf8'))
        lines = stdout.decode('utf8').strip().split('\n')

        demangled = {}
        for symbol, function_name in zip(symbols, lines):
            demangled[symbol] = function_name

        return demangled
    except FileNotFoundError:
        return {}


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
        # Collect all symbols
        symbols = set()

        for row in all_rows:
            for callsite in row[1].split(':'):
                if '@' not in callsite:
                    continue
                symbol, line = callsite.split('@')
                symbols.add(symbol)

        # Demangle
        demangled = demangle_symbols(symbols)

        # Update all_rows
        for i, row in enumerate(all_rows):
            context = []
            for callsite in row[1].split(':'):
                if '@' not in callsite:
                    context.append(callsite)
                else:
                    symbol, line = callsite.split('@')
                    context.append('%s@%s' % (demangled.get(symbol, symbol), line))

            context = '/'.join(context)
            filepath = os.path.relpath(row[2], os.getcwd())
            all_rows[i] = (row[0], context, filepath, row[3], row[4], row[6])
    else:
        for i, row in enumerate(all_rows):
            filepath = os.path.relpath(row[1], os.getcwd())
            all_rows[i] = (row[0], filepath, row[2], row[3], row[5])

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
