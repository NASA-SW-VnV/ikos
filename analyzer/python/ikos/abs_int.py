###############################################################################
#
# Abstract interpretation utilities for python
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018-2019 United States Government as represented by the
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


class Signedness:
    SIGNED = 0
    UNSIGNED = 1


def assert_compatible(a, b):
    ''' Check that the given parameters have the same bit-width and sign '''
    assert a.bit_width == b.bit_width and a.sign == b.sign


class MachineInt:
    ''' Represents a machine integer '''

    __slots__ = ('n', 'bit_width', 'sign')

    def __init__(self, n, bit_width, sign):
        self.n = n
        self.bit_width = bit_width
        self.sign = sign
        self._normalize()

    def _normalize(self):
        if self.is_signed():
            # n needs to be within [-2**(n-1), 2**(n-1)-1]
            self.n = self.n + 2**(self.bit_width - 1)
            self.n = self.n % 2**self.bit_width
            self.n = self.n - 2**(self.bit_width - 1)
        else:
            # n needs to be within [0, 2**n-1]
            self.n = self.n % 2**self.bit_width

    def is_signed(self):
        return self.sign == Signedness.SIGNED

    def is_unsigned(self):
        return self.sign == Signedness.UNSIGNED

    @staticmethod
    def min(bit_width, sign):
        if sign == Signedness.SIGNED:
            return MachineInt(-2**(bit_width - 1), bit_width, sign)
        else:
            return MachineInt(0, bit_width, sign)

    def is_min(self):
        if self.is_signed():
            return self.n == -2**(self.bit_width - 1)
        else:
            return self.n == 0

    @staticmethod
    def max(bit_width, sign):
        if sign == Signedness.SIGNED:
            return MachineInt(2**(bit_width - 1) - 1, bit_width, sign)
        else:
            return MachineInt(2**bit_width - 1, bit_width, sign)

    def is_max(self):
        if self.is_signed():
            return self.n == 2**(self.bit_width - 1) - 1
        else:
            return self.n == 2**self.bit_width - 1

    def high_bit(self):
        if self.is_signed():
            return self.n < 0
        else:
            return self.n >= 2**(self.bit_width - 1)

    def sign_cast(self, sign):
        return MachineInt(self.n, self.bit_width, sign)

    def __eq__(self, other):
        assert_compatible(self, other)
        return self.n == other.n

    def __lt__(self, other):
        assert_compatible(self, other)
        return self.n < other.n

    def __le__(self, other):
        assert_compatible(self, other)
        return self.n <= other.n

    def __gt__(self, other):
        assert_compatible(self, other)
        return self.n > other.n

    def __ge__(self, other):
        assert_compatible(self, other)
        return self.n >= other.n

    def __str__(self):
        return str(self.n)


class Interval:
    ''' Represents an interval [a, b], or bottom '''

    __slots__ = ('lb', 'ub')

    def __init__(self, lb, ub):
        assert_compatible(lb, ub)
        self.lb = lb
        self.ub = ub

    @staticmethod
    def top(bit_width, sign):
        return Interval(MachineInt.min(bit_width, sign),
                        MachineInt.max(bit_width, sign))

    @staticmethod
    def bottom(bit_width, sign):
        return Interval(MachineInt.max(bit_width, sign),
                        MachineInt.min(bit_width, sign))

    @staticmethod
    def from_dict(info):
        ''' Create an interval from a dictionary '''
        t = info['type']
        sign = Signedness.SIGNED if t[0] == 's' else Signedness.UNSIGNED
        bit_width = int(t[1:])
        return Interval(MachineInt(info['lb'], bit_width, sign),
                        MachineInt(info['ub'], bit_width, sign))

    @property
    def bit_width(self):
        return self.lb.bit_width

    @property
    def sign(self):
        return self.lb.sign

    def is_bottom(self):
        return self.lb > self.ub

    def is_top(self):
        return self.lb.is_min() and self.ub.is_max()

    def is_constant(self):
        return self.lb == self.ub

    def to_constraints(self, var):
        assert not self.is_bottom()

        if self.is_top():
            return 'could not bound %s' % var
        elif self.lb == self.ub:
            return '%s = %s' % (var, self.lb)
        elif self.lb.is_min():
            return '%s <= %s' % (var, self.ub)
        elif self.ub.is_max():
            return '%s >= %s' % (var, self.lb)
        else:
            return '%s <= %s <= %s' % (self.lb, var, self.ub)

    def sign_cast(self, sign):
        if self.sign == sign:
            return self
        elif self.is_bottom():
            return Interval.bottom(self.bit_width, sign)
        else:
            if self.lb.high_bit() == self.ub.high_bit():
                lb = self.lb.sign_cast(sign)
                ub = self.ub.sign_cast(sign)
                if lb <= ub:
                    return Interval(lb, ub)
                else:
                    return Interval(ub, lb)
            else:
                return Interval.top(self.bit_width, sign)

    def __str__(self):
        if self.is_bottom():
            return '⊥'
        elif self.is_top():
            return 'T'
        else:
            return '[%s, %s]' % (self.lb, self.ub)


class Congruence:
    ''' Represents a congruence aZ + b, or bottom '''

    __slots__ = ('a', 'b', 'bit_width', 'sign', 'bottom')

    def __init__(self, a, b, bit_width, sign, bottom):
        self.a = a
        self.b = b
        self.bit_width = bit_width
        self.sign = sign
        self.bottom = bottom
        self._normalize()

    def _normalize(self):
        if self.bottom:
            self.a = self.b = None
            return

        assert self.a >= 0

        if self.a != 0:
            self.b = self.b % self.a

    @staticmethod
    def from_dict(info):
        ''' Create a congruence from a dictionary '''
        t = info['type']
        sign = Signedness.SIGNED if t[0] == 's' else Signedness.UNSIGNED
        bit_width = int(t[1:])
        return Congruence(a=info['a'],
                          b=info['b'],
                          bit_width=bit_width,
                          sign=sign,
                          bottom=False)

    def is_bottom(self):
        return self.bottom

    def is_top(self):
        return not self.bottom and self.a == 1

    def is_constant(self):
        return not self.bottom and self.a == 0

    def __str__(self):
        if self.is_bottom():
            return '⊥'
        if self.a == 0:
            return str(self.b)
        if self.b == 0:
            return '%dZ' % self.a
        return '%dZ+%d' % (self.a, self.b)
