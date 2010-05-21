from wimac.Layer2 import *
import wimac.Scheduler
import wimac.FUReseter
import wimac.FUs
import openwns.SAR
import openwns.Tools
import math
from wimac.FrameBuilder import ActivationAction, OperationMode
import wimac.support.FrameSetup as FrameSetup

class Association:
    def __init__(self,source, destination, id):
        self.source = source
        self.destination = destination
        self.id = id


class BaseStation(Layer2):

    subscriberStations = None
    relayStations = None

    def __init__(self, node, config) : #, registryProxy = wimac.Scheduler.RegistryProxyWiMAC):
        super(BaseStation, self).__init__(node, "BS", config)
        myFrameSetup = FrameSetup.FrameSetup(config)

        self.ring = 1
        self.qosCategory = 'NoQoS'

        # BaseStation specific components
        self.upperconvergence = wimac.FUs.UpperConvergence()
        self.stationType = "AP"

        # frame elements
        self.framehead = wimac.FrameBuilder.FrameHeadCollector('frameBuilder')
        self.dlmapcollector = wimac.FrameBuilder.DLMapCollector('frameBuilder', 'dlscheduler')
        self.ulmapcollector = wimac.FrameBuilder.ULMapCollector('frameBuilder', 'ulscheduler')

        subStrategies = []
        subStrategies.append(openwns.Scheduler.RoundRobin())

        dsastrategy  = openwns.scheduler.DSAStrategy.LinearFFirst(oneUserOnOneSubChannel = True)
        dsafbstrategy= openwns.scheduler.DSAStrategy.LinearFFirst(oneUserOnOneSubChannel = True)
        apcstrategy  = openwns.scheduler.APCStrategy.UseNominalTxPower()          

        strategyDL = openwns.Scheduler.StaticPriority(
            parentLogger = self.logger, 
            txMode = True,  
            subStrategies = subStrategies, 
            dsastrategy = dsastrategy, 
            dsafbstrategy = dsafbstrategy, 
            apcstrategy = apcstrategy)     

        self.dlscheduler = wimac.FrameBuilder.DataCollector('frameBuilder')
        self.dlscheduler.txScheduler = wimac.Scheduler.Scheduler(
            "frameBuilder",
            config.parametersPhy.symbolDuration,
            slotDuration = config.parametersPhy.slotDuration,
            dataSubCarrier = config.parametersPhy.subcarrierPerSubchannel,
            mapHandlerName = "dlmapcollector",
            strategy = strategyDL,
            freqChannels = config.parametersPhy.subchannels,
            maxBeams = 1, # Currently no beamforming
            beamforming = False,
            #friendliness_dBm = config.friendliness_dBm,
            plotFrames = False,
            uplink = False,
            callback = wimac.Scheduler.DLCallback(
                    beamforming = False, 
                    slotLength = config.parametersPhy.slotDuration),
                    numberOfTimeSlots = config.numberOfTimeSlots)

        self.dlscheduler.txScheduler.strategy.logger.enabled = True
        self.dlscheduler.txScheduler.strategy.dsastrategy.logger.enabled = True
        self.dlscheduler.txScheduler.strategy.dsafbstrategy.logger.enabled = True

        strategyUL = openwns.Scheduler.StaticPriority(
            parentLogger = self.logger, 
            txMode = False,  
            subStrategies = subStrategies, 
            dsastrategy = dsastrategy, 
            dsafbstrategy = dsafbstrategy, 
            apcstrategy = apcstrategy)

        self.ulscheduler = wimac.FrameBuilder.DataCollector('frameBuilder')
        self.ulscheduler.rxScheduler = wimac.Scheduler.Scheduler(
            "frameBuilder", 
            config.parametersPhy.symbolDuration,
            slotDuration = config.parametersPhy.slotDuration,
            dataSubCarrier = config.parametersPhy.subcarrierPerSubchannel,
            mapHandlerName = "ulmapcollector",
            strategy = strategyUL,
            freqChannels = config.parametersPhy.subchannels,
            maxBeams = 1, # Cureently no beamforming
            beamforming =  False,
            #friendliness_dBm = config.friendliness_dBm,
            callback = wimac.Scheduler.ULMasterCallback(slotLength = config.parametersPhy.slotDuration),
            plotFrames = False,
            uplink = True,
            numberOfTimeSlots = config.numberOfTimeSlots
            )
        self.ulscheduler.rxScheduler.strategy.logger.enabled = True
        self.ulscheduler.rxScheduler.strategy.dsastrategy.logger.enabled = True
        self.ulscheduler.rxScheduler.strategy.dsafbstrategy.logger.enabled = True

        self.dlscheduler.txScheduler.queue = openwns.Scheduler.SegmentingQueue(
            "deSegAndDeConcat",
            "deSegAndDeConcat",
            self.logger,
            minimumSegmentSize = 1,
            fixedHeaderSize = 0,
            extensionHeaderSize = 0,
            delayProbeName = self.schedQueueTick.commandName)

        # Use the QueueProxy in UL Master Scheduler
        queueManager = wimac.Services.QueueManager("queueManager", "connectionManager", self.logger)
        self.managementServices.append(queueManager)                      
        
        self.ulscheduler.rxScheduler.queue = openwns.Scheduler.QueueProxy(
            queueManager.serviceName,
            True,
            self.logger)
            
        self.ulscheduler.rxScheduler.queue.setSegmentingQueueConfig(
            openwns.Scheduler.SegQueueConfig("deSegAndDeConcat"))
                    
        self.setupFrame(config)
        self.fun = FUN()
        self.buildFUN(config)
        self.connect()

    def connect(self):
        # Connections Dataplane
        self.upperconvergence.connect(self.topTpProbe)
        self.topTpProbe.connect(self.topPProbe)

        self.topPProbe.connect(self.bufferTick)
        self.bufferTick.connect(self.classifier)
        self.classifier.connect(self.synchronizer)
        self.synchronizer.connect(self.flowSeparator)
        self.flowSeparator.connect(self.bufferTack)
        self.bufferTack.connect(self.schedQueueTick)
        self.schedQueueTick.connect(self.reassemblyTick)
        self.reassemblyTick.connect(self.crcTack)
        self.crcTack.connect(self.crc)
        self.crc.connect(self.crcTick)
        self.crcTick.connect(self.errormodelling)
        self.errormodelling.connect(self.dlscheduler)
        self.errormodelling.upConnect(self.ulscheduler)

        self.framehead.connect(self.frameBuilder)
        self.dlmapcollector.connect(self.frameBuilder)
        self.ulmapcollector.connect(self.frameBuilder)
        self.dlscheduler.connect(self.frameBuilder)
        self.ulscheduler.connect(self.frameBuilder)
        self.frameBuilder.connect(self.phyUser)

    def setupFrame(self, config):

        myFrameSetup = FrameSetup.FrameSetup(config)

        # Insert Activations into Timing Control
        # first, configure only real compound collectors
        activation = wimac.FrameBuilder.Activation('framehead',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.frameHeadLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlmapcollector',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.dlMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulmapcollector',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.ulMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlscheduler',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.dlDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.ulDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlscheduler',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.FinishCollection ),
                                                   myFrameSetup.dlDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.FinishCollection ),
                                                   myFrameSetup.ulDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        # second, now set phase durations
        activation = wimac.FrameBuilder.Activation('framehead',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.frameHeadLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlmapcollector',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.dlMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulmapcollector',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.ulMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlscheduler',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.dlDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        # name of compound collector (here "ttg") is just informative
        activation = wimac.FrameBuilder.Activation('ttg',
                                                   OperationMode( OperationMode.Pausing ),
                                                   ActivationAction( ActivationAction.Pause ),
                                                   myFrameSetup.ttgLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.ulDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        # next activations could be a pause, too
        # but they might be exchanged by real compound collector in the future
        activation = wimac.FrameBuilder.Activation('bwReq',
                                                   OperationMode( OperationMode.Pausing ),
                                                   ActivationAction( ActivationAction.Pause ),
                                                   myFrameSetup.bwReqLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ranging',
                                                   OperationMode( OperationMode.Pausing ),
                                                   ActivationAction( ActivationAction.Pause ),
                                                   myFrameSetup.rangingLength )
        self.frameBuilder.timingControl.addActivation( activation )

        # final phase RTG is modeled as simple gap only (no Timing Control activation)


class SubscriberStation(Layer2):
    forwarder = None

    def __init__(self, node, config) : #, registryProxy = wimac.Scheduler.RegistryProxyWiMAC() ):
        super(SubscriberStation, self).__init__(node, "SS", config)       
        self.stationType = "UT"

        # frame elements
        self.framehead = wimac.FrameBuilder.FrameHeadCollector('frameBuilder')
        self.dlmapcollector = wimac.FrameBuilder.DLMapCollector('frameBuilder', None)
        self.ulmapcollector = wimac.FrameBuilder.ULMapCollector('frameBuilder', None)

        self.dlscheduler = wimac.FrameBuilder.DataCollector('frameBuilder')
        self.dlscheduler.rxScheduler = None

        self.ulscheduler = wimac.FrameBuilder.DataCollector('frameBuilder')

        dsastrategyULSlave_  = openwns.scheduler.DSAStrategy.DSASlave(oneUserOnOneSubChannel = True)
        dsafbstrategyULSlave_  = openwns.scheduler.DSAStrategy.DSASlave(oneUserOnOneSubChannel = True)
        apcstrategy_  = openwns.scheduler.APCStrategy.UseNominalTxPower()

        subStrategiesTXUL_ = []
        subStrategiesTXUL_.append(openwns.Scheduler.RoundRobin())
        	
        strategyUL = openwns.Scheduler.StaticPriority(
                parentLogger = self.logger, 
                txMode = True, 
                subStrategies = subStrategiesTXUL_, 
                dsastrategy = dsastrategyULSlave_, 
                dsafbstrategy = dsafbstrategyULSlave_, 
                apcstrategy = apcstrategy_)

        self.ulscheduler.txScheduler = wimac.Scheduler.Scheduler(
            "frameBuilder",
            config.parametersPhy.symbolDuration,
            slotDuration = config.parametersPhy.slotDuration,
            dataSubCarrier = config.parametersPhy.subcarrierPerSubchannel,
            mapHandlerName = "ulmapcollector",
            strategy = strategyUL,
            freqChannels = config.parametersPhy.subchannels,
            maxBeams = 1, # Currently no beamforming
            beamforming =  False,
            #friendliness_dBm = config.friendliness_dBm,
            callback = wimac.Scheduler.ULSlaveCallback(slotLength = config.parametersPhy.slotDuration),
            plotFrames = False,
            uplink = True,
            numberOfTimeSlots = config.numberOfTimeSlots
            )
        self.ulscheduler.txScheduler.strategy.logger.enabled = True
        self.ulscheduler.txScheduler.strategy.dsastrategy.logger.enabled = True
        self.ulscheduler.txScheduler.strategy.dsafbstrategy.logger.enabled = True
        
        self.ulscheduler.txScheduler.queue = openwns.Scheduler.SegmentingQueue(
            "deSegAndDeConcat",
            "deSegAndDeConcat",
            self.logger,
            minimumSegmentSize = 1,
            fixedHeaderSize = 0,
            extensionHeaderSize = 0,
            delayProbeName = self.schedQueueTick.commandName)

        
        self.setupFrame(config)
        self.fun = FUN()
        self.buildFUN(config)
        self.connect()


    def associate(self, destination):
        assert isinstance(destination, Layer2)
        it = Association(self, destination, destination.getAssociationID())
        self.associations.append(it)
        self.associateTo = destination.stationID
        self.qosCategory = 'BE'
        self.ring = destination.ring + 1
        self.connectionControl.associateTo(destination.stationID)

    def connect(self):
        # Connections Dataplane
        self.upperconvergence.connect(self.topTpProbe)
        self.topTpProbe.connect(self.topPProbe)
        self.topPProbe.connect(self.bufferTick)
        self.bufferTick.connect(self.classifier)
        self.classifier.connect(self.synchronizer)
        self.synchronizer.connect(self.flowSeparator)
        self.flowSeparator.connect(self.bufferTack)
        self.bufferTack.connect(self.schedQueueTick)
        self.schedQueueTick.connect(self.reassemblyTick)
        self.reassemblyTick.connect(self.crcTack)
        self.crcTack.connect(self.crc)
        self.crc.connect(self.crcTick)
        self.crcTick.connect(self.errormodelling)
        self.errormodelling.upConnect(self.dlscheduler)
        self.errormodelling.connect(self.ulscheduler)

        self.framehead.connect(self.frameBuilder)
        self.dlmapcollector.connect(self.frameBuilder)
        self.ulmapcollector.connect(self.frameBuilder)
        self.dlscheduler.connect(self.frameBuilder)
        self.ulscheduler.connect(self.frameBuilder)
        self.frameBuilder.connect(self.phyUser)


    def setupFrame(self, config):

        myFrameSetup = FrameSetup.FrameSetup(config)

        # Insert Activations into Timing Control
        # first, configure only real compound collectors
        activation = wimac.FrameBuilder.Activation('framehead',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.frameHeadLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlmapcollector',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.dlMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulmapcollector',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.ulMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.dlDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulscheduler',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.StartCollection ),
                                                   myFrameSetup.ulDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.FinishCollection ),
                                                   myFrameSetup.dlDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.FinishCollection ),
                                                   myFrameSetup.ulDataLength )
        self.frameBuilder.timingControl.addActivation( activation )


        # second, now set phase durations
        activation = wimac.FrameBuilder.Activation('framehead',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.frameHeadLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlmapcollector',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.dlMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulmapcollector',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.ulMapLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('dlscheduler',
                                                   OperationMode( OperationMode.Receiving ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.dlDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ttg',
                                                   OperationMode( OperationMode.Pausing ),
                                                   ActivationAction( ActivationAction.Pause ),
                                                   myFrameSetup.ttgLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ulscheduler',
                                                   OperationMode( OperationMode.Sending ),
                                                   ActivationAction( ActivationAction.Start ),
                                                   myFrameSetup.ulDataLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('bwReq',
                                                   OperationMode( OperationMode.Pausing ),
                                                   ActivationAction( ActivationAction.Pause ),
                                                   myFrameSetup.bwReqLength )
        self.frameBuilder.timingControl.addActivation( activation )

        activation = wimac.FrameBuilder.Activation('ranging',
                                                   OperationMode( OperationMode.Pausing ),
                                                   ActivationAction( ActivationAction.Pause ),
                                                   myFrameSetup.rangingLength )
        self.frameBuilder.timingControl.addActivation( activation )

