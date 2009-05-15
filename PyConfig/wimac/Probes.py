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

from speetcl.probes import ProbeFactory, ProbeModding, ProbeHelpers, AccessList, Probe
from speetcl.probes.StatEval import *
from speetcl.probes.AccessList import AccessList
from speetcl.probes.Probe import Probe
    
def getProbesDict(_accessPointIDs, _userTerminalIDs):
    SelfMadeProbes = {}
    SortedTextEvalProbes = {}
    SortedLogEvalProbes = {}
    PDFEvalProbes = {}
    SortedPDFEvalProbes = {}


    ###### Define access list for sorting criteria ##########
    ## Only add ids addId(). Don't use addRange(), otherwise
    ## it will crash in the generateSortingProbe() function.
    restriction = AccessList('MAC.Id')
    for i in _accessPointIDs:
        restriction.addId(i)
    for i in _userTerminalIDs:
        restriction.addId(i)

    alUTsAssociatedToAP = AccessList('MAC.UTAssocitedToAP')
    for i in _accessPointIDs:
        alUTsAssociatedToAP.addId(i)

    alQoSCategory = AccessList('MAC.UTQoSCategory')
    alQoSCategory.addId(0)  #  NoQoS
    alQoSCategory.addId(1)  #  Signaling
    alQoSCategory.addId(2)  #  UGS
    alQoSCategory.addId(3)  #  rtPS
    alQoSCategory.addId(4)  #  nrtPS
    alQoSCategory.addId(5)  #  BE

    alStationType = AccessList('MAC.StationType')
    alStationType.addId(1)  #  AccessPoint
    alStationType.addId(2)  #  FixedRelayStation
    alStationType.addId(3)  #  UserTerminal
    alStationType.addId(4)  #  RemoteTerminal


    ###### Define statEval objects ##########################
    statEval = {}

    statEval['Throughput'] = PDFEval()
    statEval['Throughput'].minXValue = 0.0
    statEval['Throughput'].maxXValue = 150000000.0
    statEval['Throughput'].resolution = 1000

    statEval['bitThroughput'] = PDFEval()
    statEval['bitThroughput'].minXValue = 0.0
    statEval['bitThroughput'].maxXValue = 200000000.0
    statEval['bitThroughput'].resolution = 1000
    statEval['bitThroughput'].format = 'scientific'

    statEval['compoundThroughput'] = PDFEval()
    statEval['compoundThroughput'].minXValue = 0.0
    statEval['compoundThroughput'].maxXValue = 200000.0
    statEval['compoundThroughput'].resolution = 1000
    statEval['compoundThroughput'].format = 'scientific'

    statEval['size'] = PDFEval()
    statEval['size'].minXValue = 0.0
    statEval['size'].maxXValue = 6000.0
    statEval['size'].resolution = 1000

    statEval['delay'] = PDFEval()
    statEval['delay'].minXValue = 0.0
    statEval['delay'].maxXValue = 2.0
    statEval['delay'].resolution = 1000

    statEval['bits'] = PDFEval()
    statEval['bits'].minXValue = 0.0
    statEval['bits'].maxXValue = 10000.0
    statEval['bits'].resolution = 10000

    statEval['compounds'] = PDFEval()
    statEval['compounds'].minXValue = 0.0
    statEval['compounds'].maxXValue = 500.0
    statEval['compounds'].resolution = 500

    statEval['handoverDuration'] = PDFEval()
    statEval['handoverDuration'].minXValue = 0.0
    statEval['handoverDuration'].maxXValue = 0.5
    statEval['handoverDuration'].resolution = 100

    statEval['interference'] = PDFEval()
    statEval['interference'].minXValue = -100.0
    statEval['interference'].maxXValue = -50.0
    statEval['interference'].resolution = 1000

    statEval['cir'] = PDFEval()
    statEval['cir'].minXValue = -40.0
    statEval['cir'].maxXValue = 100.0
    statEval['cir'].resolution = 1000

    statEval['carrier'] = PDFEval()
    statEval['carrier'].minXValue = -120.0
    statEval['carrier'].maxXValue = -10.0
    statEval['carrier'].resolution = 1000

    statEval['deltaSignal'] = PDFEval()
    statEval['deltaSignal'].minXValue = -30
    statEval['deltaSignal'].maxXValue = 30
    statEval['deltaSignal'].resolution = 1000

    statEval['deltaPHYMode'] = PDFEval()
    statEval['deltaPHYMode'].minXValue = -7
    statEval['deltaPHYMode'].maxXValue = 7
    statEval['deltaPHYMode'].resolution = 14
    statEval['deltaPHYMode'].format = 'fixed'

    statEval['groupingGain'] = PDFEval()
    statEval['groupingGain'].minXValue = 0.0
    statEval['groupingGain'].maxXValue = 10.0
    statEval['groupingGain'].resolution = 100

    statEval['bufferSize'] = PDFEval()
    statEval['bufferSize'].minXValue = 0.0
    statEval['bufferSize'].maxXValue = 1.0
    statEval['bufferSize'].resolution = 100

    statEval['bufferLossRatio'] = PDFEval()
    statEval['bufferLossRatio'].minXValue = 0.0
    statEval['bufferLossRatio'].maxXValue = 2000.0
    statEval['bufferLossRatio'].resolution = 1000

    statEval['CRCLossRatio'] = PDFEval()
    statEval['CRCLossRatio'].minXValue = 0.0
    statEval['CRCLossRatio'].maxXValue = 100000.0 #packets 
    statEval['CRCLossRatio'].resolution = 10000


    #### Packet Probes ####################################################
    for loc in [ 'top', 'controlPlane']:
        for unit in [ 'bitThroughput', 'size']:
            name = 'wimac.' + loc + '.packet.incoming.' + unit
            SelfMadeProbes[name] = generateSortingProbe(name,
                                                        statEval[unit],
                                                        restriction)

        for direction in [ 'incoming', 'outgoing' ]:
            name = 'wimac.' + loc + '.packet.' + direction + '.delay'
            SelfMadeProbes[name] = generateSortingProbe(name,
                                                        statEval['delay'],
                                                        restriction)
            
            name = 'wimac.' + loc + '.packet.' + direction + '.size'
            SelfMadeProbes[name] = generateSortingProbe(name,
                                                        statEval['size'],
                                                        restriction)



    #### Throughput Probes #################################################
    for loc in ['top', 'controlPlane']:
        for direction in [ 'incoming.', 'outgoing.', 'aggregated.' ]:
            for unit in['bitThroughput', 'compoundThroughput']:
                name = 'wimac.' + loc + '.window.' + direction + unit
                SelfMadeProbes[name] = generateSortingProbe(name,
                                                            statEval[unit],
                                                            restriction)

    loc = 'upperConvergenceDrop'
    direction = 'outgoing'
    for unit in['bits', 'compounds']:
        name = 'wimac.' + loc + '.' + direction + '.' + unit
        SelfMadeProbes[name] = generateSortingProbe(name,
                                                    statEval[unit],
                                                    restriction)



    ###### Special Probes ####################################################
    SelfMadeProbes['wimac.handoverDuration'] = generateSortingProbe('wimac.handoverDuration',
                                                              statEval['handoverDuration'],
                                                              restriction)


    for loc in ['buffer', 'schedulerQueue']:
        for unit in ['bits', 'compounds']:
            name = 'wimac.' + loc + '.reseted.' + unit
            SelfMadeProbes[name] = generateSortingProbe(name,
                                                        statEval[unit],
                                                        restriction)



    ###### Controlling Probes #################################################
    SortedLogEvalProbes['wimac.controlPlaneManagerFailure'] = { 'sorting': restriction }


    ## Signal measurement FrameHead
    SelfMadeProbes['wimac.interferenceFrameHead'] = generateSortingProbe(
        'wimac.interferenceFrameHead',
        statEval['interference'],
        restriction)
    SelfMadeProbes['wimac.cirFrameHead'] = generateSortingProbe(
        'wimac.cirFrameHead',
        statEval['cir'],
        restriction)

    ## Signal measurement contention access
    SelfMadeProbes['wimac.interferenceContention'] = generateSortingProbe(
        'wimac.interferenceContention',
        statEval['interference'],
        restriction)
    SelfMadeProbes['wimac.cirContention'] = generateSortingProbe(
        'wimac.cirContention',
        statEval['cir'],
        restriction)

    ## Signal measurement SDMA
    SelfMadeProbes['wimac.interferenceSDMA'] = generateSortingProbe(
        'wimac.interferenceSDMA',
        statEval['interference'],
        restriction)
    SelfMadeProbes['wimac.carrierSDMA'] = generateSortingProbe(
        'wimac.carrierSDMA',
        statEval['carrier'],
        restriction)
    SelfMadeProbes['wimac.cirSDMA'] = generateSortingProbe(
        'wimac.cirSDMA',
        statEval['cir'],
        restriction)
    SelfMadeProbes['wimac.deltaPHYModeSDMA'] = generateSortingProbe(
        'wimac.deltaPHYModeSDMA',
        statEval['deltaPHYMode'],
        restriction)
    SelfMadeProbes['wimac.deltaCarrierSDMA'] = generateSortingProbe(
        'wimac.deltaCarrierSDMA',
        statEval['deltaSignal'],
        restriction)
    SelfMadeProbes['wimac.deltaInterferenceSDMA'] = generateSortingProbe(
        'wimac.deltaInterferenceSDMA',
        statEval['deltaSignal'],
        restriction)


    SelfMadeProbes['groupingGain'] = generateSortingProbe(
        'groupingGain',
        statEval['groupingGain'],
        restriction)


    SelfMadeProbes['groupingGainUL'] = generateSortingProbe(
        'groupingGainUL',
        statEval['groupingGain'],
        restriction)


    SelfMadeProbes['wimac.buffer.size'] = generateSortingProbe(
        'wimac.buffer.size',
        statEval['bufferSize'],
        restriction)
    
    SelfMadeProbes['wimac.buffer.lossRatio'] = generateSortingProbe(
        'wimac.buffer.lossRatio',
         statEval['bufferLossRatio'],
        restriction)

    SelfMadeProbes['wimac.crc.CRCLossRatio'] = generateSortingProbe(
        'wimac.crc.CRCLossRatio',
        statEval['CRCLossRatio'],
        restriction)

    SelfMadeProbes['wimac.up.relayInject.size'] = generateSortingProbe(
        'wimac.up.relayInject.size',
        statEval['bufferSize'],
        restriction)
    
    SelfMadeProbes['wimac.down.relayInject.size'] = generateSortingProbe(
        'wimac.down.relayInject.size',
        statEval['bufferSize'],
        restriction)

    SelfMadeProbes['wimac.up.relayInject.lossRatio'] = generateSortingProbe(
        'wimac.up.relayInject.lossRatio',
        statEval['bufferLossRatio'],
        restriction)

    SelfMadeProbes['wimac.down.relayInject.lossRatio'] = generateSortingProbe(
        'wimac.down.relayInject.lossRatio',
        statEval['bufferLossRatio'],
        restriction)



    ###### Create Probes from Probe Factory ####################################
    SortedLogEvalProbes = ProbeFactory.withSortingCriterionByDict(
        ProbeHelpers.buildSortLogEvalFromDict(SortedLogEvalProbes) )

    PDFEvalProbes = ProbeFactory.withSortingCriterionByDict(
        ProbeHelpers.buildSortPDFEvalFromDict(PDFEvalProbes) )

    SortedPDFEvalProbes = ProbeFactory.withSortingCriterionByDict(
        ProbeHelpers.buildSortPDFEvalFromDict(SortedPDFEvalProbes) )

    SortedTextProbes = ProbeFactory.withSortingCriterionByDict(
        ProbeHelpers.buildTextEvalFromDict(SortedTextEvalProbes) )


    ###### Merge all Probes for return ########################################
    WiMACProbes = {}
    WiMACProbes.update(SelfMadeProbes)
    WiMACProbes.update(SortedLogEvalProbes)
    WiMACProbes.update(PDFEvalProbes)
    WiMACProbes.update(SortedPDFEvalProbes)
    WiMACProbes.update(SortedTextProbes)

    for (k,v) in WiMACProbes.items():
        ProbeModding.doNotIgnore(v)
    for key in ['wimac.controlPlane.packet.incoming.bitThroughput',
                'wimac.controlPlane.packet.incoming.size',
                'wimac.controlPlane.packet.incoming.delay',
                'wimac.controlPlane.packet.outgoing.delay',
                'wimac.controlPlane.window.incoming.bitThroughput',
                'wimac.controlPlane.window.incoming.compoundThroughput',
                'wimac.controlPlane.window.outgoing.bitThroughput',
                'wimac.controlPlane.window.outgoing.compoundThroughput',
                'wimac.controlPlane.window.aggregated.bitThroughput',
                'wimac.controlPlane.window.aggregated.compoundThroughput',
                'wimac.upperConvergenceDrop.outgoing.bits',
                'wimac.upperConvergenceDrop.outgoing.compounds',
                'wimac.handoverDuration',
                'wimac.schedulerQueue.reseted.bits',
                'wimac.schedulerQueue.reseted.compounds',
                'wimac.controlPlaneManagerFailure',
                'wimac.interferenceContention',
                'wimac.cirContention',
                'wimac.buffer.reseted.bits',
                'wimac.buffer.reseted.compounds',
                'wimac.top.window.aggregated.compoundThroughput',
                'wimac.top.window.incoming.compoundThroughput',
                'wimac.top.window.outgoing.compoundThroughput']:
        ProbeModding.doIgnore( WiMACProbes[key])
    return(WiMACProbes)



