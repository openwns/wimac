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


#ifndef WIMAC_FRAME_CONTENTIONCOLLECTOR_H
#define WIMAC_FRAME_CONTENTIONCOLLECTOR_H

#include <queue>

#include <WNS/ldk/fcf/CompoundCollector.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>

namespace wimac {

    namespace service{
        class ConnectionManager;
    }
    class Component;
    class ConnectionClassifier;
    class PhyUser;
    namespace frame {


        /**
         * @brief  A CompoundCollector that collects multiple compounds.
         */
        class ContentionCollector :
            public wns::ldk::fcf::CompoundCollector,
            public wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::Cloneable<ContentionCollector>,
            public wns::events::CanTimeout,
            public scheduler::PDUWatchProvider

        {
        public:
            ContentionCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

            void doOnData( const wns::ldk::CompoundPtr& );
            void doSendData( const wns::ldk::CompoundPtr& );

            bool doIsAccepting( const wns::ldk::CompoundPtr& compound ) const;

            void doStart(int);

            void onTimeout();

            void doStartCollection(int);
            void finishCollection(){ accepting_ = false; }
            wns::simulator::Time getCurrentDuration() const { return accumulatedDuration_; }

            virtual void onFUNCreated();

            void setBackOff(int backOff);


        protected:

            /**
             * @brief Calculate the duraction of this compound.
             */
            wns::simulator::Time getDuration(const wns::ldk::CompoundPtr&) const;


            bool accepting_;

            int backOff_;
            wns::simulator::Time maximumDuration_;
            wns::simulator::Time accumulatedDuration_;

            typedef std::deque<wns::ldk::CompoundPtr> Compounds;
            Compounds compounds_;

            Component* layer_;

            struct
            {
                wimac::ConnectionClassifier* classifier_;
                wimac::PhyUser* phyUser_;
                service::ConnectionManager* connectionManager;
            } friends_;


            // values from PyConfig
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyMode;

            struct{
                bool enabled;
                int slotLengthInSymbols;
                int numberOfSlots;
            } contentionAccess_;

        };

    }
}
#endif


