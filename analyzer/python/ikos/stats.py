###############################################################################
#
# Simple statistics module
#
# Author: Jorge A. Navas
#
# Contributors: Maxime Arthaud
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
import time


# use monotonic clock if available
if hasattr(time, 'monotonic'):
    _current_time = time.monotonic
else:
    _current_time = time.time


class Stopwatch:
    ''' A stop watch '''

    def __init__(self):
        self._started = 0
        self._finished = -1
        self._elapsed = 0
        self.start()

    @property
    def elapsed(self):
        ''' Returns time (in seconds) since the stopwatch has been started '''
        if self._finished < self._started:
            return self._elapsed + (_current_time() - self._started)
        else:
            return self._elapsed + (self._finished - self._started)

    def start(self):
        ''' Starts or resumes the stopwatch '''
        # collect elapsed time so far
        if self._finished >= self._started:
            self._elapsed += (self._finished - self._started)

        self._started = _current_time()
        self._finished = -1

    def stop(self):
        ''' Stops the stopwatch '''
        if self._finished < self._started:
            self._finished = _current_time()

    def reset(self):
        ''' Resets the stopwatch by erasing all elapsed time '''
        self._elapsed = 0
        self._finished = -1
        self.start()

    def __str__(self):
        ''' Reports time in seconds up to three decimal places '''
        return '{0:.3f}'.format(self.elapsed)


_statistics = dict()


def get(key):
    ''' Gets a value from statistics table '''
    return _statistics.get(key)


def put(key, v):
    ''' Puts a value in statistics table '''
    _statistics[key] = v


def start(key):
    ''' Starts (or resumes) a named stopwatch '''
    sw = get(key)
    if sw is None:
        sw = Stopwatch()
        put(key, sw)
        return sw
    else:
        sw.start()


def stop(key):
    ''' Stops a named stopwatch '''
    sw = get(key)
    if sw is not None:
        sw.stop()


def rows():
    ''' Return all statistics as rows '''
    return [(name, sw.elapsed) for name, sw in _statistics.items()]


def timer(key):
    '''
    ContextManager to help measuring time.

    with timer('myname') as t:
        do_code_that_is_timed
    '''

    class TimerContextManager(object):
        def __init__(self, key):
            self._key = key

        def __enter__(self):
            start(self._key)
            return None

        def __exit__(self, exc_type, exc_value, traceback):
            stop(self._key)
            return False

    return TimerContextManager(key)
