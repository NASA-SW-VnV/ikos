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
import argparse
import codecs
import collections
import itertools
import json
import os
import os.path
import random
import re
import subprocess
import sys
import tempfile
import threading

from ikos import analyzer
from ikos import args
from ikos import colors
from ikos import http
from ikos import log
from ikos import settings
from ikos.analyzer import command_string
from ikos.filetype import filetype
from ikos.log import printf


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
            '-': (0, self._set_standard_in),
            '-o': (1, self._set_output_file),
            '-c': (0, self._set_compile_only),
            '-E': (0, self._set_preprocess_only),
            '-S': (0, self._set_assemble_only),
            '--version': (0, self._set_version_only),
            '-emit-llvm': (0, self._set_emit_llvm),
            '-v': (0, self._set_verbose),
            '--verbose': (0, self._set_verbose),
            # Warnings
            '-w': (0, self._add_compile_link_unary),
            '-W': (0, self._add_compile_link_unary),
            # Preprocessor assertions
            '-A': (1, self._add_compile_binary),
            '-D': (1, self._add_compile_binary),
            '-U': (1, self._add_compile_binary),
            # Dependency generator
            '-M': (0, self._add_dependency_unary),
            '-MM': (0, self._add_dependency_unary),
            '-MF': (1, self._add_dependency_binary),
            '-MG': (0, self._add_dependency_unary),
            '-MP': (0, self._add_dependency_unary),
            '-MT': (1, self._add_dependency_binary),
            '-MQ': (1, self._add_dependency_binary),
            '-MD': (0, self._add_dependency_unary),
            '-MMD': (0, self._add_dependency_unary),
            # Include
            '-I': (1, self._add_compile_binary),
            '-idirafter': (1, self._add_compile_binary),
            '-include': (1, self._add_compile_binary),
            '-imacros': (1, self._add_compile_binary),
            '-iprefix': (1, self._add_compile_binary),
            '-iwithprefix': (1, self._add_compile_binary),
            '-iwithprefixbefore': (1, self._add_compile_binary),
            '-isystem': (1, self._add_compile_binary),
            '-isysroot': (1, self._add_compile_binary),
            '-iquote': (1, self._add_compile_binary),
            '-imultilib': (1, self._add_compile_binary),
            # Linker
            '-l': (1, self._add_link_binary),
            '-L': (1, self._add_link_binary),
            '-T': (1, self._add_link_binary),
            '-u': (1, self._add_link_binary),
            # Language
            '-ansi': (0, self._add_compile_unary),
            '-pedantic': (0, self._add_compile_unary),
            '-x': (1, self._add_compile_binary),
            # Ignore debug flags
            '-g': (0, self._add_compile_unary),
            '-g0': (0, self._add_compile_unary),
            '-g1': (0, self._add_compile_unary),
            '-ggdb': (0, self._add_compile_unary),
            '-ggdb3': (0, self._add_compile_unary),
            '-gdwarf-2': (0, self._add_compile_unary),
            '-gdwarf-3': (0, self._add_compile_unary),
            '-gline-tables-only': (0, self._add_compile_unary),
            '-p': (0, self._add_compile_unary),
            '-pg': (0, self._add_compile_unary),
            # Ignore optimization flags
            '-O': (0, self._add_compile_unary),
            '-O0': (0, self._add_compile_unary),
            '-O1': (0, self._add_compile_unary),
            '-O2': (0, self._add_compile_unary),
            '-O3': (0, self._add_compile_unary),
            '-Os': (0, self._add_compile_unary),
            '-Ofast': (0, self._add_compile_unary),
            '-Og': (0, self._add_compile_unary),
            '-Oz': (0, self._add_compile_unary),
            # Ignore code coverage instrumentation
            '-fprofile-arcs': (0, self._add_compile_link_unary),
            '-coverage': (0, self._add_compile_link_unary),
            '--coverage': (0, self._add_compile_link_unary),
            # Component-specifiers
            '-Xclang': (1, self._add_compile_binary),
            '-Xpreprocessor': (1, self._ignore),
            '-Xassembler': (1, self._ignore),
            '-Xlinker': (1, self._ignore),
            # Link options
            '-e': (1, self._add_link_binary),
            '-rpath': (1, self._add_link_binary),
            '-shared': (0, self._add_link_unary),
            '-static': (0, self._add_link_unary),
            '-pie': (0, self._add_link_unary),
            '-nostdlib': (0, self._add_link_unary),
            '-nodefaultlibs': (0, self._add_link_unary),
            '-rdynamic': (0, self._add_link_unary),
            '-dynamiclib': (0, self._add_link_unary),
            '-current_version': (1, self._add_link_binary),
            '-compatibility_version': (1, self._add_link_binary),
            '-install_name': (1, self._add_link_binary),
            # Misc.
            '-arch': (1, self._add_compile_binary),
            '/dev/null': (0, self._add_source_file),
            '-pipe': (0, self._add_compile_unary),
            '-undef': (0, self._add_compile_unary),
            '-nostdinc': (0, self._add_compile_unary),
            '-nostdinc++': (0, self._add_compile_unary),
            '-nostdlibinc': (0, self._add_compile_unary),
            '-Qunused-arguments': (0, self._add_compile_unary),
            '-no-integrated-as': (0, self._add_compile_unary),
            '-integrated-as': (0, self._add_compile_unary),
            '-no-canonical-prefixes': (0, self._add_compile_link_unary),
            '-pthread': (0, self._add_compile_unary),
            '--param': (1, self._ignore),
            '-aux-info': (1, self._ignore),
            '-no-cpp-precomp': (0, self._add_compile_unary),
            '-mno-omit-leaf-frame-pointer': (0, self._add_compile_unary),
            '-maes': (0, self._add_compile_unary),
            '-mno-aes': (0, self._add_compile_unary),
            '-mavx': (0, self._add_compile_unary),
            '-mno-avx': (0, self._add_compile_unary),
            '-mcmodel=kernel': (0, self._add_compile_unary),
            '-mno-red-zone': (0, self._add_compile_unary),
            '-mmmx': (0, self._add_compile_unary),
            '-mno-mmx': (0, self._add_compile_unary),
            '-msse': (0, self._add_compile_unary),
            '-mno-sse': (0, self._add_compile_unary),
            '-msse2': (0, self._add_compile_unary),
            '-mno-sse2': (0, self._add_compile_unary),
            '-msse3': (0, self._add_compile_unary),
            '-mno-sse3': (0, self._add_compile_unary),
            '-mssse3': (0, self._add_compile_unary),
            '-mno-ssse3': (0, self._add_compile_unary),
            '-msse4': (0, self._add_compile_unary),
            '-mno-sse4': (0, self._add_compile_unary),
            '-msse4.1': (0, self._add_compile_unary),
            '-mno-sse4.1': (0, self._add_compile_unary),
            '-msse4.2': (0, self._add_compile_unary),
            '-mno-sse4.2': (0, self._add_compile_unary),
            '-msoft-float': (0, self._add_compile_unary),
            '-m3dnow': (0, self._add_compile_unary),
            '-mno-3dnow': (0, self._add_compile_unary),
            '-m16': (0, self._add_compile_unary),
            '-m32': (0, self._add_compile_unary),
            '-m64': (0, self._add_compile_unary),
            '-mx32': (0, self._add_compile_unary),
            '-miamcu': (0, self._add_compile_unary),
            '-mstackrealign': (0, self._add_compile_unary),
            '-mretpoline-external-thunk': (0, self._add_compile_unary),
            '-mno-fp-ret-in-387': (0, self._add_compile_unary),
            '-mskip-rax-setup': (0, self._add_compile_unary),
            '-mindirect-branch-register': (0, self._add_compile_unary),
            '-print-multi-directory': (0, self._add_compile_unary),
            '-print-multi-lib': (0, self._add_compile_unary),
            '-print-libgcc-file-name': (0, self._add_compile_unary),
            '-print-search-dirs': (0, self._add_compile_unary),
            '-mno-80387': (0, self._add_compile_unary),
            '-mno-global-merge': (0, self._add_compile_unary),
            '-Wl,-dead_strip': (0, self._warning_link_unary),
            '-dead_strip': (0, self._warning_link_unary),
        }

        pattern_arguments = {
            r'^.+\.(c|cc|cpp|C|cxx|i|s|S|bc)$': (0, self._add_source_file),
            r'^.+\.([fF](|[0-9]{2}|or|OR|pp|PP))$': (0, self._add_source_file),
            r'^.+\.(o|lo|So|so|po|a|dylib)$': (0, self._add_object_file),
            r'^.+\.dylib(\.\d)+$': (0, self._add_object_file),
            r'^.+\.(So|so)(\.\d)+$': (0, self._add_object_file),
            r'^-(l|L).+$': (0, self._add_link_unary),
            r'^-I.+$': (0, self._add_compile_unary),
            r'^-D.+$': (0, self._add_compile_unary),
            r'^-B.+$': (0, self._add_compile_link_unary),
            r'^-isystem.+$': (0, self._add_compile_link_unary),
            r'^-U.+$': (0, self._add_compile_unary),
            r'^-Wl,.+$': (0, self._add_link_unary),
            r'^-W[^l].+$': (0, self._add_compile_unary),
            r'^-Wl[^,].+$': (0, self._add_compile_unary),
            r'^-fsanitize=.+$': (0, self._add_compile_link_unary),
            r'^-f.+$': (0, self._add_compile_unary),
            r'^-rtlib=.+$': (0, self._add_link_unary),
            r'^-std=.+$': (0, self._add_compile_unary),
            r'^-stdlib=.+$': (0, self._add_compile_link_unary),
            r'^-mtune=.+$': (0, self._add_compile_unary),
            r'^-mstack-alignment=.+$': (0, self._add_compile_unary),
            r'^-mcmodel=.+$': (0, self._add_compile_unary),
            r'^-mpreferred-stack-boundary=.+$': (0, self._add_compile_unary),
            r'^-mindirect-branch=.+$': (0, self._add_compile_unary),
            r'^-mregparm=.+$': (0, self._add_compile_unary),
            r'^-march=.+$': (0, self._add_compile_unary),
            r'^--param=.+$': (0, self._add_compile_unary),
            r'-mmacosx-version-min=.+$': (0, self._add_compile_unary),
            r'^--sysroot=.+$': (0, self._add_compile_unary),
            r'^-print-prog-name=.*$': (0, self._add_compile_unary),
            r'^-print-file-name=.*$': (0, self._add_compile_unary),
            r'^-x.+$': (0, self._add_compile_unary),
        }

        self.args = collections.deque(cmd)

        while self.args:
            if (self.is_standard_in or
                    self.is_preprocess or
                    self.is_assemble or
                    self.is_assembly or
                    self.is_version or
                    self.is_emit_llvm):
                break  # no need to emit llvm bitcode

            # next argument
            arg = self.args.popleft()

            # check if it's an exact argument
            if arg in exact_arguments:
                (arity, handler) = exact_arguments[arg]
                params = self._shift(arity)
                handler(arg, *params)
            else:
                matched = False

                # check if it matches a known pattern
                for pattern, (arity, handler) in pattern_arguments.items():
                    if re.match(pattern, arg):
                        params = self._shift(arity)
                        handler(arg, *params)
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

    def _warning_link_unary(self, flag):
        log.warning("Flag '%s' cannot be used with ikos-scan, ignored." % flag)

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


