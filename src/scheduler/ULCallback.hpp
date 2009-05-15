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

#ifndef WIMAC_SCHEDULER_ULCALLBACK_HPP
#define WIMAC_SCHEDULER_ULCALLBACK_HPP

#include <WNS/scheduler/CallBackInterface.hpp>
#include <WIMAC/scheduler/Callback.hpp>

namespace wimac { namespace scheduler {

	class ULCallback :
		public virtual wns::scheduler::CallBackInterface,
		public Callback
	{
	public:
		ULCallback(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

		void
		callBack(wns::scheduler::MapInfoEntryPtr mapInfoEntry);
	  /*
		void callBack(unsigned int fSlot,
			      simTimeType startTime,
			      simTimeType endTime,
			      wns::scheduler::UserID user,
			      const wns::ldk::CompoundPtr& pdu,
			      float cidColor,
			      unsigned int beam,
			      wns::service::phy::ofdma::PatternPtr pattern,
			      wns::scheduler::MapInfoEntryPtr burst,
			      const wns::service::phy::phymode::PhyModeInterface& phyMode,
			      bool measureInterference,
			      wns::Power txPower,
			      wns::CandI estimatedCandI);
	  */
		void deliverNow(wns::ldk::Connector*);

	private:
		wns::ldk::fun::FUN* fun_;
	};

}}
#endif



