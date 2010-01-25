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

#ifndef WIMAC_FRAME_DATACOLLECTOR_HPP
#define WIMAC_FRAME_DATACOLLECTOR_HPP

#include <WNS/ldk/fcf/CompoundCollector.hpp>

namespace wimac {
    namespace scheduler {
        class Interface;
        class Scheduler;
    }
}

namespace wimac {
    namespace frame {

        /**
         * @brief The DataCollector provides a common frame for user data scheduler.
         */
        class DataCollector :
            public wns::ldk::fcf::CompoundCollector,
            public wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::Cloneable<DataCollector>,
            public wns::events::CanTimeout
        {
        public:
            DataCollector(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& view);
            DataCollector(const DataCollector& rhs);

            // CompoundHandler interface
            void onFUNCreated();
            void doSendData(const wns::ldk::CompoundPtr&);
            void doOnData(const wns::ldk::CompoundPtr&);
            bool doIsAccepting(const wns::ldk::CompoundPtr&) const;

            // CompoundCollector interface
            void doStartCollection(int);
            void finishCollection();
            void doStart(int);
            wns::simulator::Time getCurrentDuration() const;

            // CanTimeout interface
            void onTimeout();

            wimac::scheduler::Interface* getTxScheduler() const
            {
                return txScheduler.get();
            }

            wimac::scheduler::Interface* getRxScheduler() const
            {
                return rxScheduler.get();
            }

        private:

            wimac::scheduler::Interface*
            getCurrentScheduler() const;

            std::auto_ptr<wimac::scheduler::Interface> txScheduler;
            std::auto_ptr<wimac::scheduler::Interface> rxScheduler;
        };
    }
}
#endif
