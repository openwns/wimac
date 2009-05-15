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

#ifndef WIMAC_SCHEDULER_SCHEDULERINTERFACE_HPP
#define WIMAC_SCHEDULER_SCHEDULERINTERFACE_HPP

namespace wns {

	namespace ldk {
		class Connector;
		namespace fun {
			class FUN;
		}
	}
}

namespace wimac { namespace scheduler {

	class SchedulerInterface {
	public:
		virtual ~SchedulerInterface() {}


		/**
		 * @brief Check whether the scheduler accepts the compound.
		 */
		bool isAccepting(const wns::ldk::CompoundPtr& compound) const
		{
			return doIsAccepting(compound);
		}

		/**
		 * @brief Store a single compound for later scheduling.
		 */
		virtual void schedule(const wns::ldk::CompoundPtr&) = 0;

		/**
		 * @brief Trigger the schedule process for the stored compounds.
		 */
		virtual void startScheduling() = 0;

		/**
		 * @brief Inform the scheduler about the end of the scheduling
		 * process.
		 */
		virtual void finishScheduling() = 0;

		/**
		 * @brief Deliver the scheduled compounds to the Connector.
		 */
		virtual void deliverSchedule(wns::ldk::Connector*) = 0;

		/**
		 * @brief Makes the scheduler familiar with the FUN.
		 */
		virtual void setFUN(wns::ldk::fun::FUN*) = 0;

		/**
		 * @brief Makes the scheduler familiar with its receptor.
		 */
		virtual void setReceptor(wns::ldk::Receptor*) = 0;

		/**
		 * @brief Set the duration for the next schedule round.
		 */
		virtual void setDuration(double) = 0;

		/**
		 * @brief Get the duration of the next schedule.
		 */
		virtual double getDuration() const = 0;


	private:

		virtual bool doIsAccepting(const wns::ldk::CompoundPtr& compound) const = 0;

	};

	typedef wns::ldk::FUNConfigCreator<SchedulerInterface> SchedulerCreator;
	typedef wns::StaticFactory<SchedulerCreator> SchedulerFactory;

}}
#endif



