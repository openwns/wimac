/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
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


#include <WIMAC/PhyUser.hpp>
#include <cmath>

#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/rng/RNGen.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/fcf/FrameBuilder.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/utils.hpp>


#include <WIMAC/Classifier.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/services/InterferenceCache.hpp>
#include <WIMAC/StationManager.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/frame/DataCollector.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(PhyUser, wns::ldk::FunctionalUnit, "wimac.PhyUser", wns::ldk::FUNConfigCreator);

PhyUser::PhyUser(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::CommandTypeSpecifier<PhyUserCommand>(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasDeliverer<>(),
    wns::Cloneable<PhyUser>(),
    cacheEntryTimeStamp(-1),
    maxAgeCacheEntry(1.0),
    friends_()
{
    friends_.interferenceCacheName = "interferenceCache";
    friends_.connectionManagerName = "connectionManager";
    friends_.connectionClassifierName = "classifier";
    friends_.dataCollectorName = "ulscheduler";

    friends_.layer = NULL;
    friends_.interferenceCache = NULL;
    friends_.connectionManager = NULL;
    friends_.connectionClassifier = NULL;

    // Probes configure
	wns::probe::bus::ContextProviderCollection* cpcParent = &fun->getLayer()->getContextProviderCollection();

    wns::probe::bus::ContextProviderCollection cpc(cpcParent);

    probes_.interferenceSDMA = 
        wns::probe::bus::collector(cpc, config, "iProbeName");

    probes_.cirSDMA = 
        wns::probe::bus::collector(cpc, config, "cirProbeName");

    probes_.carrierSDMA = 
        wns::probe::bus::collector(cpc, config, "cProbeName");

    probes_.deltaInterferenceSDMA =
        wns::probe::bus::collector(cpc, config, "deltaIProbeName");

    probes_.deltaCarrierSDMA = 
        wns::probe::bus::collector(cpc, config, "deltaCProbeName");

    probes_.PHYModeSDMA = 
        wns::probe::bus::collector(cpc, config, "phyModeProbeName");

    probes_.deltaPHYModeSDMA = 
        wns::probe::bus::collector(cpc, config, "deltaPhyProbeName");

    probes_.interferenceFrameHead = 
        wns::probe::bus::collector(cpc, config, "iFCHProbeName");

    probes_.cirFrameHead = 
        wns::probe::bus::collector(cpc, config, "cirFCHProbeName");

    probes_.interferenceContention = 
        wns::probe::bus::collector(cpc, config, "iContentionProbeName");

    probes_.cirContention = 
        wns::probe::bus::collector(cpc, config, "cirContentionProbeName");

    probes_.pathloss = 
        wns::probe::bus::collector(cpc, config, "pathlossProbeName");

    probes_.jsonTracing = 
        wns::probe::bus::collector(cpc, config, "phyTraceProbeName");

}


PhyUser::PhyUser( const PhyUser& rhs ):
	wns::ldk::CompoundHandlerInterface<FunctionalUnit>( rhs ),
	wns::ldk::CommandTypeSpecifierInterface( rhs ),
	wns::ldk::HasReceptorInterface( rhs ),
	wns::ldk::HasConnectorInterface( rhs ),
	wns::ldk::HasDelivererInterface( rhs ),
	wns::CloneableInterface( rhs ),
	wns::IOutputStreamable( rhs ),
	wns::PythonicOutput( rhs ),
	wns::ldk::FunctionalUnit( rhs ),
	wns::service::phy::ofdma::Handler( rhs ),
	wns::ldk::CommandTypeSpecifier<PhyUserCommand>( rhs ),
	wns::ldk::HasReceptor<>( rhs ),
	wns::ldk::HasConnector<>( rhs ),
	wns::ldk::HasDeliverer<>( rhs ),
	wns::Cloneable<PhyUser>( rhs ),
	cacheEntryTimeStamp(-1),
	maxAgeCacheEntry(1.0),
	friends_()
{
	friends_.interferenceCacheName = rhs.friends_.interferenceCacheName;
	friends_.connectionManagerName = rhs.friends_.connectionManagerName;
    friends_.connectionClassifierName = rhs.friends_.connectionClassifierName;
    friends_.dataCollectorName = rhs.friends_.dataCollectorName;

	friends_.layer = NULL;
	friends_.interferenceCache = NULL;
	friends_.connectionManager = NULL;


    probes_.interferenceSDMA = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.interferenceSDMA ));
    probes_.carrierSDMA = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.carrierSDMA ));
	probes_.cirSDMA = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.cirSDMA ));
	probes_.deltaPHYModeSDMA = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.deltaPHYModeSDMA));
    probes_.PHYModeSDMA = wns::probe::bus::ContextCollectorPtr(
    new wns::probe::bus::ContextCollector(
        *rhs.probes_.PHYModeSDMA));
    probes_.deltaInterferenceSDMA = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.deltaInterferenceSDMA ));
	probes_.deltaCarrierSDMA = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.deltaCarrierSDMA ));
	probes_.interferenceFrameHead = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.interferenceFrameHead ));
	probes_.cirFrameHead = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.cirFrameHead ));
	probes_.interferenceContention = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.interferenceContention ));
	probes_.cirContention = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.cirContention ));
    probes_.pathloss = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.pathloss ));
    probes_.jsonTracing = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(
            *rhs.probes_.jsonTracing ));
}
PhyUser::PhyUser::~PhyUser()
{
}


