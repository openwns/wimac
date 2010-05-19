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

from openwns.evaluation import *
import string

class SeparateByRingAndType(openwns.evaluation.ITreeNodeGenerator):

    def __init__(self, forAll):
        self.forAll = forAll

    def __call__(self, pathname):
        for node in Accept(by = "MAC.Id", ifIn = self.forAll)(pathname):
            node.appendChildren(Separate(by = 'MAC.Ring', forAll = [1, 2, 3, 4], format = "MAC.Ring%d"))
            node.appendChildren(Enumerated(by = 'MAC.StationType',
                                           keys = [1, 2, 3],
                                           names = ['BS', 'RS', 'UT'],
                                           format = "MAC.StationType%s"))
            yield node
        
def installDebugEvaluation(sim, loggingStationIDs, kind = "PDF"):
    sources = ["wimac.top.window.incoming.bitThroughput", 
                "wimac.top.window.aggregated.bitThroughput", 
                "wimac.cirSDMA",
                "wimac.top.packet.incoming.delay",
                "wimac.top.packet.incoming.size",
                "wimac.top.packet.outgoing.size",
                "wimac.buffer.delay",
                "wimac.buffer.start.compoundSize",
                "wimac.buffer.stop.compoundSize",
                "wimac.schedulerQueue.delay",
                "wimac.frameOffsetDelay",
                "wimac.transmissionDelay",
                "wimac.crc.start.compoundSize",
                "wimac.crc.stop.compoundSize",
                "wimac.reassembly.minDelay",
                "wimac.reassembly.maxDelay",
                "wimac.reassembly.start.compoundSize",
                "wimac.reassembly.stop.compoundSize"]
                
    probedAtTx = ["wimac.buffer.delay",
                "wimac.buffer.start.compoundSize",
                "wimac.buffer.stop.compoundSize",
                "wimac.schedulerQueue.delay",
                "wimac.frameOffsetDelay",
                "wimac.transmissionDelay"]

    for src in sources:
        node = openwns.evaluation.createSourceNode(sim, src)
        nodeBS = node.appendChildren(openwns.evaluation.generators.Accept(
                            by = 'MAC.StationType', ifIn = [1], suffix = "BS"))
        #nodeRS = node.appendChildren(openwns.evaluation.generators.Accept(
        #                    by = 'MAC.StationType', ifIn = [2], suffix = "RS"))
        nodeUT = node.appendChildren(openwns.evaluation.generators.Accept(
                            by = 'MAC.StationType', ifIn = [3], suffix = "UT"))
        nodeBS.appendChildren(openwns.evaluation.generators.Separate(
                            by = 'MAC.Id', forAll = [1], format = "Id%d"))

        if src not in probedAtTx:
            nodeBS.appendChildren(openwns.evaluation.generators.Separate(
                            by = 'MAC.SourceId', forAll = loggingStationIDs, format = "FromId%d"))
        # Measurements collected at the transmitter are probed per target station
        else:
            nodeBS.appendChildren(openwns.evaluation.generators.Separate(
                            by = 'MAC.TargetId', forAll = loggingStationIDs, format = "ToId%d"))

        nodeUT.appendChildren(openwns.evaluation.generators.Separate(
                            by = 'MAC.Id', forAll = loggingStationIDs, format = "Id%d"))

        if kind == "Moments":                            
            node.getLeafs().appendChildren(openwns.evaluation.generators.Moments())
        else:
            if src == "wimac.cirSDMA":
                node.getLeafs().appendChildren(openwns.evaluation.generators.PDF(
                                                            minXValue = -100,
                                                            maxXValue = 100,
                                                            resolution =  2000))
            elif "window" in src:                          
                node.getLeafs().appendChildren(openwns.evaluation.generators.PDF(
                                                            minXValue = 0.0,
                                                            maxXValue = 120.0e+6,
                                                            resolution =  1000))
                                                
            elif "delay" in src:                          
                node.getLeafs().appendChildren(openwns.evaluation.generators.PDF(
                                                            minXValue = 0.0,
                                                            maxXValue = 0.25,
                                                            resolution =  25000))
                                                
            elif "Delay" in src:                          
                node.getLeafs().appendChildren(openwns.evaluation.generators.PDF(
                                                            minXValue = 0.0,
                                                            maxXValue = 0.01,
                                                            resolution =  10000))
                                                
            elif "size" in string.lower(src):                          
                node.getLeafs().appendChildren(openwns.evaluation.generators.PDF(
                                                            minXValue = 0.0,
                                                            maxXValue = 15000.0,
                                                            resolution =  1500))
        
        
def installEvaluation(sim, _accessPointIDs, _userTerminalIDs):

    sourceName = 'wimac.top.packet.incoming.bitThroughput'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
    node.getLeafs().appendChildren(PDF( name = sourceName,
                                        description = 'Top incoming bit throughput [bit/s]',
                                        minXValue = 0.0,
                                        maxXValue = 100e6,
                                        resolution =  1000))
    
    sourceName = 'wimac.top.packet.incoming.size'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
    node.getLeafs().appendChildren(PDF( name = sourceName,
                                        description = 'Top incoming bit size [bit]',
                                        minXValue = 0.0,
                                        maxXValue = 6000.0,
                                        resolution =  1000))
                    
    for direction in [ 'incoming', 'outgoing', 'aggregated' ]:
        for what in [ 'bit', 'compound' ]:
            sourceName = 'wimac.top.window.' + direction + '.' + what + 'Throughput'
            node = openwns.evaluation.createSourceNode(sim, sourceName)
            node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
            node.getLeafs().appendChildren(PDF(name = sourceName,
                                               description = 'Top %s %s throughput [bit/s]' % (direction, what),
                                               minXValue = 0.0,
                                               maxXValue = 120.0e+6,
                                               resolution =  1000))

    sourceName = 'wimac.cirSDMA'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
    node.getLeafs().appendChildren(PDF(name = sourceName,
                                       description = 'Carrier to Interference for data packets [dB]',
                                       minXValue = -40.0,
                                       maxXValue = 100.0,
                                       resolution =  1000))

    sourceName = 'wimac.buffer.size'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
    node.getLeafs().appendChildren(Moments(name = sourceName,
                                           description = 'Buffer fill level'))
    
    sourceName = 'wimac.buffer.lossRatio'
    node = openwns.evaluation.createSourceNode(sim, sourceName)
    node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
    node.getLeafs().appendChildren(Moments(name = sourceName,
                                           description = 'Loss Ratio'))
def installOverFrameOffsetEvaluation(sim, symbolsInFrame, _accessPointIDs, _userTerminalIDs):
  
    for sourceName in ['wimac.cirSDMA', 'wimac.interferenceSDMA', 'wimac.carrierSDMA']:
        try:
            node = openwns.evaluation.getSourceNode(sim, sourceName)
        except SourceNotRegistered:         
            node = openwns.evaluation.createSourceNode(sim, sourceName)
            node.appendChildren(SeparateByRingAndType(_accessPointIDs + _userTerminalIDs))
            
        node.getLeafs().appendChildren(Plot2D(xDataKey = 'OffsetFromFrameStart',
                                            minX = 0,
                                            maxX = symbolsInFrame - 1,
                                            resolution = symbolsInFrame - 1,
                                            statEvals = ['deviation','trials','mean']))

