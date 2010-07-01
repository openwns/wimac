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

#include <WIMAC/services/ConnectionManager.hpp>
#include <sstream>


#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/service/dll/StationTypes.hpp>

#include <WIMAC/StationManager.hpp>
#include <WIMAC/UpperConvergence.hpp>

#include <WIMAC/ConnectionRule.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/Classifier.hpp>



STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::ConnectionManager,
    wns::ldk::ManagementServiceInterface,
    "wimac.services.ConnectionManager",
    wns::ldk::MSRConfigCreator);


using namespace wimac;
using namespace wimac::service;

ConnectionManager::ConnectionManager( wns::ldk::ManagementServiceRegistry* msr,
                                      const wns::pyconfig::View& config ) :
    ManagementService( msr ),
    connectionIdentifiers_(),
    highestCID_( 1 ),
    config_( config )
{
    layer_ = dynamic_cast<Component*>(msr->getLayer());
    assure(layer_, "ConnectionManager can only be used in wimac::Component environmnet." );
}

ConnectionManager::ConnectionManager( const ConnectionManager& other ) :
    wns::ldk::ManagementServiceInterface( other ),
    Subject::SubjectType( other ),
    ConnectionManagerInterface(),
    wns::ldk::ManagementService( other ),
    Subject( other ),
    connectionIdentifiers_( other.connectionIdentifiers_ ),
    highestCID_( other.highestCID_ ),
    layer_( other.layer_ ),
    config_( other.config_ )
{}

ConnectionIdentifier
ConnectionManager::appendConnection( const ConnectionIdentifier& connection )
{
    assure(connection.integrityCheck(),
           "ConnectionManager::appendConnection: New ConnectionIdentifier doesn't pass the integrityCheck!");

    // append ConnectionIdentifier
    ConnectionIdentifierPtr connectionPtr( new ConnectionIdentifier( connection ) );

    if ( connectionPtr->cid_ == -1 )
        connectionPtr->cid_ = getAndIncreaseHighestCellCID();


    if( layer_->getStationType() == wns::service::dll::StationTypes::AP() )  //set CID in AP
    {
        if(connectionPtr->connectionType_ == ConnectionIdentifier::InitialRanging)
        {
            if( this->getConnectionWithID(0) )
            {
                LOG_INFO( getMSR()->getLayer()->getName(),
                          " Ranging ConnectionIdentifier (CID: 0) already exist.");
                return ConnectionIdentifier(*(this->getConnectionWithID(0)));
            }else
            {
                connectionPtr->cid_ = 0; //Ranging CID
            }
        }
    }

    // remove all CI for station if double basic CI happened
    //this->doubleBasicCIDeleteAllOtherCI( connectionPtr );

    // Delete ConnectionIdentifier if it always exist
    //this->singularityDelete( connectionPtr );

    // Check for validation
    assure(connectionPtr->cid_ >= 0,
           "ConnectionManager::appendConnection: CID of ConectionIdentifier isn't valid!");

    connectionIdentifiers_.push_back( connectionPtr );

    LOG_INFO( getMSR()->getLayer()->getName() , ": Register",
              *connectionPtr );

    return ConnectionIdentifier(*connectionPtr);
}



void
ConnectionManager::deleteAllConnections()
{
    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); ++it) {

        LOG_INFO( getMSR()->getLayer()->getName() , ": delete ",
                  **it );
        wns::Subject<ConnectionDeletedNotification>::sendNotifies
            (&ConnectionDeletedNotification::notifyAboutConnectionDeleted, **it);

    }
    connectionIdentifiers_.clear();
}



void
ConnectionManager::deleteConnectionsForBS( ConnectionIdentifier::StationID baseStation )
{
    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); )
    {
        if (  (*it)->baseStation_ == baseStation )
        {
            std::ostringstream log;
            log << getMSR()->getLayer()->getName() << ": delete "
                << **it;
            LOG_INFO( log.str() );

            wns::Subject<ConnectionDeletedNotification>::sendNotifies
                (&ConnectionDeletedNotification::notifyAboutConnectionDeleted, **it);

            connectionIdentifiers_.erase( it++ );
        } else
            ++it;
    }
}