bool
PhyUser::doIsAccepting(const wns::ldk::CompoundPtr& /*compound*/) const
{
	return true;
}


void
PhyUser::doSendData(const wns::ldk::CompoundPtr& compound)
{
	COMMANDTYPE* command = getCommand( compound->getCommandPool() );
	(*command->local.pAFunc_.get())( this, compound );

    int macaddr = address.getInteger();
}


void PhyUser::onFUNCreated()
{
    friends_.layer = dynamic_cast<wimac::Component*>( getFUN()->getLayer() );
    assure(friends_.layer, "must be part of wimac::Component");
    friends_.interferenceCache = getFUN()->getLayer()
        ->getManagementService<service::InterferenceCache>(friends_.interferenceCacheName);

	friends_.connectionManager = getFUN()->getLayer()
		->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);
	friends_.connectionClassifier = getFUN()
		->findFriend<ConnectionClassifier*>(friends_.connectionClassifierName);

    // Get the Registry Proxy either from the TX (UT) or RX (BS) scheduler 
    if(getFUN()->findFriend<frame::DataCollector*>(friends_.dataCollectorName)
            ->getTxScheduler() != NULL)
    {
        friends_.registry = getFUN()
            ->findFriend<frame::DataCollector*>(friends_.dataCollectorName)
                ->getTxScheduler()->getRegistryProxy();
    }
    else
    {
        friends_.registry = getFUN()
            ->findFriend<frame::DataCollector*>(friends_.dataCollectorName)
                ->getRxScheduler()->getRegistryProxy();
    }
    assure(friends_.registry != NULL, "Unable to get RegistryProxy");

	cacheEntryTimeStamp = -maxAgeCacheEntry;
}



void
PhyUser::doOnData(const wns::ldk::CompoundPtr& compound)
{
	assure(compound, "doOnData called with an invalid compound.");

	PhyUserCommand* puCommand = getCommand( compound->getCommandPool() );
	LOG_INFO( getFUN()->getName(), ": doOnData source: ", puCommand->peer.source_->getName(),
			  "  C/I = ", puCommand->local.rxPower_, " / ", puCommand->local.interference_,
			  " = ", getCommand( compound->getCommandPool() )->magic.rxMeasurement->getSINR()
			  );
	if(puCommand->peer.estimatedCQI.interference.get_mW() > 0)
	{
		LOG_INFO( "estimated C/I = ",
				  puCommand->peer.estimatedCQI.carrier, " / ", puCommand->peer.estimatedCQI.interference,
				  " = " , puCommand->peer.estimatedCQI.carrier / puCommand->peer.estimatedCQI.interference);/*,
				  "\n estimated intra-cell interference: ", puCommand->getEstimatedIintra()
			);*/
	}
	else{
		LOG_INFO( "estimated C/I = ",
				  puCommand->peer.estimatedCQI.carrier, " / ", puCommand->peer.estimatedCQI.interference);/*,
				  "\n estimated intra-cell interference: ", puCommand->getEstimatedIintra()
			);*/
	}

	getDeliverer()->getAcceptor(compound)->onData(compound);
}

