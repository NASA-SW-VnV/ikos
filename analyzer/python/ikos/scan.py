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
import collections
import os
import os.path
import re
import subprocess
import sys

from ikos import args
from ikos import colors
from ikos import log
from ikos import settings
from ikos.analyzer import command_string
from ikos.log import printf


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

    # remove leading '--'
    while opt.args and opt.args[0] == '--':
        opt.args.pop(0)

    if not opt.args:
        parser.error("too few arguments")

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


class ClangArgumentParser:
    ''' Parser for clang arguments '''

    def __init__(self, cmd):
        # inputs and outputs
        self.source_files = []
        self.object_files = []
        self._output_file = None

        # compile and link arguments
        self.compile_args = []
        self.link_args = []

        # properties
        self.is_standard_in = False  # -
        self.is_link = True  # false if -c
        self.is_preprocess = False  # -E
        self.is_assemble = False  # -S
        self.is_assembly = False  # .s or .S
        self.is_version = False  # --version
        self.is_emit_llvm = False  # -emit-llvm
        self.is_verbose = False  # --verbose
        self.is_dependency = False  # -M

        exact_arguments = {
            '-': (0, ClangArgumentParser._set_standard_in),
            '-o': (1, ClangArgumentParser._set_output_file),
            '-c': (0, ClangArgumentParser._set_compile_only),
            '-E': (0, ClangArgumentParser._set_preprocess_only),
            '-S': (0, ClangArgumentParser._set_assemble_only),
            '--version': (0, ClangArgumentParser._set_version_only),
            '-emit-llvm': (0, ClangArgumentParser._set_emit_llvm),
            '-v': (0, ClangArgumentParser._set_verbose),
            '--verbose': (0, ClangArgumentParser._set_verbose),
            # Preprocessor assertions
            '-A': (1, ClangArgumentParser._add_compile_binary),
            '-D': (1, ClangArgumentParser._add_compile_binary),
            '-U': (1, ClangArgumentParser._add_compile_binary),
            # Dependency generator
            '-M': (0, ClangArgumentParser._add_dependency_unary),
            '-MM': (0, ClangArgumentParser._add_dependency_unary),
            '-MF': (1, ClangArgumentParser._add_dependency_binary),
            '-MG': (0, ClangArgumentParser._add_dependency_unary),
            '-MP': (0, ClangArgumentParser._add_dependency_unary),
            '-MT': (1, ClangArgumentParser._add_dependency_binary),
            '-MQ': (1, ClangArgumentParser._add_dependency_binary),
            '-MD': (0, ClangArgumentParser._add_dependency_unary),
            '-MMD': (0, ClangArgumentParser._add_dependency_unary),
            # Include
            '-I': (1, ClangArgumentParser._add_compile_binary),
            '-idirafter': (1, ClangArgumentParser._add_compile_binary),
            '-include': (1, ClangArgumentParser._add_compile_binary),
            '-imacros': (1, ClangArgumentParser._add_compile_binary),
            '-iprefix': (1, ClangArgumentParser._add_compile_binary),
            '-iwithprefix': (1, ClangArgumentParser._add_compile_binary),
            '-iwithprefixbefore': (1, ClangArgumentParser._add_compile_binary),
            '-isystem': (1, ClangArgumentParser._add_compile_binary),
            '-isysroot': (1, ClangArgumentParser._add_compile_binary),
            '-iquote': (1, ClangArgumentParser._add_compile_binary),
            '-imultilib': (1, ClangArgumentParser._add_compile_binary),
            # Linker
            '-l': (1, ClangArgumentParser._add_link_binary),
            '-L': (1, ClangArgumentParser._add_link_binary),
            '-T': (1, ClangArgumentParser._add_link_binary),
            '-u': (1, ClangArgumentParser._add_link_binary),
            # Language
            '-ansi': (0, ClangArgumentParser._add_compile_unary),
            '-pedantic': (0, ClangArgumentParser._add_compile_unary),
            '-x': (1, ClangArgumentParser._add_compile_binary),
            # Ignore debug flags
            '-g': (0, ClangArgumentParser._ignore),
            '-g0': (0, ClangArgumentParser._ignore),
            '-ggdb': (0, ClangArgumentParser._ignore),
            '-ggdb3': (0, ClangArgumentParser._ignore),
            '-gdwarf-2': (0, ClangArgumentParser._ignore),
            '-gdwarf-3': (0, ClangArgumentParser._ignore),
            '-gline-tables-only': (0, ClangArgumentParser._ignore),
            '-p': (0, ClangArgumentParser._ignore),
            '-pg': (0, ClangArgumentParser._ignore),
            # Ignore optimization flags
            '-O': (0, ClangArgumentParser._ignore),
            '-O0': (0, ClangArgumentParser._ignore),
            '-O1': (0, ClangArgumentParser._ignore),
            '-O2': (0, ClangArgumentParser._ignore),
            '-O3': (0, ClangArgumentParser._ignore),
            '-Os': (0, ClangArgumentParser._ignore),
            '-Ofast': (0, ClangArgumentParser._ignore),
            '-Og': (0, ClangArgumentParser._ignore),
            '-Oz': (0, ClangArgumentParser._ignore),
            # Ignore code coverage instrumentation
            '-fprofile-arcs': (0, ClangArgumentParser._ignore),
            '-coverage': (0, ClangArgumentParser._ignore),
            '--coverage': (0, ClangArgumentParser._ignore),
            # Component-specifiers
            '-Xclang': (1, ClangArgumentParser._add_compile_binary),
            '-Xpreprocessor': (1, ClangArgumentParser._ignore),
            '-Xassembler': (1, ClangArgumentParser._ignore),
            '-Xlinker': (1, ClangArgumentParser._ignore),
            # Link options
            '-e': (1, ClangArgumentParser._add_link_binary),
            '-rpath': (1, ClangArgumentParser._add_link_binary),
            '-shared': (0, ClangArgumentParser._add_link_unary),
            '-static': (0, ClangArgumentParser._add_link_unary),
            '-pie': (0, ClangArgumentParser._add_link_unary),
            '-nostdlib': (0, ClangArgumentParser._add_link_unary),
            '-nodefaultlibs': (0, ClangArgumentParser._add_link_unary),
            '-rdynamic': (0, ClangArgumentParser._add_link_unary),
            '-dynamiclib': (0, ClangArgumentParser._add_link_unary),
            '-current_version': (1, ClangArgumentParser._add_link_binary),
            '-compatibility_version': (1, ClangArgumentParser._add_link_binary),
            # Misc.
            '/dev/null': (0, ClangArgumentParser._add_source_file),
            '-pipe': (0, ClangArgumentParser._add_compile_unary),
            '-undef': (0, ClangArgumentParser._add_compile_unary),
            '-nostdinc': (0, ClangArgumentParser._add_compile_unary),
            '-nostdinc++': (0, ClangArgumentParser._add_compile_unary),
            '-nostdlibinc': (0, ClangArgumentParser._add_compile_unary),
            '-Qunused-arguments': (0, ClangArgumentParser._add_compile_unary),
            '-no-integrated-as': (0, ClangArgumentParser._add_compile_unary),
            '-integrated-as': (0, ClangArgumentParser._add_compile_unary),
            '-pthread': (0, ClangArgumentParser._add_compile_unary),
            '-mno-omit-leaf-frame-pointer': (0, ClangArgumentParser._add_compile_unary),
            '-maes': (0, ClangArgumentParser._add_compile_unary),
            '-mno-aes': (0, ClangArgumentParser._add_compile_unary),
            '-mavx': (0, ClangArgumentParser._add_compile_unary),
            '-mno-avx': (0, ClangArgumentParser._add_compile_unary),
            '-mcmodel=kernel': (0, ClangArgumentParser._add_compile_unary),
            '-mno-red-zone': (0, ClangArgumentParser._add_compile_unary),
            '-mmmx': (0, ClangArgumentParser._add_compile_unary),
            '-mno-mmx': (0, ClangArgumentParser._add_compile_unary),
            '-msse': (0, ClangArgumentParser._add_compile_unary),
            '-mno-sse2': (0, ClangArgumentParser._add_compile_unary),
            '-msse2': (0, ClangArgumentParser._add_compile_unary),
            '-mno-sse3': (0, ClangArgumentParser._add_compile_unary),
            '-msse3': (0, ClangArgumentParser._add_compile_unary),
            '-mno-sse': (0, ClangArgumentParser._add_compile_unary),
            '-msoft-float': (0, ClangArgumentParser._add_compile_unary),
            '-m3dnow': (0, ClangArgumentParser._add_compile_unary),
            '-mno-3dnow': (0, ClangArgumentParser._add_compile_unary),
            '-m16': (0, ClangArgumentParser._add_compile_unary),
            '-m32': (0, ClangArgumentParser._add_compile_unary),
            '-mx32': (0, ClangArgumentParser._add_compile_unary),
            '-m64': (0, ClangArgumentParser._add_compile_unary),
            '-miamcu': (0, ClangArgumentParser._add_compile_unary),
            '-mstackrealign': (0, ClangArgumentParser._add_compile_unary),
            '-mretpoline-external-thunk': (0, ClangArgumentParser._add_compile_unary),
            '-mno-fp-ret-in-387': (0, ClangArgumentParser._add_compile_unary),
            '-mskip-rax-setup': (0, ClangArgumentParser._add_compile_unary),
            '-mindirect-branch-register': (0, ClangArgumentParser._add_compile_unary),
            '-print-multi-directory': (0, ClangArgumentParser._add_compile_unary),
            '-print-multi-lib': (0, ClangArgumentParser._add_compile_unary),
            '-print-libgcc-file-name': (0, ClangArgumentParser._add_compile_unary),
            '-mno-80387': (0, ClangArgumentParser._add_compile_unary),
            '-mno-global-merge': (0, ClangArgumentParser._add_compile_unary),
            '-Wl,-dead_strip': (0, ClangArgumentParser._darwin_warning_add_link_unary),
        }

        pattern_arguments = {
            r'^.+\.(c|cc|cpp|C|cxx|i|s|S|bc)$': (0, ClangArgumentParser._add_source_file),
            r'^.+\.([fF](|[0-9][0-9]|or|OR|pp|PP))$': (0, ClangArgumentParser._add_source_file),
            r'^.+\.(o|lo|So|so|po|a|dylib)$': (0, ClangArgumentParser._add_object_file),
            r'^.+\.dylib(\.\d)+$': (0, ClangArgumentParser._add_object_file),
            r'^.+\.(So|so)(\.\d)+$': (0, ClangArgumentParser._add_object_file),
            r'^-(l|L).+$': (0, ClangArgumentParser._add_link_unary),
            r'^-I.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-D.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-U.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-Wl,.+$': (0, ClangArgumentParser._add_link_unary),
            r'^-W(?!l,).*$': (0, ClangArgumentParser._add_compile_unary),
            r'^-f.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-rtlib=.+$': (0, ClangArgumentParser._add_link_unary),
            r'^-std=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-stdlib=.+$': (0, ClangArgumentParser._add_compile_link_unary),
            r'^-mtune=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-mstack-alignment=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-mcmodel=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-mpreferred-stack-boundary=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-mindirect-branch=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-mregparm=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-march=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^--param=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'-mmacosx-version-min=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^--sysroot=.+$': (0, ClangArgumentParser._add_compile_unary),
            r'^-print-prog-name=.*$': (0, ClangArgumentParser._add_compile_unary),
            r'^-print-file-name=.*$': (0, ClangArgumentParser._add_compile_unary),
            r'^-x.+$': (0, ClangArgumentParser._add_compile_unary),
        }

        self.args = collections.deque(cmd)

        while self.args:
            if (self.is_preprocess or
                    self.is_assemble or
                    self.is_assembly or
                    self.is_version):
                break  # no need to emit llvm bitcode

            # next argument
            arg = self.args.popleft()

            # check if it's an exact argument
            if arg in exact_arguments:
                (arity, handler) = exact_arguments[arg]
                params = self._shift(arity)
                handler(self, arg, *params)
            else:
                matched = False

                # check if it matches a known pattern
                for pattern, (arity, handler) in pattern_arguments.items():
                    if re.match(pattern, arg):
                        params = self._shift(arity)
                        handler(self, arg, *params)
                        matched = True
                        break

                if not matched:
                    # unknown argument, just keep it
                    self._add_compile_unary(arg)

    def _shift(self, nargs):
        if len(self.args) < nargs:
            printf('error: unexpected command line argument\n',
                   file=sys.stderr)
            sys.exit(1)

        ret = []
        for _ in range(nargs):
            ret.append(self.args.popleft())

        return ret

    def _set_standard_in(self, flag):
        self.is_standard_in = True

    def _set_output_file(self, flag, path):
        self._output_file = path

    def _add_source_file(self, path):
        self.source_files.append(path)

        if re.search('\\.(s|S)$', path):
            self.is_assembly = True

    def _add_object_file(self, path):
        self.object_files.append(path)

    def _set_compile_only(self, flag):
        self.is_link = False

    def _set_preprocess_only(self, flag):
        self.is_preprocess = True

    def _set_assemble_only(self, flag):
        self.is_assemble = True

    def _set_version_only(self, flag):
        self.is_version = True

    def _set_emit_llvm(self, flag):
        self.is_emit_llvm = True

    def _set_verbose(self, flag):
        self.is_verbose = True

    def _ignore(self, flag, *params):
        pass

    def _add_compile_unary(self, flag):
        self.compile_args.append(flag)

    def _add_compile_binary(self, flag, param):
        self.compile_args.append(flag)
        self.compile_args.append(param)

    def _add_dependency_unary(self, flag):
        self.is_dependency = True
        self.compile_args.append(flag)

    def _add_dependency_binary(self, flag, param):
        self.is_dependency = True
        self.compile_args.append(flag)
        self.compile_args.append(param)

    def _add_link_unary(self, flag):
        self.link_args.append(flag)

    def _add_link_binary(self, flag, param):
        self.link_args.append(flag)
        self.link_args.append(param)

    def _add_compile_link_unary(self, flag):
        self.compile_args.append(flag)
        self.link_args.append(flag)

    def _darwin_warning_add_link_unary(self, flag):
        if sys.platform.startswith('darwin'):
            printf('error: flag "%s" cannot be used with ikos-scan\n', flag,
                   file=sys.stderr)
            sys.exit(1)
        else:
            self.link_args.append(flag)

    @property
    def output_file(self):
        if self._output_file is not None:
            return self._output_file
        elif not self.is_link:
            # -c but no -o, guess the file name
            base = os.path.basename(self.source_files[0])
            return os.path.splitext(base)[0] + '.o'
        else:
            return 'a.out'

    def skip_bitcode_gen(self):
        ''' Return True if it is unnecessary to generate llvm bitcode '''
        if not self.source_files and not self.object_files:
            return True
        if self.is_standard_in:
            return True
        if self.is_preprocess:
            return True
        if self.is_assemble or self.is_assembly:
            return True
        if self.is_version:
            return True
        if self.is_emit_llvm:
            return True
        if self.is_dependency and self.is_link:
            return True
        return False


