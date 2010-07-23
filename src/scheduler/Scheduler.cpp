/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
 * email: info@openwns.org
 * www: http://www.openwns.org
 * _____________________________________________________________________________
 *
 * openWNS is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 2 as published by the
 * Free Software Foundation;
 *
 * openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/


#include <WIMAC/scheduler/Scheduler.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Deliverer.hpp>
#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/scheduler/strategy/Strategy.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/utils.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/scheduler/PseudoBWRequestGenerator.hpp>
#include <WIMAC/scheduler/Callback.hpp>
#include <WIMAC/parameter/PHY.hpp>
#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>
#include <WIMAC/FUConfigCreator.hpp>
#include <WIMAC/frame/ULMapCollector.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::scheduler::Scheduler,
	wimac::scheduler::Interface,
	"wimac.scheduler.Scheduler",
        wimac::FUConfigCreator );

using namespace wimac::scheduler;

Scheduler::Scheduler(wns::ldk::FunctionalUnit* parent, const wns::pyconfig::View& config) :
	plotFrames(config.get<bool>("plotFrames")),
	usedSlotDuration(0.0),
	offsetInSlot(0.0),
        slotDuration(config.get<double>("slotDuration")),
	freqChannels(config.get<int>("freqChannels")),
	maxBeams(config.get<int>("maxBeams")),
	beamforming(config.get<bool>("beamforming")),
    numberOfTimeSlots_(config.get<int>("numberOfTimeSlots")),
	uplink(config.get<bool>("uplink")),
	alwaysAcceptIfQueueAccepts(config.get<bool>("alwaysAcceptIfQueueAccepts")),
	logger("W-NS", "Scheduler",
	       wns::simulator::getMasterLogger()),
	ofdmaProvider(0),
	strategyName(config.get<std::string>("strategy.nameInStrategyFactory")),
	grouperName(config.get<std::string>("grouper.nameInGrouperFactory")),
	queueName(config.get<std::string>("queue.nameInQueueFactory")),
	registryName(config.get<std::string>("registry.nameInRegistryProxyFactory")),
	callbackName(config.get<std::string>("callback.__plugin__")),
	duration_(0.0),
	pduCount(0),
	frameNo(0),
	pyConfig(config),
	resetedBitsProbe(),
	resetedCompoundsProbe(),
	parent_(parent),
	accepting_(false),
	mapHandler_(0),
	mapHandlerName_( config.get<std::string>("mapHandlerName") )
{
    strategyResult_ = wns::scheduler::strategy::StrategyResultPtr();
    outputDir = "output";

    wns::probe::bus::ContextProviderCollection& cpc =
        parent_->getFUN()->getLayer()->getContextProviderCollection();

    resetedBitsProbe = wns::probe::bus::collector(cpc, config, "resettedBitsProbeBusName");
    resetedCompoundsProbe = wns::probe::bus::collector(cpc, config, "resettedCompoundsProbeBusName");

    colleagues.grouper = 0;
    colleagues.queue = 0;
    colleagues.strategy = 0;
    colleagues.registry = 0;
    colleagues.pseudoGenerator = 0;
    colleagues.callback = 0;

    if (!config.isNone("pseudoGenerator"))
    {
        colleagues.pseudoGenerator =
            new wimac::scheduler::PseudoBWRequestGenerator(config.getView("pseudoGenerator"));
    }

    wns::pyconfig::View registryView =
        pyConfig.get<wns::pyconfig::View>("registry");
    wns::scheduler::RegistryCreator* registryCreator =
        wns::scheduler::RegistryFactory::creator(registryName);
    colleagues.registry = dynamic_cast<wimac::scheduler::RegistryProxyWiMAC*>
        (registryCreator->create(parent_->getFUN(), registryView));
    assure(colleagues.registry, "Registry creation failed");
}