void
ConnectionManager::deleteConnectionsForSS( ConnectionIdentifier::StationID subscriberStation )
{
    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); )
    {
        if ( (*it)->subscriberStation_ == subscriberStation)
        {
            std::ostringstream log;
            log << getMSR()->getLayer()->getName() << ": delete "
                << **it;
            LOG_INFO( log.str() );

            wns::Subject<ConnectionDeletedNotification>::sendNotifies
                (&ConnectionDeletedNotification::notifyAboutConnectionDeleted, **it);

            connectionIdentifiers_.erase( it++ );
        } else
            ++it;
    }

}

void
ConnectionManager::deleteCI( ConnectionIdentifier::CID cid )
{
    int ciDeleted = 0;

    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); )
    {
        if ( (*it)->cid_ == cid )
        {
            std::ostringstream log;
            log << getMSR()->getLayer()->getName() << ": delete "
                << **it;
            LOG_INFO( log.str() );

            wns::Subject<ConnectionDeletedNotification>::sendNotifies
                (&ConnectionDeletedNotification::notifyAboutConnectionDeleted, **it);

            connectionIdentifiers_.erase( it++ );
            ++ciDeleted;
        } else
            ++it;
    }

    std::ostringstream log1, log2;
    log1 << getMSR()->getLayer()->getName()
         <<"ConnectionManager::deleteCI: No ConnectionIdentifier found! CID:"
         << cid << "\n";
    assure( ciDeleted > 0,
            log1.str() );

    log2 << getMSR()->getLayer()->getName()
         << "ConnectionManager::deleteCI: More than one ConnectoinIdentifer deleted! CID:"
         << cid << "\n";
    assure( ciDeleted <= 1,
            log2.str() );
}


void
ConnectionManager::changeConnection( const ConnectionIdentifier& connection )
{
    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); )
    {
        if ( **it == connection )
        {
            std::ostringstream log;
            log << getMSR()->getLayer()->getName() << ": Changes"
                << **it;
            LOG_INFO( log.str() );

            ConnectionIdentifierPtr connectionPtr(
                new ConnectionIdentifier( connection ) );
            connectionIdentifiers_.insert( it, connectionPtr);
            connectionIdentifiers_.erase( it++ );
            return;
        } else
            ++it;
    }
    throw wns::Exception( "wimac::ConnectionManager::changeConnection: ConnectionIdentifier not found" );
}



void
ConnectionManager::changeConnections( ConnectionIdentifiers& connections )
{
    for ( ConnectionIdentifiers::iterator it1 = connections.begin();
          it1 != connections.end();
          ++it1)
    {

        std::ostringstream log;
        log << getMSR()->getLayer()->getName() << ": Changes"
            << **it1;
        LOG_INFO( log.str() );

        bool ciFound = false;
        for (ConnectionIdentifiers::iterator it2 = connectionIdentifiers_.begin();
             it2 != connectionIdentifiers_.end(); )
        {
            if ( **it1 == **it2 )
            {
                ciFound = true;
                ConnectionIdentifierPtr connectionPtr(
                    new ConnectionIdentifier(*(*it1)));
                connectionIdentifiers_.insert( it2, connectionPtr);
                connectionIdentifiers_.erase( it2++ );
                break;
            } else
                ++it2;
        }

        if(!ciFound)
            throw wns::Exception("wimac::ConnectionManager::changeConnections: would change a ConnectionIdentifier which isn't available! \n");
    }
}



ConnectionIdentifiers
ConnectionManager::getAllConnections( )
{
    ConnectionIdentifiers connections;

    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); ++it )
    {
        connections.push_back( ConnectionIdentifierPtr(
                                   new ConnectionIdentifier(*(*it)) ) );
    }
    return connections;
}



ConnectionIdentifiers
ConnectionManager::getAllCIForSS( ConnectionIdentifier::StationID subscriberStation ) const
{
    ConnectionIdentifiers results
        = ConnectionIdentifiers();

    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if ( (*it)->subscriberStation_ == subscriberStation)
        {
            results.push_back( ConnectionIdentifierPtr(
                                   new ConnectionIdentifier(*(*it)) ) );
        }
    }

    return results;
}


ConnectionIdentifiers
ConnectionManager::getAllCIForBS( ConnectionIdentifier::StationID baseStation )
{
    ConnectionIdentifiers results
        = ConnectionIdentifiers();

    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if ( (*it)->baseStation_ == baseStation)
        {
            results.push_back( ConnectionIdentifierPtr(
                                   new ConnectionIdentifier(*(*it)) ) );
        }
    }

    return results;
}


