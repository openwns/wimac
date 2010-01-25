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

"""HandoverStrategy


"""

from openwns.pyconfig import Sealed, attrsetter

class HandoverStrategy(Sealed):
    __plugin__ = None


class Averaging(HandoverStrategy):
     __plugin__ = 'wimac.services.handoverStrategy.Averaging'

     alpha = 1.0   # [0.0-1.0] influence of the new added value to the average 

     def __init__(self, **kw):
          attrsetter(self, kw)


class AverageWindow(HandoverStrategy):
     __plugin__ = 'wimac.services.handoverStrategy.AverageWindow'

     windowForAverage = 1

     def __init__(self, **kw):
          attrsetter(self, kw)



class BestStation(HandoverStrategy):
     __plugin__ = 'wimac.services.handoverStrategy.BestStation'

     minCIR = 6.4

     def __init__(self, **kw):
         attrsetter(self, kw)
