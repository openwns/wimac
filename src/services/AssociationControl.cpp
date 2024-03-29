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

#include <WIMAC/services/AssociationControl.hpp>

#include <WIMAC/services/ConnectionManager.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WIMAC/relay/RelayMapper.hpp>
#include <WIMAC/StationManager.hpp>

using namespace wimac::service::associationcontrol;
using namespace wimac::service;
using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::associationcontrol::Fixed,
    wns::ldk::ControlServiceInterface,
    "wimac.services.AssociationControl.Fixed",
    wns::ldk::CSRConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::associationcontrol::BestAtGivenTime,
    wns::ldk::ControlServiceInterface,
    "wimac.services.AssociationControl.BestAtGivenTime",
    wns::ldk::CSRConfigCreator);

STATIC_FACTORY_REGISTER(BestPathloss, IDecideBest, "wimac.services.AssociationControl.Best.BestPathloss");
STATIC_FACTORY_REGISTER(BestRxPower, IDecideBest, "wimac.services.AssociationControl.Best.BestRxPower");
STATIC_FACTORY_REGISTER(BestSINR, IDecideBest, "wimac.services.AssociationControl.Best.BestSINR");

BestAtGivenTime::BestAtGivenTime(wns::ldk::ControlServiceRegistry* csr,
                                      wns::pyconfig::View& config) :
    AssociationControl(csr, config),
    deciderStrategy_(NULL),
    decisionTime_(config.get<wns::simulator::Time>("decisionTime"))
{
    std::string pluginName = config.get<std::string>("decisionStrategy.__plugin__");
    deciderStrategy_ = IDecideBest::Factory::creator(pluginName)->create();
}

BestAtGivenTime::~BestAtGivenTime()
{
    assure(deciderStrategy_, "Decider strategy is NULL");

    delete deciderStrategy_;
}

void
BestAtGivenTime::doOnCSRCreated()
{
    wns::simulator::getEventScheduler()->scheduleDelay(
        boost::bind(&BestAtGivenTime::associateNow, this),
        decisionTime_);
}

void
BestAtGivenTime::doStoreMeasurement(StationID source,
    const wns::service::phy::power::PowerMeasurementPtr& pm)
{
    assure(deciderStrategy_, "Decider strategy is NULL");

    deciderStrategy_->put(source, pm);
}

void
BestAtGivenTime::associateNow()
{
    assure(deciderStrategy_, "Decider strategy is NULL");
    assure(deciderStrategy_->isInitialized(), 
        "Decision strategy did not receive any measurements yet.");

    associateTo(deciderStrategy_->getBest());
}


Fixed::Fixed(wns::ldk::ControlServiceRegistry* csr,
                                      wns::pyconfig::View& config) :
    AssociationControl(csr, config),
    associatedWithID_(0)
{
    assure(!config.isNone("associatedWith"), "Missing association target ID in PyConfig");

    associatedWithID_ = config.get<wimac::StationID>("associatedWith");
}

Fixed::~Fixed()
{
}

void
Fixed::doOnCSRCreated()
{
    associateTo(associatedWithID_);
}

void
Fixed::doStoreMeasurement(StationID, 
    const wns::service::phy::power::PowerMeasurementPtr&)
{
}

AssociationControl::AssociationControl( wns::ldk::ControlServiceRegistry* csr,
                                      wns::pyconfig::View& config ) :
    wns::ldk::ControlService(csr)
{
}



void
AssociationControl::onCSRCreated()
{
    friends_.connectionManager = getCSR()->getLayer()
        ->getManagementService<wimac::service::ConnectionManager>("connectionManager");
    assure(friends_.connectionManager,
           "ConnectionManager must be of type wimac::service::ConnectionManager");

    doOnCSRCreated();        
}

void
AssociationControl::storeMeasurement(StationID source, 
    const wns::service::phy::power::PowerMeasurementPtr& pm)
{
    doStoreMeasurement(source, pm);
}


