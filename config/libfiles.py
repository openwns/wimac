libname = 'wimac'
srcFiles = [
     # arranged alphabetically
    'src/ACKSwitch.cpp',
    'src/Classifier.cpp',
    'src/Component.cpp',
	'src/ConnectionIdentifier.cpp',
    'src/ConnectionKey.cpp',
    'src/ConnectionRule.cpp',
    'src/ErrorModelling.cpp',
    'src/Logger.cpp',
    'src/PhyAccessFunc.cpp',
    'src/PhyUser.cpp',
    'src/RANG.cpp',
    'src/UpperConvergence.cpp',
	'src/GuiWriter.cpp',
    'src/Utilities.cpp',

    'src/ProbeStartStop.cpp',
    'src/EventSubjectObserver.cpp',

    'src/frame/ContentionCollector.cpp',
    'src/frame/DataCollector.cpp',
    'src/frame/DLMapCollector.cpp',
    'src/frame/FrameHeadCollector.cpp',
    'src/frame/SingleCompoundCollector.cpp',
    'src/frame/TimingControl.cpp',
    'src/frame/ULMapCollector.cpp',
    'src/parameter/PHY.cpp',
    'src/PhyMode.cpp',
    'src/relay/RelayMapper.cpp',
    'src/scheduler/BypassQueue.cpp',
    'src/scheduler/Callback.cpp',
    'src/scheduler/DLCallback.cpp',
    'src/scheduler/PDUWatchProviderObserver.cpp',
    'src/scheduler/PseudoBWRequestGenerator.cpp',
    'src/scheduler/RegistryProxyWiMAC.cpp',
    'src/scheduler/Scheduler.cpp',
    'src/scheduler/SpaceTimeSectorizationRegistryProxy.cpp',
    'src/scheduler/ULCallback.cpp',
    'src/controlplane/Handover.cpp',
    'src/controlplane/MessageExchanger.cpp',
    'src/controlplane/Ranging.cpp',
    'src/controlplane/Scanning.cpp',
    'src/controlplane/SetupConnection.cpp',
    'src/services/Associating.cpp',
    'src/services/ConnectionControl.cpp',
    'src/services/ConnectionManager.cpp',
    'src/services/ControlPlaneManager.cpp',
    'src/services/ControlPlaneManagerSimple.cpp',
    'src/services/DeadStationDetect.cpp',
    'src/services/Dissociating.cpp',
    'src/services/FUReseter.cpp',
    'src/services/handoverStrategy/Averaging.cpp',
    'src/services/handoverStrategy/AverageWindow.cpp',
    'src/services/handoverStrategy/BestStation.cpp',
    'src/services/InterferenceCache.cpp',
    'src/services/QueueManager.cpp',
    'src/services/scanningStrategy/ScanningStrategy.cpp',
    'src/services/scanningStrategy/Interupted.cpp',
    'src/services/scanningStrategy/Plain.cpp',
    'src/services/tests/InterferenceCacheTest.cpp',
    'src/StationManager.cpp',
    'src/tests/ACKSwitchTest.cpp',
    'src/tests/PhyModeTest.cpp',
    'src/WiMAC.cpp',
    'src/compoundSwitch/CompoundSwitch.cpp',
    'src/compoundSwitch/CompoundSwitchConnector.cpp',
    'src/compoundSwitch/CompoundSwitchDeliverer.cpp',
    'src/compoundSwitch/Filter.cpp',
    'src/compoundSwitch/filter/FilterAll.cpp',
    'src/compoundSwitch/filter/FilterNone.cpp',
    'src/compoundSwitch/filter/FilterCommand.cpp',
    'src/compoundSwitch/filter/FilterFilterName.cpp',
    'src/compoundSwitch/filter/RelayDirection.cpp',
    'src/helper/ContextProvider.cpp'
]
hppFiles = [
    'src/ACKSwitch.hpp',
    'src/CIRMeasureInterface.hpp',
    'src/CIRProvider.hpp',
    'src/Classifier.hpp',
    'src/Code.hpp',
    'src/Component.hpp',
    'src/compoundSwitch/CompoundSwitchConfigCreator.hpp',
    'src/compoundSwitch/CompoundSwitchConnector.hpp',
    'src/compoundSwitch/CompoundSwitchDeliverer.hpp',
    'src/compoundSwitch/CompoundSwitch.hpp',
    'src/compoundSwitch/filter/FilterAll.hpp',
    'src/compoundSwitch/filter/FilterCommand.hpp',
    'src/compoundSwitch/filter/FilterFilterName.hpp',
    'src/compoundSwitch/filter/FilterNone.hpp',
    'src/compoundSwitch/Filter.hpp',
    'src/compoundSwitch/filter/RelayDirection.hpp',
    'src/ConnectionIdentifier.hpp',
    'src/ConnectionKey.hpp',
    'src/ConnectionRule.hpp',
    'src/controlplane/Handover.hpp',
    'src/controlplane/MessageExchanger.hpp',
    'src/controlplane/Ranging.hpp',
    'src/controlplane/Scanning.hpp',
    'src/controlplane/SetupConnection.hpp',
    'src/ErrorModelling.hpp',
    'src/EventSubjectObserver.hpp',
    'src/frame/ContentionCollector.hpp',
    'src/frame/DataCollector.hpp',
    'src/frame/DLMapCollector.hpp',
    'src/frame/FrameHeadCollector.hpp',
    'src/frame/MapCommand.hpp',
    'src/frame/SingleCompoundCollector.hpp',
    'src/frame/TimingControl.hpp',
    'src/frame/ULMapCollector.hpp',
    'src/GuiWriter.hpp',
    'src/FUConfigCreator.hpp',
    'src/Logger.hpp',
    'src/MACHeader.hpp',
    'src/Modulation.hpp',
    'src/parameter/PHY.hpp',
    'src/PhyAccessFunc.hpp',
    'src/PhyMode.hpp',
    'src/PhyModeProviderCommand.hpp',
    'src/PhyUserCommand.hpp',
    'src/PhyUser.hpp',
    'src/ProbeStartStop.hpp',
    'src/RANG.hpp',
    'src/relay/RelayMapper.hpp',
    'src/scheduler/BypassQueue.hpp',
    'src/scheduler/Callback.hpp',
    'src/scheduler/DLCallback.hpp',
    'src/scheduler/PDUWatchProviderObserver.hpp',
    'src/scheduler/PseudoBWRequestGenerator.hpp',
    'src/scheduler/RegistryProxyWiMAC.hpp',
    'src/scheduler/Scheduler.hpp',
    'src/scheduler/Interface.hpp',
    'src/scheduler/SpaceTimeSectorizationRegistryProxy.hpp',
    'src/scheduler/ULCallback.hpp',
    'src/services/Associating.hpp',
    'src/services/ConnectionControl.hpp',
    'src/services/ConnectionManager.hpp',
    'src/services/ControlPlaneManager.hpp',
    'src/services/ControlPlaneManagerInterface.hpp',
    'src/services/ControlPlaneManagerSimple.hpp',
    'src/services/DeadStationDetect.hpp',
    'src/services/Dissociating.hpp',
    'src/services/FUReseter.hpp',
    'src/services/handoverStrategy/AverageWindow.hpp',
    'src/services/handoverStrategy/Averaging.hpp',
    'src/services/handoverStrategy/BestStation.hpp',
    'src/services/handoverStrategy/Interface.hpp',
    'src/services/InterferenceCache.hpp',
    'src/services/QueueManager.hpp',
    'src/services/scanningStrategy/Interface.hpp',
    'src/services/scanningStrategy/Interupted.hpp',
    'src/services/scanningStrategy/Plain.hpp',
    'src/services/scanningStrategy/ScanningStrategy.hpp',
    'src/services/scanningStrategy/VersusInterfaceLayerConfigCreator.hpp',
    'src/StationManager.hpp',
    'src/tests/ConnectionManagerTest.hpp',
    'src/tests/ErrorModellingTest.hpp',
    'src/tests/PHYToolsTest.hpp',
    'src/tests/SchedulerTest2.hpp',
    'src/tests/SchedulerTest.hpp',
    'src/UpperConvergence.hpp',
    'src/Utilities.hpp',
    'src/WiMAC.hpp',
    'src/helper/ContextProvider.hpp'
    ]

