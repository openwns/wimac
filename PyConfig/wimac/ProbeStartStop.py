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

"""BlackHole implementation

"""

from openwns.pyconfig import Sealed, attrsetter

class ProbeStartStop(Sealed):
    """

    """
    __plugin__ = 'wimac.ProbeStartStop'
    name = None

    probeIncomingBitsName = ""
    probeIncomingCompoundsName = ""
    probeOutgoingBitsName = ""
    probeOutgoingCompoundsName = ""
    probeAggregatedBitsName = None
    probeAggregatedCompoundsName = None

    eventStart = None
    eventStop = None
    eventReset = None

    eventStartStopSubjectName = None
    eventStartStopSubjectType = None

    def __init__(self, _name, _prefix, _eventStart, _eventStop, _eventReset, **kw):
        self.name = _name
        self.probeIncomingBitsName = _prefix + 'IncomingBits'
        self.probeIncomingCompoundsName = _prefix + 'IncomingCompounds'
        self.probeOutgoingBitsName = _prefix + 'OutgoingBits'
        self.probeOutgoingCompoundsName = _prefix + 'OutgoingCompounds'
        self.probeAggregatedBitsName = _prefix + 'AggregatedBits'
        self.probeAggregatedCompoundsName = _prefix + 'AggregatedCompounds'
        self.eventStart = _eventStart
        self.eventStop = _eventStop
        self.eventReset = _eventReset
        attrsetter(self, kw)
