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

#include <WIMAC/frame/ULMapCollector.hpp>

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/fcf/TimingControl.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/PhyUserCommand.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/frame/DataCollector.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/parameter/PHY.hpp>
using namespace wimac::frame;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::frame::ULMapCollector,
    wns::ldk::FunctionalUnit,
    "wimac.frame.ULMapCollector",
    wns::ldk::FUNConfigCreator );


struct UserFind :
    public std::unary_function<wns::scheduler::MapInfoEntryPtr, bool>
{
    explicit UserFind( wns::scheduler::UserID user ) : user_(user){}
    bool operator()(const wns::scheduler::MapInfoEntryPtr& mapInfo)
    {
        return user_ == mapInfo->user;
    }
private:
    wns::scheduler::UserID user_;
};

ULMapCollector::ULMapCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config ) :
    wns::ldk::fcf::CompoundCollector( config ),
    wns::ldk::CommandTypeSpecifier<ULMapCommand>(fun),
    ulSchedulerName_(),
    phyUser_(0),
    phyMode(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
            (wns::service::phy::phymode::createPhyMode( config.getView("phyMode") ) ) ),
    //hasUplinkBurst_(false),
    //myBursts_(new wns::scheduler::MapInfoCollection)
    ulResourcesAvailable_(false),
    scheduledULMap_(wns::scheduler::SchedulingMapPtr()),
    estimatedCQI_(wns::scheduler::ChannelQualityOnOneSubChannel())

{
    if (!config.isNone("ulSchedulerName") )
        ulSchedulerName_ = config.get<std::string>("ulSchedulerName");
}


void ULMapCollector::onFUNCreated()
{
    wns::ldk::fcf::CompoundCollector::onFUNCreated();

    assureType(getFUN()->getLayer(), wimac::Component*);
    component_ = dynamic_cast<wimac::Component*>(getFUN()->getLayer());

    if ( ulSchedulerName_ != "" )
    {
        ulScheduler_ = dynamic_cast<wimac::scheduler::Scheduler*>
            (getFUN()->findFriend<DataCollector*>(ulSchedulerName_)->getRxScheduler());
        assure( ulScheduler_, "Uplink Scheduler not present in FUN" );
    }

    phyUser_ = getFUN()->findFriend<wimac::PhyUser*>("phyUser");
    assure( phyUser_, "PhyUser is not of type wimac::PhyUser");

    connectionManager_ =
        component_->getManagementService<service::ConnectionManager>("connectionManager");

    setFrameBuilder( getFUN()->findFriend<wns::ldk::fcf::FrameBuilder*>("frameBuilder") );

    me_ = wns::scheduler::UserID(component_->getNode());

    CompoundCollector::onFUNCreated();
}

void ULMapCollector::doStart(int mode)
{
    switch (mode) {
    case Sending:
    {
        wns::ldk::CompoundPtr compound
            ( new wns::ldk::Compound( getFUN()->getProxy()->createCommandPool() ) );
        ULMapCommand* command = activateCommand( compound->getCommandPool() );
        command->peer.phaseDuration =
            ulScheduler_->getDuration();
        command->peer.baseStationID =
            component_->getID();
        command->local.numBursts =
            ulScheduler_->getNumBursts();
        //command->peer.mapInfo =
          //  ulScheduler_->getMapInfo();
        command->peer.schedulingMap =
            ulScheduler_->getSchedulingMap();
        // map duration is a little shorter than the phase duration
        command->local.mapDuration =
            getCurrentDuration() - Utilities::getComputationalAccuracyFactor();

        assure( command->local.mapDuration <= this->getMaximumDuration(),
                "ULMapCollector: PDU overun the maximum duration of the frame phase!");

        PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>
            ( getFUN()->getProxy()->activateCommand( compound->getCommandPool(), phyUser_) );
        phyCommand->peer.destination_ = 0;
        phyCommand->peer.cellID_ = component_->getCellID();
        phyCommand->peer.source_ = component_->getNode();
        assureNotNull(phyMode.getPtr());
        phyCommand->peer.phyModePtr = phyMode;
        phyCommand->magic.sourceComponent_ = component_;

        wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();
        phyCommand->local.pAFunc_.reset
            ( new BroadcastPhyAccessFunc );
        phyCommand->local.pAFunc_->transmissionStart_ = now;
        phyCommand->local.pAFunc_->transmissionStop_ = now + getCurrentDuration()
             - Utilities::getComputationalAccuracyFactor();
        phyCommand->local.pAFunc_->phyMode_ = phyMode;

        setTimeout( getCurrentDuration() - Utilities::getComputationalAccuracyFactor() );
        LOG_INFO( getFUN()->getLayer()->getName(), " send UL Map of size: ", command->local.numBursts );
        //LOG_INFO( getFUN()->getLayer()->getName(), " send UL Map of size: ", command->local.numBursts, " / ", command->peer.mapInfo->size() );
        getConnector()->getAcceptor( compound )->sendData( compound );
    }
    case Receiving:
        /* wait and do nothing */
	ulResourcesAvailable_ = false;
        break;
    default:
        throw wns::Exception("Unknown activation mode in DLMapCollector");
    }
}