pyconfigs = [
    'wimac/Component.py',
    'wimac/PhyMode.py',
    'wimac/Probes.py',
    'wimac/ControlPlaneManagerSimple.py',
    'wimac/HandoverStrategy.py',
    'wimac/FUs.py',
    'wimac/MessageExchanger.py',
    'wimac/ErrorModelling.py',
    'wimac/Services.py',
    'wimac/ControlPlaneManager.py',
    'wimac/Handover.py',
    'wimac/CompoundSwitch.py',
    'wimac/DeadStationDetect.py',
    'wimac/KeyBuilder.py',
    'wimac/Rang.py',
    'wimac/WiMAC.py',
    'wimac/ProbeStartStop.py',
    'wimac/LLMapping.py',
    'wimac/PhyUser.py',
    'wimac/ScanningStrategy.py',
    'wimac/__init__.py',
    'wimac/Scheduler.py',
    'wimac/Ranging.py',
    'wimac/Relay.py',
    'wimac/SetupConnection.py',
    'wimac/FrameBuilder.py',
    'wimac/Scanning.py',
    'wimac/FUReseter.py',
    'wimac/Layer2.py',
    'wimac/Stations.py',
    'wimac/evaluation/__init__.py',
    'wimac/evaluation/default.py',
    'wimac/support/FrameSetup.py',
    'wimac/support/__init__.py',
    'wimac/support/PostProcessor.py',
    'wimac/support/scenarioSupport.py',
    'wimac/support/Transceiver.py',
    'wimac/support/WiMACParameters.py',
    'wimac/support/Nodes.py'
    ]
dependencies = []
Return('libname srcFiles hppFiles pyconfigs dependencies')