def run(cmd):
    ''' Run the given command and return the exit code '''
    log.debug('Running %s' % command_string(cmd))

    try:
        proc = subprocess.Popen(cmd)
        rc = proc.wait()
    except OSError as e:
        printf('error: %s: %s\n', cmd[0], e.strerror, file=sys.stderr)
        sys.exit(e.errno)

    if rc != 0:
        sys.exit(rc)

    return rc


def check_output(cmd):
    ''' Run the given command and return the standard output, in bytes '''
    log.debug('Running %s' % command_string(cmd))

    try:
        return subprocess.check_output(cmd)
    except OSError as e:
        printf('error: %s: %s\n', cmd[0], e.strerror, file=sys.stderr)
        sys.exit(e.errno)


def compiler(mode):
    ''' Return the full path to the compiler for the given mode '''
    if mode == 'cc':
        return settings.clang()
    elif mode == 'c++':
        return settings.clangxx()
    else:
        assert False, 'unexpected mode'


def build_bitcode(mode, parser, src_path, bc_path):
    ''' Compile the given source file to llvm bitcode '''
    cmd = [compiler(mode)]
    cmd += analyzer.clang_emit_llvm_flags()
    cmd += parser.compile_args
    cmd += analyzer.clang_ikos_flags()
    cmd += [src_path,
            '-o',
            bc_path]
    run(cmd)


