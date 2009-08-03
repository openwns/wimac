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
#include <WIMAC/scheduler/DSAStrategy.hpp>
#include <WNS/PyConfigViewCreator.hpp>

using namespace wimac::scheduler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::scheduler::DSAStrategy,
    wns::scheduler::strategy::dsastrategy::DSAStrategyInterface,
    "wimac.scheduler.DSAStrategy",
    wns::PyConfigViewCreator );


DSAStrategy::DSAStrategy(const wns::pyconfig::View& config):
    wns::scheduler::strategy::dsastrategy::DSAStrategy(config),
    randomDist(NULL),
    lastUsedSubChannel(0),
    lastUsedBeam(0)
{
  if (config.get<bool>("useRandomChannelAtBeginning"))
      randomDist.reset(new wns::distribution::StandardUniform());
}

// call this before each timeSlot/frame
void
DSAStrategy::initialize(wns::scheduler::strategy::SchedulerStatePtr schedulerState,
                        wns::scheduler::SchedulingMapPtr schedulingMap)
{
	wns::scheduler::strategy::dsastrategy::DSAStrategy::initialize(schedulerState,schedulingMap); // must always initialize base class too
	lastUsedSubChannel = 0;
	// with beamforming/grouping it might be useful to remember lastUsedSubChannel[userID] separately
	// ^ this would form a new strategy "BFOptimizedLinearFFirst" or "LinearFFirstForBeamForming"
        if (randomDist.get())
	{
          lastUsedSubChannel = (*randomDist)() * schedulingMap->subChannels.size();
	}
}

wns::scheduler::strategy::dsastrategy::DSAResult
DSAStrategy::getSubChannelWithDSA(wns::scheduler::strategy::RequestForResource& request,
                                  wns::scheduler::strategy::SchedulerStatePtr schedulerState,
                                  wns::scheduler::SchedulingMapPtr schedulingMap)
{
    //simTimeType requestedCompoundDuration = getCompoundDuration(request);
    //MESSAGE_SINGLE(NORMAL, logger, "getSubChannelWithDSA("<<request.toString()<<"): d="<<requestedCompoundDuration<<"s");
    int subChannel = lastUsedSubChannel;
    int beam = lastUsedBeam;
    int maxSubChannel = schedulerState->currentState->strategyInput->getFChannels();
    int maxBeams = schedulerState->currentState->strategyInput->getMaxBeams();
    assure(subChannel<maxSubChannel,"invalid subChannel="<<subChannel);
    MESSAGE_SINGLE(NORMAL, logger, "getSubChannelWithDSA("<<request.toString()<<"): lastSC="<<lastUsedSubChannel);
    bool found  = false;
    bool giveUp = false;
    while(!found && !giveUp) {
        if (channelIsUsable(subChannel, beam, request, schedulerState, schedulingMap))
        { // PDU fits in
            found=true; break;
        }
        if (++beam>=maxBeams)
        { // all beams full; take next subChannel
            beam=0;
            if (++subChannel>=maxSubChannel)
            { // wraparound
                subChannel=0;
            }
        }
        if (subChannel==lastUsedSubChannel)
        { // one complete round already done
            giveUp=true; break;
        }
    } // while
    if (giveUp) {
        MESSAGE_SINGLE(NORMAL, logger, "getSubChannelWithDSA(): no free subchannel");
        return wns::scheduler::strategy::dsastrategy::DSAResult(); // empty with subChannel=DSAsubChannelNotFound
    }
    MESSAGE_SINGLE(NORMAL, logger, "getSubChannelWithDSA(): subChannel="<<subChannel<<"."<<beam);
    lastUsedSubChannel = subChannel;
    lastUsedBeam = beam;
    wns::scheduler::strategy::dsastrategy::DSAResult dsaResult;
    dsaResult.subChannel = subChannel;
    dsaResult.beam = beam;
    return dsaResult;
}
