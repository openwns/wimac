###############################################################################
# This file is part of openWNS (open Wireless Network Simulator)
# _____________________________________________________________________________
#
# Copyright (C) 2004-2007
# Chair of Communication Networks (ComNets)
# Kopernikusstr. 16, D-52074 Aachen, Germany
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

import constanze.traffic
from constanze.node import IPBinding, IPListenerBinding, Listener
from openwns import dBm, dB, fromdB, fromdBm
from scenarios.channelmodel.channelmodelcreator import *
import wimac.qos

try:
    import applications.clientSessions
    import applications.serverSessions
    import applications.codec
    import applications.component

    ### Application VoIP
    def createDLVoIPTraffic(simulator, codecType = applications.codec.AMR_12_2(), comfortNoiseChoice = True, settlingTime = 0.0):
        rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
        rang = rangs[0]
        utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
        
        for ut in utNodes:
            voipDL = applications.serverSessions.VoIP(codecType = codecType,
                                                    comfortNoiseChoice = comfortNoiseChoice, settlingTime = settlingTime)
    
            tlListenerBinding = applications.component.TLListenerBinding(rang.nl.domainName, "127.0.0.1", 1028,
                                                                        wimac.qos.UGSQosClass, 1028, voipDL,
                                                                        parentLogger = rang.logger)
            rang.load.addListenerBinding(tlListenerBinding)
    
    def createULVoIPTraffic(simulator, codecType = applications.codec.AMR_12_2(), comfortNoiseChoice = True,
                            settlingTime = 0.0, minStartDelay = 0.1, maxStartDelay = 1.0):
        rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
        rang = rangs[0]
        utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
                
        for ut in utNodes:
            voipUL = applications.clientSessions.VoIP(codecType = codecType,
                                                    comfortNoiseChoice = comfortNoiseChoice, settlingTime = settlingTime,
                                                    minStartDelay = minStartDelay, maxStartDelay = maxStartDelay)
    
            tlBinding = applications.component.TLBinding(ut.nl.domainName, rang.nl.domainName,
                                                        1028, wimac.qos.UGSQosClass,
                                                        1028, parentLogger = ut.logger)
            ut.load.addTraffic(tlBinding, voipUL)
    ###
    
    
    ### Application Video(WiMAX)
    def createDLWiMAXVideoTraffic(simulator, framesPerSecond = 10.0, numberOfPackets = 8.0, shapeOfPacketSize = 1.2,
                            scaleOfPacketSize = 40.0, shapeOfPacketIat = 1.2, scaleOfPacketIat = 2.5,
                            settlingTime = 0.0):
        rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
        rang = rangs[0]
        utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
        
        for ut in utNodes:
            wimaxVideoDL = applications.serverSessions.WiMAXVideo(framesPerSecond = framesPerSecond, numberOfPackets = numberOfPackets,
                                                            shapeOfPacketSize = shapeOfPacketSize, scaleOfPacketSize = scaleOfPacketSize,
                                                            shapeOfPacketIat = shapeOfPacketIat, scaleOfPacketIat = scaleOfPacketIat,
                                                            settlingTime = settlingTime, parentLogger = rang.logger)
    
            tlListenerBinding = applications.component.TLListenerBinding(rang.nl.domainName, "127.0.0.1", 1032,
                                                                        wimac.qos.rtPSQosClass, 1032, wimaxVideoDL,
                                                                        parentLogger = rang.logger)
            rang.load.addListenerBinding(tlListenerBinding)
    
    
    def createULWiMAXVideoTraffic(simulator, settlingTime = 0.0, minStartDelay = 1.0, maxStartDelay = 2.0):
        rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
        rang = rangs[0]
        utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
                
        for ut in utNodes:
            wimaxVideoUL = applications.clientSessions.WiMAXVideo(settlingTime = settlingTime, minStartDelay = minStartDelay, maxStartDelay = maxStartDelay)
    
            tlBinding = applications.component.TLBinding(ut.nl.domainName, rang.nl.domainName,
                                                        1032, wimac.qos.rtPSQosClass,
                                                        1032, parentLogger = ut.logger)
            ut.load.addTraffic(tlBinding, wimaxVideoUL)
    ###

except ImportError:
    pass


