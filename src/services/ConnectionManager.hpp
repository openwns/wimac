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

#ifndef WIMAC_SERVICES_CONNECTIONMANAGER_HPP
#define WIMAC_SERVICES_CONNECTIONMANAGER_HPP

#include <WNS/ldk/ldk.hpp>
#include <list>
#include <WNS/Cloneable.hpp>
#include <WNS/SmartPtr.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>
#include <WNS/Subject.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/ConnectionRule.hpp>
#include <WIMAC/Logger.hpp>

namespace wns {
    namespace ldk {
        class ManagementServiceRegistry;
    }
}

namespace wimac {


    namespace service {

        class CIDNotFound :
            public wns::Exception
        {
        public:
            CIDNotFound(int line, const char* file) :
                wns::Exception()
            {
                *this << "Connection Identifier not found in "
                      << file << ":" << line;
            }
        private:
            CIDNotFound();
        };

        /**
         * @brief Simplest ConnectionManagerInterface for ACKSwitch
         * Unit Test.
         *
         * @todo Extend Interface maybe with refactoring of the
         * ConnectionManager (bmw)
         */
        class ConnectionManagerInterface
        {
        public:

            ConnectionIdentifierPtr
            virtual getBasicConnectionFor( const ConnectionIdentifier::CID cid ) = 0;
            virtual ~ConnectionManagerInterface() {}

        };

        class ConnectionDeletedNotification {
        public:
            virtual void notifyAboutConnectionDeleted( const ConnectionIdentifier ) = 0;
            virtual ~ConnectionDeletedNotification(){}
        };

