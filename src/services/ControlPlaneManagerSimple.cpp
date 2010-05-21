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
 * @author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WNS/service/dll/StationTypes.hpp>

#include <WIMAC/services/ControlPlaneManagerSimple.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/services/ConnectionControl.hpp>

using namespace wimac;
using namespace wimac::service;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::ControlPlaneManagerSimpleSS,
    wns::ldk::ManagementServiceInterface,
    "wimac.services.ControlPlaneManagerSimpleSS",
    wns::ldk::MSRConfigCreator);


ControlPlaneManagerSimpleSS::ControlPlaneManagerSimpleSS(wns::ldk::ManagementServiceRegistry* msr,
                                                         const wns::pyconfig::View& config) :
    ManagementService(msr),
    msName_(config.get<std::string>("serviceName")),
    component_(dynamic_cast<wimac::Component*>( getMSR()->getLayer() )),
    probeAssociatedToContextProvider_("MAC.UTAssocitedToAP", 0),
    probeQoSCategoryContextProvider_("MAC.UTQoSCategory", 0)
{
    friends_.connectionManagerName = "connectionManager";

    friends_.connectionManager = NULL;
}


void
ControlPlaneManagerSimpleSS::start(StationID associateTo, int qosCategory)
{
    // Probe settings
    // Set access point to probe IDProvider, we're associated/associating to.

    wns::probe::bus::ContextProviderCollection& cpc =
        dynamic_cast<Component*>( getMSR()->getLayer() )
        ->getFUN()->getLayer()->getContextProviderCollection();

    cpc.addProvider(wns::probe::bus::contextprovider::Container(
                        &probeAssociatedToContextProvider_));
    cpc.addProvider(wns::probe::bus::contextprovider::Container(
                        &probeQoSCategoryContextProvider_));

    probeAssociatedToContextProvider_.set(associateTo);
    probeQoSCategoryContextProvider_.set(qosCategory);

    this->associateAndConnectTo(associateTo, qosCategory);
}



void
ControlPlaneManagerSimpleSS::associateAndConnectTo(StationID associateTo,
                                                   int qosCategory)
{
    assure( associateTo,
            "wimac::ControlPlaneManagerSimpleSS::associateAndConnectTo: Function need an destinationId!" );
    assure( qosCategory,
            "wimac::ControlPlaneManagerSimpleSS::associateAndConnectTo: Function need an qosCategory!" );


  LOG_INFO(component_->getName(), ": associate to Station:", associateTo , "     QoSCategory:", qosCategory);


  StationID stationID = component_->getID();
  wimac::Component* destination = static_cast<wimac::Component*>
      ( TheStationManager::getInstance()->getStationByID( associateTo ) );
  assureType( destination , wimac::Component*);


  if( component_->getStationType() != wns::service::dll::StationTypes::AP() )
  { // create CID 0 (Initinal Ranging) in SS, to declare to wich BS we are associated
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
  ConnectionManager* destinationConnectionManager
      = this->getConnectionManagerMaster();

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

}


void
ControlPlaneManagerSimpleSS::createRecursiveConnection(CID basicCID,
                                                       CID primaryCID,
                                                       CID downlinkTransportCID,
                                                       CID uplinkTransportCID,
                                                       StationID remote,
                                                       int qosCategory)
{
    assure( component_->getStationType() == wns::service::dll::StationTypes::FRS(),
            "recursive connections only possible in relay stations" );

    relay::RSRelayMapper* relayMapper = component_->getFUN()
        ->findFriend<relay::RSRelayMapper*>("relayMapper");

    StationID associatedWithID = friends_.connectionManager
        ->getBasicConnectionFor( component_->getID() )->baseStation_;
    wimac::Component* associatedWith = static_cast<wimac::Component*>
        ( TheStationManager::getInstance()->getStationByID(associatedWithID) );
    assureType( associatedWith, wimac::Component*);


    // Basic CI
    ConnectionIdentifier bCI( associatedWith->getID(),
                              component_->getID(),
                              remote,
                              ConnectionIdentifier::Basic,
                              ConnectionIdentifier::Bidirectional,
                              ConnectionIdentifier::Signaling );

    // Primary Management CI
    ConnectionIdentifier pmCI( associatedWith->getID(),
                               component_->getID(),
                               remote,
                               ConnectionIdentifier::PrimaryManagement,
                               ConnectionIdentifier::Bidirectional,
                               ConnectionIdentifier::Signaling );

    // Downlink Data CI
    ConnectionIdentifier dlCI ( associatedWith->getID(),
                                component_->getID(),
                                remote,
                                ConnectionIdentifier::Data,
                                ConnectionIdentifier::Downlink,
                                qosCategory );

    // Uplink Data CI
    ConnectionIdentifier ulCI( associatedWith->getID(),
                               component_->getID(),
                               remote,
                               ConnectionIdentifier::Data,
                               ConnectionIdentifier::Downlink,
                               qosCategory );

    // get master ConnectionManager from destination access point
    ConnectionManager* associatedWithConnectionManager
        = this->getConnectionManagerMaster();

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
    relayMapper->addMapping( RelayMapping(bCI.cid_, basicCID) );
    relayMapper->addMapping( RelayMapping(pmCI.cid_, primaryCID) );
    relayMapper->addMapping( RelayMapping(dlCI.cid_, downlinkTransportCID) );
    relayMapper->addMapping( RelayMapping(ulCI.cid_, uplinkTransportCID) );



    if ( associatedWith->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        associatedWith
            ->getControlService<wimac::service::ConnectionControl>("connectionControl")
            ->createRecursiveConnection(bCI.cid_,
                                        pmCI.cid_,
                                        dlCI.cid_,
                                        ulCI.cid_,
                                        remote,
                                        qosCategory);
    }
}

ConnectionManager*
ControlPlaneManagerSimpleSS::getConnectionManagerMaster()
{
    return component_->getConnectionManagerMaster();
}

void
ControlPlaneManagerSimpleSS::onMSRCreated()
{
    friends_.connectionManager = component_
        ->getManagementService<ConnectionManager>(friends_.connectionManagerName);
}


