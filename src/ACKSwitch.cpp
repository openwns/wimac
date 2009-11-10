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
 * \file
 * \author Karsten Klagges <kks@comnets.rwth-aachen.de>
 */

#include <WIMAC/ACKSwitch.hpp>

#include <WNS/Exception.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Layer.hpp>
#include <WNS/ldk/arq/ARQ.hpp>

#include <WIMAC/services/ConnectionManager.hpp>


STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::ACKSwitch,
    wns::ldk::FunctionalUnit,
    "wimac.ACKSwitch",
    wns::ldk::FUNConfigCreator);

using namespace wimac;
using namespace wimac::service;

ACKSwitch::ACKSwitch( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& ):
    wns::ldk::CommandTypeSpecifier<AckSwitchCommand>(fun),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasDeliverer<>(),
    wns::ldk::Processor<ACKSwitch>(),
    arq_(0),
    classifier_(0),
    connectionManager_(0)
{
}

void ACKSwitch::onFUNCreated()
{
    arq_ = getFUN()->findFriend<wns::ldk::arq::ARQ*>("arq");
    assure( arq_, "ARQ with name \"arq\" not found in FUN");

    classifier_ = getFUN()->findFriend<wns::ldk::CommandTypeSpecifier<wns::ldk::ClassifierCommand> *>("classifier");
    assure( classifier_, "classifier with name \"classifier\" not found in FUN" );

    connectionManager_ =
        getFUN()->getLayer()->getManagementService<service::ConnectionManagerInterface>
        ("connectionManager");
    assure( connectionManager_, "ConnectionManager not found in Configuration" );
}

void ACKSwitch::processOutgoing( const wns::ldk::CompoundPtr& compound )
{
    wns::ldk::arq::ARQCommand* arqCommand =
        dynamic_cast<wns::ldk::arq::ARQCommand*>(arq_->getCommand( compound->getCommandPool() ) );
    assure( arqCommand, "Command is not of type ARQCommand" );

    if ( arqCommand->isACK() )
    {
        // this is an ACK, CID must be switched
        AckSwitchCommand* ackSwitchCommand = activateCommand( compound->getCommandPool() );
        wns::ldk::ClassifierCommand* classifierCommand =
            classifier_->getCommand( compound->getCommandPool() );

        // backup original CID
        ackSwitchCommand->peer.originalCID = classifierCommand->peer.id;

        // get the ConnectionIdentifier for the basic compound
        ConnectionIdentifierPtr basicConnection =
            connectionManager_->getBasicConnectionFor( classifierCommand->peer.id );

        LOG_INFO( "ACKSwitch: switching outgoing ACK from CID ",
                  classifierCommand->peer.id, " to basic CID ",
                  basicConnection->getID() );

        // do the switch
        classifierCommand->peer.id = basicConnection->getID();
    }
}

void ACKSwitch::processIncoming( const wns::ldk::CompoundPtr& compound )
{
    if ( getFUN()->getProxy()->commandIsActivated( compound->getCommandPool(), this ) )
    {
        // commandPool is activated -> this is an ACK
        AckSwitchCommand* switchCommand = getCommand( compound->getCommandPool() );
        wns::ldk::ClassifierCommand* classifierCommand =
            classifier_->getCommand( compound->getCommandPool() );
        LOG_INFO( "ACKSwitch: switching incoming ACK from basic CID ",
                  classifierCommand->peer.id, " to data CID ",
                  switchCommand->peer.originalCID );
        classifierCommand->peer.id = switchCommand->peer.originalCID;
    }
}
