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

"""ErrorModelling implementation

   the class ErrorModelling configures the ErrorModelling implementation,
   which performs a Mapping from PHYMode, CIR to Symbol Error Rate (SER) and
   calculate the Packet Error Rate (PER).
"""

from openwns.pyconfig import Frozen
from openwns.pyconfig import Sealed
from openwns.pyconfig import attrsetter
from LLMapping import WIMAXMapper
from support.WiMACParameters import ParametersOFDM

class MappingObject(Sealed):
    cir = None
    ser = None

    def __init__(self, _cir, _ser):
        self.cir = _cir
        self.ser = _ser


class Cir2SerMapping(Sealed):
    mapping = None

    def __init__(self, rawMapping):
        self.mapping = []
        for ii in rawMapping:
            self.mapping.append(MappingObject(ii[0], ii[1]))


rawBPSK12  = [ [ 0,0.17284     ],
               [ 1,0.1         ],
               [ 2,0.0354839   ],
               [ 3,0.0124521   ],
               [ 4,0.00363386  ],
               [ 5,0.000665585 ],
               [ 6,0.000124797 ],
               [ 7,4.15991e-05 ],
               [ 8,0           ],
               [ 9,0           ],
               [10,0           ],
               [11,0           ],
               [12,0           ],
               [13,0           ],
               [14,0           ],
               [15,0           ],
               [16,0           ],
               [17,0           ],
               [18,0           ],
               [19,0           ],
               [20,0           ],
               [21,0           ],
               [22,0           ]
               ]

rawQPSK12  = [ [ 0,1          ],
               [ 1,1          ],
               [ 2,1          ],
               [ 3,0.416667   ],
               [ 4,0.0555556  ],
               [ 5,0.00172018 ],
               [ 6,8.31878e-05],
               [ 7,0          ],
               [ 8,0          ],
               [ 9,0          ],
               [10,0          ],
               [11,0          ],
               [12,0          ],
               [13,0          ],
               [14,0          ],
               [15,0          ],
               [16,0          ],
               [17,0          ],
               [18,0          ],
               [19,0          ],
               [20,0          ],
               [21,0          ],
               [22,0          ]
               ]

rawQPSK34  = [ [ 0,1          ],
               [ 1,1          ],
               [ 2,1          ],
               [ 3,1          ],
               [ 4,1          ],
               [ 5,0.684211   ],
               [ 6,0.333333   ],
               [ 7,0.048583   ],
               [ 8,0.00594227 ],
               [ 9,0.00146928 ],
               [10,0.000118229],
               [11,1.97048e-05],
               [12,0          ],
               [13,0          ],
               [14,0          ],
               [15,0          ],
               [16,0          ],
               [17,0          ],
               [18,0          ],
               [19,0          ],
               [20,0          ],
               [21,0          ],
               [22,0          ]
               ]

rawQAM16_12  = [[ 0,1          ],
                [ 1,1          ],
                [ 2,1          ],
                [ 3,1          ],
                [ 4,1          ],
                [ 5,1          ],
                [ 6,1          ],
                [ 7,1          ],
                [ 8,0.88       ],
                [ 9,0.44       ],
                [10,0.0116129  ],
                [11,1.99601e-05],
                [12,0          ],
                [13,0          ],
                [14,0          ],
                [15,0          ],
                [16,0          ],
                [17,0          ],
                [18,0          ],
                [19,0          ],
                [20,0          ],
                [21,0          ],
                [22,0          ]
                ]

rawQAM16_34  = [[ 0,1          ],
                [ 1,1          ],
                [ 2,1          ],
                [ 3,1          ],
                [ 4,1          ],
                [ 5,1          ],
                [ 6,1          ],
                [ 7,1          ],
                [ 8,1          ],
                [ 9,1          ],
                [10,1          ],
                [11,0.763158   ],
                [12,0.473684   ],
                [13,0.0488722  ],
                [14,0.00615901 ],
                [15,0.000856793],
                [16,0.000177277],
                [17,5.90923e-05],
                [18,0          ],
                [19,0          ],
                [20,0          ],
                [21,0          ],
                [22,0          ]
                ]

rawQAM64_23  = [[ 0,1          ],
                [ 1,1          ],
                [ 2,1          ],
                [ 3,1          ],
                [ 4,1          ],
                [ 5,1          ],
                [ 6,1          ],
                [ 7,1          ],
                [ 8,1          ],
                [ 9,1          ],
                [10,1          ],
                [11,1          ],
                [12,1          ],
                [13,1          ],
                [14,0.980392   ],
                [15,0.921569   ],
                [16,0.588235   ],
                [17,0.117647   ],
                [18,0.0154799  ],
                [19,0.000628457],
                [20,0.000195687],
                [21,1.95687e-05],
                [22,0          ]
                ]

rawQAM64_34  = [[ 0,1           ],
                [ 1,1           ],
                [ 2,1           ],
                [ 3,1           ],
                [ 4,1           ],
                [ 5,1           ],
                [ 6,1           ],
                [ 7,1           ],
                [ 8,1           ],
                [ 9,1           ],
                [10,1           ],
                [11,1           ],
                [12,1           ],
                [13,1           ],
                [14,1           ],
                [15,0.982759    ],
                [16,0.862069    ],
                [17,0.62069     ],
                [18,0.189655    ],
                [19,0.00907441  ],
                [20,0.000676133 ],
                [21,0           ],
                [22,0           ]
                ]

cir2ser_BPSK12 = Cir2SerMapping(rawBPSK12)
cir2ser_QPSK12 = Cir2SerMapping(rawQPSK12)
cir2ser_QPSK34 = Cir2SerMapping(rawQPSK34)
cir2ser_QAM16_12 = Cir2SerMapping(rawQAM16_12)
cir2ser_QAM16_34 = Cir2SerMapping(rawQAM16_34)
cir2ser_QAM64_23 = Cir2SerMapping(rawQAM64_23)
cir2ser_QAM64_34 = Cir2SerMapping(rawQAM64_34)


class ErrorModelling(Sealed):
    """This class mappes the cir to ser and calculate PER

       self.PrintMappings:  if True, the Mapping Tables will be prinnted
                            at the initialization,
    """
    __plugin__ = 'wimac.ErrorModelling'
    name = "ErrorModelling"
    cirProvider = None
    phyModeProvider = None
    phyModeMapping = None

    cir2ser_BPSK12 = None
    cir2ser_QPSK12 = None
    cir2ser_QPSK34 = None
    cir2ser_QAM16_12 = None
    cir2ser_QAM16_34 = None
    cir2ser_QAM64_23 = None
    cir2ser_QAM64_34 = None
    PrintMappings = False

    def __init__(self, cirProvider, phyModeProvider, **kw):
        self.cirProvider = cirProvider
        self.phyModeProvider = phyModeProvider
        symbolDuration = ParametersOFDM.symbolDuration
        subCarriersPerSubChannel = ParametersOFDM.dataSubCarrier # 192
        self.phyModeMapping = WIMAXMapper(symbolDuration, subCarriersPerSubChannel)
        self.cir2ser_BPSK12 = cir2ser_BPSK12
        self.cir2ser_QPSK12 = cir2ser_QPSK12
        self.cir2ser_QPSK34 = cir2ser_QPSK34
        self.cir2ser_QAM16_12 = cir2ser_QAM16_12
        self.cir2ser_QAM16_34 = cir2ser_QAM16_34
        self.cir2ser_QAM64_23 = cir2ser_QAM64_23
        self.cir2ser_QAM64_34 = cir2ser_QAM64_34
        attrsetter(self, kw)
