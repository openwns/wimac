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



#ifndef WIMAC_FRAME_ULMAPCOLLECTOR_H
#define WIMAC_FRAME_ULMAPCOLLECTOR_H

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/fcf/CompoundCollector.hpp>
#include <WNS/scheduler/MapInfoProviderInterface.hpp>
#include <WNS/ldk/tools/UpUnconnectable.hpp>
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
}

namespace wimac { namespace frame {

	class MapHandlerInterface
	{
	public:
		virtual bool		
		resourcesGranted() = 0;

		virtual wns::scheduler::SchedulingMapPtr
		getMasterMapForSlaveScheduling() = 0;
		
	};


        /**
         * @brief Command of the ULMapWriter and ULMapRetreiver.
         */


        typedef MapCommand ULMapCommand;

        /**
         * @brief Sending entity for the ULMapCommand.
         */

        class ULMapCollector :
            public wns::ldk::fcf::CompoundCollector,
            public wns::ldk::CommandTypeSpecifier<MapCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::tools::UpUnconnectable,
            public wns::Cloneable<ULMapCollector>,
            public wns::events::CanTimeout,
            virtual public wimac::frame::MapHandlerInterface
        {
        public:
            ULMapCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

            void onFUNCreated();
            void calculateSizes( const wns::ldk::CommandPool* commandPool, Bit& commandPoolSize, Bit& dataSize ) const;

            void doOnData( const wns::ldk::CompoundPtr& );

            void doStart(int);

            void onTimeout();

            void doStartCollection(int){}
            void finishCollection(){}

            wns::simulator::Time getCurrentDuration() const;

/* old interface - to be removed*/
            wns::simulator::Time getULPhaseDuration() const { return ulPhaseDuration_; }

            /**
             * @brief Returns the start of the subscriber station's
             * burst relative to ULPhase start time.
             */

            wns::simulator::Time getBurstStart() const {
                return burstStartTime_;
            }

            /**
             * @brief Returns the end of the subscriber station's
             * burst relative to ULPhase start time.
             */

            wns::simulator::Time getBurstEnd() const {
                return burstEndTime_;
            }

            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> getPhyMode() const {
                return burstPhyMode;
            }

            wns::CandI getEstimatedCandI() const {
                return estimatedCandI_;
            }

            bool hasUplinkBurst() const {
                return hasUplinkBurst_;
            }

            wns::scheduler::MapInfoCollectionPtr
            getBursts() {return myBursts_;};

/* new Interface*/
            bool resourcesGranted() {
                return ulResourcesAvailable_;
            }

             virtual wns::scheduler::SchedulingMapPtr getMasterMapForSlaveScheduling() { 
                return scheduledULMap_;
             }
            
        private:
            wimac::scheduler::Scheduler* ulScheduler_;
            std::string ulSchedulerName_;
            wimac::Component* component_;
            wimac::PhyUser* phyUser_;
            wimac::service::ConnectionManager* connectionManager_;

            // used variables at the receiving MAP collector

            /**
             * @brief PhyMode to be used for the MAP.
             */
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyMode;
/*old MapCollector variables*/
            wns::simulator::Time ulPhaseDuration_;
            wns::simulator::Time burstStartTime_;
            wns::simulator::Time burstEndTime_;
            wns::scheduler::MapInfoCollectionPtr myBursts_;
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> burstPhyMode;
            wns::CandI estimatedCandI_;
            bool hasUplinkBurst_;
/*new Mapcollector variables*/
            wns::scheduler::SchedulingMapPtr scheduledULMap_;

            /**
             * @brief PhyMode to be used for the burst.
             */

            bool ulResourcesAvailable_;

            wns::scheduler::UserID me_;
        };
    }
}
#endif

