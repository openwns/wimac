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

#include <WIMAC/scheduler/PriorityDLCallback.hpp>

using namespace wimac::scheduler;

/*
void
PriorityDLCallback::callBack( unsigned int fSlot, simTimeType startTime, simTimeType endTime, wns::scheduler::UserID user,
			      const wns::ldk::CompoundPtr& pdu, float cidColor, unsigned int beam,
			      wns::service::phy::ofdma::PatternPtr pattern, wns::scheduler::MapInfoEntryPtr burst,
			      const wns::service::phy::phymode::PhyModeInterface& phyMode,
			      bool measureInterference, wns::Power requestedTxPower,
			      wns::CandI estimatedCandI)
*/
void
PriorityDLCallback::callBack(wns::scheduler::MapInfoEntryPtr mapInfoEntry)
{
	// here we have to handle the offset resulting from running the scheduling
	// in several phases

	mapInfoEntry->start += offsetInSlot;
	mapInfoEntry->end   += offsetInSlot;
	//startTime += offsetInSlot;
	//endTime += offsetInSlot;

	//assure(endTime <= this->getDuration(), "Timing inconsistency");
	//assure(startTime < endTime, "Timing inconsistency");


	//if (endTime > usedSlotDuration)
	//	usedSlotDuration = endTime;

	// the rest can be handled by the normal callBack function
	DLCallback::callBack(mapInfoEntry);
	/*
	DLCallback::callBack(fSlot,
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


