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

#ifndef WIMAC_SCHEDULER_PDUWATCHPROVIDEROBSERVER_HPP
#define WIMAC_SCHEDULER_PDUWATCHPROVIDEROBSERVER_HPP

#include <list>
#include <WNS/logger/Logger.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>


namespace wimac { namespace scheduler{

class PDUWatchProvider;

class PDUWatchObserver
	: public virtual wns::ldk::FunctionalUnit
	{
	public:
		PDUWatchObserver();

		virtual
        ~PDUWatchObserver();

		void
		setPDUWatchProvider(PDUWatchProvider* pduWatchProvider);

		virtual void
	    pduWatchProviderDeleted();

		virtual void
        notifyPDUWatch( wns::ldk::CommandPool commandPool ) = 0;

		PDUWatchProvider*
		testGetPDUWatchProvider_()
			{
				return pduWatchProvider_;
			}

	private:

		PDUWatchProvider* pduWatchProvider_;

	};



class PDUWatchProvider
	{
	public:
		typedef std::list<PDUWatchObserver*> PDUWatchObservers;

		PDUWatchProvider(wns::ldk::fun::FUN* fun);

		~PDUWatchProvider();

	    void
		attachObserver(PDUWatchObserver* pduWatchObserver);

		void
		detachObserver(PDUWatchObserver* pduWatchObserver);

		/// Watch for special compounds to inform its observer
		void
		pduWatch(const wns::ldk::CompoundPtr& compound);

		PDUWatchObservers
        testGetPDUWatchObservers_()
			{
				return pduWatchObservers_;
			}


	private:

		wns::ldk::fun::FUN* fun_;
		PDUWatchObservers pduWatchObservers_;

		wns::logger::Logger logger_;
	};

}} // wimac scheduler

#endif //WIMAC_SCHEDULER_PDUWATCHPROVIDEROBSERVER