Scheduler::~Scheduler()
{
    if ( colleagues.strategy )
        delete colleagues.strategy;

    if ( colleagues.queue )
        delete colleagues.queue;

    if ( colleagues.grouper )
        delete colleagues.grouper;

    if ( colleagues.registry )
        delete colleagues.registry;

    if ( colleagues.callback )
        delete colleagues.callback;
    if ( colleagues.pseudoGenerator )
        delete colleagues.pseudoGenerator;
    strategyResult_= wns::scheduler::strategy::StrategyResultPtr();
}

void Scheduler::schedule(const wns::ldk::CompoundPtr& compound)
{
    assure(doIsAccepting(compound), "sendData called but not isAccepting");
    LOG_INFO("Forwarding accepted PDU to queue.");
    colleagues.queue->put(compound);
}

bool Scheduler::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
    if(alwaysAcceptIfQueueAccepts)
	return colleagues.queue->isAccepting(compound);
    else
        return accepting_ && colleagues.queue->isAccepting(compound);
}

void Scheduler::resetAllQueues()
{
    colleagues.queue->resetAllQueues();
}

void
Scheduler::notifyAboutConnectionDeleted(const ConnectionIdentifier cid)
{
    if( colleagues.queue->hasQueue(cid.getID()) )
    {
        LOG_INFO( parent_->getFUN()->getName(),
                  ": Scheduler deleting Queue for CID: ", cid.getID() );

        wns::scheduler::queue::QueueInterface::ProbeOutput probeOutput;
        probeOutput = colleagues.queue->resetQueue(cid.getID());

        // put Probe
        if(cid.connectionType_ == ConnectionIdentifier::Data)
            this->putProbe(probeOutput.bits, probeOutput.compounds);
    }
}

void
Scheduler::deliverSchedule(wns::ldk::Connector* connector)
{

    if (schedulerSpot_ == wns::scheduler::SchedulerSpot::ULSlave()) 
    {
        if (mapHandler_->resourcesGranted())
        {
            /****************** Scheduling Phase ****************************************/
            // trigger the scheduling process of the strategy module
            wns::scheduler::strategy::StrategyInput strategyInput(freqChannels, 
                slotDuration, 
                numberOfTimeSlots_, 
                maxBeams,
                NULL);

            strategyInput.beamforming = beamforming;
            strategyInput.setInputSchedulingMap(mapHandler_->getMasterMapForSlaveScheduling());

            strategyResult_ = wns::scheduler::strategy::StrategyResultPtr(
            new wns::scheduler::strategy::StrategyResult(colleagues.strategy->startScheduling(strategyInput))); 

            if (strategyResult_ == wns::scheduler::strategy::StrategyResultPtr())
            {
                LOG_INFO(parent_->getFUN()->getName(), 
                    ": ULSlave::deliverSchedule: Resources granted but no Compounds to schedule");
                return;//empty map do nothing
            }
            LOG_INFO(parent_->getFUN()->getName(), 
                ": ULSlave::deliverSchedule: ULSlaveCallback will now finalize the schedule.");
            colleagues.callback->callBack(strategyResult_->schedulingMap);
        }
        else
        {
            LOG_INFO(parent_->getFUN()->getName(), 
                ": ULSlave::deliverSchedule: No resources granted");
            return; //ULSlave scheduling not required without granted resources
        }
    }
    
    if (schedulerSpot_ != wns::scheduler::SchedulerSpot::ULMaster())     
        colleagues.callback->deliverNow(connector);
}

