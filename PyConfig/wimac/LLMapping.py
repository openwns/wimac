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
from openwns.interval import Interval

#from support.WiMACParameters import ParametersOFDM,ParametersOFDMA

from rise.PhyMode import PhyMode,PhyModeMapper

# Mappings for PER = 0.0001
class WIMAXMapper(PhyModeMapper):
    lowestPhyMode = None
  
    def __init__(self, symbolDuration, subCarriersPerSubChannel):
        PhyModeMapper.__init__(self, symbolDuration, subCarriersPerSubChannel)
        
        
        self.lowestPhyMode = PhyMode("QPSK-WiMAX-320")
        self.setMinimumSINR(-5.331243)
        
        self.addPhyMode(Interval(-200.000000,-3.464765,"(]"),self.lowestPhyMode)
        self.addPhyMode(Interval(-3.464765,-2.952957,"(]"),PhyMode("QPSK-WiMAX-720"))
        self.addPhyMode(Interval(-2.952957,-0.421870,"(]"),PhyMode("QPSK-WiMAX-1020"))
        self.addPhyMode(Interval(-0.421870,1.085567,"(]"),PhyMode("QPSK-WiMAX-1320"))
        self.addPhyMode(Interval(1.085567,2.683285,"(]"),PhyMode("QPSK-WiMAX-1520"))
        self.addPhyMode(Interval(2.683285,3.925478,"(]"),PhyMode("QPSK-WiMAX-1720"))
        self.addPhyMode(Interval(3.925478,5.219113,"(]"),PhyMode("QAM16-WiMAX-1920"))
        self.addPhyMode(Interval(5.219113,8.770903,"(]"),PhyMode("QAM16-WiMAX-2120"))
        self.addPhyMode(Interval(8.770903,10.678538,"(]"),PhyMode("QAM16-WiMAX-2320"))
        self.addPhyMode(Interval(10.678538,11.279353,"(]"),PhyMode("QAM64-WiMAX-2420"))
        self.addPhyMode(Interval(11.279353,13.452776,"(]"),PhyMode("QAM64-WiMAX-2520"))
        self.addPhyMode(Interval(13.452776,14.450271,"(]"),PhyMode("QAM64-WiMAX-2620"))
        self.addPhyMode(Interval(14.450271,16.728635,"(]"),PhyMode("QAM64-WiMAX-2720"))
        self.addPhyMode(Interval(16.728635,17.728189,"(]"),PhyMode("QAM64-WiMAX-2820"))
        self.addPhyMode(Interval(17.728189,19.580673,"(]"),PhyMode("QAM64-WiMAX-2920"))
        self.addPhyMode(Interval(19.580673,200.000000,"(]"),PhyMode("QAM64-WiMAX-3020"))




