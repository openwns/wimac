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

#ifndef WIMAC_SERVICES_FURESETER_HPP
#define WIMAC_SERVICES_FURESETER_HPP


#include <WNS/pyconfig/View.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>

#include <WIMAC/Logger.hpp>
#include <WIMAC/ConnectionKey.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>

#include <list>

namespace wns{ namespace probe{ namespace idprovider{
            class Variable;
}}}

namespace wns { namespace ldk { 
    class ManagementServiceRegistry;
}}

namespace wimac{ namespace service{



        /**
         * @brief FUResetInterface for a Functional Unit, which should be reset by
         * FUReseter.
         */
        class FUResetInterface
        {
        public:
            virtual ~FUResetInterface(){}
            virtual void resetCID(ConnectionIdentifier::CID cid) = 0;
        };

        /**
         * @brief FUReseter is an management service, which reset all Functional Units
         * and Flows it knows by PyConfig.
         */
        class FUReseter :
            public wns::ldk::ManagementService
        {
        public:
            FUReseter(wns::ldk::ManagementServiceRegistry* msr, const wns::pyconfig::View& config );

            void
            onMSRCreated();

            void
            resetAll(ConnectionIdentifier::Ptr ci)
            {
                this->resetFlowSeparator(ci);
                this->resetCID(ci);
            }

            void
            resetFlowSeparator(ConnectionIdentifier::Ptr ci);

            void
            resetCID(ConnectionIdentifier::Ptr ci);

        private:
            wns::ldk::fun::FUN* fun_;

            struct{
                std::list<std::string> flowSeparatorNames;
                std::list<std::string> resetFUNames;

                std::list<wns::ldk::FlowSeparator*> flowSeparators;
                std::list<FUResetInterface*> resetFUs;
            } friends_;
        };
    }
}

#endif
