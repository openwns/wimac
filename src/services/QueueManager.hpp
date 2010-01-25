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

#ifndef WIMAC_SCHEDULER_QUEUEMANAGER_HPP
#define WIMAC_SCHEDULER_QUEUEMANAGER_HPP

#include <WNS/scheduler/queue/IQueueManager.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/frame/DataCollector.hpp>
#include <WNS/logger/Logger.hpp>

namespace wimac { namespace service {

            typedef std::map<wns::scheduler::ConnectionID, wns::scheduler::queue::QueueInterface*> CIDtoQueueMap;
            typedef std::map<wns::scheduler::ConnectionID, wimac::frame::DataCollector*> CIDtoDCMap;

            /**
             * @brief System specific implementation to map CIDs to queues. Calls only return UL
             * slave queues.
             */            
            class QueueManager:
                public wns::scheduler::queue::IQueueManager
            {
            public:
                QueueManager(wns::ldk::ManagementServiceRegistry* msr, const wns::pyconfig::View& config);

                virtual ~QueueManager();

                /**
                 * @brief Return all managed queues
                 */
                virtual wns::scheduler::queue::QueueContainer
                getAllQueues();

                /**
                 * @brief Get queue for CID
                 */
                virtual wns::scheduler::queue::QueueInterface*
                getQueue(wns::scheduler::ConnectionID cid);

                /**
                * @brief Calls wakeUp to fill the queue
                */
                virtual void
                startCollection(wns::scheduler::ConnectionID cid);

                virtual void
                onMSRCreated();

            private:
                wimac::ConnectionIdentifier::StationID
                getStationID(wns::scheduler::ConnectionID cid);

                wns::scheduler::queue::QueueInterface*
                getQueue(wimac::ConnectionIdentifier::StationID peerStationId, wns::scheduler::ConnectionID cid);

                std::string connectionManagerServiceName_;
                wimac::service::ConnectionManager* connectionManager_;
                CIDtoQueueMap cache_;
                CIDtoDCMap dcCache_;
                wns::logger::Logger logger_;
            };
        }} // namespace wimac::service
#endif // WIMAC_SCHEDULER_QUEUEMANAGER_HPP