void
Scheduler::setupPlotting()
{
    std::string direction = uplink ? std::string("UL") : std::string("DL");
    std::stringstream configFilename;
    configFilename << parent_->getFUN()->getLayer()->getName() << "_"
                   <<  direction
                   << "_frame_" << frameNo << ".conf";

    boost::filesystem::path ssConf(outputDir);
    ssConf /= configFilename.str();
    std::stringstream ssPlot;


    boost::filesystem::ofstream conf(ssConf);

    conf << "[main]\n";
    conf << "FreqChannels=" << freqChannels << "\n"
         << "Beams=" << maxBeams << "\n"
         << "StartTime=0.0\n"
         << "EndTime=" << this->getDuration() << "\n";
    conf.close();

    plotFiles.clear();
    for (unsigned int i = 0; i < freqChannels; ++i)
    {
        plotFiles.push_back(new boost::filesystem::fstream);
        std::stringstream plotFilename;
        plotFilename << parent_->getFUN()->getLayer()->getName() << "_"
                     << direction
                     << "_frame_" << frameNo << ".plot." << i;
        boost::filesystem::path ssPlot(outputDir);
        ssPlot /= plotFilename.str();
        plotFiles[i]->open(ssPlot);
        // format flags for the timestamps: always print 9 digits for float,
        plotFiles[i]->flags( std::ios_base::fixed );
        plotFiles[i]->precision(9);
    }
}

void
Scheduler::startScheduling()
{
    if (plotFrames)
        setupPlotting();

    accepting_ = true;

    if (colleagues.pseudoGenerator)
        colleagues.pseudoGenerator->wakeup();
    else
        receptor_->wakeup();

    accepting_ = false;

    if(schedulerSpot_ == wns::scheduler::SchedulerSpot::ULSlave())
        return;

    assure(slotDuration * numberOfTimeSlots_ < getDuration(),
        "Too many resources (" << numberOfTimeSlots_ << " * " << slotDuration 
        << "s = " << slotDuration * numberOfTimeSlots_ 
        << "s) to fit in data phase of duration " << getDuration() << "s");

    /****************** Scheduling Phase ****************************************/
    // trigger the scheduling process of the strategy module
    wns::scheduler::strategy::StrategyInput strategyInput(freqChannels, 
        slotDuration, 
        numberOfTimeSlots_, 
        maxBeams,
        NULL);
        //colleagues.callback);

    strategyInput.beamforming = beamforming;

    strategyResult_ = wns::scheduler::strategy::StrategyResultPtr(
        new wns::scheduler::strategy::StrategyResult(colleagues.strategy->startScheduling(strategyInput))); 

    // TODO: move to Scheduler::finishCollection() 
    LOG_INFO(parent_->getFUN()->getName(), " Scheduler::finishCollection() in Scheduler::startScheduling(). numberOfTimeSlots_: ", numberOfTimeSlots_, " slotDuration: ", slotDuration);
    if (strategyResult_ == wns::scheduler::strategy::StrategyResultPtr())
    {
           return;//empty map do nothing
    }

    //ULMaster requires CallBack only with beamforming
    if(schedulerSpot_ != wns::scheduler::SchedulerSpot::ULMaster() || beamforming)
    { 
        colleagues.callback->callBack(strategyResult_->schedulingMap);
    }

    if (schedulerSpot_ == wns::scheduler::SchedulerSpot::ULMaster())
    {
        LOG_INFO(parent_->getFUN()->getName(), " deleteCompoundsInBursts()+deleteCompounds()");
        strategyResult_->deleteCompoundsInBursts(); // MapInfoCollection is not needed anymore
        strategyResult_->schedulingMap->deleteCompounds(); // compounds are not needed anymore
        strategyResult_->schedulingMap->grantFullResources(); // full time length on used subchannels (only for resourceUsage probe and plot)
        //strategyResult_->schedulingMap->processMasterMap(); // must be done in slave,peer,UT
    }
}

void
Scheduler::finishCollection() 
{ 
       LOG_INFO(parent_->getFUN()->getName(), " Scheduler::finishCollection(): ");
//     if (strategyResult_ == wns::scheduler::strategy::StrategyResultPtr() || schedulerSpot_==wns::scheduler::SchedulerSpot::ULMaster())
//     {
//            return; //empty map or ulmaster nothing to do
//     }
//     colleagues.callback->callBack(strategyResult_->schedulingMap);

    if (plotFrames) {
        for (unsigned int i = 0; i < freqChannels; ++i) {
            plotFiles[i]->close();
            delete plotFiles[i];
        }
    }
    frameNo++;
}

