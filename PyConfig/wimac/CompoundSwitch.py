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

from openwns.pyconfig import Sealed
from openwns.pyconfig import Frozen
from openwns.pyconfig import attrsetter



class CompoundSwitch(Sealed):
    __plugin__="wimac.compoundSwitch.CompoundSwitch"
    name = "wimac.compoundSwitch.CompoundSwitch"
    onDataFilters = None
    sendDataFilters = None

    def __init__(self, **kw):
        self.onDataFilters = []
        self.sendDataFilters = []
        attrsetter(self, kw)


class Filter(Sealed):
    __plugin__="wimac.compoundSwitch.Filter"
    name = None
    def __init__(self, name):
        self.name = name



######### special filters implementation ###################
class FilterAll(Filter):
    __plugin__="wimac.compoundSwitch.filter.FilterAll"

    def __init__(self,name,**kw):
        super(FilterAll, self).__init__(name)
        attrsetter(self, kw)


class FilterNone(Filter):
    __plugin__="wimac.compoundSwitch.filter.FilterNone"

    def __init__(self,name,**kw):
        super(FilterNone, self).__init__(name)
        attrsetter(self, kw)


class FilterFilterName(Filter):
    __plugin__="wimac.compoundSwitch.filter.FilterFilterName"

    def __init__(self,name,**kw):
        super(FilterFilterName, self).__init__(name)
        attrsetter(self, kw)


class FilterCommand(Filter):
    __plugin__="wimac.compoundSwitch.filter.FilterCommand"
    commandProvider = None

    def __init__(self, name, commandProvider, **kw):
        super(FilterCommand, self).__init__(name)
        self.commandProvider = commandProvider
        attrsetter(self, kw)

class RelayDirection(Filter):
    __plugin__ = 'wimac.compoundSwitch.filter.RelayDirection'
    direction = None

    def __init__(self, name, direction):
        super(RelayDirection, self).__init__(name)
        self.direction = direction