ConnectionIdentifiers
ConnectionManager::getIncomingDataConnections( ConnectionIdentifier::StationID from,                                                                                   ConnectionIdentifier::QoSCategory qos)
{
    ConnectionIdentifiers connections;
    if( layer_->getStationType() == wns::service::dll::StationTypes::AP() )
    {
        // all uplink connections are incoming
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( (*conn)->remoteStation_ == from )
                 && ( (*conn)->direction_ == ConnectionIdentifier::Uplink )
                 && ( (*conn)->connectionType_ == ConnectionIdentifier::Data )
                 && ( (*conn)->qos_ == qos )
                )
                connections.push_back( *conn );
        }
    }else if (layer_->getStationType() == wns::service::dll::StationTypes::UT() )
    {
        // all downlink connections are incoming
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( (*conn)->baseStation_ == from )
                 && ( (*conn)->direction_ == ConnectionIdentifier::Downlink )
                 && ( (*conn)->connectionType_ == ConnectionIdentifier::Data )
                 && ( (*conn)->qos_ == qos )
                )
                connections.push_back( *conn );
        }
    }else if( layer_->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        // all downlink connections are incoming
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( ( (*conn)->baseStation_ == from )
                   && ( (*conn)->direction_ == ConnectionIdentifier::Downlink )
                   && ( (*conn)->connectionType_ == ConnectionIdentifier::Data ) 
                   && ( (*conn)->qos_ == qos ) )
                 ||
                 ( ( (*conn)->subscriberStation_ == from )
                   && ( (*conn)->direction_ == ConnectionIdentifier::Uplink )
                   && ( (*conn)->connectionType_ == ConnectionIdentifier::Data ) 
                   && ( (*conn)->qos_ == qos ) )
                )
                connections.push_back( *conn );
        }
    }else
    {
        assure( 0, "unsupported station type" );
    }
    return connections;
}



ConnectionIdentifiers
ConnectionManager::getOutgoingDataConnections( ConnectionIdentifier::StationID to,
                                        ConnectionIdentifier::QoSCategory qos)
{
    ConnectionIdentifiers connections;
    if( layer_->getStationType() == wns::service::dll::StationTypes::AP() )
    {
        // all downlink connections are outgoing
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( (*conn)->remoteStation_ == to )
                 && ( (*conn)->direction_ == ConnectionIdentifier::Downlink )
                 && ( (*conn)->connectionType_ == ConnectionIdentifier::Data )
                 && ( (*conn)->qos_ == qos )
                )
                connections.push_back( *conn );
        }
    } else if( layer_->getStationType() == wns::service::dll::StationTypes::UT() )
    {
        // all uplink connections are outgoing
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( (*conn)->baseStation_ == to )
                 && ( (*conn)->direction_ == ConnectionIdentifier::Uplink )
                 && ( (*conn)->connectionType_ == ConnectionIdentifier::Data )
                 && ( (*conn)->qos_ == qos )
                )
                connections.push_back( *conn );
        }
    } else if( layer_->getStationType() == wns::service::dll::StationTypes::RUT() )
    {
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( ( (*conn)->baseStation_ == to )
                   && ( (*conn)->direction_ == ConnectionIdentifier::Uplink )
                   && ( (*conn)->connectionType_ == ConnectionIdentifier::Data ) 
                   && ( (*conn)->qos_ == qos ) )
                 ||
                 ( ( (*conn)->subscriberStation_ == to )
                   && ( (*conn)->direction_ == ConnectionIdentifier::Downlink )
                   && ( (*conn)->connectionType_ == ConnectionIdentifier::Data ) 
                   && ( (*conn)->qos_ == qos ) )
                )
                connections.push_back( *conn );
        }
    } else
    {
        assure( 0, "unsupported station type" );
    }
    return connections;
}



