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

#ifndef WIMAC_SERVICES_CONNECTIONCONTROL_H
#define WIMAC_SERVICES_CONNECTIONCONTROL_H

#include <WNS/ldk/ControlServiceInterface.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/StationManager.hpp>

namespace wimac { namespace service {

        class ConnectionManager;
        class ConnectionControl :
            public wns::ldk::ControlService
        {
        public:
            ConnectionControl( wns::ldk::ControlServiceRegistry* csr,
                               wns::pyconfig::View& config );


            void
            createRecursiveConnection( ConnectionIdentifier::CID basicCID,
                                       ConnectionIdentifier::CID primaryCID,
                                       ConnectionIdentifier::CID downlinkTransportCID,
                                       ConnectionIdentifier::CID uplinkTransportCID,
                                       ConnectionIdentifier::StationID remote,
                                       int qosCategory);
            void onCSRCreated();

            void associateTo( wimac::StationID destination,
                              ConnectionIdentifier::QoSCategory category);

        private:
            struct {
                wimac::service::ConnectionManager* connectionManager;
            } friends_;

            wimac::StationID associatedWithID_;
        };
    }
}

#endif


