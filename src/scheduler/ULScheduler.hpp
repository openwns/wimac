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

#ifndef WIMAC_FRAME_ULSCHEDULER_H
#define WIMAC_FRAME_ULSCHEDULER_H

#include <WNS/scheduler/MapInfoProviderInterface.hpp>
#include <WNS/ldk/fcf/CompoundCollector.hpp>

#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>
#include <WIMAC/scheduler/SchedulerInterface.hpp>



namespace wimac {
	class PhyUser;
	class ConnectionClassifier;
	class Component;

	namespace service {
		class ConnectionManager;
	}
}
namespace wimac { namespace frame {

	class ULMapCollector;
}}

namespace wimac { namespace scheduler {



	/**
	 * @brief A scheduler that schedules multiple compounds regarding a
	 * MapInfo.
	 *
	 * The SSULScheduler interpretes the MAP information of the BS and
	 * schedulues according to the given burst timing. Due to the delayed
	 * scheduling process, the actual scheduling takes place during the
	 * deliverSchedule() method.
	 */
	class SSULScheduler :
		public SchedulerInterface,
		public wns::Cloneable<SSULScheduler>,
		public scheduler::PDUWatchProvider
	{
	public:
		SSULScheduler( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

		void schedule(const wns::ldk::CompoundPtr&);
		void deliverSchedule(wns::ldk::Connector*);
		void startScheduling(){}
		void finishScheduling(){}

		void setFUN(wns::ldk::fun::FUN*);

		void setReceptor(wns::ldk::Receptor* receptor)
		{
			receptor_ = receptor;
		}

		simTimeType
		getCurrentDuration() const;

		void setDuration(double duration)
		{ duration_ = duration; }

		double getDuration() const
		{ return duration_; }

		bool doIsAccepting( const wns::ldk::CompoundPtr& ) const;
	private:
		simTimeType currentBurstStart_;
		simTimeType currentBurstEnd_;
		simTimeType phaseStartTime_;


		wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> currentPhyModePtr;

		double duration_;

		typedef std::deque<wns::ldk::CompoundPtr> Compounds;
		Compounds compounds_;

		struct
		{
			wimac::ConnectionClassifier* classifier_;
		} friends_;

		wimac::frame::ULMapCollector* ulMapRetriever_;
		std::string ulMapRetrieverName_;

		/// Calculate the duraction of this compound.
		simTimeType getCompoundDuration(const wns::ldk::CompoundPtr&) const;

		bool accepting_;
		Component* component_;
		wimac::PhyUser* phyUser_;
		service::ConnectionManager* connectionManager_;

		wns::ldk::Receptor* receptor_;

	};

}}
#endif
