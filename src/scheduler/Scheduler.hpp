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

#ifndef WIMAC_SCHEDULER_SCHEDULER_HPP
#define WIMAC_SCHEDULER_SCHEDULER_HPP

#include <string>
#include <queue>

#include <boost/filesystem/fstream.hpp>

#include <WNS/Cloneable.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/scheduler/MapInfoProviderInterface.hpp>
#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/Observer.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/scheduler/SchedulerInterface.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

namespace wns {
	namespace ldk {
		class FunctionalUnit;
		class Receptor;
	}

	namespace scheduler {

		class RegistryProxyInterface;
		class CallBackInterface;

		namespace queue {
			class QueueInterface;
		}

		namespace strategy {
			class StrategyInterface;
		}

		namespace grouper {
			class GroupingProviderInterface;
		}
	}

	namespace service { namespace phy { namespace ofdma {
		class DataTransmission;
	}}}
}

namespace wimac {
	class PhyUser;
}
namespace wimac { namespace scheduler {
	class Callback;
	class PseudoBWRequestGenerator;
	class RegistryProxyWiMAC;

	/**
	 * @brief The scheduler aggregates the scheduler components.
	 *
	 * The Scheduler aggregates the strategy, the grouper, the queue, the
	 * registry proxy, the callback and the pseudo packet generator if
	 * necessary.
	 */
	class Scheduler :
		public wimac::scheduler::SchedulerInterface,
		public wns::scheduler::MapInfoProviderInterface,
		public wns::Cloneable<Scheduler>,
		public wimac::scheduler::PDUWatchProvider,
		public wns::Observer<wimac::service::ConnectionDeletedNotification>
	{
	public:
		Scheduler(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

		~Scheduler();

		void schedule(const wns::ldk::CompoundPtr&);
		void startScheduling();
		void finishScheduling(){}
		void deliverSchedule(wns::ldk::Connector*);

		void resetAllQueues();

		void resetCID(ConnectionIdentifier::CID cid);

		void finishCollection();

		///\todo Remove me when I have found a better testing work-around
		void setProvider(wns::service::phy::ofdma::DataTransmission* _ofdmaProvider);

		void setFUN(wns::ldk::fun::FUN*);

		void setReceptor(wns::ldk::Receptor* receptor)
		{ receptor_ = receptor; }

		void setDuration(double duration)
		{ duration_ = duration; }

		double getDuration() const
		{ return duration_; }

		// For MapInfoProviderInterface
		wns::scheduler::MapInfoCollectionPtr getMapInfo() const;
		int getNumBursts() const;


		void notifyAboutConnectionDeleted(const ConnectionIdentifier);

	protected:
		void setupPlotting();
		void handleBroadcast();

		bool plotFrames;

		struct {
			wns::scheduler::grouper::GroupingProviderInterface* grouper;
			wns::scheduler::queue::QueueInterface* queue;
			wns::scheduler::strategy::StrategyInterface* strategy;
			// we have special WiMAC extensions to the registry proxy so don't
			// use the wns::scheduler::RegistryProxyInterface* version here
			wimac::scheduler::RegistryProxyWiMAC* registry;
			wimac::scheduler::Callback* callback;
			wimac::scheduler::PseudoBWRequestGenerator* pseudoGenerator;
		} colleagues;

		simTimeType usedSlotDuration;
		simTimeType offsetInSlot;
		unsigned int freqChannels;
		unsigned int maxBeams;
		bool beamforming;
		bool uplink;

		struct Friends
		{
			wns::ldk::CommandTypeSpecifier<wns::ldk::ClassifierCommand>* classifier;
			wimac::PhyUser* phyUser;
		} friends_;

		wns::logger::Logger logger;
		std::vector<boost::filesystem::fstream*> plotFiles;


	private:
		bool doIsAccepting(const wns::ldk::CompoundPtr& compound) const;
		void doStart(int);

		void putProbe(int bits, int compounds);

		wns::service::phy::ofdma::DataTransmission* ofdmaProvider;
		std::string strategyName;
		std::string grouperName;
		std::string queueName;
		std::string registryName;
		std::string callbackName;
		double duration_;

		int pduCount;
		int frameNo;

		wns::pyconfig::View pyConfig;

		std::string outputDir;

		// Probes
		wns::probe::bus::ContextCollectorPtr resetedBitsProbe;
		wns::probe::bus::ContextCollectorPtr resetedCompoundsProbe;

		wns::ldk::fun::FUN* fun_;
		wns::ldk::Receptor* receptor_;
		bool accepting_;
	};



}} // namespace wimac::scheduler
#endif // WIMAC_SCHEDULER_SCHEDULER_HPP