ConnectionIdentifiers
ConnectionManager::getOutgoingConnections( ConnectionIdentifier::StationID to )
{
    ConnectionIdentifiers connections;
    if( layer_->getStationType() == wns::service::dll::StationTypes::AP() )
    {
        // all downlink connections are outgoing
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( ( (*conn)->remoteStation_ == to ) || ( (*conn)->subscriberStation_ == to) ) &&
                 ((*conn)->direction_ != ConnectionIdentifier::Uplink) )
                connections.push_back( *conn );
        }
    } else if( layer_->getStationType() == wns::service::dll::StationTypes::UT() )
    {
        // all uplink connections are outgoing
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( (*conn)->baseStation_ == to ) && ((*conn)->direction_ !=  ConnectionIdentifier::Downlink) )
                connections.push_back( *conn );
        }
    } else if (  layer_->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( ( (*conn)->baseStation_ == to ) && ( (*conn)->direction_ &  ConnectionIdentifier::Uplink  ) )
                 || ( ( (*conn)->subscriberStation_ == to ) && ( (*conn)->direction_ &  ConnectionIdentifier::Downlink ) ) )
                connections.push_back( *conn );
        }
    }else
    {
        std::ostringstream error;
        error << "unsupported station type: " << 
            wns::service::dll::StationTypes::toString(layer_->getStationType());
        throw wns::Exception( error.str() );
    }
    return connections;
}

ConnectionIdentifiers
ConnectionManager::getIncomingConnections( ConnectionIdentifier::StationID from )
{
    ConnectionIdentifiers connections;
    if( layer_->getStationType() == wns::service::dll::StationTypes::AP() )
    {
        // all uplink connections are incomming
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( ( (*conn)->remoteStation_ == from ) || ( (*conn)->subscriberStation_ == from) ) &&
                 ((*conn)->direction_ != ConnectionIdentifier::Downlink) )
                connections.push_back( *conn );
        }
    } else if( layer_->getStationType() == wns::service::dll::StationTypes::UT() )
    {
        // all downlink connections are incomming
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( (*conn)->baseStation_ == from ) && ((*conn)->direction_ !=  ConnectionIdentifier::Uplink) )
                connections.push_back( *conn );
        }
    } else if (  layer_->getStationType() == wns::service::dll::StationTypes::FRS() )
    {
        for ( ConnectionIdentifiers::const_iterator conn =
                  connectionIdentifiers_.begin();
              conn != connectionIdentifiers_.end();
              ++conn )
        {
            if ( ( ( (*conn)->baseStation_ == from ) && ( (*conn)->direction_ &  ConnectionIdentifier::Downlink ) )
                 || ( ( (*conn)->subscriberStation_ == from ) && ( (*conn)->direction_ &  ConnectionIdentifier::Uplink ) ) )
                connections.push_back( *conn );
        }
    }else
    {
        std::ostringstream error;
        error << "unsupported station type: " << 
            wns::service::dll::StationTypes::toString(layer_->getStationType());
        throw wns::Exception( error.str() );
    }
    return connections;
}

ConnectionIdentifiers
ConnectionManager::getAllDataConnections(int direction, 
                                         ConnectionIdentifier::QoSCategory qos)
{
    ConnectionIdentifiers connections;
    for ( ConnectionIdentifiers::const_iterator it =
              connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if (  (*it)->direction_ == direction
              &&(*it)->connectionType_ == ConnectionIdentifier::Data 
              && ( (*it)->qos_ == qos ) )
        {
            connections.push_back(*it);
        }
    }
    return connections;
}

ConnectionIdentifiers
ConnectionManager::getAllDataConnections(int direction)
{
    ConnectionIdentifiers connections;
    for ( ConnectionIdentifiers::const_iterator it =
              connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if (  (*it)->direction_ == direction
              &&(*it)->connectionType_ == ConnectionIdentifier::Data)
        {
            connections.push_back(*it);
        }
    }
    return connections;
}


class SpecialConnectionMatcher :
    public std::unary_function<ConnectionIdentifierPtr,
                               bool>
{
public:
    SpecialConnectionMatcher(
        ConnectionIdentifier::ConnectionType connectionType,
        ConnectionIdentifier::StationID baseStation,
        ConnectionIdentifier::StationID subscriber
        ):
        connectionType_( connectionType ),
        baseStation_( baseStation ),
        subscriberStation_ ( subscriber )
    {}

    bool operator()(
        const ConnectionIdentifierPtr& identifier )
    {
        return ( connectionType_ == identifier->connectionType_ )
            && ( baseStation_ == identifier->baseStation_ )
            && ( subscriberStation_ == identifier->subscriberStation_ );
    }

private:
    ConnectionIdentifier::ConnectionType connectionType_;
    ConnectionIdentifier::StationID baseStation_;
    ConnectionIdentifier::StationID subscriberStation_;
};