void
PhyUser::onData(wns::osi::PDUPtr pdu,
        wns::service::phy::power::PowerMeasurementPtr rxPowerMeasurement)
{
    wns::ldk::CompoundPtr compound = wns::staticCast<wns::ldk::Compound>(pdu);
    if(!getFUN()->getProxy()->commandIsActivated(
        compound->getCommandPool(), this))      
            return;

    PhyUserCommand* puCommand = getCommand( compound->getCommandPool() );

    // store measured signal into PhyUserCommand
    wns::Power rxPower          = rxPowerMeasurement->getRxPower();
    wns::Power txPower          = rxPowerMeasurement->getTxPower();
    wns::Power interference     = rxPowerMeasurement->getInterferencePower();
    wns::Power omniInterference = rxPowerMeasurement->getOmniInterferencePower();
    puCommand->local.rxPower_   = rxPower;
    //puCommand->local.txPower_   = txPower;
    puCommand->local.interference_ = interference;

    puCommand->magic.rxMeasurement = rxPowerMeasurement;

    // Only proceed on filtered compounds
    if ( !filter( compound ) )
        return;

    if ( puCommand->peer.measureInterference_ )
    {   // only for flaged transmissions
        // store C and I in sender's cache
        // The remote interferenceCache stores the averaged noise plus inter-cell
        // interference and the carrier signal strength separated by usedID.
        puCommand->magic.sourceComponent_
            ->getManagementService<service::InterferenceCache>
            ("interferenceCache")
            ->storeCarrier( friends_.layer->getNode(),
                            rxPower,
                            service::InterferenceCache::Remote );

        puCommand->magic.sourceComponent_
            ->getManagementService<service::InterferenceCache>
            ("interferenceCache")
            ->storePathloss( friends_.layer->getNode(),
                            rxPowerMeasurement->getPathLoss(),
                            service::InterferenceCache::Remote );

        wns::Power iInterPlusNoise;
        if(interference > wns::Power::from_mW(0.0)) { /*puCommand->getEstimatedIintra()){*/
            iInterPlusNoise = interference;/* - puCommand->getEstimatedIintra();*/
        }else{
            iInterPlusNoise = wns::Power::from_mW(0.0);
            LOG_INFO(getFUN()->getName(), " PhyUser: write iInterPlusNoise = null to interferenceCache");
        }

        puCommand->magic.sourceComponent_
            ->getManagementService<service::InterferenceCache>
            ("interferenceCache")
            ->storeInterference( friends_.layer->getNode(),
                                    iInterPlusNoise,
                                    service::InterferenceCache::Remote );

        cacheEntryTimeStamp = wns::simulator::getEventScheduler()->getTime();

        LOG_INFO(getFUN()->getName(), " wrote interference cache entry to sender ( ",
                    puCommand->magic.sourceComponent_->getFUN()->getName(), " )");
    }


    // Probes put
    if (!puCommand->peer.destination_ && !puCommand->magic.contentionAccess_ && puCommand->magic.frameHead_)
    { // Probe frameHead
        probes_.interferenceFrameHead->put(compound, interference.get_dBm() );
        probes_.cirFrameHead->put(compound, rxPower.get_dBm() - interference.get_dBm() );

        if(cacheEntryTimeStamp + maxAgeCacheEntry < wns::simulator::getEventScheduler()->getTime()){
            // write frame head C/I into interference cache
            puCommand->magic.sourceComponent_
                ->getManagementService<service::InterferenceCache>
                ("interferenceCache")
                ->storeCarrier( friends_.layer->getNode(),
                                rxPower,
                                service::InterferenceCache::Remote );

            puCommand->magic.sourceComponent_
                ->getManagementService<service::InterferenceCache>
                ("interferenceCache")
                ->storePathloss( friends_.layer->getNode(),
                                rxPowerMeasurement->getPathLoss(),
                                service::InterferenceCache::Remote );


            puCommand->magic.sourceComponent_
                ->getManagementService<service::InterferenceCache>
                ("interferenceCache")
                ->storeInterference( friends_.layer->getNode(),
                                        ( interference ),
                                        service::InterferenceCache::Remote );

            cacheEntryTimeStamp = wns::simulator::getEventScheduler()->getTime();

            LOG_INFO(getFUN()->getName(), " wrote FCH interference cache entry to sender ( ",
                        puCommand->magic.sourceComponent_->getFUN()->getName(), " )");

        }
    }
    else if(!puCommand->peer.destination_ && !puCommand->magic.contentionAccess_ && !puCommand->magic.frameHead_)
    { // Probe Broadcast transmissions without frameHead
        // Probe nothing
    }
    else if(puCommand->peer.destination_ && puCommand->magic.contentionAccess_ && !puCommand->magic.frameHead_)
    { // Probe contention based access
        probes_.interferenceContention->put(compound, interference.get_dBm() );
        probes_.cirContention->put(compound, rxPower.get_dBm() - interference.get_dBm() );
    }
    else if(puCommand->peer.destination_ && !puCommand->magic.contentionAccess_ && !puCommand->magic.frameHead_)
    { // Probe SDMA transmitted
        probes_.interferenceSDMA->put(compound, interference.get_dBm() );
        probes_.carrierSDMA->put(compound, rxPower.get_dBm() );
        probes_.cirSDMA->put(compound, rxPower.get_dBm() - interference.get_dBm() );
        probes_.pathloss->put(compound, txPower.get_dBm() - rxPower.get_dBm());
        LOG_INFO( "pathloss from PhyUser:",txPower.get_dBm() - rxPower.get_dBm());
    
        /* Probe deviation between possible and chosen PHY mode*/
		int phyModeIndex =
			friends_.registry->getPhyModeMapper()->
                getIndexForPhyMode(*puCommand->peer.phyModePtr);
        int possiblePhyModeIndex = 
            friends_.registry->getPhyModeMapper()->getIndexForPhyMode(
                *friends_.registry->getPhyModeMapper()->getBestPhyMode(rxPower / interference));

		probes_.deltaPHYModeSDMA->put(compound, possiblePhyModeIndex - phyModeIndex);

		// probe the ratio of actual-to-estimated signal strength in dB
		probes_.deltaCarrierSDMA->put(compound, 
            rxPower.get_dBm() - puCommand->peer.estimatedCQI.carrier.get_dBm());
		probes_.deltaInterferenceSDMA->put(compound, 
            interference.get_dBm() - puCommand->peer.estimatedCQI.interference.get_dBm());
	}else{
		assure(0, "PhyUser::onData: Received PDU can't be releated to a probe!");
	}

#ifndef NDEBUG
    traceIncoming(compound, rxPowerMeasurement);
#endif

	//Deliver compound
	doOnData(compound);
}

