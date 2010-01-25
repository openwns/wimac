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

from support.WiMACParameters import ParametersOFDM,ParametersOFDMA

from rise.PhyMode import PhyMode,PhyModeMapper


# Info on MI: http://en.wikipedia.org/wiki/Mutual_information

# WIMAX PHY Modes
symbolDuration = ParametersOFDMA.symbolDuration
subCarriersPerSubChannel = ParametersOFDM.dataSubCarrier
WIMAXPhyMode1 = PhyMode(modulation = "BPSK",  coding = "WIMAX-1/2")
WIMAXPhyMode2 = PhyMode(modulation = "QPSK",  coding = "WIMAX-1/2")
WIMAXPhyMode3 = PhyMode(modulation = "QPSK",  coding = "WIMAX-3/4")
WIMAXPhyMode4 = PhyMode(modulation = "QAM16", coding = "WIMAX-1/2")
WIMAXPhyMode5 = PhyMode(modulation = "QAM16", coding = "WIMAX-3/4")
WIMAXPhyMode6 = PhyMode(modulation = "QAM64", coding = "WIMAX-2/3")
WIMAXPhyMode7 = PhyMode(modulation = "QAM64", coding = "WIMAX-3/4")
WIMAXPhyMode1.setSymbolDuration(symbolDuration)
WIMAXPhyMode2.setSymbolDuration(symbolDuration)
WIMAXPhyMode3.setSymbolDuration(symbolDuration)
WIMAXPhyMode4.setSymbolDuration(symbolDuration)
WIMAXPhyMode5.setSymbolDuration(symbolDuration)
WIMAXPhyMode6.setSymbolDuration(symbolDuration)
WIMAXPhyMode7.setSymbolDuration(symbolDuration)
WIMAXPhyMode1.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXPhyMode2.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXPhyMode3.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXPhyMode4.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXPhyMode5.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXPhyMode6.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXPhyMode7.setSubCarriersPerSubChannel(subCarriersPerSubChannel)
WIMAXLowestPhyMode = WIMAXPhyMode1 # this is the PhyMode for MAP/BCH/RACH etc


# mapping from SNR range to suitable modulation&coding scheme (MCS)
class WIMAXMapper(PhyModeMapper):
    def __init__(self, symbolDuration, subCarriersPerSubChannel):
        super(WIMAXMapper, self).__init__(symbolDuration, subCarriersPerSubChannel)

        self.setMinimumSINR(6.4);
        self.addPhyMode(Interval(   6.4,   9.4, "(]"), WIMAXPhyMode1)
        self.addPhyMode(Interval(   9.4,  11.2, "(]"), WIMAXPhyMode2)
        self.addPhyMode(Interval(  11.2,  16.4, "(]"), WIMAXPhyMode3)
        self.addPhyMode(Interval(  16.4,  18.2, "(]"), WIMAXPhyMode4)
        self.addPhyMode(Interval(  18.2,  22.7, "(]"), WIMAXPhyMode5)
        self.addPhyMode(Interval(  22.7,  24.4, "(]"), WIMAXPhyMode6)
        self.addPhyMode(Interval(  24.4, 200.0, "(]"), WIMAXPhyMode7)



