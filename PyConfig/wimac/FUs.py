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

import openwns.Buffer



class BufferDropping(openwns.Buffer.Dropping):
    __plugin__ = 'wimac.BufferDropping'

    resetedBitsProbeName = None
    resetedCompoundsProbeName = None

    def __init__(self,**kw):
        super(BufferDropping, self).__init__(**kw)


class Classifier(Sealed):
    __plugin__ = 'wimac.ConnectionClassifier'




class ClassifierMock(Sealed):
    __plugin__ = 'wimac.ClassifierMock'




class ACKSwitch(Sealed):
    __plugin__ = 'wimac.ACKSwitch'



class PhyUser(Sealed):
    __plugin__ = 'wimac.PhyUser'

