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

"""Scanning implementation


"""

from openwns.pyconfig import Frozen
from openwns.pyconfig import Sealed
from openwns.pyconfig import attrsetter


class ScanningBS(Sealed):
    """This class

       self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.controlplane.ScanningBS'
    name = "ScanningBS"
    connectionManager = None
    connectionClassifier = None
    newFrameProvider = None
    pduWatchProvider = None
    mob_scn_rspPDUSize = 344

    def __init__(self, connectionManager, connectionClassifier, newFrameProvider, pduWatchProvider, **kw):
        self.connectionManager = connectionManager
        self.connectionClassifier = connectionClassifier
        self.newFrameProvider = newFrameProvider
        self.pduWatchProvider = pduWatchProvider
        attrsetter(self, kw)



class ScanningSS(Sealed):
    """This class

       self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.controlplane.ScanningSS'
    name = "ScanningSS"
    connectionManager = None
    connectionClassifier = None
    cirMeasureProvider = None
    newFrameProvider = None
    timerBetweenFrequencyChange = 2
    timerWaitingForRSP = 5
    mob_scn_reqPDUSize = 312

    def __init__(self, connectionManager, connectionClassifier,  cirMeasureProvider, newFrameProvider, **kw):
        self.connectionManager = connectionManager
        self.connectionClassifier = connectionClassifier
        self.cirMeasureProvider = cirMeasureProvider
        self.newFrameProvider = newFrameProvider
        attrsetter(self, kw)