def link_bitcodes(input_paths, output_path):
    ''' Link the given bitcode files to a single llvm bitcode '''
    cmd = [settings.llvm_link()]
    cmd += input_paths
    cmd += ['-o', output_path]
    run(cmd)


def build_object(mode, parser, src_path, obj_path):
    ''' Compile the given source file to an object file '''
    cmd = [compiler(mode), '-c']
    cmd += parser.compile_args
    cmd += [src_path, '-o', obj_path]
    run(cmd)


def link_objects(mode, parser, input_paths, output_path):
    cmd = [compiler(mode)]
    cmd += input_paths
    cmd += parser.link_args
    cmd += ['-o', output_path]
    run(cmd)


DARWIN_SEGMENT_NAME = '__WLLVM'
DARWIN_SECTION_NAME = '__llvm_bc'
ELF_SECTION_NAME = '.llvm_bc'
PE_SECTION_NAME = '.llvm_bc'


def attach_bitcode_path(obj_path, bc_path):
    ''' Attach the bitcode full path to the given object file '''
    ext = os.path.splitext(obj_path)[1]

    if ext not in ('.o', '.lo', '.os', '.So', '.po'):
        return  # unexpected file format

    if not os.path.exists(obj_path):
        log.warning(
            "Cannot attach bitcode path to missing file '%s'" % obj_path
        )
        return

    abs_bc_path = os.path.abspath(bc_path)

    # write the absolute path to the bitcode file in a temporary file
    f = tempfile.NamedTemporaryFile(mode='w+b',
                                    suffix='.llvm_bc',
                                    delete=False)
    f.write(abs_bc_path.encode('utf-8'))
    f.write(b'\n')
    f.flush()
    os.fsync(f.fileno())
    f.close()

    # add a section in the object file
    if sys.platform.startswith('darwin'):
        # TODO(marthaud): use llvm-objcopy when they start supporting Mach-O
        cmd = ['ld',
               '-r',
               '-keep_private_externs',
               obj_path,
               '-sectcreate',
               DARWIN_SEGMENT_NAME,
               DARWIN_SECTION_NAME, f.name,
               '-o',
               obj_path]
        run(cmd)
    elif sys.platform.startswith('linux') or sys.platform.startswith('freebsd'):
        cmd = [settings.llvm_objcopy(),
               '--add-section',
               '%s=%s' % (ELF_SECTION_NAME, f.name),
               obj_path]
        run(cmd)
    elif sys.platform.startswith('win'):
        # TODO(marthaud): use llvm-objcopy when they start supporting COFF/PE
        cmd = ['objcopy',
               '--add-section',
               '%s=%s' % (PE_SECTION_NAME, f.name),
               obj_path]
        run(cmd)
    else:
        assert False, 'unsupported platform'

    os.remove(f.name)


