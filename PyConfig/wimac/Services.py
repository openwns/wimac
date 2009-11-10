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

class ConnectionManager():
    __plugin__ = 'wimac.services.ConnectionManager'
    fuReseter = None

    def __init__(self, serviceName, fuReseter):
        self.serviceName = serviceName
        self.fuReseter = fuReseter

class ConnectionControl():
    __plugin__ = 'wimac.services.ConnectionControl'

    associatedWith = None
    def __init__(self, serviceName):
        self.serviceName = serviceName

    def associateTo(self, destination):
        self.associatedWith = destination
                
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
