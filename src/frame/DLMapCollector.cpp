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

#include <WIMAC/frame/DLMapCollector.hpp>

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/fcf/TimingControl.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/PhyUserCommand.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/frame/SingleCompoundCollector.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/frame/DataCollector.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/parameter/PHY.hpp>

using namespace wimac::frame;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::frame::DLMapCollector,
    wns::ldk::FunctionalUnit,
    "wimac.frame.DLMapCollector",
    wns::ldk::FUNConfigCreator );


DLMapCollector::DLMapCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config ) :
    wns::ldk::fcf::CompoundCollector( config ),
    wns::ldk::CommandTypeSpecifier<DLMapCommand>(fun),
    dlScheduler_(0),
    dlSchedulerName_(),
    phyUser_(0),
    phyMode(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
            (wns::service::phy::phymode::createPhyMode( config.getView("phyMode") ) ) )
{
    if ( !config.isNone("dlSchedulerName") )
        dlSchedulerName_ = config.get<std::string>("dlSchedulerName");
}

void DLMapCollector::onFUNCreated()
{
    wns::ldk::fcf::CompoundCollector::onFUNCreated();

    assureType(getFUN()->getLayer(), wimac::Component*);
    component_ = dynamic_cast<wimac::Component*>(getFUN()->getLayer());

    if ( dlSchedulerName_ != "" ) {
        dlScheduler_ = dynamic_cast<wimac::scheduler::Scheduler*>
            (getFUN()->findFriend<DataCollector*>(dlSchedulerName_)->getTxScheduler());
        assure( dlScheduler_, "Downlink Scheduler not present in FUN" );
    }

    phyUser_ = getFUN()->findFriend<wimac::PhyUser*>("phyUser");
    assure( phyUser_, "PhyUser is not of type wimac::PhyUser");

    connectionManager_ =
        getFUN()->getLayer()->getManagementService<service::ConnectionManager>
        ("connectionManager");

    setFrameBuilder( getFUN()->findFriend<wns::ldk::fcf::FrameBuilder*>("frameBuilder") );
    CompoundCollector::onFUNCreated();
}

void DLMapCollector::doStart(int mode)
{
    switch (mode) {
    case Sending:
    {
        assure(dlScheduler_, "DLMapCollector can not send, no DL Scheduler set");

        // create the MAP and send it
        wns::ldk::CompoundPtr compound
            ( new wns::ldk::Compound( getFUN()->getProxy()->createCommandPool() ) );

        DLMapCommand* command = activateCommand( compound->getCommandPool() );
        command->peer.phaseDuration =
            dlScheduler_->getDuration();
        command->peer.baseStationID =
            component_->getID();
        command->local.numBursts =
            dlScheduler_->getNumBursts();
        command->local.mapDuration =
            getCurrentDuration() - Utilities::getComputationalAccuracyFactor();

        LOG_INFO("MAP duration is ", command->local.mapDuration,
                 ", phase duration is ",
                 this->getMaximumDuration());
        assure( command->local.mapDuration <= this->getMaximumDuration(),
                "DLMapWriter: PDU overun the maximum duration of the frame phase!");

        PhyUserCommand* phyCommand =
            dynamic_cast<PhyUserCommand*>(
                getFUN()->getProxy()->activateCommand( compound->getCommandPool(),
                                                       phyUser_ ) );

        phyCommand->peer.destination_ = 0;
        phyCommand->peer.cellID_ = component_->getCellID();
        phyCommand->peer.source_ = component_->getNode();
        assureNotNull(&phyMode);
        phyCommand->peer.phyModePtr = phyMode;
        phyCommand->magic.sourceComponent_ = component_;


        wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();
        phyCommand->local.pAFunc_.reset
            ( new BroadcastPhyAccessFunc );
        phyCommand->local.pAFunc_->transmissionStart_ = now;
        phyCommand->local.pAFunc_->transmissionStop_ =
            now + getCurrentDuration() - Utilities::getComputationalAccuracyFactor();
        phyCommand->local.pAFunc_->phyMode_ = phyMode;

        setTimeout( getCurrentDuration() );
        LOG_INFO("Sending DL MAP in ", getFUN()->getName(),
                 " and setting timeout to ", getCurrentDuration());
        getConnector()->getAcceptor( compound )->sendData( compound );
        break;
    }
    case Receiving:
        /* wait and do nothing */
        break;
    default:
        throw wns::Exception("Unknown activation mode in DLMapCollector");
    }
}

void
DLMapCollector::calculateSizes( const wns::ldk::CommandPool* commandPool,
                                Bit& commandPoolSize, Bit& dataSize ) const
{
    // What are the sizes in the upper Layers
    getFUN()->getProxy()->calculateSizes(commandPool, commandPoolSize, dataSize, this);

    DLMapCommand* command = getCommand( commandPool );
    commandPoolSize += ( 56 + command->local.numBursts * 48 );
}

void
DLMapCollector::onTimeout()
{
    getFrameBuilder()->finishedPhase( this );
}

wns::simulator::Time DLMapCollector::getCurrentDuration() const
{
    double dataRate =
        phyMode->getDataRate();

    Bit compoundSize =
        56 + dlScheduler_->getNumBursts(); /* *48*/;

    wns::simulator::Time symbolDuration = parameter::ThePHY::getInstance()->getSymbolDuration();
    wns::simulator::Time roundedDuration =
        ceil( (compoundSize / dataRate ) / symbolDuration ) * symbolDuration;

    return roundedDuration;
}

void DLMapCollector::doOnData( const wns::ldk::CompoundPtr& compound )
{
    MapCommand* command =
        getCommand( compound->getCommandPool() );

    if(command->peer.baseStationID !=
       connectionManager_->getConnectionWithID( 0 )->baseStation_ )
    {
        throw wns::Exception
            ("DLMapRetriever::doOnData: compound is not from associated BaseStation");
    }

    LOG_INFO( getFUN()->getLayer()->getName(),
              ": received DL-MAP from station:",
              command->peer.baseStationID);

    dlPhaseDuration_ = command->peer.phaseDuration;

    getFrameBuilder()->getTimingControl()->finishedPhase( this );
}