void
AssociationControl::associateTo(StationID associateTo)
{

    LOG_INFO(getCSR()->getLayer()->getName(), ": associate to Station:", associateTo);

    wimac::Component* layer( dynamic_cast<wimac::Component*>( getCSR()->getLayer() ) );
    assure(layer, "Layer is not of type wimac::Component");

    StationID stationID = layer->getID();

    wimac::Component* destination (
        static_cast<wimac::Component*>
        ( TheStationManager::getInstance()->getStationByID( associateTo ) ) );
    assure( destination, "destination is not of type wimac::Component");


    if( layer->getStationType() != wns::service::dll::StationTypes::AP() ) {
        // create CID 0 (Initinal Ranging) in SS, to declare to wich BS we are associated
        ConnectionIdentifier connection ( associateTo,
                                          0,
                                          stationID,
                                          stationID,
                                          ConnectionIdentifier::InitialRanging,
                                          ConnectionIdentifier::Bidirectional,
                                          ConnectionIdentifier::Signaling );

        friends_.connectionManager->appendConnection( connection );
    }

    // create basic connection
    ConnectionIdentifier bCI( associateTo,
                              stationID,
                              stationID,
                              ConnectionIdentifier::Basic,
                              ConnectionIdentifier::Bidirectional,
                              ConnectionIdentifier::Signaling );

    // create primary connection
    ConnectionIdentifier pmCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::PrimaryManagement,
                               ConnectionIdentifier::Bidirectional,
                               ConnectionIdentifier::Signaling );

    // create downlink data connections
    ConnectionIdentifier dlBECI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::BE);

    ConnectionIdentifier dlRtPSCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::rtPS);

    ConnectionIdentifier dlNrtPSCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::nrtPS);

    ConnectionIdentifier dlUGSCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::UGS);


    // create uplink data connections
    ConnectionIdentifier ulBECI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::BE);

    ConnectionIdentifier ulRtPSCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::rtPS);

    ConnectionIdentifier ulNrtPSCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::nrtPS);

    ConnectionIdentifier ulUGSCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::UGS);



    // get master ConnectionManager from destination access point
    wimac::service::ConnectionManager* destinationConnectionManager
        = destination->getManagementService<service::ConnectionManager>("connectionManager");

    // append ConnectionIdentifier to access point and get CID
    bCI =  destinationConnectionManager->appendConnection( bCI );
    pmCI = destinationConnectionManager->appendConnection( pmCI );

    dlBECI = destinationConnectionManager->appendConnection( dlBECI );
    dlRtPSCI = destinationConnectionManager->appendConnection( dlRtPSCI );
    dlNrtPSCI = destinationConnectionManager->appendConnection( dlNrtPSCI );
    dlUGSCI = destinationConnectionManager->appendConnection( dlUGSCI );

    ulBECI = destinationConnectionManager->appendConnection( ulBECI );
    ulRtPSCI = destinationConnectionManager->appendConnection( ulRtPSCI );
    ulNrtPSCI = destinationConnectionManager->appendConnection( ulNrtPSCI );
    ulUGSCI = destinationConnectionManager->appendConnection( ulUGSCI );

    //append ConnectionIdentifier myself
    friends_.connectionManager->appendConnection( bCI );
    friends_.connectionManager->appendConnection( pmCI );

    friends_.connectionManager->appendConnection( dlBECI );
    friends_.connectionManager->appendConnection( dlRtPSCI );
    friends_.connectionManager->appendConnection( dlNrtPSCI );
    friends_.connectionManager->appendConnection( dlUGSCI );

    friends_.connectionManager->appendConnection( ulBECI );
    friends_.connectionManager->appendConnection( ulRtPSCI );
    friends_.connectionManager->appendConnection( ulNrtPSCI );
    friends_.connectionManager->appendConnection( ulUGSCI );


    if( destination->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        destination
            ->getControlService<service::AssociationControl>("associationControl")
            ->createRecursiveConnection(bCI.cid_,
                                        pmCI.cid_,
                                        dlBECI.cid_,
                                        dlRtPSCI.cid_,
                                        dlNrtPSCI.cid_,
                                        dlUGSCI.cid_,
                                        ulBECI.cid_,
                                        ulRtPSCI.cid_,
                                        ulNrtPSCI.cid_,
                                        ulUGSCI.cid_,
                                        stationID);
    }

    Component* layer2 =
        dynamic_cast<Component*>(getCSR()->getLayer());
    assure(layer2, "AssociationControl only works in a dll::Layer2.");
}

