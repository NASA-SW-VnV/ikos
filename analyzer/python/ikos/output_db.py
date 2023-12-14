###############################################################################
#
# Extract information from a result database
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018-2023 United States Government as represented by the
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
import collections
import json
import sqlite3

from ikos.enums import FilesTable, FunctionsTable, StatementsTable, \
    CallContextsTable, OperandsTable, MemoryLocationsTable, ChecksTable


class CachedProperty(object):
    ''' A property attribute that only calls its getter the first access. '''

    def __init__(self, factory):
        self._attr_name = factory.__name__
        self._factory = factory

    def __get__(self, instance, owner):
        value = self._factory(instance)
        setattr(instance, self._attr_name, value)
        return value


class OutputDatabase(object):
    ''' Represents an output database '''

    def __init__(self, path):
        self.path = path
        self.con = sqlite3.connect(path)

        # Use 'str' as text factory since it's the type of string literals
        # This is unicode
        self.con.text_factory = str

    def close(self):
        self.con.close()

    def load_settings(self):
        ''' Load the analysis settings from the database '''
        c = self.con.cursor()
        c.execute('SELECT name, value FROM settings')

        settings = {}
        for key, value in c:
            try:
                settings[key] = json.loads(value)
            except ValueError:
                settings[key] = value

        return settings

    def insert_settings(self, rows):
        ''' Insert the analysis settings into the database '''
        c = self.con.cursor()
        c.executemany('INSERT INTO settings VALUES (?, ?)', rows)
        self.con.commit()

    def load_timing_results(self, full=True, sort=True):
        '''
        Load the timing results from the database,
        as a list of tuples (pass, elapsed)
        '''
        c = self.con.cursor()
        where = "WHERE pass NOT LIKE '%ikos-analyzer.%'" if not full else ''
        order_by = 'ORDER BY pass' if sort else ''
        c.execute('SELECT pass, time FROM times %s %s' % (where, order_by))
        return c.fetchall()

    def insert_timing_results(self, rows):
        ''' Insert the timing results into the database '''
        c = self.con.cursor()
        c.executemany('INSERT INTO times VALUES (?, ?)', rows)
        self.con.commit()

    @CachedProperty
    def files(self):
        return self._fetch_table('files', File)

    @CachedProperty
    def functions(self):
        return self._fetch_table('functions', Function)

    @CachedProperty
    def statements(self):
        return self._fetch_table('statements', Statement)

    @CachedProperty
    def operands(self):
        return self._fetch_table('operands', Operand)

    @CachedProperty
    def call_contexts(self):
        return self._fetch_table('call_contexts', CallContext)

    @CachedProperty
    def memory_locations(self):
        return self._fetch_table('memory_locations', MemoryLocation)

    def _fetch_table(self, table, klass):
        c = self.con.cursor()
        c.execute('SELECT * FROM %s ORDER BY id' % table)
        return [klass(row, self) for row in c]


class File(object):
    ''' Represents a source file '''

    __slots__ = ('id', 'path')

    def __init__(self, row, db):
        self.id = row[FilesTable.ID]
        self.path = row[FilesTable.PATH]


class Function(object):
    ''' Represents a function '''

    __slots__ = (
        'id',
        'name',
        'demangled',
        'definition',
        'file_id',
        'line',
        'db'
    )

    def __init__(self, row, db):
        self.id = row[FunctionsTable.ID]
        self.name = row[FunctionsTable.NAME]
        self.demangled = row[FunctionsTable.DEMANGLED]  # or None
        self.definition = (row[FunctionsTable.DEFINITION] == 1)
        self.file_id = row[FunctionsTable.FILE_ID]  # or None
        self.line = row[FunctionsTable.LINE]  # or None
        self.db = db

    def pretty_name(self):
        ''' Return a pretty name '''
        if self.demangled:
            return self.demangled
        else:
            return self.name

    def file(self):
        ''' Return the source file, or None '''
        if self.file_id is None:
            return None

        return self.db.files[self.file_id]

    def file_path(self):
        ''' Return the source file path, or None '''
        if self.file_id is None:
            return None

        return self.db.files[self.file_id].path