def createDLPoissonTraffic(simulator, rate, packetSize):
    rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
    rang = rangs[0]
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
            
    for ut in utNodes:
        poisDL = constanze.traffic.Poisson(offset = 0.05, 
            throughput = rate,
            packetSize = packetSize)

        ipBinding = IPBinding(rang.nl.domainName, ut.nl.domainName)
        rang.load.addTraffic(ipBinding, poisDL)

        ipListenerBinding = IPListenerBinding(ut.nl.domainName)
        listener = Listener(ut.nl.domainName + ".listener")
        ut.load.addListener(ipListenerBinding, listener)

def createULPoissonTraffic(simulator, rate, packetSize):
    rangs = simulator.simulationModel.getNodesByProperty("Type", "RANG")
    rang = rangs[0]
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
            
    for ut in utNodes:
        if rate > 0.0:
            poisUL = constanze.traffic.Poisson(offset = 0.0, 
                                            throughput = rate,
                                            packetSize = packetSize)
        else:
            # Send one PDU to establish connection
            poisUL = constanze.traffic.CBR0(offset = 5E-3,
                                            duration = 15E-3, 
                                            packetSize = packetSize, 
                                            throughput = 1.0)      
      
        ipBinding = IPBinding(ut.nl.domainName, rang.nl.domainName)
        ut.load.addTraffic(ipBinding, poisUL)

def setupPhy(simulator, config, scenario):
    import rise.scenario.Pathloss
    from openwns.interval import Interval

    if scenario == "InH":
        setupPhyDetail(simulator, 3400, dBm(24), dBm(21), config, dB(5), dB(7))
    elif scenario == "UMa":
        setupPhyDetail(simulator, 2000, dBm(46), dBm(24), config, dB(5), dB(7))
    elif scenario == "UMi":
        setupPhyDetail(simulator, 2500, dBm(41), dBm(24), config, dB(5), dB(7))
    elif scenario == "RMa":
        setupPhyDetail(simulator, 800, dBm(46), dBm(24), config, dB(5), dB(7))
    elif scenario == "SMa":
        setupPhyDetail(simulator, 2000, dBm(46), dBm(24), config, dB(5), dB(7))
    elif scenario == "LoS_Test":
        setupPhyDetail(simulator, 5470, dBm(30), dBm(30), config, dB(5), dB(5))
    else:
        raise "Unknown scenario %s" % scenario

def setupPhyDetail(simulator, freq, bsTxPower, utTxPower, config, rxNoiseBS, rxNoiseUT):

    from ofdmaphy.OFDMAPhy import OFDMASystem
    import rise.Scenario
    from rise.scenario import Shadowing
    from rise.scenario import FastFading
    import openwns.Scheduler
    import math
    import scenarios.channelmodel

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
    for node in bsNodes + utNodes:
    # TX frequency
        node.phy.ofdmaStation.txFrequency = freq
        node.phy.ofdmaStation.rxFrequency = freq

    # Noise figure 
    for bs in bsNodes:
        bs.phy.ofdmaStation.receiver[0].receiverNoiseFigure = rxNoiseBS

    for ut in utNodes:
        ut.phy.ofdmaStation.receiver[0].receiverNoiseFigure = rxNoiseUT

    # TX Power 
    numSubchannels = config.parametersPhy.subchannels
    power = fromdBm(bsTxPower)
    bsNominalTxPower = dBm(power - 10 * math.log10(numSubchannels))
    
    bsPower = openwns.Scheduler.PowerCapabilities(bsTxPower, bsNominalTxPower, bsNominalTxPower)
    
    if config.parametersPhy.adaptUTTxPower == True:
        # Per subchannel nominal power equals the max power divided by the number of subchannel and multiplied by the number of user terminal  since we assume all UTs being served in parallel
        power = fromdBm(utTxPower)
        numberOfUts = len(simulator.simulationModel.getNodesByProperty("isCenter", True)) - 1
        utNominalTxPower = dBm(power - 10 * math.log10(numSubchannels) + 10 * math.log10(numberOfUts))
        utPower = openwns.Scheduler.PowerCapabilities(utTxPower, utNominalTxPower, utNominalTxPower)
    else:
        # Per subchannel nominal power equals the max power since we assume UTs only use few subchannels
        utPower = openwns.Scheduler.PowerCapabilities(utTxPower, utTxPower, utTxPower)
    
    for bs in bsNodes:
        bs.dll.dlscheduler.config.txScheduler.registry.powerCapabilitiesAP = bsPower
        bs.dll.dlscheduler.config.txScheduler.registry.powerCapabilitiesUT = utPower
        bs.dll.ulscheduler.config.rxScheduler.registry.powerCapabilitiesAP = bsPower
        bs.dll.ulscheduler.config.rxScheduler.registry.powerCapabilitiesUT = utPower
        
        # For broadcasts
        bs.phy.ofdmaStation.txPower = bsTxPower
    
    for ut in utNodes:
        ut.dll.ulscheduler.config.txScheduler.registry.powerCapabilitiesAP = bsPower
        ut.dll.ulscheduler.config.txScheduler.registry.powerCapabilitiesUT = utPower
        ut.phy.ofdmaStation.txPower = bsTxPower