def build_bitcode(mode, parser, src_path, bc_path):
    ''' Compile the given source file to llvm bitcode '''
    cmd = [mode,
           '-c',
           '-emit-llvm']
    cmd += parser.compile_args
    cmd += ['-g',
            '-D_FORTIFY_SOURCE=0',
            '-O0',
            src_path,
            '-o', bc_path]

    log.debug('Running %s' % command_string(cmd))
    proc = subprocess.Popen(cmd, executable=settings.clang())
    rc = proc.wait()
    if rc != 0:
        sys.exit(rc)


def link_bitcodes(mode, parser, input_paths, output_path):
    ''' Link the given bitcode files to a single llvm bitcode '''
    cmd = ['llvm-link']
    cmd += input_paths
    cmd += ['-o', output_path]

    log.debug('Running %s' % command_string(cmd))
    proc = subprocess.Popen(cmd, executable=settings.llvm_link())
    rc = proc.wait()
    if rc != 0:
        sys.exit(rc)


###########################################
# main for ikos-scan-cc and ikos-scan-c++ #
###########################################

def compile(mode, argv):
    assert mode in ('clang', 'clang++')

    # remove 'ikos-scan-cc'
    argv = list(argv[1:])

    # setup colors and logging
    colors.setup(os.environ['IKOS_SCAN_COLOR'], file=log.out)
    log.setup(os.environ['IKOS_SCAN_LOG_LEVEL'])

    # run the command
    proc = subprocess.Popen([mode] + argv, executable=settings.clang())
    rc = proc.wait()
    if rc != 0:
        sys.exit(rc)

    # parse the command line
    parser = ClangArgumentParser(argv)

    if parser.skip_bitcode_gen():
        return

    # compile to llvm bitcode
    if len(parser.source_files) == 1 and len(parser.object_files) == 0:
        src_path = parser.source_files[0]
        bc_path = '%s.bc' % parser.output_file
        build_bitcode(mode, parser, src_path, bc_path)
    else:
        bitcode_files = []

        for src_path in parser.source_files:
            bc_path = '%s.bc' % src_path
            build_bitcode(mode, parser, src_path, bc_path)
            bitcode_files.append(bc_path)

        for obj_path in parser.object_files:
            bc_path = '%s.bc' % obj_path
            bitcode_files.append(bc_path)

        bc_path = '%s.bc' % parser.output_file
        link_bitcodes(mode, parser, bitcode_files, bc_path)


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
    os.environ['IKOS_SCAN_COLOR'] = 'yes' if colors.ENABLE else 'no'
    os.environ['IKOS_SCAN_LOG_LEVEL'] = opt.log_level
    os.environ['CC'] = settings.ikos_scan_cc()
    os.environ['CXX'] = settings.ikos_scan_cxx()

    # run the build command
    log.debug('Running %s' % command_string(opt.args))
    proc = subprocess.Popen(opt.args)
    rc = proc.wait()
    sys.exit(rc)