void
PhyUser::doWakeup()
{
	// calls wakeup method of upper functional unit(s)
	getReceptor()->wakeup();
}



void
PhyUser::setDataTransmissionService(wns::service::Service* phy)
{
	assure(phy, "must be non-NULL");
	assureType(phy, wns::service::phy::ofdma::DataTransmission*);
	dataTransmission = dynamic_cast<wns::service::phy::ofdma::DataTransmission*>(phy);
}



wns::service::phy::ofdma::DataTransmission*
PhyUser::getDataTransmissionService() const
{
	assure(dataTransmission, "no ofdma::DataTransmission set. Did you call setDataTransmission()?");
	return dataTransmission;
}



void
PhyUser::setNotificationService(wns::service::Service* phy)
{
	assure(phy, "must be non-NULL");
	assureType(phy, wns::service::phy::ofdma::Notification*);
	notificationService = dynamic_cast<wns::service::phy::ofdma::Notification*>(phy);
	notificationService->registerHandler(this);
}



wns::service::phy::ofdma::Notification*
PhyUser::getNotificationService() const
{
	assure(notificationService, "no ofdma::Notification set. Did you call setNotificationService()?");
	return notificationService;
}



void
PhyUser::setMACAddress(const wns::service::dll::UnicastAddress& _address)
{
	address = _address;
	LOG_INFO( "setting MAC address of PhyUser to: ", address );
}

