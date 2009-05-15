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

#ifndef WIMAC_SCHEDULER_CALLBACK_HPP
#define WIMAC_SCHEDULER_CALLBACK_HPP

#include <queue>

#include <WNS/scheduler/CallBackInterface.hpp>
#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>

namespace wns { namespace scheduler {
	class RegistryProxyInterface;
}}

namespace wimac {
	class PhyUser;
}

namespace wimac { namespace scheduler {

	class Callback :
		public virtual wns::scheduler::CallBackInterface
		//public PDUWatchProvider
	{
	public:
		Callback(wns::ldk::fun::FUN* fun);

		virtual void setColleagues(wns::scheduler::RegistryProxyInterface* registry);

		/**
		 * @brief Deliver all scheduled compounds to the given connector now.
		 */
		virtual void deliverNow(wns::ldk::Connector*) = 0;

		virtual void setOffset(double offset)
		{ offsetInSlot = offset; }

	protected:
		struct {
			wns::scheduler::RegistryProxyInterface* registry;
		} colleagues;
		struct {
			wimac::PhyUser* phyUser;
		} friends_;
		
		std::queue<wns::ldk::CompoundPtr> scheduledPDUs;

		double offsetInSlot;
	};

	typedef wns::ldk::FUNConfigCreator<Callback> CallbackCreator;
	typedef wns::StaticFactory<CallbackCreator> CallbackFactory;

}}

#endif


