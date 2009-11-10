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

from openwns.pyconfig import Sealed, attrsetter

class PhyUser(Sealed):
    __plugin__ = 'wimac.PhyUser'
    centerFrequency = None
    bandwidth = None
    numberOfSubCarrier = None
        
    iProbeName = None
    cirProbeName = None
    cProbeName = None
    phyModeProbeName = None
    deltaPhyProbeName = None
    deltaIProbeName = None
    deltaCProbeName = None
    iFCHProbeName = None
    cirFCHProbeName = None
    iContentionProbeName = None
    cirContentionProbeName = None
    pathlossProbeName = None

    def __init__(self, **kw):
        self.iProbeName = "wimac.interferenceSDMA"
        self.cirProbeName = "wimac.cirSDMA"
        self.cProbeName = "wimac.carrierSDMA"
        self.phyModeProbeName = "wimac.PHYModeSDMA"
        self.deltaPhyProbeName = "wimac.deltaPHYModeSDMA"
        self.deltaIProbeName = "wimac.deltaInterferenceSDMA"
        self.deltaCProbeName = "wimac.deltaCarrierSDMA"
        self.iFCHProbeName = "wimac.interferenceFrameHead"
        self.cirFCHProbeName = "wimac.cirFrameHead"
        self.iContentionProbeName = "wimac.interferenceContention"
        self.cirContentionProbeName = "wimac.cirContention"      
        self.pathlossProbeName = "wimac.pathloss"      
        attrsetter(self, kw)

