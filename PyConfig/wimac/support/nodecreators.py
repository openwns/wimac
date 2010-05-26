###############################################################################
# This file is part of openWNS (open Wireless Network Simulator)
# _____________________________________________________________________________
#
# Copyright (C) 2004-2007
# Chair of Communication Networks (ComNets)
# Kopernikusstr. 16, D-52074 Aachen, Germany
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

import scenarios.builders

import Nodes

import openwns.geometry
import openwns.Scheduler

import ofdmaphy.OFDMAPhy
import ofdmaphy.Station
import scenarios.interfaces
import openwns.node
import rise.Mobility
import openwns.geometry.position
import openwns.node
import wimac.Stations

from wimac.support.Transceiver import Transceiver
import ip.Component

import constanze.traffic
from constanze.node import IPBinding, IPListenerBinding, Listener


import wimac.Rang

from wimac.support.Transceiver import Transceiver



class WiMAXBSCreator(scenarios.interfaces.INodeCreator):

    def __init__(self, stationIDs, config, oneRANGperBS = False):
        self.rang = None
        self.stationIDs = stationIDs
        self.config = config
        self.oneRANGperBS = oneRANGperBS

    def create(self):
        bs = wimac.support.Nodes.BaseStation(self.stationIDs.next(), self.config)
        bs.dll.logger.level = 2

        if self.rang == None or self.oneRANGperBS == True:
            if self.oneRANGperBS:
                rang = wimac.support.Nodes.RANG("WiMAXRang" + str(bs.nodeID), bs.nodeID)
                rang.dll.addAP(bs)
            else:
                rang = wimac.support.Nodes.RANG()
                self.rang = rang
                
            # The RANG only has one IPListenerBinding that is attached
            # to the listener. The listener is the only traffic sink
            # within the RANG
            ipListenerBinding = IPListenerBinding(rang.nl.domainName)
            listener = Listener(rang.nl.domainName + ".listener")
            rang.load.addListener(ipListenerBinding, listener)

            openwns.simulator.getSimulator().simulationModel.nodes.append(rang)

        if self.oneRANGperBS == False:
            self.rang.dll.addAP(bs)
        return bs


class WiMAXUECreator(scenarios.interfaces.INodeCreator):

    def __init__(self, stationIDs, config):
        self.stationIDs = stationIDs
        self.config = config

    def create(self):
        ss = wimac.support.Nodes.SubscriberStation(self.stationIDs.next(), self.config)
        return ss

