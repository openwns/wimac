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
        setupPhyDetail(simulator, 3400, rise.scenario.Pathloss.ITUInH(), dBm(24), dBm(24), config, dB(5), dB(7))
    elif scenario == "UMa":
        setupPhyDetail(simulator, 2000, rise.scenario.Pathloss.ITUUMa(), dBm(46), dBm(24), config, dB(5), dB(7))
    elif scenario == "UMi":
        setupPhyDetail(simulator, 2500, rise.scenario.Pathloss.ITUUMi(), dBm(41), dBm(24), config, dB(5), dB(7))
    elif scenario == "RMa":
        setupPhyDetail(simulator, 800, rise.scenario.Pathloss.ITURMa(), dBm(46), dBm(24), config, dB(5), dB(7))
    elif scenario == "SMa":
        setupPhyDetail(simulator, 2000, rise.scenario.Pathloss.ITUSMa(), dBm(46), dBm(24), config, dB(5), dB(7))
    elif scenario == "LoS_Test":
        pl = rise.scenario.Pathloss.SingleSlope(
            validFrequencies = Interval(4000, 6000),
            validDistances = Interval(2, 20000),
            offset = "41.9 dB",
            freqFactor = 0,
            distFactor = "23.8 dB",
            distanceUnit = "m", 
            minPathloss = "49.06 dB",
            outOfMinRange = rise.scenario.Pathloss.Constant("49.06 dB"),
            outOfMaxRange = rise.scenario.Pathloss.Deny()
            )
        setupPhyDetail(simulator, 5470, pl, dBm(30), dBm(30), config, dB(5), dB(5))
    else:
        raise "Unknown scenario %s" % scenario

def setupPhyDetail(simulator, freq, pathloss, bsTxPower, utTxPower, config, rxNoiseBS, rxNoiseUT):

    from ofdmaphy.OFDMAPhy import OFDMASystem
    import rise.Scenario
    from rise.scenario import Shadowing
    from rise.scenario import FastFading
    import openwns.Scheduler
    import math

    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")
    utNodes = simulator.simulationModel.getNodesByProperty("Type", "UE")

    ofdmaPhySystem = OFDMASystem('ofdma')
    ofdmaPhySystem.Scenario = rise.Scenario.Scenario()
    simulator.modules.ofdmaPhy.systems.append(ofdmaPhySystem)

    # reconfiguring propagation model according to the selected scenario environment
    propagationConfigPairs = [("AP","AP"),("AP","FRS"),("AP","UT"),
                              ("UT","UT"),("UT","FRS"),("UT","AP"),
                              ("FRS","FRS"),("FRS","AP"),("FRS","UT")]

    # Large Scale fading model
    for node in bsNodes + utNodes:
        for pair in propagationConfigPairs:
            node.phy.ofdmaStation.receiver[0].propagation.setPair(pair[0],pair[1]).pathloss = pathloss
            node.phy.ofdmaStation.receiver[0].propagation.setPair(pair[0],pair[1]).shadowing = Shadowing.No()
            node.phy.ofdmaStation.receiver[0].propagation.setPair(pair[0],pair[1]).fastFading = FastFading.No()
            
            node.phy.ofdmaStation.transmitter[0].propagation.setPair(pair[0],pair[1]).pathloss = pathloss
            node.phy.ofdmaStation.transmitter[0].propagation.setPair(pair[0],pair[1]).shadowing = Shadowing.No()
            node.phy.ofdmaStation.transmitter[0].propagation.setPair(pair[0],pair[1]).fastFading = FastFading.No()
            
            
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

def setupScheduler(simulator, sched):
    import openwns.Scheduler
    
    if sched == "RoundRobin":
        scheduler = openwns.Scheduler.RoundRobin()
    elif sched == "PropFair":
        scheduler = openwns.Scheduler.ProportionalFair()
    else:
        raise "Unknown scheduler %s" % sched
    
    bsNodes = simulator.simulationModel.getNodesByProperty("Type", "BS")
    
    for bs in bsNodes:
        bs.dll.dlscheduler.config.txScheduler.strategy.subStrategies[0] = scheduler
        bs.dll.ulscheduler.config.rxScheduler.strategy.subStrategies[0] = scheduler
    

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

    
  
