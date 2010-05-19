from openwns.pyconfig import Sealed
from openwns.FUN import FUN, Node
from openwns.FlowSeparator import FlowSeparator
from openwns.Multiplexer import Dispatcher
from math import ceil

import openwns.ldk
import wimac.KeyBuilder

from openwns.FUN import FUN, Node
from openwns.FlowSeparator import FlowSeparator

import openwns.Probe
import openwns.Buffer
import openwns.ARQ
import openwns.SAR
import openwns.Tools
import openwns.FCF
import openwns.CRC
import wimac.Component
import wimac.FrameBuilder
import wimac.KeyBuilder
import wimac.ErrorModelling
import wimac.PhyUser
import wimac.Services
import wimac.FUs


class Layer2(wimac.Component.Component):
    frameBuilder = None
    subFUN = None
    group = None

    # Probes
    topTpProbe = None
    topPProbe = None
    bufferTick = None
    bufferTack = None
    schedQueueTick = None
    crcTick = None
    crcTack = None
    reassemblyTick = None

    # DataPlane
    upperconvergence = None
    classifier = None
    synchronizer = None

    FlowSeparator = None
    crc = None
    errormodelling = None
    compoundSwitch = None
    phyUser = None

    # functional units for scheduling
    framehead = None
    dlmapcollector = None
    ulmapcollector = None
    dlscheduler = None
    ulscheduler = None

    connectionControl = None
    associateTo = None
    qosCategory = None
    randomStartDelayMax = None

    def __init__(self, node, stationName, config):
        super(Layer2, self).__init__(node, stationName)
        self.nameInComponentFactory = "wimac.Component"

        self.associations = []
        self.randomStartDelayMax = 0.0
        
        self.upperconvergence = wimac.FUs.UpperConvergence()
        
        self.frameBuilder = openwns.FCF.FrameBuilder(0, wimac.FrameBuilder.TimingControl(),
            frameDuration = config.parametersPhy.frameDuration,
            symbolDuration = config.parametersPhy.symbolDuration )
        
        self.managementServices.append(
            wimac.Services.ConnectionManager( "connectionManager", "fuReseter" ) )  

        interferenceCache = wimac.Services.InterferenceCache( 
            "interferenceCache", alphaLocal = 0.2, alphaRemote= 0.05 ) 
        interferenceCache.notFoundStrategy.averageCarrier = "-88.0 dBm"
        interferenceCache.notFoundStrategy.averageInterference = "-96.0 dBm"
        interferenceCache.notFoundStrategy.deviationCarrier = "0.0 mW"
        interferenceCache.notFoundStrategy.deviationInterference = "0.0 mW"
        interferenceCache.notFoundStrategy.averagePathloss = "0.0 dB"
        self.managementServices.append( interferenceCache )

        self.connectionControl = wimac.Services.ConnectionControl("connectionControl") 
        self.controlServices.append( self.connectionControl )
        
        self.classifier = wimac.FUs.Classifier()
        self.synchronizer = openwns.Tools.Synchronizer()

        self.subFUN = openwns.FUN.FUN()
        subFUNbuffer = openwns.FUN.Node('buffer', openwns.Buffer.Dropping( size = 320000,
                                       sizeUnit = 'Bit',
                                       lossRatioProbeName = "wimac.buffer.lossRatio",
                                       sizeProbeName = "wimac.buffer.size"))
        
        # Only used for reassembly in receiver. Segmentation is done in scheduler queue
        subFUNsegAndConcat = openwns.FUN.Node('deSegAndDeConcat', openwns.SAR.SegAndConcat(
                                        0, 0, "deSegAndDeConcat", "wimac.reassembly", 
                                        self.logger))
                                        
        # Must fit worst case: An IP-PDU gets split between two frames
        # Should be kept low as long as we do not have HARQ
        subFUNsegAndConcat.config.reorderingWindow.tReordering = 0.015
        # Should work even if packet size is one bit 
        subFUNsegAndConcat.config.reorderingWindow.snFieldLength = 20 
                                        
        self.subFUN.add(subFUNbuffer)
        self.subFUN.add(subFUNsegAndConcat) 
        subFUNbuffer.connect(subFUNsegAndConcat)
        
        self.group = openwns.Group.Group(self.subFUN, 'buffer', 'deSegAndDeConcat')
        
        creator = openwns.FlowSeparator.Config('flowSeparatorPrototype', self.group)
        ifNotFoundStrategy = openwns.FlowSeparator.CreateOnFirstCompound(creator)
        self.flowSeparator = openwns.FlowSeparator.FlowSeparator(
                                            wimac.KeyBuilder.CIDKeyBuilder(),
                                            ifNotFoundStrategy)
        
        self.reassemblyTick = openwns.Probe.Tick("wimac.reassembly", probeOutgoing = False,
            parentLogger = self.logger)

        self.crcTack = openwns.Probe.Tack("wimac.crc", probeOutgoing = False,
            parentLogger = self.logger)

        # size of CRC command is abused to model overhead due to entire MAC header (48 bit without CRC)
        self.crc = openwns.CRC.CRC("errormodelling",
                               lossRatioProbeName = "wimac.crc.CRCLossRatio",
                               CRCsize = config.parametersMAC.pduOverhead,
                               isDropping = False)
                               
        self.crcTick = openwns.Probe.Tick("wimac.crc", probeOutgoing = False,
            parentLogger = self.logger)
        
        self.errormodelling = wimac.ErrorModelling.ErrorModelling('phyUser','phyUser',
                                config.parametersPhy.symbolDuration, config.parametersPhy.dataSubCarrier, PrintMappings=False)
        self.compoundSwitch = wimac.CompoundSwitch.CompoundSwitch()

        self.phyUser = wimac.PhyUser.PhyUser(
            centerFrequency = config.parametersSystem.centerFrequency,
            bandwidth = config.parametersPhy.channelBandwidth,
            numberOfSubCarrier = config.parametersPhy.subchannels )

        self.topTpProbe = openwns.Probe.Window( "TopTp", "wimac.top", windowSize=0.005 )        
        self.topPProbe = openwns.Probe.Packet( "TopP", "wimac.top" )
        self.bufferTick = openwns.Probe.Tick("wimac.buffer", probeOutgoing = True,
            parentLogger = self.logger)
        self.bufferTack = openwns.Probe.Tack("wimac.buffer", probeOutgoing = True, 
            parentLogger = self.logger)
        self.schedQueueTick = openwns.Probe.Tick("wimac.schedulerQueue", probeOutgoing = True,
            parentLogger = self.logger)


    def buildFUN(self, config):
        #DataPlane
        self.upperconvergence = Node(self.upperConvergenceName, self.upperconvergence)
        self.topTpProbe = Node('topTpProbe', self.topTpProbe)
        self.topPProbe = Node('topPProbe', self.topPProbe)
        self.classifier = Node('classifier', self.classifier)
        self.synchronizer = Node('synchronizer', self.synchronizer)
        self.flowSeparator = Node('bufferSep', self.flowSeparator)
        self.crc = Node('crc', self.crc)
        self.errormodelling = Node('errormodelling', self.errormodelling)
        self.phyUser = Node('phyUser', self.phyUser)
        self.framehead = Node('framehead', self.framehead)
        self.dlmapcollector = Node('dlmapcollector', self.dlmapcollector)
        self.ulmapcollector = Node('ulmapcollector', self.ulmapcollector)
        self.dlscheduler = Node('dlscheduler', self.dlscheduler)
        self.ulscheduler = Node('ulscheduler', self.ulscheduler)
        self.frameBuilder = Node('frameBuilder', self.frameBuilder)

        #Dataplane
        self.compoundSwitch = Node('compoundSwitchWiMAX', self.compoundSwitch)

        self.fun.setFunctionalUnits(
        self.compoundSwitch,
        self.upperconvergence,
        self.topTpProbe,
        self.topPProbe,
        self.bufferTick,
        self.bufferTack,
        self.schedQueueTick,
        self.classifier,   
        self.synchronizer,
        self.reassemblyTick,
        self.crcTack,
        self.crc,
        self.crcTick,
        self.errormodelling,
        self.phyUser,
        self.flowSeparator,
        self.framehead,
        self.dlmapcollector,
        self.ulmapcollector,
        self.dlscheduler,
        self.ulscheduler,
        self.frameBuilder,
        )





