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

#ifndef WIMAC_SCHEDULER_DLCALLBACK_HPP
#define WIMAC_SCHEDULER_DLCALLBACK_HPP

#include <WIMAC/scheduler/Callback.hpp>

namespace wimac { namespace scheduler {

    class DLCallback :
        public Callback
    {
    public:
        DLCallback(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

        void
        callBack(wns::scheduler::SchedulingMapPtr schedulingMap);

        void 
        deliverNow(wns::ldk::Connector*);

    private:
        void
        processPacket(const wns::scheduler::SchedulingCompound& compound);

        wns::ldk::fun::FUN* fun_;
        bool beamforming;
        wns::simulator::Time slotLength_;
    };

}}
#endif