        /**
         * @brief Manager to manage connections.
         *
         * The ConnectionManager holds a database for registered
         * connections.  Each connection is associated with a
         * connection rule. The ConnectionManager also acts like a
         * Classifier. Compounds may be classified by the
         * findConnection() method.
         *
         * @todo The ConnectionManager needs refactoring of the
         * interface. I suppose there are some redundant methods.
         */
        class ConnectionManager :
            public ConnectionManagerInterface,
            public wns::ldk::ManagementService,
            public wns::Subject<ConnectionDeletedNotification>
        {

            typedef wns::Subject<ConnectionDeletedNotification> Subject;
        public:

            ConnectionManager( wns::ldk::ManagementServiceRegistry* msr,
                               const wns::pyconfig::View& config);

            explicit ConnectionManager( const wns::pyconfig::View& config);

            ConnectionManager( const ConnectionManager& other );


            /**
             * @brief Register a new connection. Return
             * ConnectionIdentifier with new CID from AP()
             */
            ConnectionIdentifier
            appendConnection( const ConnectionIdentifier& connection );

            /**
             * @brief Delete all connections
             */
            void
            deleteAllConnections();

            /**
             * @brief Delete connections with given ID for departure
             * and destination.
             */
            void
            deleteConnectionsForBS( ConnectionIdentifier::StationID baseStation );

            /**
             * @brief Delete connections with given ID for departure
             * and destination.
             */
            void
            deleteConnectionsForSS( ConnectionIdentifier::StationID subscriberStation );

            /**
             * @brief Delete connection with given CID
             */
            void
            deleteCI( ConnectionIdentifier::CID cid );

            /**
             * @brief Changes values but not the primary keys of the
             * connection
             */
            void
            changeConnection( const ConnectionIdentifier& connection );

            /**
             * @brief Changes values but not the primary keys of the
             * connections
             */
            void
            changeConnections( ConnectionIdentifiers& connections );


            /**
             * @brief Find a registered connection for the given Compound.
             *
             * The compound does not need to be classified before. The
             * connection manager looks after a match in the
             * registered connection rules. This method is usually
             * only used by the ConnectionClassifier. To look for
             * connections for a already classified compound use
             * getConnectionWithID().
             *
             * @return pointer to the registered connection or an empty pointer
             */
            ConnectionIdentifierPtr
            findConnection( const wns::ldk::CompoundPtr& compound ) const;


            /**
             * @brief Get ConnectionIdentifier for that compound by
             * using CID from the wimac::Classifier.
             */
            ConnectionIdentifierPtr
            getConnection( const wns::ldk::CompoundPtr& compound ) const;


            /**
             * @brief Get a list of ConnectionIdentifiers for all
             * connections stored in the Connection Manager.
             */
            ConnectionIdentifiers
            getAllConnections();

            /**
             * @brief Get a list of all ConnectionIdentifiers which
             * belong to this subscriber station.
             */
            ConnectionIdentifiers
            getAllCIForSS( ConnectionIdentifier::StationID subscriberStation ) const;

            /**
             * @brief Get a list of all ConnectionIdentifiers which
             * belong to this base station.
             */
            ConnectionIdentifiers
            getAllCIForBS( ConnectionIdentifier::StationID baseStation );

            /**
             * @brief Get a list of ConnectionIdentifiers for incoming
             * connections from a specified departure.
             */
            ConnectionIdentifiers
            getIncomingConnections ( ConnectionIdentifier::StationID from );


            /**
             * @brief Get a list of ConnectionIdentifiers for incoming
             * data connections from a specified departure
             */
            ConnectionIdentifiers
            getIncomingDataConnections(ConnectionIdentifier::StationID from, 
                                       ConnectionIdentifier::QoSCategory);

            /**
             * @brief Get a list of ConnectionIdentifiers for outgoing
             * connections to a specified destination
             */
            ConnectionIdentifiers
            getOutgoingConnections( ConnectionIdentifier::StationID to);

            /**
             * @brief Get a list of ConnectionIdentifiers for outgoin
             * data connections to a specified destination
             */
            ConnectionIdentifiers
            getOutgoingDataConnections(ConnectionIdentifier::StationID to, 
                                       ConnectionIdentifier::QoSCategory);

            /**
             * @brief Get all data connections with the specified direction.
             *
             * @sa ConnectionIdentifier::Direction
             */
            ConnectionIdentifiers
            getAllDataConnections(int direction, 
                                  ConnectionIdentifier::QoSCategory);

            ConnectionIdentifiers
            getAllDataConnections(int direction);


            /**
             * @brief Get a special connection of type connectionType.
             *
             * Find a special connection between the baseStation and
             * the subscriber of type connectionType.
             *
             * @sa ConnectionIdentifier::ConnectionType
             */
            ConnectionIdentifierPtr getSpecialConnection(
                ConnectionIdentifier::ConnectionType connectionType,
                ConnectionIdentifier::StationID baseStation,
                ConnectionIdentifier::StationID subscriber );


            /**
             * @brief Return the basic connection that belongs to the
             * given connection.
             *
             * This is used by the ACKSwitch to switch the connection
             * of an ACK that needs to be transmitted via the basic
             * connection.
             */
            ConnectionIdentifierPtr
            getBasicConnectionFor( const ConnectionIdentifier::CID cid );


            /**
             * @brief Return the basic connection that belongs to the
             * given subscriberStation.
             *
             * This is used by the Handover.
             */
            ConnectionIdentifierPtr
            getBasicConnectionFor( const StationID subscriberStation ) const;

            /**
             * @brief Return the primary management connection that
             * belongs to the given connection
             */
            ConnectionIdentifierPtr
            getPrimaryConnectionFor( ConnectionIdentifier::StationID stationID ) const;

            /**
             * @brief Find the connection with the specified CID.
             */
            ConnectionIdentifierPtr
            getConnectionWithID( ConnectionIdentifier::CID cid ) const;

            /**
             * @brief Find all basic connection.
             */
            ConnectionIdentifiers getAllBasicConnections() const;


            /**
             * @brief Decrease all ConnectionIdentifiers who are not listening.
             */
            void
            decreaseCINotListening();

            void onMSRCreated();

            ConnectionIdentifier::CID getAndIncreaseHighestCellCID();

        private:
            /**
             * @brief Delete ConnectionIdentifier from
             * ConnectionManager if they exist.
             */
            void singularityDelete(const ConnectionIdentifierPtr ci);

            /**
             * @brief On a double basic ConnectionIdentifier all other
             * ConnectionIdentifier for peer station will be deleted.
             */
            void doubleBasicCIDeleteAllOtherCI(const ConnectionIdentifierPtr ci);

            ConnectionIdentifiers connectionIdentifiers_;

            ConnectionIdentifier::CID highestCID_;

            /**
             * @brief Station this ConnectionManager belongs to.
             */
            Component* layer_;

            /**
             * @todo Remove the pyconfig view from here.
             */
            wns::pyconfig::View config_;
        };
    }
}
#endif

