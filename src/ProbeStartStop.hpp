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

#ifndef WIMAC_PROBESTARTSTOP_HPP
#define WIMAC_PROBESTARTSTOP_HPP

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Forwarding.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>

#include <WNS/ldk/probe/Probe.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>

#include <WIMAC/EventSubjectObserver.hpp>


namespace wimac{

	class ProbeStartStop;

	/**
	 * @brief Interface to start and stop the Probe.
         *
	 *
	 */
	class ProbeStartStopInterface
	{
	public:
		virtual ~ProbeStartStopInterface(){};

		virtual void event(const std::string event) = 0;
	};


	class ProbeStartStopCommand :
		public wns::ldk::Command
	{
	public:
		struct {} local;
		struct {} peer;
		struct
		{
			ProbeStartStop* probingFU;
		} magic;
	};


	/**
	 * @brief FunctionalUnit to probe the number of compounds and the
         *        total size between the start stop signal
	 *
	 *  @todo (gra): It should migrate to the LDK, after it is unit tested
	 *               and some more detailed documentation.
	 */
	class ProbeStartStop :
		public wns::ldk::probe::Probe,
		public wns::ldk::CommandTypeSpecifier<ProbeStartStopCommand>,
		public wns::ldk::HasReceptor<>,
		public wns::ldk::HasConnector<>,
		public wns::ldk::HasDeliverer<>,
		public wns::ldk::Forwarding<ProbeStartStop>,
		public wns::Cloneable<ProbeStartStop>,
		public ProbeStartStopInterface,
		public EventObserver
	{
	public:
		ProbeStartStop(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
		virtual ~ProbeStartStop();

		// Processor interface
		virtual void processOutgoing(const wns::ldk::CompoundPtr& compound);
		virtual void processIncoming(const wns::ldk::CompoundPtr& compound);


		virtual void event(const std::string event);

		virtual void onFUNCreated();



	private:
		void reset();
		void start();
		void stop();


		bool probing_;

		double cumulatedIncomingBits_;
		double cumulatedIncomingCompounds_;
		double cumulatedOutgoingBits_;
		double cumulatedOutgoingCompounds_;
		double cumulatedAggregatedBits_;
		double cumulatedAggregatedCompounds_;


		// Values form PyConfig
		wns::logger::Logger logger_;

		struct{
			wns::probe::bus::ContextCollectorPtr incomingBits;
			wns::probe::bus::ContextCollectorPtr incomingCompounds;
			wns::probe::bus::ContextCollectorPtr outgoingBits;
			wns::probe::bus::ContextCollectorPtr outgoingCompounds;
			wns::probe::bus::ContextCollectorPtr aggregatedBits;
			wns::probe::bus::ContextCollectorPtr aggregatedCompounds;
		} probe_;

		struct{
			std::string eventStartStopSubjectName;

			std::string eventStartStopSubjectType;

			EventSubject* eventStartStopSubject;
		} friends_;

		const std::string eventReset_;
		const std::string eventStart_;
		const std::string eventStop_;
	};

}

#endif