# begin example "wimac.tutorial.experiment2.staticFactory.substrategy.ProportionalFair.helper.py"
def setupScheduler(simulator, sched):
    import openwns.Scheduler
    
    if sched == "RoundRobin":
        scheduler = openwns.Scheduler.RoundRobin
        dsa = openwns.scheduler.DSAStrategy.LinearFFirst
    elif sched == "PropFair":
        scheduler = openwns.Scheduler.ProportionalFair
        dsa = openwns.scheduler.DSAStrategy.LinearFFirst
    elif sched == "ExhaustiveRR":
        scheduler = openwns.Scheduler.ExhaustiveRoundRobin
        dsa = openwns.scheduler.DSAStrategy.LinearFFirst
    elif sched == "Random":
        scheduler = openwns.Scheduler.RoundRobin
        dsa = openwns.scheduler.DSAStrategy.Random
    elif sched == "Fixed":
        scheduler = openwns.Scheduler.DSADrivenRR
        dsa = openwns.scheduler.DSAStrategy.Fixed
    else:
        raise "Unknown scheduler %s" % sched

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")

    for bs in bsNodes:
        for i in xrange(len(bs.dll.dlscheduler.config.txScheduler.strategy.subStrategies)):
            bs.dll.dlscheduler.config.txScheduler.strategy.subStrategies[i] = scheduler()
            bs.dll.dlscheduler.config.txScheduler.strategy.dsastrategy = dsa(
                oneUserOnOneSubChannel = True)

            bs.dll.ulscheduler.config.rxScheduler.strategy.subStrategies[i] = scheduler()
            bs.dll.ulscheduler.config.rxScheduler.strategy.dsastrategy = dsa(
                oneUserOnOneSubChannel = True)
# end example


def setupSchedulerDetails(simulator, packetScheduler, dsaStrategy):
    import openwns.Scheduler
    sched = packetScheduler
        
    if sched == "RoundRobin":
        scheduler = openwns.Scheduler.RoundRobin
    elif sched == "PropFair":
        scheduler = openwns.Scheduler.ProportionalFair
    elif sched == "ExhaustiveRR":
        scheduler = openwns.Scheduler.ExhaustiveRoundRobin
    elif sched == "DSADrivenRR":
        scheduler = openwns.Scheduler.DSADrivenRR
    else:
        raise "Unknown scheduler %s" % sched

    if dsaStrategy == "LinearFFirst":
        dsa = openwns.scheduler.DSAStrategy.LinearFFirst
    elif dsaStrategy == "Random":
        dsa = openwns.scheduler.DSAStrategy.Random
    elif dsaStrategy == "Fixed":
        dsa = openwns.scheduler.DSAStrategy.Fixed
    else:
        raise "Unknown DSA strategy %s" % dsaStr
    
    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")

    for bs in bsNodes:
        for i in xrange(len(bs.dll.dlscheduler.config.txScheduler.strategy.subStrategies)):
            bs.dll.dlscheduler.config.txScheduler.strategy.subStrategies[i] = scheduler()
            bs.dll.dlscheduler.config.txScheduler.strategy.dsastrategy = dsa(
                oneUserOnOneSubChannel = True)

            bs.dll.ulscheduler.config.rxScheduler.strategy.subStrategies[i] = scheduler()
            bs.dll.ulscheduler.config.rxScheduler.strategy.dsastrategy = dsa(
                oneUserOnOneSubChannel = True)


def disableIPHeader(simulator):
  
    rang = simulator.simulationModel.getNodesByProperty("Type", "RANG")
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
    
    for node in rang + utNodes:    
        node.nl.ipHeader.config.headerSize = 0
    
def setL2ProbeWindowSize(simulator, size):

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")
    
    for node in bsNodes + utNodes:
        node.dll.topTpProbe.config.windowSize = size
        node.dll.topTpProbe.config.sampleInterval = size

