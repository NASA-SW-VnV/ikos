################################################################################
#
# Demangle C++ symbols.
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2017 United States Government as represented by the
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
################################################################################
import atexit
import subprocess


# is c++filt available?
_cppfilt_available = None

# current c++filt process
_cppfilt_proc = None

# cache demangled symbols
_cache = {}


def _cppfilt_kill():
    ''' Kill the current c++filt process '''
    if not _cppfilt_proc or _cppfilt_proc.poll() is not None:
        return

    try:
        _cppfilt_proc.terminate()
        _cppfilt_proc.kill()
        _cppfilt_proc.wait()
    except OSError:
        pass


def is_mangled(symbol):
    return len(symbol) >= 2 and symbol[0] == '_' and symbol[1] >= 'A' and symbol[1] <= 'Z'


def demangle(symbol):
    '''
    Demangle a symbol name using c++filt if available,
    otherwise return the symbol name unchanged.
    '''
    global _cppfilt_available
    global _cppfilt_proc

    if not is_mangled(symbol):
        return symbol

    if _cppfilt_available is False:
        return symbol

    if symbol in _cache:
        return _cache[symbol]

    try:
        if not _cppfilt_proc:
            _cppfilt_proc = subprocess.Popen(['c++filt', '--no-strip-underscores', '--no-verbose'],
                                             stdin=subprocess.PIPE,
                                             stdout=subprocess.PIPE,
                                             stderr=subprocess.PIPE)
            _cppfilt_available = True
            atexit.register(_cppfilt_kill)

        if _cppfilt_proc.poll() is not None:
            _cppfilt_available = False
            return symbol

        _cppfilt_proc.stdin.write(symbol.encode('utf8') + b'\n')
        _cppfilt_proc.stdin.flush()
        stdout = _cppfilt_proc.stdout.readline()
        result = stdout.decode('utf8').strip()
    except OSError:
        _cppfilt_available = False
        result = symbol

    _cache[symbol] = result
    return result
