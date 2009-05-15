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

"""LogicHandover implementation


"""

import dll.Services
from openwns.pyconfig import attrsetter

import HandoverStrategy
import ScanningStrategy


class ControlPlaneManagerSS(dll.Services.Service):
    """This class

       self.qosPriority: Signaling, UGS, rtPS, nrtPS, BE

    """
    __plugin__ = 'wimac.services.ControlPlaneManagerSS'
    nameInServiceFactory = __plugin__
    retries = 3

    strategyHandover = None
    strategyBestStation = None

    scanningStrategyInitial = None
    scanningStrategyMain = None

    handoverDurationProbeName = None
    handoverFailureProbeName = None

    minFrequency = None
    bandwidth = None
    bandwidthMakeshift = None
    subCarriers = None
    numberOfChannelsToScan = 7


    handoverProvider = None
    rangingProvider = None
    regristrationProvider = None
    setupConnectionProvider = None
    connectionManager = None
    phyUser = None
    newFrameProvider = None

    def __init__(self, _serviceName, _scanningProvider, _handoverProvider, _rangingProvider, _regristrationProvide, _setupConnectionProvider, _connectionManager, _phyUser, _newFrameProvider, _minFrequency, _bandwidth, _bandwidthMakeshift
, _subCarriers, _numberOfChannelsToScan, **kw):
        self.serviceName = _serviceName
        self.handoverProvider = _handoverProvider
        self.rangingProvider = _rangingProvider
        self.regristrationProvider = _regristrationProvide
        self.setupConnectionProvider = _setupConnectionProvider
        self.connectionManager = _connectionManager
        self.phyUser = _phyUser
        self.newFrameProvider = _newFrameProvider
        self.strategyHandover = HandoverStrategy.Averaging()
        self.strategyBestStation = HandoverStrategy.BestStation()
        self.scanningStrategyInitial = ScanningStrategy.Plain(_scanningProvider, _newFrameProvider )
        self.scanningStrategyMain = ScanningStrategy.Interupted(_scanningProvider, _newFrameProvider)
        self.minFrequency = _minFrequency
        self.bandwidth = _bandwidth
        self.bandwidthMakeshift = _bandwidthMakeshift
        self.subCarriers = _subCarriers
        self.numberOfChannelsToScan = _numberOfChannelsToScan
        attrsetter(self, kw)

