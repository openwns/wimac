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

#ifndef WIMAC_SERVICES_ASSOCIATIONCONTROL_H
#define WIMAC_SERVICES_ASSOCIATIONCONTROL_H

#include <WNS/ldk/ControlServiceInterface.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/StationManager.hpp>

namespace wimac { namespace service {

        class ConnectionManager;
        class AssociationControl :
            public wns::ldk::ControlService
        {
        public:
            AssociationControl( wns::ldk::ControlServiceRegistry* csr,
                               wns::pyconfig::View& config );


            void
            createRecursiveConnection( ConnectionIdentifier::CID basicCID,
                                       ConnectionIdentifier::CID primaryCID,
                                       ConnectionIdentifier::CID downlinkTransportCID,
                                       ConnectionIdentifier::CID uplinkTransportCID,
                                       ConnectionIdentifier::StationID remote,
                                       int qosCategory);
            void onCSRCreated();

            virtual void 
            doOnCSRCreated() = 0;

            void associateTo( wimac::StationID destination,
                              ConnectionIdentifier::QoSCategory category);

        private:
            struct {
                wimac::service::ConnectionManager* connectionManager;
            } friends_;

        };

    namespace associationcontrol {

        /** @brief Associate to node ID provided by PyConfig
         */
        class Fixed :
            public AssociationControl
        {
        public:
            Fixed(wns::ldk::ControlServiceRegistry* csr,
                                wns::pyconfig::View& config );
            virtual
            ~Fixed();
    
        private:
                virtual void
                doOnCSRCreated();

                wimac::StationID associatedWithID_;
    
        };
    }

    }
}

#endif


