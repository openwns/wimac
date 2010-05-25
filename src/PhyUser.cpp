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
#include <WIMAC/GuiWriter.hpp>

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

    guiProbe_ = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(cpc, "wimac.guiProbe"));

    GuiWriter_ = new GuiWriter(guiProbe_, this);
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
}
PhyUser::PhyUser::~PhyUser()
{
    delete GuiWriter_;
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
    GuiWriter_->writeToProbe(compound,macaddr);
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

	cacheEntryTimeStamp = -maxAgeCacheEntry;
}



void
PhyUser::doOnData(const wns::ldk::CompoundPtr& compound)
{
	assure(compound, "doOnData called with an invalid compound.");

	PhyUserCommand* puCommand = getCommand( compound->getCommandPool() );
	LOG_INFO( getFUN()->getName(), ": doOnData source: ", puCommand->peer.source_->getName(),
			  "  C/I = ", puCommand->local.rxPower_, " / ", puCommand->local.interference_,
			  " = ", puCommand->local.rxPower_ / puCommand->local.interference_
			  );
	if(puCommand->peer.estimatedCandI_.I.get_mW() > 0)
	{
		LOG_INFO( "estimated C/I = ",
				  puCommand->peer.estimatedCandI_.C, " / ", puCommand->peer.estimatedCandI_.I,
				  " = " , puCommand->peer.estimatedCandI_.C / puCommand->peer.estimatedCandI_.I,
				  "\n estimated intra-cell interference: ", puCommand->getEstimatedIintra()
			);
	}
	else{
		LOG_INFO( "estimated C/I = ",
				  puCommand->peer.estimatedCandI_.C, " / ", puCommand->peer.estimatedCandI_.I,
				  "\n estimated intra-cell interference: ", puCommand->getEstimatedIintra()
			);
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

        wns::Power iInterPlusNoise;
        if(interference > puCommand->getEstimatedIintra()){
            iInterPlusNoise = interference - puCommand->getEstimatedIintra();
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
        probes_.interferenceFrameHead->put( interference.get_dBm() );
        probes_.cirFrameHead->put( rxPower.get_dBm() - interference.get_dBm() );

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
        probes_.interferenceContention->put( interference.get_dBm() );
        probes_.cirContention->put( rxPower.get_dBm() - interference.get_dBm() );
    }
    else if(puCommand->peer.destination_ && !puCommand->magic.contentionAccess_ && !puCommand->magic.frameHead_)
    { // Probe SDMA transmitted
        probes_.interferenceSDMA->put( interference.get_dBm() );
        probes_.carrierSDMA->put( rxPower.get_dBm() );
        probes_.cirSDMA->put( rxPower.get_dBm() - interference.get_dBm() );
        probes_.pathloss->put( txPower.get_dBm() - rxPower.get_dBm());
        LOG_INFO( "pathloss from PhyUser:",txPower.get_dBm() - rxPower.get_dBm());
    
		// TODO: puCommand->peer.phyModePtr
		/*
		unsigned int phyModeIndex =
			phyModeMapper->getIndexForPhyMode(*puCommand->peer.phyModePtr);
			// scheduler has phyUser as friend...
		probes_.deltaPHYModeSDMA->put(
			(puCommand->peer.phyMode_)
			- PHYTools::getPHYModeBySNR( rxPower.get_dBm() - interference.get_dBm() )
			);
		*/
		// probe the ratio of actual-to-estimated signal strength in dB
		probes_.deltaCarrierSDMA->put( rxPower.get_dBm() - puCommand->peer.estimatedCandI_.C.get_dBm() );
		probes_.deltaInterferenceSDMA->put( interference.get_dBm() - puCommand->peer.estimatedCandI_.I.get_dBm() );
	}else{
		assure(0, "PhyUser::onData: Received PDU can't be releated to a probe!");
	}

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
        assure(rngCI ,
                "PhyUser::filter: Can't filter Compounds without ConnectionIdentifier with CID=0");

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

