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

/**
 * @file
 * @author Karsten Klagges <kks@comnets.rwth-aachen.de>
 */

#include <WIMAC/relay/RelayMapper.hpp>

#include <WNS/ldk/Layer.hpp>
#include <WNS/ldk/Classifier.hpp>

#include <WIMAC/ACKSwitch.hpp>
#include <WIMAC/Classifier.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::relay::BSRelayMapper,
    wns::ldk::FunctionalUnit,
    "wimac.relay.BSRelayMapper",
    wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::relay::RSRelayMapper,
    wns::ldk::FunctionalUnit,
    "wimac.relay.RSRelayMapper",
    wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::relay::SSRelayMapper,
    wns::ldk::FunctionalUnit,
    "wimac.relay.SSRelayMapper",
    wns::ldk::FUNConfigCreator);

using namespace wimac::relay;

RelayMapper::RelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& )
    : wns::ldk::CommandTypeSpecifier<RelayMapperCommand>(fun)
{}

void BSRelayMapper::processOutgoing( const wns::ldk::CompoundPtr& compound )
{
    RelayMapperCommand* command = dynamic_cast<RelayMapperCommand*>
        (getFUN()->getProxy()->activateCommand( compound->getCommandPool(), this ));

    command->peer.direction_ =
        RelayMapperCommand::Down;
}

BSRelayMapper::BSRelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config )
    : RelayMapper( fun, config )
{}

RSRelayMapper::RSRelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config )
    : RelayMapper( fun, config )
{
}

void RSRelayMapper::onFUNCreated()
{
    downRelayInject_ =
        getFUN()->getFunctionalUnit( "downRelayInject" );
    upRelayInject_ =
        getFUN()->getFunctionalUnit( "upRelayInject" );
    classifier_ =
        getFUN()->findFriend<wimac::ConnectionClassifier*>( "classifier" );
    assure( classifier_, "classifier not found in FUN" );

    ackSwitch_ = 0;
    if( getFUN()->knowsFunctionalUnit("ackSwitch") )
    {
        ackSwitch_ =
            getFUN()->findFriend<wimac::ACKSwitch*>("ackSwitch");
        assure( ackSwitch_, "ackSwitch not found in FUN");
    }
}

void RSRelayMapper::processOutgoing( const wns::ldk::CompoundPtr&
#ifndef NDEBUG
                                     compound
#endif
    )
{
    assure(getFUN()->getProxy()->commandIsActivated(compound->getCommandPool(), this),
           "RelayMapper Command should be activated for outgoing compounds.");
}

void RSRelayMapper::processIncoming( const wns::ldk::CompoundPtr& compound )
{
    wns::ldk::ClassifierCommand* clcom =
        classifier_->getCommand( compound->getCommandPool() );

    RelayMapping mapping =
        findMapping( clcom->peer.id );
    if ( mapping == RelayMapping() )
    {
        std::stringstream ss;
        ss << "no mapping registered for CID " << clcom->peer.id;
        assure( 0, ss.str() );
    }
    wns::ldk::CommandPool* injection =
        getFUN()->getProxy()->createCommandPool();

    getFUN()->getProxy()
        ->partialCopy( this, injection, compound->getCommandPool() );

    if ( ackSwitch_
         && getFUN()->getProxy()->commandIsActivated( compound->getCommandPool(), ackSwitch_ ) )
    {
        // we need to switch the CID of the ACK too
        wimac::AckSwitchCommand* ackCom =
            ackSwitch_->getCommand( compound->getCommandPool() );
        wimac::AckSwitchCommand* injectACKCom =
            ackSwitch_->getCommand( injection );
        RelayMapping ackMapping =
            findMapping( ackCom->peer.originalCID );
        if ( ackCom->peer.originalCID == ackMapping.upperConnection_ )
            injectACKCom->peer.originalCID = ackMapping.lowerConnection_;
        else
            injectACKCom->peer.originalCID = ackMapping.upperConnection_;
    }

    wns::ldk::ClassifierCommand* injectClcom =
        classifier_->getCommand( injection );
    wns::ldk::CompoundPtr injectCompound( new wns::ldk::Compound(injection, compound->getData()));

    if ( clcom->peer.id == mapping.upperConnection_ )
    {
        LOG_INFO(getFUN()->getLayer()->getName(),
                 " maps compound CID from ", clcom->peer.id, " down to ",
                 mapping.lowerConnection_ );
        injectClcom->peer.id = mapping.lowerConnection_;


        RelayMapperCommand* rCommand =
            this->activateCommand(injectCompound->getCommandPool());
        rCommand->peer.direction_ = RelayMapperCommand::Down;

        downRelayInject_->sendData( injectCompound );
    }
    else
    {
        LOG_INFO(getFUN()->getLayer()->getName(),
                 " maps compound CID from ", clcom->peer.id, " up to ",
                 mapping.upperConnection_ );
        injectClcom->peer.id = mapping.upperConnection_;

        RelayMapperCommand* rCommand =
            this->activateCommand(injectCompound->getCommandPool());
        rCommand->peer.direction_ = RelayMapperCommand::Up;

        upRelayInject_->sendData( injectCompound );
    }
}

RSRelayMapper::RelayMapping
RSRelayMapper::findMapping( const wimac::ConnectionIdentifier::CID& id ) const
{
    for ( Mappings::const_iterator it = mappings_.begin();
          it != mappings_.end();
          ++it )
    {
        if ( id == it->upperConnection_ || id == it->lowerConnection_ )
            return *it;
    }
    return RelayMapping();
}

void RSRelayMapper::addMapping( const wimac::relay::RSRelayMapper::RelayMapping& mapping )
{
    mappings_.push_back( mapping );
}

bool RSRelayMapper::RelayMapping::operator==(const RelayMapping& rhs ) const
{
    return upperConnection_ == rhs.upperConnection_
        && lowerConnection_ == rhs.lowerConnection_ ;
}

SSRelayMapper::SSRelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config )
    : RelayMapper( fun, config )
{}

void SSRelayMapper::processOutgoing( const wns::ldk::CompoundPtr& compound )
{
    RelayMapperCommand* command = dynamic_cast<RelayMapperCommand*>
        (getFUN()->getProxy()->activateCommand( compound->getCommandPool(), this ));

    command->peer.direction_ =
        RelayMapperCommand::Up;
}
