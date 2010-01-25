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


/**
 * @file
 * @author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#ifndef WIMAC_SERVICES_DEADSTATIONDETECT_HPP
#define WIMAC_SERVICES_DEADSTATIONDETECT_HPP

#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>
#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/SuspendableInterface.hpp>
#include <WNS/ldk/SuspendSupport.hpp>

#include <WIMAC/ConnectionIdentifier.hpp>


namespace wimac {

    class ConnectionClassifier;

    namespace service {

        class ConnectionManager;


        class DeadStationDetectNotifyInterface
        {
        public:
            typedef ConnectionIdentifier::CID CID;

            virtual
            ~DeadStationDetectNotifyInterface()
            {
            }

            virtual void
            notifyActivityFor(CID cid) = 0;
        };

        /**
         * @brief DeadStationDetect
         */
        class DeadStationDetect:
            public wns::ldk::ManagementService,
            public DeadStationDetectNotifyInterface,
            public wns::ldk::fcf::NewFrameObserver
        {
            typedef ConnectionIdentifier::StationID StationID;
            typedef ConnectionIdentifier::Frames Frames;
            typedef std::map<StationID, wns::simulator::Time> MapStationTime;

        public:
            DeadStationDetect(wns::ldk::ManagementServiceRegistry* msr,
                              const  wns::pyconfig::View& config);

            ~DeadStationDetect()
            {
                friends_.newFrameProvider->detachObserver(this);
            }

            /**
             * @brief DeadStationDetect notify interface implementation
             */
            virtual void
            notifyActivityFor(const CID cid);

            /**
             * @brief NewFrameObserver interface implementation
             */
            virtual void
            messageNewFrame();

            void
            onMSRCreated();


        private:
            /**
             * @brief Update SimTime to an existing stationLastActive_
             * entry or create a new one with current SimTime
             */
            void
            setStationLastActive(const StationID stationID);

            /**
             * @brief Adjust stationLastActive_ Map with stations
             * stored in the ConnectionManager
             */
            void
            adjustStations();

            /**
             * @brief Delete all ConnectionIdentifer for stations,
             * with their last activity older than deltaTime
             */
            void
            deleteDeadStations(const wns::simulator::Time deltaTime);

            /**
             * @brief Execute on timer_ has run out.
             */
            void
            timerExecute();

            int timer_;
            MapStationTime stationLastActive_;

            // Static values from PyConfig
            const int checkInterval_;
            const wns::simulator::Time timeToLive_;

            struct{
                std::string connectionManagerName;
                std::string newFrameProviderName;

                wimac::service::ConnectionManager* connectionManager;
                wns::ldk::fcf::NewFrameProvider* newFrameProvider;
            } friends_;

            wns::logger::Logger logger_;
        };



        /**
         * @brief DSDSensor is the sensor for DeadStationDetect management service.
         */
        class DSDSensor :
            public virtual wns::ldk::FunctionalUnit,
            public wns::ldk::CommandTypeSpecifier<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasDeliverer<>,
            public wns::ldk::Processor< DSDSensor>,
            public wns::Cloneable<DSDSensor>
        {
        public:
            // FUNConfigCreator interface realisation
            DSDSensor(wns::ldk::fun::FUN* fuNet, const wns::pyconfig::View& config);

            virtual void onFUNCreated();


        private:
            /**
             * @brief Procesor interface implementation
             */
            virtual void
            processIncoming(const wns::ldk::CompoundPtr& compound);

            /**
             * @brief Procesor interface implementation
             */
            virtual void
            processOutgoing(const wns::ldk::CompoundPtr&);


            struct {
                std::string connectionClassifierName;
                std::string deadStationDetectName;

                ConnectionClassifier* connectionClassifier;
                DeadStationDetectNotifyInterface* deadStationDetect;
            } friends_;
        };
    }
}
#endif


