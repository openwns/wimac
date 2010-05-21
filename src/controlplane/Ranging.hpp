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

#ifndef WIMAC_CONTROLPLANE_RANGING_HPP
#define WIMAC_CONTROLPLANE_RANGING_HPP

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>

#include <WNS/ldk/tools/UpUnconnectable.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WIMAC/Logger.hpp>
#include <WNS/distribution/Uniform.hpp>

#include <WIMAC/MACHeader.hpp>
#include <WIMAC/CIRMeasureInterface.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>


namespace wimac {

    namespace service {
        class ConnectionManager;
    }

    namespace controlplane {



        /**
         * @todo (gra): It seems to be better, if only one management
         * message Command exist for all ControlFUs. Members of the
         * peer struct should be only the ManagementMessageType and a
         * Container with the specific message informations.
         */
        class RangingCommand :
                public wns::ldk::Command
        {
        public:
            typedef unsigned int TransactionID;

            RangingCommand()
            {
                peer.managementMessageType = MACManagementMessage::UCD;
                // UCD is wrong, but 0

                peer.rng_req.transactionID = 0;
                peer.rng_req.baseStation = 0;
                peer.rng_req.subscriberStation = 0;

                peer.rng_rsp.transactionID = 0;
                peer.rng_rsp.baseStation = 0;
                peer.rng_rsp.subscriberStation = 0;
                peer.rng_rsp.basicCID = 0;
                peer.rng_rsp.primaryManagementCID = 0;

                magic.size = 0;
            }

            virtual
            Bit getSize() const
            {
                return magic.size;
            }

            struct {} local;

            struct {
                MACManagementMessage::ManagementMessageType managementMessageType;

                struct RNG_REQ{
                    ///Unique identifier for this transaction
                    TransactionID transactionID;
                    ConnectionIdentifier::StationID baseStation;
                    ConnectionIdentifier::StationID subscriberStation; // Own ID
                } rng_req;

                struct RNG_RSP{
                    ///Unique identifier for this transaction
                    TransactionID transactionID;
                    ConnectionIdentifier::StationID baseStation;
                    ConnectionIdentifier::StationID subscriberStation;
                    ConnectionIdentifier::CID basicCID;
                    ConnectionIdentifier::CID primaryManagementCID;
                } rng_rsp;

            } peer;

            struct {
                Bit size;
            } magic;

        };



        /********** RangingCallBackInterface *****************************************/
        class RangingCallBackInterface
        {
        public:
            virtual ~RangingCallBackInterface()
            {
            }

            virtual void
            resultRanging(bool result) = 0;

        };

        class Ranging :
            public virtual wns::ldk::FunctionalUnit,
            public wns::ldk::CommandTypeSpecifier< RangingCommand >,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::tools::UpUnconnectable
        {
        public:
            Ranging(wns::ldk::fun::FUN*, const wns::pyconfig::View&);
        };


        /**
         * @brief RangingBS implementation for message exchange in the Base Station.
         *
         * @author Markus Grauer<gra@comnets.rwth-aachen.de>
         *
         * RangingBS reacts on an RNG_REQ(base station ID, subscriber
         * station ID) from RangingSS with an RNG_RSP(subscriber
         * station ID, PM-CID)
         */
        class RangingBS:
            public Ranging,
            public wns::Cloneable< RangingBS >
        {
        public:
            RangingBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& );

            virtual void
            doOnData(const wns::ldk::CompoundPtr& compound);

            virtual void
            doWakeup();

            virtual void
            onFUNCreated();

        private:

            void doOnRNG_REQ(const wns::ldk::CompoundPtr& compound);

            std::list<wns::ldk::CompoundPtr> compoundQueue_;


            //Static values from PyConfig
            Bit rng_rspPDUSize_;

            struct{
                std::string connectionManagerName;
                std::string connectionClassifierName;

                service::ConnectionManager* connectionManager;
                FunctionalUnit* connectionClassifier;
            } friends_;

        };

        /**
         * @brief RangingSS implementation for message exchange in the
         * Subscriber Station.
         *
         * @author Markus Grauer <gra@comnets.rwth-aachen.de>
         *
         * \li Start RangingSS with function range( base station, callBackInterface)
         * \li Sends an RNG_REQ(base station ID,subscriber station ID)
         * \li Receives an RNG_RSP(subscriber station ID, PM-CID)
         *
         */
        class RangingSS :
            public Ranging,
            public wns::Cloneable< RangingSS >,
            public wns::ldk::fcf::NewFrameObserver
        {
        public:

            RangingSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

            void
            start(wns::service::phy::ofdma::Tune tune,
                  ConnectionIdentifier::StationID baseStation,
                  RangingCallBackInterface* callBackInterface);

            virtual void
            doOnData(const wns::ldk::CompoundPtr& compound);

            virtual void
            doWakeup();

            virtual void
            onFUNCreated();

            virtual void
            messageNewFrame();

        private:

            void doOnRNG_RSP(const wns::ldk::CompoundPtr& compound);

            void resultRanging(bool result);

            void sendContentionAccess(const wns::ldk::CompoundPtr compound);

            RangingCommand::TransactionID activeTransactionID_;
            RangingCommand::TransactionID highestTransactionID_;

            ConnectionIdentifier::Frames remainTimerWaitingForRSP_;
            int remainNumberOfRetries_;

            std::list<wns::ldk::CompoundPtr> compoundQueue_;
            RangingCallBackInterface* callBackInterface_;
            wns::ldk::CompoundPtr rngCompound_;


            /// Pointer to the used Uniform Distribution.
            wns::distribution::StandardUniform rngDis_;

            //Static values from PyConfig
            Bit rng_reqPDUSize_;
            ConnectionIdentifier::Frames timerWaitingForRSP_;
            int numberOfRetries_;
            int boWindowSizeMin_;
            int boWindowSizeMax_;


            struct{
                std::string connectionManagerName;
                std::string connectionClassifierName;
                std::string newFrameProviderName;
                std::string rngCompoundCollectorName;

                service::ConnectionManager* connectionManager;
                wns::ldk::FunctionalUnit* connectionClassifier;
                wns::ldk::fcf::NewFrameProvider* newFrameProvider;
            } friends_;
        };
    }}

#endif
