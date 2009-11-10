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

#include <WIMAC/scheduler/PriorityULCallback.hpp>

using namespace wimac::scheduler;

/*
void
PriorityULCallback::callBack( unsigned int fSlot, wns::simulator::Time startTime, wns::simulator::Time endTime, wns::scheduler::UserID user,
			      const wns::ldk::CompoundPtr& pdu, float cidColor, unsigned int beam,
			      wns::service::phy::ofdma::PatternPtr pattern, wns::scheduler::MapInfoEntryPtr burst,
			      const wns::service::phy::phymode::PhyModeInterface& phyMode,
			      bool measureInterference, wns::Power requestedTxPower,
			      wns::CandI estimatedCandI)
*/
void
PriorityULCallback::callBack(wns::scheduler::MapInfoEntryPtr mapInfoEntry)
{
	// here we have to handle the offset resulting from running the scheduling
	// in several phases

	//startTime += offsetInSlot;
	//endTime += offsetInSlot;
	mapInfoEntry->start += offsetInSlot;
	mapInfoEntry->end   += offsetInSlot;

	//assure(endTime <= this->getDuration(), "Timing inconsistency");
	assure(mapInfoEntry->start < mapInfoEntry->end, "Timing inconsistency");


	//if (endTime > usedSlotDuration)
	//	usedSlotDuration = endTime;

	// the rest can be handled by the normal callBack function
	ULCallback::callBack(mapInfoEntry);
	/*
	ULCallback::callBack(fSlot,
			    startTime,
			    endTime,
			    user,
			    pdu,
			    cidColor,
			    beam,
			    pattern,
			    burst,
			    phyMode,
			    measureInterference,
			    requestedTxPower,
			    estimatedCandI);
	*/
}