def extract_bitcode(exe_path, bc_path):
    ''' Extract the llvm bitcode for the given executable file '''

    # first, extract the llvm bitcode paths
    cmd = [settings.llvm_objdump(), '-s']
    if sys.platform.startswith('darwin'):
        cmd.append('--section=%s' % DARWIN_SECTION_NAME)
    elif sys.platform.startswith('linux') or sys.platform.startswith('freebsd'):
        cmd.append('--section=%s' % ELF_SECTION_NAME)
    elif sys.platform.startswith('win'):
        cmd.append('--section=%s' % PE_SECTION_NAME)
    else:
        assert False, 'unsupported platform'
    cmd.append(exe_path)

    output = check_output(cmd)
    section_content = b''

    # The output of llvm-objdump is prefixed by three lines: an empty one, one
    # that specifies the format, and one that describes what comes next (the
    # contents of the section requested). After that, lines have an address,
    # the hex representation (36 characters plus spaces), and the ASCII
    # representation, with some spaces in between sections or columns. The
    # following obtains only the hex code, ignoring the ASCII and the
    # addresses.
    for line in itertools.islice(output.splitlines(), 3, None):
        n = line.find(b' ', 1)
        line = line[n + 1:n + 36]
        for item in line.split(b' '):
            section_content += codecs.decode(item, 'hex')

    section_content = section_content.rstrip(b'\x00')
    bc_paths = [path.strip(b'\x00').decode('utf-8')
                for path in section_content.splitlines()]
    link_bitcodes(bc_paths, bc_path)


