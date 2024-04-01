###############################################################################
#
# Highlight utility for ikos-view
#
# Author: Thomas Bailleux
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
import html

from ikos import log


class DummyLexer(object):
    def __init__(self, *args, **kwargs):
        pass


class DummyHtmlFormatter(object):
    def __init__(self):
        pass

    def get_style_defs(self, cl):
        return ''

    def wrap(self, src):
        pass


def dummy_highlight(data, lexer, formatter):
    lines = ((1, html.escape(line)) for line in data.split('\n'))
    code = ''
    for _, line in formatter.wrap(lines):
        code += line
    return code


try:
    from pygments.lexers import CLexer, CppLexer
    from pygments.formatters import HtmlFormatter
    from pygments import highlight
except ImportError:
    log.warning("Syntax highlighting is not available:"
                " module `pygments` is not installed.")
    log.warning("Note: use `pip install --user pygments`"
                " to install `pygments`")
    CLexer = DummyLexer
    CppLexer = DummyLexer
    HtmlFormatter = DummyHtmlFormatter
    highlight = dummy_highlight
