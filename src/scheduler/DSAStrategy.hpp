/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
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
#ifndef WIMAC_SCHEDULER_DSASTATEGY
#define WIMAC_SCHEDULER_DSASTATEGY

#include <WNS/scheduler/strategy/dsastrategy/DSAStrategy.hpp>
#include <WNS/scheduler/strategy/SchedulerState.hpp>

namespace wimac { namespace scheduler {

    class DSAStrategy :
        public virtual wns::scheduler::strategy::dsastrategy::DSAStrategy
    {
    public:

        DSAStrategy(const wns::pyconfig::View& );

        wns::scheduler::strategy::dsastrategy::DSAResult
        getSubChannelWithDSA(wns::scheduler::strategy::RequestForResource& request,
                             wns::scheduler::strategy::SchedulerStatePtr schedulerState,
                             wns::scheduler::SchedulingMapPtr schedulingMap);

        bool
        requiresCQI() const
        {
            return false;
        }

        void
        initialize(wns::scheduler::strategy::SchedulerStatePtr schedulerState,
                   wns::scheduler::SchedulingMapPtr schedulingMap);

    private:
        std::auto_ptr<wns::distribution::StandardUniform> randomDist;
        /** @brief remember position of last used subChannel */
        int lastUsedSubChannel;
        int lastUsedBeam;

    };


    }}

#endif