bool PhyUser::filter( const wns::ldk::CompoundPtr& compound)
{
    PhyUserCommand* phyCommand = getCommand( compound->getCommandPool() );


    // reject own compounds
    if ( phyCommand->peer.source_ == friends_.layer->getNode() )
        return false;


    // SS should receive all broadcasts
    if( friends_.layer->getStationType() != wns::service::dll::StationTypes::AP() )
    {
        ConnectionIdentifier::Ptr rngCI;
        rngCI = friends_.connectionManager->getConnectionWithID(0);

        if(rngCI == NULL)
        {
            if(phyCommand->magic.frameHead_)
            {
                // Receive frame head from other BSs while not associated
                return true;
            }
            else
            {
                // Do not receive other broadcasts like MAPs
                return false;
            }

        }
        if ( !phyCommand->peer.destination_       //broadcast
                && ( phyCommand->magic.sourceComponent_->getID()
                    == rngCI->baseStation_ )                // from our BaseStation
            )
        {
            return true;
        }
    }

    // Receive all compounds for us
    if ( phyCommand->peer.destination_ )  //no broadcast
    {
        if ( phyCommand->peer.destination_ == friends_.layer->getNode() ) //for us
        {
            // return true;

            wns::ldk::ClassifierCommand* cCommand;
            cCommand = friends_.connectionClassifier->getCommand(compound->getCommandPool());

            if(   (friends_.connectionManager->getConnectionWithID(cCommand->peer.id))
                || (cCommand->peer.id == 0) )
            {   //Only receive compounds for a registered CID
                ///ToDo: (gra) This isn't a good behavoir and the wrong
                ///place for it. Compounds for an outdated / delted
                ///ConnectionIdentifier shouldn't be sent.
                ///It could happen, if the subscriber station does a
                ///reset and delete all ConnectionIdentifier. The base
                ///station doesn't know it and sends compounds on the old
                ///ConnectionIdentifiers.
                return true;
            }
        }
    }

    return false;
}

void
PhyUser::traceIncoming(wns::ldk::CompoundPtr compound, wns::service::phy::power::PowerMeasurementPtr rxPowerMeasurement)
{
    wns::probe::bus::json::Object objdoc;

    PhyUserCommand* myCommand = getCommand(compound->getCommandPool());

    objdoc["Transmission"]["ReceiverID"] = 
        wns::probe::bus::json::String(getFUN()->getLayer()->getNodeName());
    objdoc["Transmission"]["SenderID"] = 
        wns::probe::bus::json::String(myCommand->peer.source_->getName());
    objdoc["Transmission"]["SourceID"] = 
        wns::probe::bus::json::String(myCommand->peer.source_->getName());
    if(myCommand->peer.destination_ == NULL)
    {
        objdoc["Transmission"]["DestinationID"] = 
            wns::probe::bus::json::String("Broadcast");
    }
    else
    {
        objdoc["Transmission"]["DestinationID"] = 
            wns::probe::bus::json::String(myCommand->peer.destination_->getName());
    }

    objdoc["Transmission"]["Start"] = 
        wns::probe::bus::json::Number(myCommand->local.pAFunc_->transmissionStart_);
    objdoc["Transmission"]["Stop"] = 
        wns::probe::bus::json::Number(myCommand->local.pAFunc_->transmissionStop_);
    objdoc["Transmission"]["Subchannel"] = 
        wns::probe::bus::json::Number(myCommand->local.pAFunc_->subBand_);
    objdoc["Transmission"]["TxPower"] = 
        wns::probe::bus::json::Number(rxPowerMeasurement->getTxPower().get_dBm());
    objdoc["Transmission"]["RxPower"] = 
        wns::probe::bus::json::Number(rxPowerMeasurement->getRxPower().get_dBm());
    objdoc["Transmission"]["InterferencePower"] = 
        wns::probe::bus::json::Number(rxPowerMeasurement->getInterferencePower().get_dBm());

    if (myCommand->peer.estimatedCQI.carrier != wns::Power() &&
        myCommand->peer.estimatedCQI.interference != wns::Power())
    {
        objdoc["SINREst"]["C"] = 
            wns::probe::bus::json::Number(myCommand->peer.estimatedCQI.carrier.get_dBm());
        objdoc["SINREst"]["I"] = 
            wns::probe::bus::json::Number(myCommand->peer.estimatedCQI.interference.get_dBm());
    }
    wns::probe::bus::json::probeJSON(probes_.jsonTracing, objdoc);
}


