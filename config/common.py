import os
import CNBuildSupport
from CNBuildSupport import CNBSEnvironment
import wnsbase.RCS as RCS

commonEnv = CNBSEnvironment(PROJNAME       = 'wimac',
                            PROJMODULES    = ['TEST', 'BASE'],
                            FLATINCLUDES   = False,
                            LIBRARY        = True,
                            SHORTCUTS      = True,
                            DEFAULTVERSION = True,
			    REVISIONCONTROL = RCS.Bazaar('../', 'WiMAC', 'main', '1.0'),
                            )
Return('commonEnv')
