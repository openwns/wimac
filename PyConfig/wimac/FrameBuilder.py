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

from openwns.pyconfig import Sealed, Frozen, attrsetter
#from support.WiMACParameters import ParametersOFDMA

import openwns.FCF
import openwns.Multiplexer

class FrameHeadCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.FrameHeadCollector"
    phyMode = None

    def __init__(self, frameBuilder, phyMode):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.phyMode = phyMode


class DLMapCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.DLMapCollector"
    dlSchedulerName = None
    phyMode = None
    
    def __init__(self, frameBuilder, dlSchedulerName, phyMode):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.dlSchedulerName = dlSchedulerName
        self.phyMode = phyMode


class ULMapCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.ULMapCollector"
    ulSchedulerName = None
    phyMode = None

    def __init__(self, frameBuilder, ulSchedulerName, phyMode):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.ulSchedulerName = ulSchedulerName
        self.phyMode = phyMode

class DataCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.DataCollector"

    txScheduler = None
    rxScheduler = None

    def __init__(self, frameBuilder):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        
class ActivationAction(Sealed):
    Start = 0
    StartCollection = 1
    FinishCollection = 2
    Pause = 3

    action = None
    def __init__(self, action):
        self.action = action
        
    def asString(self):
        if self.action == self.Start:
            return "start"
        elif self.action == self.StartCollection:
            return "start collection"
        elif self.action == self.FinishCollection:
            return "finish collection"
        elif self.action == self.Pause:
            return "pause"
        return "unknown"
    
class OperationMode(Sealed):
    Sending = 0
    Receiving = 1
    Pausing = 2

    mode = None

    def __init__(self, mode):
        self.mode = mode
        
    def asString(self):
        if self.mode == self.Sending:
            return "sending"
        elif self.mode == self.Receiving:
            return "receiving"
        elif self.mode == self.Pause:
            return "pausing"
        return "unknown"

class Activation(Sealed):
    """ Describes the activation of a phase in the frame """
    compoundCollector = None
    mode = None
    action = None
    duration = None
    
    def __init__(self, compoundCollector, mode, action, duration):
        self.compoundCollector = compoundCollector
        self.mode = mode
        self.action = action
        self.duration = duration
        
class TimingControl(Sealed):
    __plugin__ = "wimac.frame.TimingControl"
    name = "wimac.frame.TimingControl"
    activations = None
    phaseDescriptor = None
    frameStartupDelay = None

    def __init__(self):
        self.activations = []
        self.phaseDescriptor = []
        self.frameStartupDelay = 0.0
        
    def addActivation(self, activation):
        assert isinstance(activation, Activation)
        self.activations.append(activation)

    def prependActivationb(self, activation):
        assert isinstance(activation, Activation)
        self.activations.prepend(activation)
        
    def printActivations(self):
        for activation in self.activations:
            print "Compound Collector: " + activation.compoundCollector + \
                  "Action: " + activation.action.asString() + \
                  ", Mode: " + activation.mode.asString() + \
                  ", Duration: " + str(activation.duration)
