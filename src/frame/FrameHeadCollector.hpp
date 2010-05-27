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

#ifndef WIMAC_FRAME_FRAMEHEADCOLLECTOR_H
#define WIMAC_FRAME_FRAMEHEADCOLLECTOR_H

#include <WIMAC/services/IChannelQualityObserver.hpp>

#include <WNS/scheduler/MapInfoProviderInterface.hpp>

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/fcf/CompoundCollector.hpp>
#include <WNS/ldk/tools/UpUnconnectable.hpp>

namespace wimac {
    class Component;
    class PhyUser;

    namespace service {
        class ConnectionManager;
    }
}

namespace wimac { namespace frame {

        /**
         * @brief Command for the FrameHeadWriter and FrameHeadRetreiver.
         */
        class FrameHeadCommand :
            public wns::ldk::Command
        {
        public:
            struct {
                wns::simulator::Time duration;
            } local;
            struct {
                //MapInfoType mapInfo;
                int baseStationID;
            } peer;
            struct {
            } magic;
        };

        /**
         * @brief The CompoundCollector for frame control headers.
         */
        class FrameHeadCollector :
            public wns::ldk::fcf::CompoundCollector,
            public wns::ldk::CommandTypeSpecifier<FrameHeadCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::tools::UpUnconnectable,
            public wns::Cloneable<FrameHeadCollector>,
            public wns::events::CanTimeout
        {
        public:
            FrameHeadCollector( wns::ldk::fun::FUN* fun,
                                const wns::pyconfig::View& config );

            void onFUNCreated();

            void doOnData( const wns::ldk::CompoundPtr& );

            void doStart(int);
            void doStartCollection(int){}
            void finishCollection(){}

            virtual wns::simulator::Time getCurrentDuration() const
            {
                return getMaximumDuration();
            }

            void onTimeout();

        private:
            wimac::Component* layer_;
            wimac::PhyUser* phyUser_;
            wimac::service::ConnectionManager* connectionManager_;
            wimac::service::IChannelQualityObserver* channelQualityObserver_;
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyMode_;
        };
    }
}
#endif

