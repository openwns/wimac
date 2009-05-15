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

"""ControlPlaneManagerSS implementation


"""

import dll.Services
from openwns.pyconfig import attrsetter


class ControlPlaneManagerSimpleSS(dll.Services.Service):
    """This class

       self.qosCategory: UGS, rtPS, nrtPS, BE (Signaling, NoQoS are possible, too.
                         But, it make no sense)

    """
    __plugin__ = 'wimac.services.ControlPlaneManagerSimpleSS'
    nameInServiceFactory = __plugin__

    def __init__(self, _serviceName):
        self.serviceName = _serviceName
