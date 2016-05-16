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

import sqlite3


# Get number of errors, warnings, and safe checks FROM the analysis
def get_analysis_results(db, table):
    conn = sqlite3.connect(db)
    cursor = conn.cursor()
    table = table + '_results'
    cursor.execute('SELECT COUNT(*) FROM %s' % table)
    all_rows = cursor.fetchall()
    num_total = all_rows[0][0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="ok"' % table)
    all_rows = cursor.fetchall()
    num_ok = all_rows[0][0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="error"' % table)
    all_rows = cursor.fetchall()
    num_err = all_rows[0][0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="warning"' % table)
    all_rows = cursor.fetchall()
    num_war = all_rows[0][0]
    cursor.execute('SELECT COUNT(*) FROM %s WHERE status="unreachable"' % table)
    all_rows = cursor.fetchall()
    num_unr = all_rows[0][0]

    return (num_total, num_ok, num_err, num_war, num_unr)


# Print all analysis checks to standard output
def print_checks(db, inter_mode, table):
    if inter_mode:
        header = [('check', 'context', 'file', 'line', 'result')]
        order = 'context, file, line'
    else:
        header = [('check', 'file', 'line', 'result')]
        order = 'file, line'

    conn = sqlite3.connect(db)
    cursor = conn.cursor()
    table = table + '_results'
    cursor.execute('SELECT * FROM %s ORDER BY %s' % (table, order))
    all_rows = cursor.fetchall()
    # Reorganize data by columns
    all_rows = header + all_rows
    cols = zip(*all_rows)
    # Compute column widths by taking maximum length of values per column
    col_widths = [max(len(str(value)) for value in col) for col in cols]
    # Create a suitable format string
    format = ' '.join(['%%-%ds' % width for width in col_widths])
    # Print each row using the computed format
    for row in all_rows:
        print(format % tuple(row))
    conn.close()