def notify_binary_built(exe_path, bc_path):
    ''' Notify the scan server that a binary was built '''
    abs_bc_path = os.path.abspath(bc_path)
    abs_exe_path = os.path.abspath(exe_path)

    if 'IKOS_SCAN_NOTIFIER_FILES' in os.environ:
        bc_base_path, _ = os.path.splitext(abs_bc_path)
        indicator_path = bc_base_path + '.ikosbin'
        with open(indicator_path, 'w') as indicator_file:
            indicator_file.write(json.dumps({
                'exe': abs_exe_path,
                'bc': abs_bc_path,
            }))
    else:
        binary = {
            'exe': abs_exe_path,
            'bc': abs_bc_path,
        }
        data = http.urlencode(binary).encode('utf-8')
        http.urlopen(os.environ['IKOS_SCAN_SERVER'], data)


class ScanServerRequestHandler(http.BaseHTTPRequestHandler):
    def do_POST(self):
        # parse request
        length = int(self.headers['content-length'])
        data = http.parse_qs(self.rfile.read(length).decode('utf-8'))
        binary = {
            'exe_path': data['exe'][0],
            'bc_path': data['bc'][0],
        }
        self.server.binaries.append(binary)
        log.debug('Received %r' % binary)

        # send response
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b'OK\n')

    def log_message(self, fmt, *args):
        return  # disable logging


class ScanServer(threading.Thread):
    '''
    HTTP server that logs the output files of the compiler

    Note that the server is single threaded.
    '''

    def __init__(self):
        super(ScanServer, self).__init__()

        self.port = None
        while self.port is None:
            try:
                # try to start the http server on a random port
                self.port = random.randint(8000, 9000)
                self.httpd = http.HTTPServerIPv6(('', self.port),
                                                 ScanServerRequestHandler)
            except (OSError, IOError):
                self.port = None  # port already in use

        self.httpd.timeout = 0.1
        self.httpd.binaries = []  # list of built binaries
        self.running = False

    def run(self):
        self.running = True
        while self.running:
            self.httpd.handle_request()

    def cancel(self):
        self.running = False

    @property
    def binaries(self):
        return self.httpd.binaries


###########################################
# main for ikos-scan-cc and ikos-scan-c++ #
###########################################

def compile_main(mode, argv):
    progname = os.path.basename(argv[0])

    if not ('IKOS_SCAN_SERVER' in os.environ or 'IKOS_SCAN_NOTIFIER_FILES' in os.environ):
        printf('error: %s: missing environment variable IKOS_SCAN_SERVER or IKOS_SCAN_NOTIFIER_FILES.\n',
               progname, file=sys.stderr)
        sys.exit(1)

    # setup colors and logging
    colors.setup(os.environ.get('IKOS_SCAN_COLOR', args.default_color),
                 file=log.out)
    log.setup(os.environ.get('IKOS_SCAN_LOG_LEVEL', args.default_log_level))

    # first step, run the actual command
    run([compiler(mode)] + argv[1:])

    # second step, parse the command line and compile to llvm bitcode
    parser = ClangArgumentParser(argv[1:])

    if parser.skip_bitcode_gen():
        return

    try:
        if (len(parser.source_files) == 1 and
                len(parser.object_files) == 0 and
                not parser.is_link):
            # in this case, just compile to llvm bitcode and attach the llvm
            # bitcode path to the output object file
            src_path = parser.source_files[0]
            obj_path = parser.output_file
            bc_path = '%s.bc' % obj_path
            build_bitcode(mode, parser, src_path, bc_path)
            attach_bitcode_path(obj_path, bc_path)
            return

        # compile the source files one by one and attach the llvm bitcode path
        new_object_files = []
        for src_path in parser.source_files:
            # build the object file
            obj_path = '%s.o' % src_path
            build_object(mode, parser, src_path, obj_path)
            new_object_files.append(obj_path)

            # build the bitcode file
            if src_path.endswith('.bc'):
                bc_path = src_path
            else:
                bc_path = '%s.bc' % obj_path
                build_bitcode(mode, parser, src_path, bc_path)

            # attach the bitcode path to the object file, ready to be linked
            attach_bitcode_path(obj_path, bc_path)

        # re-link to merge the llvm bitcode paths section
        if new_object_files:
            if parser.is_link:
                link_objects(mode,
                             parser,
                             new_object_files + parser.object_files,
                             parser.output_file)
            else:
                log.warning('New object files but nothing to link')

        if parser.is_link and u'executable' in filetype(parser.output_file):
            bc_path = '%s.bc' % parser.output_file
            extract_bitcode(parser.output_file, bc_path)
            notify_binary_built(parser.output_file, bc_path)
    except IOError as error:
        # ./configure sometimes removes the file while we are still running
        if error.filename == parser.output_file:
            pass
        else:
            raise error


