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


#ifndef WIMAC_SCHEDULER_SPACETIMESECTORIZATIONREGISTRYPROXY_HPP
#define WIMAC_SCHEDULER_SPACETIMESECTORIZATIONREGISTRYPROXY_HPP


#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/ldk/fcf/FrameBuilder.hpp>
#include <WNS/logger/Logger.hpp>

#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>


namespace wimac { namespace scheduler {

        class SpaceTimeSectorizationRegistryProxy :
            public wimac::scheduler::RegistryProxyWiMAC
        {
        public:
            SpaceTimeSectorizationRegistryProxy(wns::ldk::fun::FUN*, const wns::pyconfig::View&);
            ~SpaceTimeSectorizationRegistryProxy() {}

            wns::scheduler::UserSet filterReachable( wns::scheduler::UserSet users );
            void setFUN(const wns::ldk::fun::FUN *fun);


        private:
            wns::service::phy::ofdma::DataTransmission* ofdmaProvider;
            wns::ldk::fcf::FrameBuilder* frameBuilder;
            wns::logger::Logger logger;

            bool isUserinActiveGroup(double doa, int group) const;

            int numberOfSectors,numberOfSubsectors;
            double mutualAngleOfSubsectors;
        };
    }
}
#endif


