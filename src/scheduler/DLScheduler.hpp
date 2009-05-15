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

#ifndef WIMAC_FRAME_DLSCHEDULER_H
#define WIMAC_FRAME_DLSCHEDULER_H

#include <WNS/scheduler/MapInfoProviderInterface.hpp>
#include <WNS/ldk/fcf/CompoundCollector.hpp>

#include <WIMAC/frame/ContentionCollector.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/frame/DLMapCollector.hpp>
#include <WIMAC/scheduler/SchedulerInterface.hpp>
#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>

namespace wimac { namespace frame {

		class DLScheduler
			: public virtual wns::ldk::fcf::CompoundCollectorInterface
		{};

		class BSDLScheduler
			: public virtual wns::scheduler::MapInfoProviderInterface,
			  public DLScheduler
		{};

		/// A ContentionCollector that is used in the BS.
		class MultiCompoundBSDLScheduler
			: public BSDLScheduler,
			  public ContentionCollector
		{
		public:
			MultiCompoundBSDLScheduler( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

			wns::scheduler::MapInfoCollectionPtr getMapInfo() const
			{
				assure(0,"should not happen" );
				return wns::scheduler::MapInfoCollectionPtr();
			}

			void onFUNCreated();

			//void doSendData( const wns::ldk::CompoundPtr& );

			int getNumBursts() const
			{
				return (connectionManager_->getAllBasicConnections()).size();
			}

		private:
			service::ConnectionManager* connectionManager_;
			wimac::PhyUser* phyUser_;
		};

	/**
	 * @brief A DLScheduler that is used in the SS.
	 *
	 * The SSDLScheduler is a receiving Scheduler at the subscriber
	 * station. Actually the SSDLScheduler does not schedule anything.
	 */
	class SSDLScheduler :
		public wimac::scheduler::SchedulerInterface,
		public wns::Cloneable<SSDLScheduler>
	{
	public:
		SSDLScheduler( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

		void schedule(const wns::ldk::CompoundPtr&){}

		void startScheduling(){}

		void finishScheduling(){}

		void deliverSchedule(wns::ldk::Connector*){}

		void setFUN(wns::ldk::fun::FUN* fun)
		{
			dlMapRetriever_ =
				fun->findFriend<DLMapCollector*>(dlMapRetrieverName_);
			assure( dlMapRetriever_, "dlmapcollector not of type DLMapRetriever" );
		}

		bool doIsAccepting( const wns::ldk::CompoundPtr& ) const
		{
			return false;
		}

		void setReceptor(wns::ldk::Receptor*){}


		void setDuration(double) {}


		double getDuration() const
		{
			/**
			 * @todo replace this with a observer pattern like behaviour for
			 * incoming maps
			 */
			return dlMapRetriever_->getDLPhaseDuration();
		}

		private:
			std::string dlMapRetrieverName_;
			DLMapCollector* dlMapRetriever_;
		};
	}
}
#endif