##############################
# main for ikos-scan-extract #
##############################

def extract_parse_arguments(argv):
    usage = '%(prog)s [options] file'
    description = 'Extract the llvm bitcode generated for a given file'
    formatter_class = argparse.RawTextHelpFormatter
    parser = argparse.ArgumentParser(usage=usage,
                                     description=description,
                                     formatter_class=formatter_class)

    # Positional arguments
    parser.add_argument('input',
                        metavar='file',
                        help='Input file')

    # Optional arguments
    parser.add_argument('-o',
                        dest='output',
                        metavar='<file>',
                        help='Output file')
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


def extract_main(argv):
    # parse arguments
    opt = extract_parse_arguments(argv[1:])

    # setup colors and logging
    colors.setup(opt.color, file=log.out)
    log.setup(opt.log_level)

    input_path = opt.input
    output_path = opt.output if opt.output else '%s.bc' % input_path
    extract_bitcode(input_path, output_path)


######################
# main for ikos-scan #
######################

def scan_parse_arguments(argv):
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


def scan_main(argv):
    # parse arguments
    opt = scan_parse_arguments(argv[1:])

    # setup colors and logging
    colors.setup(opt.color, file=log.out)
    log.setup(opt.log_level)

    # scan server
    server = ScanServer()
    server.daemon = True
    server.start()

    # setup environment variables
    os.environ['IKOS_SCAN_COLOR'] = 'yes' if colors.ENABLE else 'no'
    os.environ['IKOS_SCAN_LOG_LEVEL'] = opt.log_level
    os.environ['IKOS_SCAN_SERVER'] = 'http://localhost:%d' % server.port
    os.environ['PATH'] += os.path.pathsep + settings.BIN_DIR
    os.environ['CC'] = 'ikos-scan-cc'
    os.environ['CXX'] = 'ikos-scan-c++'
    os.environ['LD'] = 'ikos-scan-cc'

    # add -e to make commands, to avoid makefiles overriding CC/CXX/LD
    if os.path.basename(opt.args[0]) in ('make', 'gmake'):
        opt.args.insert(1, '-e')

    # run the build command
    rc = run(opt.args)

    # stop the scan server
    server.cancel()
    server.join()

    # skip binaries that have been removed
    binaries = [binary for binary in server.binaries
                if os.path.exists(binary['exe_path'])]

    if not binaries:
        printf('Nothing to analyze.\n')

    # analyze each binary
    for binary in binaries:
        exe_path = os.path.relpath(binary['exe_path'])
        bc_path = os.path.relpath(binary['bc_path'])

        printf('Analyze %s? [Y/n] ', colors.bold(exe_path))
        answer = sys.stdin.readline().strip().lower()

        if answer in ('', 'y', 'yes'):
            cmd = ['ikos', bc_path, '-o', '%s.db' % exe_path]
            log.info('Running %s' % colors.bold(command_string(cmd)))

            cmd = [sys.executable,
                   settings.ikos(),
                   bc_path,
                   '-o',
                   '%s.db' % exe_path,
                   '--color=%s' % opt.color,
                   '--log=%s' % opt.log_level]
            run(cmd)
