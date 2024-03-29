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
    'src/Utilities.cpp',

    'src/frame/DataCollector.cpp',
    'src/frame/DLMapCollector.cpp',
    'src/frame/FrameHeadCollector.cpp',
    'src/frame/TimingControl.cpp',
    'src/frame/ULMapCollector.cpp',
    'src/parameter/PHY.cpp',
    'src/relay/RelayMapper.cpp',
    'src/scheduler/BypassQueue.cpp',
    'src/scheduler/Callback.cpp',
    'src/scheduler/DLCallback.cpp',
    'src/scheduler/PseudoBWRequestGenerator.cpp',
    'src/scheduler/RegistryProxyWiMAC.cpp',
    'src/scheduler/Scheduler.cpp',
    'src/scheduler/SpaceTimeSectorizationRegistryProxy.cpp',
    'src/scheduler/ULCallback.cpp',
    'src/services/AssociationControl.cpp',
    'src/services/ConnectionManager.cpp',
    'src/services/InterferenceCache.cpp',
    'src/services/QueueManager.cpp',
    'src/services/tests/InterferenceCacheTest.cpp',
    'src/StationManager.cpp',
    'src/tests/ACKSwitchTest.cpp',
    'src/WiMAC.cpp',
    'src/compoundSwitch/filter/RelayDirection.cpp',
    'src/helper/ContextProvider.cpp'
]
hppFiles = [
    'src/ACKSwitch.hpp',
    'src/CIRProvider.hpp',
    'src/Classifier.hpp',
    'src/Component.hpp',
    'src/compoundSwitch/filter/RelayDirection.hpp',
    'src/ConnectionIdentifier.hpp',
    'src/ConnectionKey.hpp',
    'src/ConnectionRule.hpp',
    'src/ErrorModelling.hpp',
    'src/frame/DataCollector.hpp',
    'src/frame/DLMapCollector.hpp',
    'src/frame/FrameHeadCollector.hpp',
    'src/frame/MapCommand.hpp',
    'src/frame/TimingControl.hpp',
    'src/frame/ULMapCollector.hpp',
    'src/FUConfigCreator.hpp',
    'src/Logger.hpp',
    'src/parameter/PHY.hpp',
    'src/PhyAccessFunc.hpp',
    'src/PhyModeProviderCommand.hpp',
    'src/PhyUserCommand.hpp',
    'src/PhyUser.hpp',
    'src/RANG.hpp',
    'src/relay/RelayMapper.hpp',
    'src/scheduler/BypassQueue.hpp',
    'src/scheduler/Callback.hpp',
    'src/scheduler/DLCallback.hpp',
    'src/scheduler/PseudoBWRequestGenerator.hpp',
    'src/scheduler/RegistryProxyWiMAC.hpp',
    'src/scheduler/Scheduler.hpp',
    'src/scheduler/Interface.hpp',
    'src/scheduler/SpaceTimeSectorizationRegistryProxy.hpp',
    'src/scheduler/ULCallback.hpp',
    'src/services/AssociationControl.hpp',
    'src/services/ConnectionManager.hpp',
    'src/services/InterferenceCache.hpp',
    'src/services/QueueManager.hpp',
    'src/services/IChannelQualityObserver.hpp',
    'src/StationManager.hpp',
    'src/UpperConvergence.hpp',
    'src/Utilities.hpp',
    'src/WiMAC.hpp',
    'src/helper/ContextProvider.hpp'
    ]

pyconfigs = [
    'wimac/FUs.py',
    'wimac/ErrorModelling.py',
    'wimac/Services.py',
    'wimac/CompoundSwitch.py',
    'wimac/KeyBuilder.py',
    'wimac/qos.py',
    'wimac/Rang.py',
    'wimac/WiMAC.py',
    'wimac/LLMapping.py',
    'wimac/PhyUser.py',
    'wimac/__init__.py',
    'wimac/Scheduler.py',
    'wimac/Relay.py',
    'wimac/FrameBuilder.py',
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
    'wimac/support/Parameters16m.py',
    'wimac/support/Nodes.py',
    'wimac/support/nodecreators.py',
    'wimac/support/helper.py'
    ]
dependencies = []
Return('libname srcFiles hppFiles pyconfigs dependencies')