void
AssociationControl::createRecursiveConnection(ConnectionIdentifier::CID basicCID,
                                             ConnectionIdentifier::CID primaryCID,
                                             ConnectionIdentifier::CID downlinkBETransportCID,
                                             ConnectionIdentifier::CID downlinkRtPSTransportCID,
                                             ConnectionIdentifier::CID downlinkNrtPSTransportCID,
                                             ConnectionIdentifier::CID downlinkUGSTransportCID,
                                             ConnectionIdentifier::CID uplinkBETransportCID,
                                             ConnectionIdentifier::CID uplinkRtPSTransportCID,
                                             ConnectionIdentifier::CID uplinkNrtPSTransportCID,
                                             ConnectionIdentifier::CID uplinkUGSTransportCID,
                                             ConnectionIdentifier::StationID remote)
{
    wimac::Component* layer = dynamic_cast<wimac::Component*>(getCSR()->getLayer());
    assure(layer, "Layer must be of type dll::Layer2");

    assure( layer->getStationType() == wns::service::dll::StationTypes::FRS(),
            "recursive connections only possible in relay stations" );

    ConnectionIdentifier::StationID associatedWithID =
        friends_.connectionManager
        ->getBasicConnectionFor( layer->getID() )->baseStation_;

    wimac::Component* associatedWith = static_cast<wimac::Component*>
        ( TheStationManager::getInstance()->getStationByID(associatedWithID) );
    assure( associatedWith, "Station is not of type wimac::Component");


    // Basic CI
    ConnectionIdentifier bCI( associatedWith->getID(),
                              layer->getID(),
                              remote,
                              ConnectionIdentifier::Basic,
                              ConnectionIdentifier::Bidirectional,
                              ConnectionIdentifier::Signaling );

    // Primary Management CI
    ConnectionIdentifier pmCI( associatedWith->getID(),
                               layer->getID(),
                               remote,
                               ConnectionIdentifier::PrimaryManagement,
                               ConnectionIdentifier::Bidirectional,
                               ConnectionIdentifier::Signaling );

    // create downlink data connections
    ConnectionIdentifier dlBECI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::BE);

    ConnectionIdentifier dlRtPSCI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::rtPS);

    ConnectionIdentifier dlNrtPSCI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::nrtPS);

    ConnectionIdentifier dlUGSCI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               ConnectionIdentifier::UGS);


    // create uplink data connections
    ConnectionIdentifier ulBECI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::BE);

    ConnectionIdentifier ulRtPSCI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::rtPS);

    ConnectionIdentifier ulNrtPSCI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::nrtPS);

    ConnectionIdentifier ulUGSCI( associatedWith->getID(),
                                layer->getID(),
                                remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               ConnectionIdentifier::UGS);


    // get master ConnectionManager from destination access point
    wimac::service::ConnectionManager* associatedWithConnectionManager = 
        associatedWith->getManagementService<service::ConnectionManager>("connectionManager");

    // append ConnectionIdentifier to access point and get CID
    bCI = associatedWithConnectionManager->appendConnection( bCI );
    pmCI = associatedWithConnectionManager->appendConnection( pmCI );

    dlBECI = associatedWithConnectionManager->appendConnection( dlBECI );
    dlRtPSCI = associatedWithConnectionManager->appendConnection( dlRtPSCI );
    dlNrtPSCI = associatedWithConnectionManager->appendConnection( dlNrtPSCI );
    dlUGSCI = associatedWithConnectionManager->appendConnection( dlUGSCI );

    ulBECI = associatedWithConnectionManager->appendConnection( ulBECI );
    ulRtPSCI = associatedWithConnectionManager->appendConnection( ulRtPSCI );
    ulNrtPSCI = associatedWithConnectionManager->appendConnection( ulNrtPSCI );
    ulUGSCI = associatedWithConnectionManager->appendConnection( ulUGSCI );

    //append ConnectionIdentifier myself
    friends_.connectionManager->appendConnection( bCI );
    friends_.connectionManager->appendConnection( pmCI );

    friends_.connectionManager->appendConnection( dlBECI );
    friends_.connectionManager->appendConnection( dlRtPSCI );
    friends_.connectionManager->appendConnection( dlNrtPSCI );
    friends_.connectionManager->appendConnection( dlUGSCI );

    friends_.connectionManager->appendConnection( ulBECI );
    friends_.connectionManager->appendConnection( ulRtPSCI );
    friends_.connectionManager->appendConnection( ulNrtPSCI );
    friends_.connectionManager->appendConnection( ulUGSCI );

    // append to relayMapper
    relay::RSRelayMapper* relayMapper = layer->getFUN()
        ->findFriend<relay::RSRelayMapper*>("relayMapper");

    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(bCI.cid_, basicCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(pmCI.cid_, primaryCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(dlBECI.cid_, downlinkBETransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(dlRtPSCI.cid_, downlinkRtPSTransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(dlNrtPSCI.cid_, downlinkNrtPSTransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(dlUGSCI.cid_, downlinkUGSTransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(ulBECI.cid_, uplinkBETransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(ulRtPSCI.cid_, uplinkRtPSTransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(ulNrtPSCI.cid_, uplinkNrtPSTransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(ulUGSCI.cid_, uplinkUGSTransportCID) );

    if ( associatedWith->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        associatedWith
            ->getControlService<AssociationControl>("associationControl")
            ->createRecursiveConnection(bCI.cid_,
                                        pmCI.cid_,
                                        dlBECI.cid_,
                                        dlRtPSCI.cid_,
                                        dlNrtPSCI.cid_,
                                        dlUGSCI.cid_,
                                        ulBECI.cid_,
                                        ulRtPSCI.cid_,
                                        ulNrtPSCI.cid_,
                                        ulUGSCI.cid_,
                                        remote);
    }

    Component* layer2 =
        dynamic_cast<Component*>(getCSR()->getLayer());
    assure(layer2, "AssociationControl only works in a wimac component.");
}

