###############################################################################
#
# Color utilities
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
import os
import sys


# are colors enable?
ENABLE = True

# available colors
COLORS = {
    'grey': 0,
    'red': 1,
    'green': 2,
    'yellow': 3,
    'blue': 4,
    'magenta': 5,
    'cyan': 6,
    'white': 7,
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


def setup(color, file=sys.stdout):
    global ENABLE

    if color in ('yes', 'on', 'always'):
        ENABLE = True
    elif color in ('no', 'off', 'never'):
        ENABLE = False
    elif color == 'auto':
        ENABLE = os.isatty(file.fileno())


def colorize(text, color=None, on_color=None, attrs=None):
    '''
    Colorize text.

    >>> colorize('Hello, World!', 'red', 'grey', ['bold', 'blink'])
    '\x1b[5m\x1b[1m\x1b[40m\x1b[31mHello, World!\x1b[0m'
    >>> colorize('Hello, World!', 'green')
    '\x1b[32mHello, World!\x1b[0m'
    '''
    if ENABLE:
        if color:
            text = '\033[%dm%s' % (30 + COLORS[color], text)

        if on_color:
            text = '\033[%dm%s' % (40 + COLORS[on_color], text)

        if attrs:
            for attr in attrs:
                text = '\033[%dm%s' % (ATTRIBUTES[attr], text)

        text += '\033[0m'

    return text


###########
# helpers #
###########


def bold(s):
    return colorize(s, attrs=['bold'])


for color in COLORS:
    globals()[color] = (lambda s, color=color: colorize(s, color))
    globals()['bold_%s' % color] = (lambda s, color=color:
                                    colorize(s, color, attrs=['bold']))
    globals()['on_%s' % color] = (lambda s, color=color:
                                  colorize(s, on_color=color))
