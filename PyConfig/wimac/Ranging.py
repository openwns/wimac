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

"""Ranging implementation


"""

from openwns.pyconfig import Frozen
from openwns.pyconfig import Sealed
from openwns.pyconfig import attrsetter


class RangingBS(Sealed):
    """This class

       self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.controlplane.RangingBS'
    name = "RangingBS"
    connectionManager = None
    connectionClassifier = None
    rng_rspPDUSize = 408

    def __init__(self, connectionManager, connectionClassifier, **kw):
	self.connectionManager = connectionManager
        self.connectionClassifier = connectionClassifier
        attrsetter(self, kw)



class RangingSS(Sealed):
    """This class

       self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.controlplane.RangingSS'
    name = "RangingSS"
    connectionManager = None
    connectionClassifier = None
    phyUser = None
    newFrameProvider = None
    rngCompoundCollector = None

    rng_reqPDUSize = 208

    numberOfRetries = 5
    boWindowSizeMin = 15
    boWindowSizeMax = 30
    timerWaitingForRSP =  boWindowSizeMax + 5

    def __init__(self, connectionManager, connectionClassifier, phyUser, newFrameProvider, rngCompoundCollector, **kw):
        self.connectionManager = connectionManager
        self.connectionClassifier = connectionClassifier
        self.phyUser = phyUser
        self.newFrameProvider = newFrameProvider
        self.rngCompoundCollector = rngCompoundCollector
        attrsetter(self, kw)
