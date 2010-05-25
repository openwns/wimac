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

#include <WIMAC/frame/FrameHeadCollector.hpp>

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/fcf/TimingControl.hpp>
#include <WNS/logger/Logger.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

using namespace wimac::frame;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::frame::FrameHeadCollector,
    wns::ldk::FunctionalUnit,
    "wimac.frame.FrameHeadCollector",
    wns::ldk::FUNConfigCreator );


FrameHeadCollector::FrameHeadCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config ) :
    wns::ldk::fcf::CompoundCollector( config ),
    wns::ldk::CommandTypeSpecifier<FrameHeadCommand>(fun),
    phyUser_(NULL),
    phyMode_( wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
              (wns::service::phy::phymode::createPhyMode( config.getView("phyMode") ) ) )
{
}

void FrameHeadCollector::onFUNCreated()
{
    assureType(getFUN()->getLayer(), wimac::Component*);
    layer_ = dynamic_cast<wimac::Component*>(getFUN()->getLayer());

    phyUser_ = getFUN()->findFriend<wimac::PhyUser*>("phyUser");
    assure( phyUser_, "PhyUser is not of type wimac::PhyUser");

    connectionManager_ =
        layer_->getManagementService<service::ConnectionManager>
        ("connectionManager");

    wns::ldk::fcf::CompoundCollector::onFUNCreated();
}

void FrameHeadCollector::doStart(int mode)
{
    switch (mode) {
    case CompoundCollector::Sending:
    {
        wns::ldk::CompoundPtr compound (
            new wns::ldk::Compound( getFUN()->getProxy()->createCommandPool() ) );

        FrameHeadCommand* command = activateCommand( compound->getCommandPool() );
        command->peer.baseStationID = layer_->getID();
        command->local.duration =
            getCurrentDuration() - Utilities::getComputationalAccuracyFactor();

        PhyUserCommand* phyCommand = dynamic_cast<PhyUserCommand*>(
            getFUN()->getProxy()->activateCommand( compound->getCommandPool(),
                                                   phyUser_ ) );
        phyCommand->peer.destination_ = 0;
        phyCommand->peer.cellID_ = layer_->getCellID();
        phyCommand->peer.source_ = layer_->getNode();
        assureNotNull(phyMode_.getPtr());
        phyCommand->peer.phyModePtr = phyMode_;
        phyCommand->magic.sourceComponent_ = layer_;
        phyCommand->magic.frameHead_ = true;

        wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();
        phyCommand->local.pAFunc_.reset
            ( new BroadcastPhyAccessFunc);
        phyCommand->local.pAFunc_->transmissionStart_ = now;
        phyCommand->local.pAFunc_->transmissionStop_ = now + command->local.duration - 1e-13;
        phyCommand->local.pAFunc_->phyMode_ = phyMode_;

        assure( command->local.duration <= this->getMaximumDuration(),
                "FrameHeadWriter: PDU overun the maximum duration of the frame phase!");

        setTimeout( command->local.duration );

        LOG_INFO( getFrameBuilder()->getFUN()->getName(),
                  ": frame head started with duration: ", command->local.duration);

        getConnector()->getAcceptor( compound )->sendData( compound );
                break;
    }
    case CompoundCollector::Receiving:
        break;
    default:
        throw wns::Exception("Unknown activation in FrameHeadCollector");
    }
}

void
FrameHeadCollector::onTimeout()
{
    getFrameBuilder()->finishedPhase( this );
}


void FrameHeadCollector::doOnData( const wns::ldk::CompoundPtr& compound )
{
    FrameHeadCommand* command =
        getCommand( compound->getCommandPool() );

    LOG_INFO( getFUN()->getLayer()->getName(), ": received FCH from station:",command->peer.baseStationID);

    getFrameBuilder()->getTimingControl()->finishedPhase( this );
}
