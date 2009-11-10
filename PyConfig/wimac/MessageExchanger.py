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

"""MessageExchanger implementation

   This classs provide message exchange between two peer.
   The messages which will be send, are defind by their  MessageType and the size.

  _messages = [ [MessageType, CompoundSize], [MessageType, CompoundSize] ]

"""

from openwns.pyconfig import Frozen, Sealed, attrsetter

class Map(Sealed):
        messageType = None
        size = None

        def __init__(self, _messageType, _size):
            self.messageType = _messageType
            self.size = _size


class MessageExchanger(Sealed):
    """
    """
    __plugin__ = 'wimac.controlplane.MessageExchanger'
    name = "MessageExchanger"

    connectionManager = None
    connectionClassifier = None
    newFrameProvider = None

    connectionType = None
    messages = None

    timerWaitingForReply = None

    def __init__(self, _connectionManager, _connectionClassifier, _newFrameProvider, _messages = [ ], **kw):
        self.connectionManager = _connectionManager
        self.connectionClassifier = _connectionClassifier
        self.newFrameProvider = _newFrameProvider
        self.connectionType = 2
        self.timerWaitingForReply = 5
        self.messages = []
        attrsetter(self, kw)

        for ii in _messages:
            self.messages.append(Map(ii[0], ii[1]))
