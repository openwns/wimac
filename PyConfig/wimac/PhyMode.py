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

class Modulation(Sealed):
    """The modulation class represents a physical layer modulation.
    """
    capacity = None
    
    def __init__(self, capacity):
        self.capacity = capacity

class Code(Sealed):
    numerator = None
    denominator = None
    
    def __init__(self, numerator, denominator = 1):
        self.numerator = numerator
        self.denominator = denominator


class PhyMode(Sealed):
    modulation = None
    code = None
    minSINR = None

BPSK = Modulation(1)
QPSK = Modulation(2)
QAM16 = Modulation(4)
QAM64 = Modulation(6)

Code12 = Code(1,2)
Code23 = Code(2,3)
Code34 = Code(3,4)
Code56 = Code(5,6)

modes = []

phyMode1 = PhyMode()
phyMode1.modulation =   QPSK
phyMode1.code =         Code12
phyMode1.minSINR =      "6.4 dB"
modes.append(phyMode1)

phyMode2 = PhyMode()
phyMode2.modulation =  QPSK
phyMode2.code =        Code34
phyMode2.minSINR =     "7.4 dB"
modes.append(phyMode2)

phyMode3 = PhyMode()
phyMode3.modulation =  QAM16
phyMode3.code =        Code12
phyMode3.minSINR =     "8.4 dB"
modes.append(phyMode3)

phyMode4 = PhyMode()
phyMode4.modulation =  QAM16
phyMode4.code =        Code34
phyMode4.minSINR =     "9.4 dB"
modes.append(phyMode4)

phyMode5 = PhyMode()
phyMode5.modulation =  QAM64
phyMode5.code =        Code12
phyMode5.minSINR =     "10.4 dB"
modes.append(phyMode5)

phyMode6 = PhyMode()
phyMode6.modulation =  QAM64
phyMode6.code =        Code23
phyMode5.minSINR =     "11.4 dB"
modes.append(phyMode6)

phyMode7 = PhyMode()
phyMode7.modulation =  QAM64
phyMode7.code =        Code34
phyMode7.minSINR =     "12.4 dB"
modes.append(phyMode7)

phyMode8 = PhyMode()
phyMode8.modulation =  QAM64
phyMode8.code =        Code56
phyMode8.minSINR =     "13.4 dB"
modes.append(phyMode8)
