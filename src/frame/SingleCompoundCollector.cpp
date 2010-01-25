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

#include <WIMAC/frame/SingleCompoundCollector.hpp>

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Deliverer.hpp>
#include <WNS/ldk/fcf/FrameBuilder.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/StationManager.hpp>

#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/parameter/PHY.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::frame::SingleCompoundCollector,
    wns::ldk::FunctionalUnit,
    "wimac.frame.SingleCompoundCollector",
    wns::ldk::FUNConfigCreator );

using namespace wimac;
using namespace wimac::frame;


void SingleCompoundCollector::doOnData( const wns::ldk::CompoundPtr& compound )
{
    getDeliverer()->getAcceptor( compound )->onData( compound );
}

bool SingleCompoundCollector::doIsAccepting( const wns::ldk::CompoundPtr& ) const
{
    return accepting_ && !compound_;
}

void SingleCompoundCollector::doSendData( const wns::ldk::CompoundPtr& compound )
{
    wns::ldk::ClassifierCommand* clcom =
        friends_.classifier_->getCommand( compound->getCommandPool() );

    Component::StationID destinationID = -1;

    ConnectionIdentifierPtr connection =
        component_->getManagementService<service::ConnectionManager>("connectionManager")
        ->getConnectionWithID( clcom->peer.id );

    std::string stationType = StationType::toString( component_->getStationType() );
    if( stationType == "AP" )
    {
        destinationID =
            connection->subscriberStation_;
    }
    else if( stationType == "UT" )
    {
        destinationID =
            connection->baseStation_;
    }
    else
    {
        throw wns::Exception("Unknown station type in SingleCompoundCollector");
    }

    PhyUserCommand* phyUserCommand =
        friends_.phyUser_->activateCommand( compound->getCommandPool() );


    wns::node::Interface* destination =
        TheStationManager::getInstance()->
        getStationByID( destinationID )->getNode();

    double dataRate =
        phyMode->getDataRate();

    double transmissionDuration =
        compound->getLengthInBits() / dataRate;
    wns::simulator::Time now ( wns::simulator::getEventScheduler()->getTime() );

    OmniUnicastPhyAccessFunc* func = new OmniUnicastPhyAccessFunc;
    func->destination_ = destination;
    func->transmissionStart_ = now;
    func->transmissionStop_ = now + transmissionDuration;
    func->subBand_ = 0;
    assureNotNull(phyMode.getPtr());
    func->phyMode_ = phyMode;

    phyUserCommand->local.pAFunc_.reset( func );
    phyUserCommand->peer.cellID_ = component_->getCellID();
    phyUserCommand->peer.source_ = component_->getNode();
    phyUserCommand->peer.phyModePtr = phyMode;
    phyUserCommand->magic.sourceComponent_ = component_;

    getConnector()->getAcceptor( compound )->sendData( compound );

    assure( getCurrentDuration() >= compound->getLengthInBits() / dataRate,
            "Transmission of compound would exceed phase duration");

    LOG_INFO("setting destination of compound to: ",
             TheStationManager::getInstance()->
             getStationByID( destinationID )->getName() );

    compound_ = compound;
}

void SingleCompoundCollector::doStart(int mode)
{
    switch (mode)
    {
    case CompoundCollector::Sending:
        assure( !compound_, "compound of last frame not sent yet" );
        accepting_ = true;
        getReceptor()->wakeup();

        if ( compound_ )
        {
            getConnector()->getAcceptor( compound_ )->sendData( compound_ );
            compound_ = wns::SmartPtr<wns::ldk::Compound>();
        }
        break;
    case CompoundCollector::Receiving:

        break;
    default:
        throw wns::Exception("Unknown mode in CompoundCollector");
    }
}


wns::simulator::Time SingleCompoundCollector::getCurrentDuration() const
{
    if ( !compound_ )
        return 0.0;

    double dataRate =
        phyMode->getDataRate();

    Bit compoundSize =
        compound_->getLengthInBits() + getFrameBuilder()->getOpcodeSize();
    return compoundSize / dataRate;
}

void SingleCompoundCollector::onFUNCreated()
{
    wns::ldk::fcf::CompoundCollector::onFUNCreated();

    friends_.classifier_ =
        getFUN()->findFriend<wimac::ConnectionClassifier*>("classifier");
    friends_.phyUser_ =
        getFUN()->findFriend<wimac::PhyUser*>("phyuser");
    CompoundCollector::onFUNCreated();
}


