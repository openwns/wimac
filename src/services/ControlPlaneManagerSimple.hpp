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

#ifndef WIMAC_SERVICES_CONTROLPLANESIMPLE_HPP
#define WIMAC_SERVICES_CONTROLPLANESIMPLE_HPP

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>
#include <WNS/pyconfig/View.hpp>

#include <WIMAC/services/ControlPlaneManagerInterface.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/relay/RelayMapper.hpp>



namespace wimac{

    class Layer2;

    namespace service{

        class ConnectionManager;


        /**
         * @brief ControlplaneSimple provides simple Controlplane functionality
         *
         *
         *
         */
        class ControlPlaneManagerSimpleSS :
            public ControlPlaneManagerInterface,
            public wns::ldk::ManagementService
        {
            typedef relay::RSRelayMapper::RelayMapping RelayMapping;

        public:
            typedef ConnectionIdentifier::StationID StationID;
            typedef ConnectionIdentifier::CID CID;


            ControlPlaneManagerSimpleSS(wns::ldk::ManagementServiceRegistry* msr,
                                        const wns::pyconfig::View& config);

            /**
             * @brief ControlPlaneManagerInterface implementation
             */
            void
            start(StationID associateTo, int qosCategory);

            void
            associateAndConnectTo(StationID associateTo, int qosCategory);

            void
            createRecursiveConnection(CID basicCID,
                                      CID primaryCID,
                                      CID downlinkTransportCID,
                                      CID uplinkTransportCID,
                                      StationID remote,
                                      int qosCategory);

            ConnectionManager*
            getConnectionManagerMaster();

            /**
             * @brief ControlPlaneManagerInterface implementation
             */
            void
            onMSRCreated();



        private:
            std::string msName_;
            wimac::Component* component_;

            struct{
                std::string connectionManagerName;

                ConnectionManager* connectionManager;
            } friends_;

            // Probes
            wns::probe::bus::contextprovider::Variable probeAssociatedToContextProvider_;
            wns::probe::bus::contextprovider::Variable probeQoSCategoryContextProvider_;
            // Static values from PyConfig
        };
    }
}

#endif


