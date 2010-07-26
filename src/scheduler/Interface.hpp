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

#ifndef WIMAC_SCHEDULER_SCHEDULERINTERFACE_HPP
#define WIMAC_SCHEDULER_SCHEDULERINTERFACE_HPP

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/StaticFactory.hpp>
#include <WNS/scheduler/queue/QueueInterface.hpp>
#include <WNS/scheduler/harq/HARQInterface.hpp>
#include <WIMAC/FUConfigCreator.hpp>
#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>

namespace wns {
namespace ldk {

class Connector;
class Receptor;

namespace fun {

class FUN;

}
}
}

namespace wimac {
    namespace scheduler {

        class Interface {
        public:
            virtual ~Interface() {}


            /**
             * @brief Check whether the scheduler accepts the compound.
             */
            bool isAccepting(const wns::ldk::CompoundPtr& compound) const
            {
                return doIsAccepting(compound);
            }

            /**
             * @brief Store a single compound for later scheduling.
             */
            virtual void schedule(const wns::ldk::CompoundPtr&) = 0;

            /**
             * @brief Trigger the schedule process for the stored compounds.
             */
            virtual void startScheduling() = 0;

            /**
             * @brief Inform the scheduler about the end of the scheduling
             * process.
             */
            virtual void finishScheduling() = 0;

            /**
             * @brief Deliver the scheduled compounds to the Connector.
             */
            virtual void deliverSchedule(wns::ldk::Connector*) = 0;

            /**
             * @brief Makes the scheduler familiar with the FUN.
             */
            virtual void setFUN(wns::ldk::fun::FUN*) = 0;

            /**
             * @brief Makes the scheduler familiar with its receptor.
             */
            virtual void setReceptor(wns::ldk::Receptor*) = 0;

            /**
             * @brief Set the duration for the next schedule round.
             */
            virtual void setDuration(const wns::simulator::Time&) = 0;

            /**
             * @brief Get the duration of the next schedule.
             */
            virtual wns::simulator::Time getDuration() const = 0;

            /**
            * @bried Returns a pointer to the queue of the current scheduling strategy.
            */
            virtual wns::scheduler::queue::QueueInterface* 
            getQueue() const = 0;

            virtual wimac::scheduler::RegistryProxyWiMAC* 
            getRegistryProxy() = 0;

            virtual wns::scheduler::harq::HARQInterface*
            getHARQ() = 0;

        private:

            virtual bool doIsAccepting(const wns::ldk::CompoundPtr& compound) const = 0;

        };

    typedef wimac::FUConfigCreator<Interface> SchedulerCreator;
    typedef wns::StaticFactory<SchedulerCreator> SchedulerFactory;

    }
}
#endif