ConnectionIdentifierPtr
ConnectionManager::getSpecialConnection(
    ConnectionIdentifier::ConnectionType connectionType, ConnectionIdentifier::StationID baseStation,
    ConnectionIdentifier::StationID subscriber )
{
    ConnectionIdentifiers::const_iterator match =
        find_if( connectionIdentifiers_.begin(), connectionIdentifiers_.end(),
                 SpecialConnectionMatcher( connectionType, baseStation,
										   subscriber ) );
    if ( match == connectionIdentifiers_.end() )
        return ConnectionIdentifierPtr();
    return *match;
}



class ConnectionWithIDMatcher :
    public std::unary_function<ConnectionIdentifierPtr, bool>
{
public:
    ConnectionWithIDMatcher( ConnectionIdentifier::CID cid )
        : cid_( cid )
    {}

    bool operator()(const ConnectionIdentifierPtr& identifier )
    {
        return cid_ == identifier->cid_;
    }

private:
    ConnectionIdentifier::CID cid_;
};



ConnectionIdentifierPtr
ConnectionManager::getConnectionWithID( ConnectionIdentifier::CID cid ) const
{
    ConnectionIdentifiers::const_iterator match =
        find_if( connectionIdentifiers_.begin(), connectionIdentifiers_.end(),
                 ConnectionWithIDMatcher( cid ) );

    if ( match == connectionIdentifiers_.end() )
    {
        //std::stringstream ss;
        //ss <<": No ConnectionIdentifier registered for cid:" << cid;
        //assure( 0, ss.str() );
        return ConnectionIdentifierPtr();
    }
    return *match;
}



ConnectionIdentifierPtr
ConnectionManager::getBasicConnectionFor( const ConnectionIdentifier::CID cid )
{
    ConnectionIdentifierPtr ci = getConnectionWithID( cid );

    if ( ci->connectionType_ == ConnectionIdentifier::Basic )
        return ci;

    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); ++it )
    {
        if(    ( (*it)->baseStation_  == ci->baseStation_ )
               && ( (*it)->subscriberStation_ == ci->subscriberStation_ )
               && ( (*it)->connectionType_ == ConnectionIdentifier::Basic ) )
        {
            return *it;
        }
    }

    throw CIDNotFound(__LINE__, __FILE__);
}



ConnectionIdentifierPtr
ConnectionManager::getBasicConnectionFor( const StationID subscriberStation ) const
{
    ConnectionIdentifierPtr basicConnection;
    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if ( (*it)->subscriberStation_ == subscriberStation &&
             (*it)->connectionType_ == ConnectionIdentifier::Basic )
        {
            return *it;
        }
    }
    throw CIDNotFound(__LINE__, __FILE__);
}



ConnectionIdentifiers
ConnectionManager::getAllBasicConnections( ) const
{
    ConnectionIdentifiers basics = ConnectionIdentifiers();
    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if ( (*it)->connectionType_ == ConnectionIdentifier::Basic )
            basics.push_back( *it );
    }
    return basics;
}



ConnectionIdentifierPtr
ConnectionManager::getPrimaryConnectionFor( ConnectionIdentifier::StationID stationID )
    const
{
    ConnectionIdentifiers primaryCIs;
    ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
    for (; it != connectionIdentifiers_.end(); ++it )
    {
        if (   ( (*it)->connectionType_ == ConnectionIdentifier::PrimaryManagement )
               && (    ( (*it)->subscriberStation_ == stationID )
                       || ( (*it)->baseStation_ == stationID )
                   )
            )
        {
            primaryCIs.push_back(*it);
        }

    }

    assure( primaryCIs.size() <=1,
            "ConntionManager::getPrimaryconnectionFor: Only one primary ConnectionIdentifier should exist for each station");

    if( primaryCIs.size() )
    {
        return ConnectionIdentifierPtr( new ConnectionIdentifier( *primaryCIs.front() ) );
    }else
    {
        return ConnectionIdentifierPtr();
    }
}


void
ConnectionManager::onMSRCreated()
{}

void
ConnectionManager::decreaseCINotListening()
{
    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end();
          ++it )
    {
        if ( (*it)->ciNotListening_ > 0 )
        {
            std::ostringstream log;
            log << getMSR()->getLayer()->getName() << ": decrease CINotListening"
                << **it;
            LOG_INFO( log.str() );

            ((*it)->ciNotListening_)--;
        }
    }
}



