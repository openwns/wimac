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

"""DeadStationDetect implementation


"""
from openwns.pyconfig import Sealed

import dll.Services
from openwns.pyconfig import attrsetter


class DeadStationDetect(dll.Services.Service):
    """This class

       self.qosPriority: Signaling, UGS, rtPS, nrtPS, BE

    """
    __plugin__ = 'wimac.services.DeadStationDetect'
    nameInServiceFactory = __plugin__

    connectionManager = None
    newFrameProvider = None

    checkInterval = 50      ## [Frames]
    timeToLive = 0.5        ## [SimTime]

    def __init__(self, _serviceName, _connectionManager, _newFrameProvider, **kw):
        self.serviceName = _serviceName
        self.connectionManager = _connectionManager
        self.newFrameProvider = _newFrameProvider
        attrsetter(self, kw)

class DSDSensor(Sealed):
    __plugin__ = 'wimac.services.DSDSensor'

    deadStationDetect = None
    connectionClassifier = None

    def __init__(self, _deadStationDetect, _connectionClassifier, **kw):
        self.deadStationDetect = _deadStationDetect
        self.connectionClassifier = _connectionClassifier
        attrsetter(self, kw)
