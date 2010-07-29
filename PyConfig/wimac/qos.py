###############################################################################
# This file is part of openWNS (open Wireless Network Simulator)
# _____________________________________________________________________________
#
# Copyright (C) 2004-2007
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

from openwns.pyconfig import attrsetter
import openwns.qos


undefinedQosClass = openwns.qos.QoSClass("UNDEFINED",0,priority=5)
SignalingQosClass = openwns.qos.QoSClass("SIGNALING",1,priority=0)
UGSQosClass = openwns.qos.QoSClass("UGS",2,priority=1)
rtPSQosClass = openwns.qos.QoSClass("RTPS",3,priority=2)
nrtPSQosClass = openwns.qos.QoSClass("NRTPS",4,priority=3)
BEQosClass = openwns.qos.QoSClass("BE",5,priority=4)


class QoSClasses(openwns.qos.QoSClasses):

    def __init__(self, **kw):
        openwns.qos.QoSClasses.__init__(self)
        self.mapEntries = []
        self.mapEntries.append(undefinedQosClass)
        self.mapEntries.append(SignalingQosClass)
        self.mapEntries.append(UGSQosClass)
        self.mapEntries.append(rtPSQosClass)
        self.mapEntries.append(nrtPSQosClass)
        self.mapEntries.append(BEQosClass)
        attrsetter(self, kw)