"""
   This Function generates and returns a Probe with the given name,
   statEval Object and sorted by the accessLists.
   The accessLists are two nested lists. (e.g. [[,][,]])

   The outside list commands the Probe to generate the output files
   for different accessLists.

   The inside list gernerates a branched sorting with the given 
   accessLists in the inside list.

   It is possible to have gaps in the idResolution space, but only add
   values to the AccessLists with the addId() methode, otherwise 
   this function will crash.


   e.g:
   ~~~~

   alAP               - AccessList with the Access Points (1,2,3)
   alUTAssociatedToAP - AccessList with the User Terminals (1,2,3)
   alQosCategory      - AccessList with the QoS category of the Terminal (1,3)

   generateSoringProbe("ProbeName",PDFEval(),
                       [ [alAP],[alUTAssociatedToAP,alQoSCategory] ] )


   Generated output files with the [alAP] AccessList:
   ProbeName_MAC.Id1....
   ProbeName_MAC.Id2....
   ProbeName_MAC.ID3....
   ProbeName_MAC.ID_....

   Generated output files with the [alUTAssociatedToAP,alQoSCategory] AccessList:
   ProbeName_MAC.UTAssociatedToAP1.QoSCategory1...
   ProbeName_MAC.UTAssociatedToAP1.QoSCategory3...
   ProbeName_MAC.UTAssociatedToAP2.QoSCategory1...
   ProbeName_MAC.UTAssociatedToAP2.QoSCategory3...
   ProbeName_MAC.UTAssociatedToAP3.QoSCategory1...
   ProbeName_MAC.UTAssociatedToAP3.QoSCategory3...
   ProbeName_MAC.UTAssociatedToAP_.QoSCategory1...
   ProbeName_MAC.UTAssociatedToAP_.QoSCategory3...
   ProbeName_MAC.UTAssociatedToAP_.QoSCategory_...


   The Files with the '_' (e.g: MAC.ID_) are a summarisation
   of all members of the corresponding AccessList.

"""
stationTypeAccessList =  AccessList(name = 'MAC.StationType')
stationTypeAccessList.addId(1)
stationTypeAccessList.addId(2)
stationTypeAccessList.addId(3)
stal1 = AccessList(name = 'BS')
stal1.addId(1)
stal2 = AccessList(name = 'RS')
stal2.addId(2)
stal3 = AccessList(name = 'UT')
stal3.addId(3)

