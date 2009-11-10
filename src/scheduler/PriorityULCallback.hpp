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

#ifndef WIMAC_SCHEDULER_PRIORITYULCALLBACK_HPP
#define WIMAC_SCHEDULER_PRIORITYULCALLBACK_HPP

#include <WNS/scheduler/CallBackInterface.hpp>

#include <WIMAC/scheduler/ULCallback.hpp>

namespace wimac { namespace scheduler {

	class PriorityULCallback :
		virtual public wns::scheduler::CallBackInterface,
		public ULCallback
	{

		PriorityULCallback(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

		void
		callBack(wns::scheduler::MapInfoEntryPtr mapInfoEntry);
	  /*
		void callBack(unsigned int fSlot,
			      wns::simulator::Time startTime,
			      wns::simulator::Time endTime,
			      wns::scheduler::UserID user,
			      const wns::ldk::CompoundPtr& pdu,
			      float cidColor,
			      unsigned int beam,
			      wns::service::phy::ofdma::PatternPtr pattern,
			      wns::scheduler::MapInfoEntryPtr burst,
			      const wns::service::phy::phymode::PhyModeInterface& phyMode,
			      bool measureInterference,
			      wns::Power requestedTxPower,
			      wns::CandI estimatedCandI) = 0;

	  */
	};

}}
#endif



