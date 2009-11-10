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
#ifndef WIMAC_CONNECTIONIDENTIFIER_HPP
#define WIMAC_CONNECTIONIDENTIFIER_HPP

#include <string>

#include <WNS/SmartPtr.hpp>
#include <WNS/Cloneable.hpp>

#include <WIMAC/StationManager.hpp>

namespace wimac {
    class Component;
    class ConnectionClassifier;
    class ConnectionRule;

    namespace service {
        class ConnectionManager;
    }

    namespace tests {
        class TestConnectionManager;
    }

    /**
     * @brief ConnectionIdentifier hold information for one connection.
     *
     * @sa ConnectionManager
     */
    class ConnectionIdentifier :
            public wns::RefCountable,
            public wns::Cloneable<ConnectionIdentifier>,
            public wns::IOutputStreamable
    {
    public:
        typedef wns::SmartPtr<ConnectionIdentifier> Ptr;
        typedef std::list<Ptr> List;
        typedef wimac::StationID StationID;
        typedef unsigned int CID;

        /**
         * @brief The type of the connection.
         */
        enum ConnectionType {
            NoType = 0,
            InitialRanging = 1,
            Basic = 2,
            PrimaryManagement = 3,
            SecondaryManagement = 4,
            Data = 5
        };

        enum Direction {
            NoDirection = 0x0,
            Downlink = 0x1,
            Uplink = 0x2,
            Bidirectional = 0x3 // Downlink | Uplink
        };

        enum QoSCategory {
            NoQoS = 0,
            Signaling = 1,
            UGS = 2,
            rtPS = 3,
            nrtPS = 4,
            BE = 5,
            MaxQoSCategory = BE
        };

        typedef int Frames;

        ConnectionIdentifier(StationID baseStation,
                              StationID subscriberStation,
                              StationID remoteStation,
                              ConnectionType connectionType,
                              Direction direction,
                              int qos);


        ConnectionIdentifier (StationID baseStation,
                              CID cid,
                              StationID subscriberStation,
                              StationID remoteStation,
                              ConnectionType connectionType,
                              Direction direction,
                              int qos);

        ConnectionIdentifier( const ConnectionIdentifier& other );

        /**
         * @brief Returns the connection id as integer.
         */
        CID getID() const { return cid_; }

        /**
         * @brief Prints the contents of the connection identifier as string.
         */
        std::string doToString() const;

        bool operator==( const ConnectionIdentifier& rhs ) const
        {
            return ( baseStation_ == rhs.baseStation_ )
                && ( cid_ == rhs.cid_ );
        }

        bool operator<( const ConnectionIdentifier& rhs ) const
        {
            if ( baseStation_ == rhs.baseStation_ )
                return cid_ < rhs.cid_;
            return baseStation_ < rhs.baseStation_;
        }

        /**
         * @brief Checks the data of the connection identifier for integrity.
         *
         * @todo Remove this method. This should not be necessary.
         */
        bool integrityCheck() const;

        ///****** Stored  informations about connection  *************/

        /**
         * @brief Primary keys for singularity of ConnectionIdentifier object.
         */
        ConnectionIdentifier::StationID baseStation_;
        CID cid_;


        ConnectionIdentifier::StationID subscriberStation_;
        ConnectionIdentifier::StationID remoteStation_;
        ConnectionType connectionType_;
        Direction direction_;
        int qos_;

        /// Simulator specific values, used by Scanning
        Frames ciNotListening_;

        /// commandKeyClasses to get right Command
        struct {
            wimac::ConnectionClassifier* connectionClassifier;
        } commandKeyClasses_;

        // Is this a valid ConnectionIdentifier or an empty one?
        bool valid_;

    private:
        explicit ConnectionIdentifier ();

        friend class wimac::service::ConnectionManager;
        friend class wimac::tests::TestConnectionManager;
    };

    typedef ConnectionIdentifier::Ptr ConnectionIdentifierPtr;
    typedef std::list<ConnectionIdentifierPtr> ConnectionIdentifiers;
}
#endif
