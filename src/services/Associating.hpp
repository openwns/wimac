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

#ifndef WIMAC_SERVICES_ASSOCIATING_HPP
#define WIMAC_SERVICES_ASSOCIATING_HPP

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/Logger.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/controlplane/Ranging.hpp>
#include <WIMAC/controlplane/MessageExchanger.hpp>
#include <WIMAC/controlplane/SetupConnection.hpp>

#include <WIMAC/services/handoverStrategy/Interface.hpp>


namespace wimac {
    class Component;

    namespace service {


        class AssociatingCallBackInterface
        {
        public:
            virtual ~AssociatingCallBackInterface()
            {
            }

            virtual void
            resultAssociating( const bool result, const double failure ) = 0;
        };

        /**
         * @brief Associating
	 */
        class Associating :
            public wimac::controlplane::RangingCallBackInterface,
            public wimac::controlplane::MessageExchangerCallBackInterface,
            public wimac::controlplane::SetupConnectionCallBackInterface
        {
            enum State{
                None,
                Ranging,
                Regristration,
                SetupConnection
            };

        public:
            typedef ConnectionIdentifier::QoSCategory  QoSCategory;


            Associating(Component* layer,
                        AssociatingCallBackInterface* const callBack,
                        const  wns::pyconfig::View& config);


            void
            start(handoverStrategy::Interface::Station targetBaseStation,
                  int qosCategory);


            /**
             * @brief RangingCallBackInterface implementation
             */
            virtual void
            resultRanging(bool result);


            /**
             * @brief MessageExchangeCallBackInterface implementation
             */
            virtual void
            resultMessageExchanger(std::string name, bool result);


            /**
             * @brief SetupConnectionCallBackInterface implementation
             */
            virtual void
            resultSetupConnection(bool result);


            void
            onMSRCreated();


        private:

            void doNextStep(State state);

            void result(bool result);


            handoverStrategy::Interface::Station targetBaseStation_;

            State state_;
            int remainRetries_;
            int qosCategory_;


            wimac::Component* layer_;
            AssociatingCallBackInterface* callBack_;

            // Static values from PyConfig
            const int retries_;

            struct{
                std::string rangingProviderName;
                std::string regristrationProviderName;
                std::string setupConnectionProviderName;

                wimac::controlplane::RangingSS* rangingProvider;
                wimac::controlplane::MessageExchanger* regristrationProvider;
                wimac::controlplane::SetupConnectionSS* setupConnectionProvider;
            } friends_;

        };
    }
}

#endif // WIMAC_SERVICES_ASSOCIATING_HPP