void Scheduler::setFUN(wns::ldk::fun::FUN* fun)
{
    assure(fun == parent_->getFUN(), "my fun and parent's fun do not match");


	LOG_INFO(parent_->getFUN()->getName(),
			 "Scheduler::setFUN() called and now setting up friends and colleagues");

	mapHandler_= fun->findFriend< wimac::frame::MapHandlerInterface*>(mapHandlerName_);
	assure( mapHandler_, "mapcollector not of type wimac::scheduler::MapHandler");

	// the first thing to do is to set up the registry because other colleagues
	// may depend on it for their initialization

	colleagues.registry->setFUN(fun);

	if (!ofdmaProvider) {

		friends_.phyUser = fun->findFriend<wimac::PhyUser*>("phyUser");
		assure(friends_.phyUser, "Could not get phyUser from my FUN");

		ofdmaProvider = friends_.phyUser->getDataTransmissionService();
		assure(ofdmaProvider, "Could not get OFDMA Provider from PhyUser");

	}

	friends_.classifier = fun->findFriend<wns::ldk::CommandTypeSpecifier
		<wns::ldk::ClassifierCommand> *>("classifier");
	assure(friends_.classifier, "Could not get the Classifier from my FUN");

	service::ConnectionManager* connectionManager = dynamic_cast<wimac::Component*>
		(parent_->getFUN()->getLayer())->getManagementService<service::ConnectionManager>("connectionManager");

	startObserving(connectionManager);

	colleagues.registry->setFriends(friends_.classifier);


	// create the modules
	wns::scheduler::grouper::SpatialGrouperCreator* grouperCreator =
		wns::scheduler::grouper::SpatialGrouperFactory::creator(grouperName);
	colleagues.grouper = grouperCreator->create(pyConfig.getView("grouper"));
	assure(colleagues.grouper, "Grouper creation failed");

	wns::pyconfig::View queueView = pyConfig.get<wns::pyconfig::View>("queue");
	wns::scheduler::queue::QueueCreator* queueCreator =
		wns::scheduler::queue::QueueFactory::creator(queueName);
        colleagues.queue = queueCreator->create( parent_, queueView );
	assure(colleagues.queue, "Queue creation failed");

    colleagues.queue->setFUN(fun);
    colleagues.queue->setColleagues(colleagues.registry);

	wns::scheduler::strategy::StrategyCreator* strategyCreator =
		wns::scheduler::strategy::StrategyFactory::creator(strategyName);
	colleagues.strategy = strategyCreator->create(pyConfig.get<wns::pyconfig::View>("strategy"));
	assure(colleagues.strategy, "Strategy module creation failed");

	wimac::scheduler::CallbackCreator* callbackCreator =
		wimac::scheduler::CallbackFactory::creator(callbackName);
	colleagues.callback = callbackCreator->create(fun, pyConfig.getView("callback"));
	assure(colleagues.callback, "Callback creation failed");

	// tell the modules who friends and colleagues are
	colleagues.grouper->setColleagues(colleagues.registry);
	colleagues.strategy->setColleagues(colleagues.queue,
					   colleagues.grouper,
					   colleagues.registry,
					   NULL
					   );

	colleagues.callback->setColleagues(colleagues.registry);
	colleagues.grouper->setFriends(ofdmaProvider);
	colleagues.strategy->setFriends(ofdmaProvider);

	if (colleagues.pseudoGenerator)
	{
		colleagues.pseudoGenerator->setFUN(fun);
		colleagues.pseudoGenerator->setScheduler(this);
	}
	schedulerSpot_ = colleagues.strategy->getSchedulerSpotType();
}


void
Scheduler::setProvider(wns::service::phy::ofdma::DataTransmission* _ofdmaProvider) {
	ofdmaProvider = _ofdmaProvider;
}

wns::scheduler::SchedulingMapPtr
Scheduler::getSchedulingMap() const {
    assure(strategyResult_, "StrategyResult not present");
    return strategyResult_->schedulingMap;
}

