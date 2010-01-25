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

#include <WIMAC/services/ConnectionControl.hpp>

#include <WIMAC/services/ConnectionManager.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WIMAC/relay/RelayMapper.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/StationManager.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::ConnectionControl,
    wns::ldk::ControlServiceInterface,
    "wimac.services.ConnectionControl",
    wns::ldk::CSRConfigCreator);

using namespace wimac::service;
using namespace wimac;

ConnectionControl::ConnectionControl( wns::ldk::ControlServiceRegistry* csr,
                                      wns::pyconfig::View& config ) :
    wns::ldk::ControlService(csr),
    associatedWithID_(0)
{
    if ( !config.isNone("associatedWith") )
    {
        associatedWithID_ = config.get<wimac::StationID>("associatedWith");
    }
}

void
ConnectionControl::onCSRCreated()
{
    friends_.connectionManager = getCSR()->getLayer()
        ->getManagementService<wimac::service::ConnectionManager>("connectionManager");
    assure(friends_.connectionManager,
           "ConnectionManager must be of type wimac::service::ConnectionManager");

    if (associatedWithID_)
        associateTo(associatedWithID_, ConnectionIdentifier::BE);
}

void
ConnectionControl::associateTo(StationID associateTo,
                               ConnectionIdentifier::QoSCategory qosCategory)
{

    LOG_INFO(getCSR()->getLayer()->getName(), ": associate to Station:", associateTo , "     QoSCategory:", qosCategory);

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

    // create downlink data connection
    ConnectionIdentifier dlCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               qosCategory );

    // create uplink data connection
    ConnectionIdentifier ulCI( associateTo,
                               stationID,
                               stationID,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               qosCategory );


    // get master ConnectionManager from destination access point
    wimac::service::ConnectionManager* destinationConnectionManager
        //	  = this->getConnectionManagerMaster();
        = destination->getManagementService<service::ConnectionManager>("connectionManager");

    // append ConnectionIdentifier to access point and get CID
    bCI =  destinationConnectionManager->appendConnection( bCI );
    pmCI = destinationConnectionManager->appendConnection( pmCI );
    dlCI = destinationConnectionManager->appendConnection( dlCI );
    ulCI = destinationConnectionManager->appendConnection( ulCI );

    //append ConnectionIdentifier myself
    friends_.connectionManager->appendConnection( bCI );
    friends_.connectionManager->appendConnection( pmCI );
    friends_.connectionManager->appendConnection( dlCI );
    friends_.connectionManager->appendConnection( ulCI );


    if( destination->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        destination
            ->getControlService<service::ConnectionControl>("connectionControl")
            ->createRecursiveConnection(bCI.cid_,
                                        pmCI.cid_,
                                        dlCI.cid_,
                                        ulCI.cid_,
                                        stationID,
                                        qosCategory);
    }

    Component* layer2 =
        dynamic_cast<Component*>(getCSR()->getLayer());
    assure(layer2, "ConnectionControl only works in a dll::Layer2.");
    layer2->getFUN()->findFriend<PhyUser*>("phyUser")->startMeasuring();
    layer2->getFUN()->findFriend<PhyUser*>("phyUser")->stopMeasuring();
}

void
ConnectionControl::createRecursiveConnection(ConnectionIdentifier::CID basicCID,
                                             ConnectionIdentifier::CID primaryCID,
                                             ConnectionIdentifier::CID downlinkTransportCID,
                                             ConnectionIdentifier::CID uplinkTransportCID,
                                             ConnectionIdentifier::StationID remote,
                                             int qosCategory)
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

    // Downlink Data CI
    ConnectionIdentifier dlCI ( associatedWith->getID(),
                                layer->getID(),
                                remote,
                                ConnectionIdentifier::Data,
                                ConnectionIdentifier::Downlink,
                                qosCategory );

    // Uplink Data CI
    ConnectionIdentifier ulCI( associatedWith->getID(),
                               layer->getID(),
                               remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Uplink,
                               qosCategory );

    // get master ConnectionManager from destination access point
    wimac::service::ConnectionManager* associatedWithConnectionManager
        = layer->getConnectionManagerMaster();

    // append ConnectionIdentifier to access point and get CID
    bCI = associatedWithConnectionManager->appendConnection( bCI );
    pmCI = associatedWithConnectionManager->appendConnection( pmCI );
    dlCI = associatedWithConnectionManager->appendConnection( dlCI );
    ulCI = associatedWithConnectionManager->appendConnection( ulCI );

    //append ConnectionIdentifier myself
    friends_.connectionManager->appendConnection( bCI );
    friends_.connectionManager->appendConnection( pmCI );
    friends_.connectionManager->appendConnection( dlCI );
    friends_.connectionManager->appendConnection( ulCI );

    // append to relayMapper
    relay::RSRelayMapper* relayMapper = layer->getFUN()
        ->findFriend<relay::RSRelayMapper*>("relayMapper");

    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(bCI.cid_, basicCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(pmCI.cid_, primaryCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(dlCI.cid_, downlinkTransportCID) );
    relayMapper->addMapping( relay::RSRelayMapper::RelayMapping(ulCI.cid_, uplinkTransportCID) );

    if ( associatedWith->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        associatedWith
            ->getControlService<ConnectionControl>("connectionControl")
            ->createRecursiveConnection(bCI.cid_,
                                        pmCI.cid_,
                                        dlCI.cid_,
                                        ulCI.cid_,
                                        remote,
                                        qosCategory);
    }

    Component* layer2 =
        dynamic_cast<Component*>(getCSR()->getLayer());
    assure(layer2, "ConnectionControl only works in a wimac component.");
    layer2->getFUN()->findFriend<PhyUser*>("phyUser")->startMeasuring();
    layer2->getFUN()->findFriend<PhyUser*>("phyUser")->stopMeasuring();
}

