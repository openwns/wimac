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
from openwns.pyconfig import Frozen
from openwns.pyconfig import attrsetter
from support.WiMACParameters import ParametersOFDMA
from LLMapping import WIMAXLowestPhyMode

import openwns.FCF
import openwns.Multiplexer

class SingleCompoundCollector(Sealed):
    __plugin__ = "wimac.frame.SingleCompoundCollector"
    phyMode = None

    def __init__(self):
        self.phyMode = WIMAXLowestPhyMode

class ContentionCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.ContentionCollector"
    contentionAccess = None
    phyMode = None

    class ContentionAccess:
        enabled = None
        slotLengthInSymbols = None
        numberOfSlots = None

        def __init__(self, enabled, slotLengthInSymbols, numberOfSlots):
            self.enabled = enabled
            self.slotLengthInSymbols = slotLengthInSymbols
            self.numberOfSlots = numberOfSlots

    def __init__(self, frameBuilder, **kw):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.contentionAccess = self.ContentionAccess(False,0 , 0)
        self.phyMode = WIMAXLowestPhyMode
        attrsetter(self, kw)


class FrameHeadCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.FrameHeadCollector"
    phyMode = None

    def __init__(self, frameBuilder):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.phyMode = WIMAXLowestPhyMode


class DLMapCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.DLMapCollector"
    dlSchedulerName = None
    phyMode = None
    
    def __init__(self, frameBuilder, dlSchedulerName):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.dlSchedulerName = dlSchedulerName
        self.phyMode = WIMAXLowestPhyMode


class ULMapCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.ULMapCollector"
    ulSchedulerName = None
    phyMode = None

    def __init__(self, frameBuilder, ulSchedulerName):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.ulSchedulerName = ulSchedulerName
        self.phyMode = WIMAXLowestPhyMode

class MultiCompoundBSDLScheduler(ContentionCollector):
    __plugin__ = "wimac.frame.MultiCompoundBSDLScheduler"

    def __init__(self, frameBuilder):
        super(MultiCompoundBSDLScheduler, self).__init__(frameBuilder)


class SSDLScheduler(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.SSDLScheduler"
    dlMapRetrieverName = None

    def __init__(self, frameBuilder, dlMapRetrieverName):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.dlMapRetrieverName = dlMapRetrieverName

class DataCollector(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.DataCollector"

    txScheduler = None
    rxScheduler = None

    def __init__(self, frameBuilder):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        

class StaticBSULScheduler(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.StaticBSULScheduler"
    phyMode = None

    def __init__(self, frameBuilder):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.phyMode = WIMAXLowestPhyMode


class SSULScheduler(openwns.FCF.CompoundCollector):
    __plugin__ = "wimac.frame.SSULScheduler"
    ulMapRetrieverName = None
    symbolDuration = ParametersOFDMA.symbolDuration
    phyMode = None

    def __init__(self, frameBuilder, ulMapRetrieverName):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.ulMapRetrieverName = ulMapRetrieverName
        self.phyMode = WIMAXLowestPhyMode


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
