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

#ifndef WIMAC_SERVICES_DISSOCIATING_HPP
#define WIMAC_SERVICES_DISSOCIATING_HPP

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/Logger.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/controlplane/Handover.hpp>


namespace wimac { namespace service {


        class DissociatingCallBackInterface
        {
        public:
            virtual ~DissociatingCallBackInterface()
            {
            }

            virtual void
            resultDissociating( const handoverStrategy::Interface::Stations
                                ackBaseStations )
            = 0;

        };



        /**
         * @brief Dissociating
         */
        class Dissociating:
            public wimac::controlplane::HandoverCallBackInterface
        {
        public:

            Dissociating( Component* layer,
                          DissociatingCallBackInterface* callBack,
                          const  wns::pyconfig::View& config );

            ~Dissociating()
            {
            }

            void
            start(const handoverStrategy::Interface::Stations targetBaseStations);

            /**
             * @brief HandoverCallBackInterface implementation
             */
            virtual void
            resultHandover(handoverStrategy::Interface::Stations newBaseStations);

            void
            onMSRCreated();

        private:

            void
            dissociating();

            int remainRetries_;
            handoverStrategy::Interface::Stations targetBaseStations_;

            wimac::Component* layer_;
            DissociatingCallBackInterface* callBack_;
            // Static values from PyConfig
            const int retries_;

            struct{
                std::string handoverProviderName;

                wimac::controlplane::HandoverSS* handoverProvider;
            } friends_;

        };
    }
}
#endif


