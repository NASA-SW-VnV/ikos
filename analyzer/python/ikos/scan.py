###############################################################################
#
# IKOS scan
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018 United States Government as represented by the
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
import os
import os.path
import subprocess
import sys

from ikos import args
from ikos import colors
from ikos import log
from ikos import settings
from ikos.analyzer import command_string


def parse_arguments(argv):
    usage = '%(prog)s [options] command'
    description = 'python helper to analyze whole C/C++ projects'
    formatter_class = argparse.RawTextHelpFormatter
    parser = argparse.ArgumentParser(usage=usage,
                                     description=description,
                                     formatter_class=formatter_class)

    # Positional arguments
    parser.add_argument('args',
                        nargs=argparse.REMAINDER,
                        help=argparse.SUPPRESS)

    # Optional arguments
    parser.add_argument('-v',
                        dest='verbosity',
                        help='Increase verbosity',
                        action='count',
                        default=1)
    parser.add_argument('-q',
                        dest='verbosity',
                        help='Be quiet',
                        action='store_const',
                        const=0)
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
    parser.add_argument('--log',
                        dest='log_level',
                        metavar='',
                        help=args.help('Log level:',
                                       args.log_levels,
                                       args.default_log_level),
                        choices=args.choices(args.log_levels),
                        default=None)

    opt = parser.parse_args(argv)

    if not opt.args:
        parser.error("too few arguments")

    # remove leading '--'
    if opt.args[0] == '--':
        opt.args.pop(0)

    # verbosity changes the log level, if --log is not specified
    if opt.log_level is None:
        if opt.verbosity <= 0:
            opt.log_level = 'error'
        elif opt.verbosity == 1:
            opt.log_level = 'info'
        elif opt.verbosity == 2:
            opt.log_level = 'debug'
        else:
            opt.log_level = 'all'

    return opt


###########################################
# main for ikos-scan-cc and ikos-scan-c++ #
###########################################

def compile(mode, argv):
    # run the command
    proc = subprocess.Popen([mode] + argv[1:], executable=settings.clang())
    rc = proc.wait()
    sys.exit(rc)


######################
# main for ikos-scan #
######################

def main(argv):
    progname = os.path.basename(argv[0])

    # parse arguments
    opt = parse_arguments(argv[1:])

    # setup colors and logging
    colors.setup(opt.color, file=log.out)
    log.setup(opt.log_level)

    # setup environment variables
    os.environ['CC'] = settings.ikos_scan_cc()
    os.environ['CXX'] = settings.ikos_scan_cxx()

    # run the build command
    log.debug('Running %s' % command_string(opt.args))
    proc = subprocess.Popen(opt.args)
    rc = proc.wait()
    sys.exit(rc)
