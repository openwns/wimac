###############################################################################
# This file is part of openWNS (open Wireless Network Simulator)
# _____________________________________________________________________________
#
# Copyright (C) 2004-2009
# Chair of Communication Networks (ComNets)
# Kopernikusstr. 5, D-52074 Aachen, Germany
# phone: ++49-241-80-27910,
# fax: ++49-241-80-22242
# email: info@openwns.org
# www: http://www.openwns.org
# _____________________________________________________________________________
#
# openWNS is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License version 2 as published by the
# Free Software Foundation;
#
# openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

"""ScanningStrategy


"""

from openwns.pyconfig import Sealed
from openwns.pyconfig import attrsetter

class ScanningStrategy(Sealed):
    __plugin__ = None

    scanningProvider = None
    newFrameProvider = None

    retries = 3
    framesBetweenScanning = 3   ## should be 200

    def __init__(self, _scanningProvider, _newFrameProvider , **kw):
        self.scanningProvider = _scanningProvider
        self.newFrameProvider = _newFrameProvider
        attrsetter(self, kw)

class Plain(ScanningStrategy):
     __plugin__ = 'wimac.services.scanningStrategy.Plain'


     def __init__(self, _scanningProvider, _newFrameProvider, **kw):
         super(Plain, self).__init__(_scanningProvider, _newFrameProvider, **kw)
         attrsetter(self, kw)


class Interupted(ScanningStrategy):
     __plugin__ = 'wimac.services.scanningStrategy.Interupted'

     framesBetweenSubScanning = 2 ##should be 30
     maxNumberOfStationsToScan = 4

     def __init__(self, _scanningProvider, _newFrameProvider, **kw):
         super(Interupted, self).__init__(_scanningProvider, _newFrameProvider, **kw)
         attrsetter(self, kw)
