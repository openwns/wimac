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

import openwns.logger

class QueueManager(object):
    __plugin__ = "wimac.services.QueueManager"
    connectionManagerServiceName = None
    logger = None
    
    def __init__(self, 
            serviceName = "queueManager", 
            cmServiceName = "connectionManager", 
            parentLogger = None):
        self.serviceName = serviceName
        self.connectionManagerServiceName = cmServiceName
        self.logger = openwns.logger.Logger("WIMAC", "QueueManager", True, parentLogger);
      

class ConnectionManager():
    __plugin__ = 'wimac.services.ConnectionManager'

    def __init__(self, serviceName):
        self.serviceName = serviceName


class AssociationControl(object):
    serviceName = "associationControl"
                
class Fixed(AssociationControl):
    __plugin__ = 'wimac.services.AssociationControl.Fixed'

    associatedWith = None
    def __init__(self, associatedWith = None):
        self.associatedWith = associatedWith

    def associateTo(self, destination):
        self.associatedWith = destination                
                
class BestAtGivenTime(AssociationControl):
    __plugin__ = 'wimac.services.AssociationControl.BestAtGivenTime'
    decisionStrategy = None
    decisionTime = None
    
    class BestPathloss:
        __plugin__ = "wimac.services.AssociationControl.Best.BestPathloss"
    
    class BestRxPower:
        __plugin__ = "wimac.services.AssociationControl.Best.BestRxPower"
    
    class BestSINR:
        __plugin__ = "wimac.services.AssociationControl.Best.BestSINR"

    # RxPower, SINR, Pathloss
    criterion = None
    def __init__(self, decisionStrategy, decisionTime):        
        self.decisionStrategy = decisionStrategy
        self.decisionTime = decisionTime


class ConstantValue(object):
    __plugin__ = 'wimac.services.InterferenceCache.ConstantValue'
    
    averageCarrier = None
    averageInterference = None
    deviationCarrier = None
    deviationInterference = None
    averagePathloss = None

class Complain(object):
    __plugin__ = 'wimac.services.InterferenceCache.Complain'


class InterferenceCache(object):
    __plugin__ = 'wimac.services.InterferenceCache'

    alphaLocal = None
    alphaRemote= None
    notFoundStrategy = None

    def __init__(self, serviceName, alphaLocal, alphaRemote):
        self.serviceName = serviceName
        self.alphaLocal = alphaLocal
        self.alphaRemote = alphaRemote
        self.notFoundStrategy = ConstantValue()

class InterferenceCacheDropin(InterferenceCache):
    def __init__(self):
        super(InterferenceCacheDropin,self).__init__(serviceName = "INTERFERENCECACHE",
                                                     alphaLocal  = 0.2,
                                                     alphaRemote = 0.05)
        self.notFoundStrategy = Complain()
