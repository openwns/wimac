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

#ifndef WIMAC_FRAME_DLMAPCOLLECTOR_H
#define WIMAC_FRAME_DLMAPCOLLECTOR_H

#include <WNS/scheduler/MapInfoProviderInterface.hpp>

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/fcf/CompoundCollector.hpp>
#include <WNS/ldk/tools/UpUnconnectable.hpp>
#include <WNS/events/CanTimeout.hpp>

#include <WIMAC/frame/MapCommand.hpp>


namespace wimac {
    class Component;
    class PhyUser;
    namespace service {
        class ConnectionManager;
    }
    namespace scheduler {
        class Scheduler;
    }

    namespace frame {
        class BSDLScheduler;


        typedef MapCommand DLMapCommand;

        /**
         * @brief The sending entity of the DL MAP.
         */
        class DLMapCollector :
            public wns::ldk::fcf::CompoundCollector,
            public wns::ldk::CommandTypeSpecifier<DLMapCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::Cloneable<DLMapCollector>,
            public wns::events::CanTimeout,
            public wns::ldk::tools::UpUnconnectable
        {
        public:
            DLMapCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

            void onFUNCreated();

            void calculateSizes( const wns::ldk::CommandPool* commandPool, Bit& commandPoolSize, Bit& dataSize ) const;

            void doOnData( const wns::ldk::CompoundPtr& );

            void doStart(int);

            void doStartCollection(int){}
            void finishCollection(){}

            void onTimeout();

            wns::simulator::Time getCurrentDuration() const;


            /**
             * @brief Returns the duration of the DL phase.
             *
             * This method is only usefull in the receiving MapCollector.
             */
            wns::simulator::Time getDLPhaseDuration() const { return dlPhaseDuration_; }

        private:
            wimac::scheduler::Scheduler* dlScheduler_;
            std::string dlSchedulerName_;
            wimac::Component* component_;
            wimac::PhyUser* phyUser_;
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyMode;

            // members that are only used by the receiving MapCollector
            wimac::service::ConnectionManager* connectionManager_;
            wns::simulator::Time dlPhaseDuration_;
        };
    }
}
#endif

