# Copyright © 2012 CCP ehf.

"""
Wrapper module to access blue exposed things from EVE's Localization DLL.
"""

import blue
import sys

sys.modules[__name__] = blue.LoadExtension("_evelocalization")
