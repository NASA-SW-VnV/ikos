###############################################################################
#
# Logging utilities
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
import sys

from ikos import colors


# Logging levels
NONE = 60
CRITICAL = 50
ERROR = 40
WARNING = 30
INFO = 20
DEBUG = 10
ALL = 0

LEVELS = {
    'none': NONE,
    'critical': CRITICAL,
    'error': ERROR,
    'warning': WARNING,
    'info': INFO,
    'debug': DEBUG,
    'all': ALL,
}

# Current log level
LEVEL = WARNING

# output file
out = sys.stdout


def setup(level):
    global LEVEL

    if level in LEVELS:
        LEVEL = LEVELS[level]
    else:
        LEVEL = WARNING


def printf(fmt, *args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    file.write(fmt % args if args else fmt)
    file.flush()


def is_enabled_for(user_level):
    return LEVEL <= user_level


def critical(message):
    if is_enabled_for(CRITICAL):
        printf('[%s] %s\n', colors.on_red('CRITICAL'), message, file=out)


def error(message):
    if is_enabled_for(ERROR):
        printf('[%s] %s\n', colors.on_red('ERROR'), message, file=out)


def warning(message):
    if is_enabled_for(WARNING):
        printf('[%s] %s\n', colors.bold_yellow('!'), message, file=out)


def info(message):
    if is_enabled_for(INFO):
        printf('[%s] %s\n', colors.bold_blue('*'), message, file=out)


def debug(message):
    if is_enabled_for(DEBUG):
        printf('[%s] %s\n', colors.magenta('.'), message, file=out)