class Statement(object):
    ''' Represents a statement '''

    __slots__ = (
        'id',
        'kind',
        'function_id',
        'file_id',
        'line',
        'column',
        'db'
    )

    def __init__(self, row, db):
        self.id = row[StatementsTable.ID]
        self.kind = row[StatementsTable.KIND]
        self.function_id = row[StatementsTable.FUNCTION_ID]
        self.file_id = row[StatementsTable.FILE_ID]  # or None
        self.line = row[StatementsTable.LINE]  # or None
        self.column = row[StatementsTable.COLUMN]  # or None
        self.db = db

    def function(self):
        ''' Return the function '''
        return self.db.functions[self.function_id]

    def file(self):
        ''' Return the source file, or None '''
        if self.file_id is None:
            return None

        return self.db.files[self.file_id]

    def file_path(self):
        ''' Return the source file path, or None '''
        if self.file_id is None:
            return None

        return self.db.files[self.file_id].path

    def file_id_or(self, default):
        ''' Return the file id, or default '''
        if self.file_id is None:
            return default

        return self.file_id

    def line_or(self, default):
        ''' Return the line number, or default '''
        if self.line is None:
            return default

        return self.line

    def column_or(self, default):
        ''' Return the column number, or None '''
        if self.column is None:
            return default

        return self.column


class Operand(object):
    ''' Represents an operand '''

    __slots__ = ('id', 'kind', 'repr')

    def __init__(self, row, db):
        self.id = row[OperandsTable.ID]
        self.kind = row[OperandsTable.KIND]
        self.repr = row[OperandsTable.REPR]


# Represents a pair (operand number, operand)
NumOperandPair = collections.namedtuple('NumOperandPair', ('num', 'operand'))


class CallContext(object):
    ''' Represents a calling context '''

    __slots__ = ('id', 'call_id', 'function_id', 'parent_id', 'db')

    def __init__(self, row, db):
        self.id = row[CallContextsTable.ID]
        self.call_id = row[CallContextsTable.CALL_ID]  # or None
        self.function_id = row[CallContextsTable.FUNCTION_ID]  # or None
        self.parent_id = row[CallContextsTable.PARENT_ID]  # or None
        self.db = db

    def empty(self):
        return self.call_id is None

    def call(self):
        ''' Return the call statement '''
        assert self.call_id is not None
        return self.db.statements[self.call_id]

    def function(self):
        ''' Return the function '''
        assert self.function_id is not None
        return self.db.functions[self.function_id]

    def parent(self):
        ''' Return the parent calling context '''
        assert self.parent_id is not None
        return self.db.call_contexts[self.parent_id]

    def str(self):
        ''' Format a calling context '''
        call_context = self
        ctx = []

        while not call_context.empty():
            function = call_context.function()
            call = call_context.call()

            ctx.append('%s@%s:%s:%d' % (function.pretty_name(),
                                        call.line_or('?'),
                                        call.column_or('?'),
                                        call_context.call_id))

            call_context = call_context.parent()

        ctx.append('.')
        ctx.reverse()
        return '/'.join(ctx)

    def __str__(self):
        ''' Format a calling context '''
        return self.str()


class MemoryLocation(object):
    ''' Represents a memory location '''

    __slots__ = ('id', 'kind', 'info', 'db')

    def __init__(self, row, db):
        self.id = row[MemoryLocationsTable.ID]
        self.kind = row[MemoryLocationsTable.KIND]
        self.info = row[MemoryLocationsTable.INFO]  # or None
        self.db = db

    def load_info(self):
        ''' Return the info, or None '''
        if not self.info:
            return None

        return json.loads(self.info)


class Check(object):
    ''' Represents a check '''

    __slots__ = (
        'id',
        'kind',
        'checker',
        'status',
        'statement_id',
        'call_context_id',
        'operands',
        'info',
        'db'
    )

    def __init__(self, row, db):
        self.id = row[ChecksTable.ID]
        self.kind = row[ChecksTable.KIND]
        self.checker = row[ChecksTable.CHECKER]
        self.status = row[ChecksTable.STATUS]
        self.statement_id = row[ChecksTable.STATEMENT_ID]
        self.call_context_id = row[ChecksTable.CALL_CONTEXT_ID]
        self.operands = row[ChecksTable.OPERANDS]  # or None
        self.info = row[ChecksTable.INFO]  # or None
        self.db = db

    def statement(self):
        ''' Return the statement '''
        return self.db.statements[self.statement_id]

    def call_context(self):
        ''' Return the call context '''
        return self.db.call_contexts[self.call_context_id]

    def load_operands(self):
        ''' Return the operands, or None '''
        if not self.operands:
            return None

        operands = json.loads(self.operands)
        return [NumOperandPair(num, self.db.operands[id])
                for num, id in operands]