void
ULMapCollector::onTimeout()
{
    getFrameBuilder()->finishedPhase( this );
}

void
ULMapCollector::calculateSizes( const wns::ldk::CommandPool* commandPool, Bit& commandPoolSize, Bit& dataSize ) const
{
    //What are the sizes in the upper Layers
    /** @todo: update to 16e or 16m MAP size */
    getFUN()->getProxy()->calculateSizes(commandPool, commandPoolSize, dataSize, this);

    ULMapCommand* command = getCommand( commandPool );
    commandPoolSize += ( 56 + command->local.numBursts * 48 );
}

wns::simulator::Time ULMapCollector::getCurrentDuration() const
{
    double dataRate =
        phyMode->getDataRate();

    Bit compoundSize =
        56 + ulScheduler_->getNumBursts(); /** 48;*/
    wns::simulator::Time symbolDuration = parameter::ThePHY::getInstance()->getSymbolDuration();
    wns::simulator::Time roundedDuration =
        ceil( (compoundSize / dataRate ) / symbolDuration ) * symbolDuration;

    //fixme(bmw) (fixed map overhead of one symbol due to wrong calculation of map duration with subchannels)
    return symbolDuration; //roundedDuration;
}

void ULMapCollector::doOnData( const wns::ldk::CompoundPtr& compound )
{
	assure (compound, "Invalid Map compound received!");
	ULMapCommand* command = getCommand( compound->getCommandPool() );

	if(command->peer.baseStationID !=
	   connectionManager_->getConnectionWithID( 0 )->baseStation_ )
	{
		throw wns::Exception(
			   "ULMapCollector::doOnData: compound is not from associated BaseStation");
	}

	//LOG_INFO( getFUN()->getLayer()->getName(), " received UL Map from station: ", command->peer.baseStationID,
	//		  " of size : ", command->peer.mapInfo->size() );
	LOG_INFO( getFUN()->getLayer()->getName(), " received UL Map from station: ", command->peer.baseStationID);

	ulPhaseDuration_ = command->peer.phaseDuration;

	// check if at least one of the frames contains granted ul resources
	ulResourcesAvailable_ = false;
        LOG_INFO("ULMAP saving now: ");
        scheduledULMap_ = command->peer.schedulingMap;
	LOG_INFO("ULMAP = ", scheduledULMap_->toString());
        //Clear old MAPlist
	//scheduledULMap_->clear();
	if (scheduledULMap_->hasResourcesForUser(me_))
	{
		ulResourcesAvailable_ = true; // assume always true when a MAP comes
                LOG_INFO( getFUN()->getLayer()->getName(), " has granted resources" );
                estimatedCQI_ = scheduledULMap_->getEstimatedCQI(me_);
	}

	getFrameBuilder()->finishedPhase(this);
}
