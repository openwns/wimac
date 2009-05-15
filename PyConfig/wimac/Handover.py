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

"""Handover implementation


"""

from openwns.pyconfig import Frozen
from openwns.pyconfig import Sealed
from openwns.pyconfig import attrsetter


class HandoverBS(Sealed):
    """This class

        self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.controlplane.HandoverBS'
    name = "HandoverBS"
    connectionManager = None
    connectionClassifier = None
    newFrameProvider = None

    timerWaitingForIND = 5
    mob_bsho_rspPDUSize = 384

    def __init__(self, _connectionManager, _connectionClassifier, _newFrameProvider, **kw):
        self.connectionManager = _connectionManager
        self.connectionClassifier = _connectionClassifier
        self.newFrameProvider = _newFrameProvider
        attrsetter(self, kw)



class HandoverSS(Sealed):
    """This class

       self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.controlplane.HandoverSS'
    name = "HandoverSS"
    connectionClassifier = None
    newFrameProvider = None
    connectionManager = None
    pduWatchProvider = None

    timerWaitingForRSP = 5
    mob_msho_reqPDUSize = 320
    mob_ho_indPDUSize = 296

    def __init__(self, _connectionManager, _connectionClassifier, _newFrameProvider, _pduWatchProvider, **kw):
        self.connectionManager = _connectionManager
        self.connectionClassifier = _connectionClassifier
        self.newFrameProvider = _newFrameProvider
        self.pduWatchProvider = _pduWatchProvider
        attrsetter(self, kw)
