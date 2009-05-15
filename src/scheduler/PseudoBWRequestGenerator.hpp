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

#ifndef WIMAC_FRAME_PSEUDOBWREQUESTGENERATOR_H
#define WIMAC_FRAME_PSEUDOBWREQUESTGENERATOR_H

#include <WNS/ldk/ldk.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/Classifier.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/scheduler/BSULSchedulerInterface.hpp>


namespace wimac { namespace scheduler {

	class Scheduler;
	class SchedulerInterface;

	/// A generator of pseude BW reqeusts that can be used by the BSScheduler.
	class PseudoBWRequestGenerator :
		public wns::Cloneable<PseudoBWRequestGenerator>
	{
	public:
		PseudoBWRequestGenerator( const wns::pyconfig::View& config );

		void setScheduler(wimac::scheduler::SchedulerInterface*);

		void setFUN(wns::ldk::fun::FUN* fun);

		void wakeup();
	private:
		wimac::Component* component_;

		struct {
			std::string connectionManagerName;
			std::string classifierName;

			service::ConnectionManager* connectionManager;
			wimac::scheduler::Scheduler* ulScheduler;
			wimac::ConnectionClassifier* classifier;
		} friends_;

		int packetSize;

	};

}} // namespace wimac::scheduler

#endif
