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

""" Experimental Scheduler
"""
from openwns.pyconfig import Sealed, attrsetter
from openwns.logger import Logger
import openwns.qos
import openwns.FCF
import openwns.Scheduler
#from support.WiMACParameters import ParametersOFDM, ParametersSystem
from support.WiMACParameters import ParametersSystem
from LLMapping import WIMAXMapper

class PhyModeMapper:
    """ put SINR-Bps lookup table here
    I can have different classes for different systems, e.g. WiMAX PhyModeMapper
    and include the one that I want to use with "phyModeMapper = ..." in class Scheduler
    """

class RegistryProxyWiMAC(openwns.Scheduler.RegistryProxy):
    nameInRegistryProxyFactory = "RegistryProxyWiMAC"
    phyModeMapper = None
    queueSize = 320000   ## maximum of 0.32MBit in one queue,
                         ## that would be 5ms at 64MBps
    powerCapabilitiesUT = None
    powerCapabilitiesAP = None
    powerCapabilitiesFRS = None
    qosClassMapping    = openwns.qos.QoSClasses()
    numberOfPriorities = qosClassMapping.getMaxPriority() + 1
    isDL = None
    
    def __init__(self, isDL = True):
        self.isDL = isDL
        
    def setPhyModeMapper(self, phyModeMapper, ):
        self.phyModeMapper = phyModeMapper
        # for the moment, max, avg and overall power are set the same. change it when necessary
        self.powerCapabilitiesUT = openwns.Scheduler.PowerCapabilities(ParametersSystem.txPower['UT'],
                                                                   ParametersSystem.txPower['UT'],
                                                                   ParametersSystem.txPower['UT'])
        self.powerCapabilitiesAP = openwns.Scheduler.PowerCapabilities(ParametersSystem.txPower['AP'],
                                                                   ParametersSystem.txPower['AP'],
                                                                   ParametersSystem.txPower['AP'])
        self.powerCapabilitiesFRS = openwns.Scheduler.PowerCapabilities(ParametersSystem.txPower['FRS'],
                                                                    ParametersSystem.txPower['FRS'],
                                                                    ParametersSystem.txPower['FRS'])

class SpaceTimeSectorizationRegistryProxy(openwns.Scheduler.RegistryProxy):
    phyModeMapper = None
    nameInRegistryProxyFactory = "SpaceTimeSectorizationRegistryProxy"
    queueSize = 320000
    logger = openwns.logger.Logger("WiMAC", "SpaceTimeSectorizationRegProxy", True)

    #each sectors is served // TODO: no QoS yet.in seperate frames
    numberOfSectors = 2

    #a sector can consist of subsectors increaing SDMA gain
    #numberOfSubsectors = 1
    #not used yet
    #a parameter to determine the mutual location of the subsectors
    #angle between bisectors of the subsectors  in radians (<=pi)
    #mutualAngelOfSubsectors = 0


class Scheduler(openwns.FCF.CompoundCollector):
    """Base class for all Schedulers
    """
    __plugin__ = 'wimac.scheduler.Scheduler'
    name = "Scheduler"

    strategy = None
    queue = None
    grouper = None
    registry = None
    callback = None
    maxBeams = None
    freqChannels = None
    beamforming = None
    plotFrames = False
    uplink = None
    alwaysAcceptIfQueueAccepts = None
    pseudoGenerator = None
    numberOfTimeSlots = None
    mapHandlerName = None
    slotDuration = None
    

    resettedBitsProbeBusName = None
    resettedCompoundsProbeBusName = None

    def __init__(self, frameBuilder,
                 symbolDuration,
                 slotDuration,
                 dataSubCarrier,
                 mapHandlerName,
                 strategy,
                 beamforming = True,
                 maxBeams = 4,
                 friendliness_dBm = "-105 dBm",
                 uplink = False,
                 **kw):
        openwns.FCF.CompoundCollector.__init__(self, frameBuilder)
        self.mapHandlerName = mapHandlerName
        self.slotDuration = slotDuration
        self.strategy = strategy
        self.strategy.logger.enabled = False
        self.strategy.dsastrategy.logger.enabled = False
        self.strategy.apcstrategy.logger.enabled = False
        self.strategy.symbolDuration = symbolDuration
        self.queue = openwns.Scheduler.SimpleQueue()
        self.grouper = openwns.Scheduler.SINRHeuristic(beamforming = beamforming)
        self.grouper.friendliness_dBm = friendliness_dBm
        self.uplink = uplink
        self.alwaysAcceptIfQueueAccepts = False
        self.grouper.uplink = self.uplink
        self.registry = RegistryProxyWiMAC(isDL = (not uplink))
        subCarriersPerSubChannel = dataSubCarrier
        phyModeMapper = WIMAXMapper(symbolDuration,subCarriersPerSubChannel)
        self.registry.setPhyModeMapper(phyModeMapper)
        self.maxBeams = maxBeams
        self.numberOfTimeSlots = 1
        self.freqChannels = 1
        self.beamforming = beamforming
        self.resettedBitsProbeBusName = "wimac.schedulerQueue.resetted.bits"
        self.resettedCompoundsProbeBusName = "wimac.schedulerQueue.resetted.compounds"
        attrsetter(self, kw)

class PseudoBWRequestGenerator(Sealed):
    __plugin__ = 'wimac.scheduler.PseudoBWRequestGenerator'

    connectionManager = None
    ulScheduler = None
    classifier = None
    packetSize = None
    pduOverhead = None

    def __init__(self, _connectionManager, _ulScheduler, _packetSize, _pduOverhead):
        self.packetSize = _packetSize
        self.connectionManager = _connectionManager
        self.ulScheduler = _ulScheduler
        self.pduOverhead = _pduOverhead
        self.classifier = 'classifier'

class Callback(Sealed):
    frameOffsetDelayProbeName = "wimac.frameOffsetDelay"
    transmissionDelayProbeName = "wimac.transmissionDelay"
    

class ULCallback(Callback):
    #__plugin__ = 'wimac.scheduler.ULCallback'
    slotLength = None

    def __init__(self, **kw):
        self.slotLength = 0
        attrsetter(self, kw) 

class ULMasterCallback(ULCallback):
    __plugin__ = 'wimac.scheduler.ULMasterCallback'

    def __init__(self, **kw):
        ULCallback.__init__(self, **kw)

class ULSlaveCallback(ULCallback):
    __plugin__ = 'wimac.scheduler.ULSlaveCallback'

    def __init__(self, **kw):
        ULCallback.__init__(self, **kw)

class DLCallback(Callback):
    __plugin__ = 'wimac.scheduler.DLCallback'
    beamforming = None
    slotLength = None

    def __init__(self, **kw):
        self.slotLength = 0
        attrsetter(self, kw)

class BypassQueue(Sealed):
    __plugin__ = 'wimac.BypassQueue'
    nameInQueueFactory = __plugin__

    def __init__(self, **kw):
        attrsetter(self, kw)