ConnectionIdentifierPtr
ConnectionManager::getConnection( const wns::ldk::CompoundPtr& compound ) const
{
    ConnectionIdentifiers connections;

    for ( ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); ++it )
    {
        if( (*it)->commandKeyClasses_.connectionClassifier )
        {
            wns::ldk::ClassifierCommand* cCommand;
            cCommand = (*it)->commandKeyClasses_.connectionClassifier
                ->getCommand( compound->getCommandPool() );

            if ( (*it)->cid_ == cCommand->peer.id )
            {
                connections.push_back( *it );
            }
        }
    }

    assure( connections.size() <= 1 ,
            "ConnectionManager::getConnections: Only one CI should be found. \n");

    if ( connections.empty() )
        return  ConnectionIdentifierPtr();
    return *connections.begin();
}


void
ConnectionManager::singularityDelete(const ConnectionIdentifierPtr ci )
{
/* At the moment every connection are singular, in future data connection won't
	if ( ci->connectionType == ConnectionIdentifier::Data )
		return;
*/

    ConnectionIdentifiers peerCIS
        = ConnectionIdentifiers();

    for ( ConnectionIdentifiers::iterator it = connectionIdentifiers_.begin();
          it != connectionIdentifiers_.end(); ++it )
    {
        if(   (layer_->getStationType() == wns::service::dll::StationTypes::AP())
              || (layer_->getStationType() == wns::service::dll::StationTypes::FRS()) )
        {// Get only CI for this UserTerminal
            if(    ((*it)->subscriberStation_ == ci->subscriberStation_)
                   &&  ((*it)->baseStation_ == ci->baseStation_) )
            {
                peerCIS.push_back(*it);
            }
        } else // Get all CI in this UserTerinal because it can only be connected to
            // on AccressPoint
        {
            peerCIS.push_back(*it);
        }
    }

    /// Check if a ConnectionIdentifier of same type exist and delete it
    for( ConnectionIdentifiers::const_iterator it = peerCIS.begin();
         it != peerCIS.end(); )
    {
        ConnectionIdentifiers::const_iterator copyIt = it;
        ++it;

        if(   ( (*copyIt)->connectionType_ == ci->connectionType_ )
              && ( (*copyIt)->direction_ == ci->direction_ ) )
        {
            LOG_INFO( getMSR()->getLayer()->getName(),
                      ": ConnectionIdentifier to append always exist! Only one ConnectionIdentifier of each Typ is allowed!");
            this->deleteCI((*copyIt)->cid_);
        }
    }
}



void
ConnectionManager::doubleBasicCIDeleteAllOtherCI(const ConnectionIdentifierPtr ci)
{
    // Only act on basic ConnectionIdentifier
    if(ci->connectionType_ != ConnectionIdentifier::Basic)
        return;

    ConnectionIdentifiers::const_iterator it = connectionIdentifiers_.begin();
    for(; it != connectionIdentifiers_.end(); ++it )
    {
        if (    ( (*it)->connectionType_ == ConnectionIdentifier::Basic )
                && ( (*it)->subscriberStation_ == ci->subscriberStation_ )
                && ( (*it)->baseStation_ == ci->baseStation_ )
            )
        {
            break;
        }
    }

    // No basic ConnectionIdentifier  found
    if(it == connectionIdentifiers_.end())
        return;

    // get all ConnectionIdentifier for peer without Ranging
    std::list<ConnectionIdentifier::CID> cids;
    for(ConnectionIdentifiers::const_iterator it2 = connectionIdentifiers_.begin();
        it2 != connectionIdentifiers_.end(); ++it2 )
    {
        if(   ( (*it2)->connectionType_ != ConnectionIdentifier::InitialRanging )
              && ( (*it2)->subscriberStation_ == (*it)->subscriberStation_ )
              && ( (*it2)->baseStation_ == (*it)->baseStation_ )
            )
        {
            cids.push_back((*it2)->cid_);
        }
    }

    // delete all ConnectionIdentifier without Ranging for peer
    while( !cids.empty() )
    {
        this->deleteCI(cids.front());
        cids.pop_front();
    }
}

ConnectionIdentifier::CID ConnectionManager::getAndIncreaseHighestCellCID()
{
    if ( layer_->getStationType() == wns::service::dll::StationTypes::AP() )
        return highestCID_++;

    Component* associatedWith = TheStationManager::getInstance()
        ->getStationByID( getBasicConnectionFor( layer_->getID() )->baseStation_ );
    return associatedWith->getManagementService<ConnectionManager>("connectionManager")
        ->getAndIncreaseHighestCellCID();
}