ringAccessList = AccessList(name = 'MAC.Ring')
ringAccessList.addId(1)
ringAccessList.addId(2)
ringAccessList.addId(3)
ringAccessList.addId(4)

def generateSortingProbe(_name, _statEval, restriction):
    probe = Probe()
    probe.name = _name

    # SC1
    probe.addSortingCriterion(scmode="accessGroups")
    probe.sortingCriteria[0].addGroup(outputFormat = 'f',
                                      accessList = stationTypeAccessList,
                                      accessGroups = [ stal1, stal2, stal3 ])
    probe.sortingCriteria[0].addGroup(outputFormat = 'n',
                                      accessList = restriction,
                                      accessGroups = [restriction])
    probe.sortingCriteria[0].addStatEval(_statEval)

    # SC2
    probe.addSortingCriterion(scmode="idResolution")
    probe.sortingCriteria[1].addGroup(outputFormat = 'f',
                                       accessList = ringAccessList,
                                       idResolution = 1)
    probe.sortingCriteria[1].addGroup(outputFormat = 'n',
                                       accessList = restriction,
                                       idResolution = 0)
    probe.sortingCriteria[1].addStatEval(_statEval)
    
    return probe



"""
   This Function equals the generateSortingProbe(). But isn't
   able to gernerate Probes for a non continued idResolution space.
   The min and the max idResolution defines the space.
   In contrast to the generateSotingProbe(), this function has an easier
   implementation.
"""
def generateSortingProbeInAnEasyWay(_name, _statEval, _accessLists):
    probe = Probe()
    probe.name = _name

    i=0
    for als in _accessLists:

        ## create detailed sorted probes
        probe.addSortingCriterion(scmode='idResolution')
        for al in als:
            probe.sortingCriteria[i].addGroup(outputFormat = 'f',
                                              accessList = al,
                                              idResolution = 1)
        probe.sortingCriteria[i].addStatEval(_statEval)
        i += 1

        ###  create summarized probes
        probe.addSortingCriterion(scmode='idResolution')
        for al in als:
            if al == als[0]:
                sort = 'n'
                idr = 0
            else:
                sort = 'f'
                idr = 1
            probe.sortingCriteria[i].addGroup(outputFormat = sort,
                                              accessList = al,
                                               idResolution = idr)
        probe.sortingCriteria[i].addStatEval(_statEval)
        i += 1

    return(probe)