// wns::scheduler::MapInfoCollectionPtr
// Scheduler::getMapInfo() const {
//     assure(colleagues.strategy, "Strategy module not present");
// 
//     return colleagues.strategy->getMapInfo();
// }

int
Scheduler::getNumBursts() const {
    assure(strategyResult_, "Strategy module not present");
    int numOfSlots = 5;
    return strategyResult_->schedulingMap->getNumberOfSubChannels() * numOfSlots;
}

void
Scheduler::handleBroadcast()
{
/// @todo enable the broadcast again

//	bool frameFull = false;

// 	// do we have a broadcast queue with backlogged pdus?
// 	if (colleagues.queue->hasQueue(0))
// 	{
// 		if (colleagues.queue->queueHasPDUs(0))
// 		{
// 			do {
// 				const wns::service::phy::phymode::PhyModeInterface& phyMode
// 					= colleagues.registry->getPhyModeMapper()->getLowestPhyMode();

// 				double dataRate =
// 					//PHYTools::getBitsPerSymbol( PHYTools::BPSK12 ) * 1.0 / this->getFrameBuilder()->getSymbolDuration();
// 					//phyMode.getBitsPerSymbol() / this->getFrameBuilder()->getSymbolDuration();
// 					phyMode.getDataRate();

//				wns::scheduler::Bits size = colleagues.queue->getHeadOfLinePDUbits(0);

//				simTimeType duration = (double(size) / dataRate);


//				if (duration  < (this->getMaximumDuration() - usedSlotDuration))
//				{

//					// if the broadcast fits, get the pdu and set the relevant
//					// commands

//					wns::ldk::CompoundPtr pdu = colleagues.queue->getHeadOfLinePDU(0);

//					BroadcastPhyAccessFunc* func = new BroadcastPhyAccessFunc;
//					func->transmissionStart_ = usedSlotDuration;
//					func->transmissionStop_ =  usedSlotDuration + duration;


//					// set PhyUser command
//					wimac::PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>(
//						parent_->getFUN()->getProxy()->activateCommand( pdu->getCommandPool(), friends_.phyUser ) );

// 					phyCommand->local.pAFunc_.reset( func );

// 					phyCommand->peer.destination_ = 0;
// 					wimac::Component* wimacLayer = dynamic_cast<wimac::Component*>(parent_->getFUN()->getLayer());
// 					phyCommand->peer.cellID_ = wimacLayer->getCellID();
// 					phyCommand->peer.source_ = wimacLayer->getNode();
// 					//phyCommand->peer.phyMode_ = wimac::PHYTools::BPSK12;
// 					//phyCommand->peer.phyMode_ = colleagues.registry->getPhyModeMapper()->getLowestPhyMode(); // TODO
// 					phyCommand->peer.phyModePtr = &colleagues.registry->getPhyModeMapper()->getLowestPhyMode(); // TODO
// 					phyCommand->peer.measureInterference_ = false;
// 					phyCommand->magic.sourceLayer2_ = wimacLayer;

// 					if (plotFrames)
// 						*plotFiles[0] << usedSlotDuration << "\t" <<  usedSlotDuration + duration << "\t"
// 										  << float(0) << "\t" << 0.999 << "\t\"" << "BC" << "\"\n";


// 					this->pduWatch(pdu);  // Watch for special compounds to inform its observer
// 					scheduledPDUs.push(pdu);

// 					MESSAGE_SINGLE(NORMAL, logger, "Scheduled a broadcast PDU");

// 					usedSlotDuration += duration;
// 				}
// 				else
// 					frameFull = true;

// 			} while (colleagues.queue->queueHasPDUs(0) && !frameFull);
// 		}
// 	}
}

void
Scheduler::putProbe(int bits, int compounds)
{
    if(resetedBitsProbe)
        resetedBitsProbe->put(bits);

    if(resetedCompoundsProbe)
        resetedCompoundsProbe->put(compounds);
}

wns::scheduler::queue::QueueInterface* 
Scheduler::getQueue() const
{
    return colleagues.queue;    
}

