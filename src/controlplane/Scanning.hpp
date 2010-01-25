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

#ifndef WIMAC_CONTROLPLANE_SCANNING_HPP
#define WIMAC_CONTROLPLANE_SCANNING_HPP

#include <WIMAC/Logger.hpp>
#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>
#include <WIMAC/MACHeader.hpp>
#include <WIMAC/CIRMeasureInterface.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/services/scanningStrategy/Interface.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>
#include <WNS/ldk/tools/UpUnconnectable.hpp>

#include <WNS/pyconfig/View.hpp>

namespace wimac{
    namespace controlplane{

        /**
         *@todo (gra): It seems to be better, if only one management message Command exist
         *             for all ControlFUs. Members of the peer struct should be only the
         *             ManagementMessageType and a Container with the specific message
         *             informations.
         *
         *@todo (gra): If the Scanning fails, it should return explicity the failing and does not
         *             indicate the failing by an empty measureValues list.
         *
         */
        /********************* ScanningCommand ******************************************/
        class ScanningCommand :
            public wns::ldk::Command
        {
        public:
            typedef unsigned int TransactionID;

            ScanningCommand()
            {
                peer.managementMessageType = MACManagementMessage::UCD;
                // UCD is wrong, but 0

                peer.mob_scn_req.transactionID = 0;
                peer.mob_scn_req.scanDuration = 0;
                peer.mob_scn_req.stationsToScan = service::scanningStrategy::Interface::Stations();

                peer.mob_scn_rsp.transactionID = 0;
                peer.mob_scn_rsp.scanDuration = 0;
                peer.mob_scn_rsp.stationsToScan = service::scanningStrategy::Interface::Stations();

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

                struct MOB_SCN_REQ{
                    ///Unique identifier for this transaction
                    TransactionID transactionID;
                    int scanDuration; // Scan Duration in Frames
                    service::scanningStrategy::Interface::Stations stationsToScan;
                } mob_scn_req;

                struct MOB_SCN_RSP{
                    ///Unique identifier for this transaction
                    TransactionID transactionID;
                    int scanDuration; // In units of frames.
                    // When Scan Duration is set to zero, no
                    // scanning parameters are specified in
                    // the message.
                    // When MOB_SCN-RSP is sent in response to
                    // MOB_SCN-REQ, setting Scan Duration to
                    // zero denies MOB_SCN-REQ.
                    service::scanningStrategy::Interface::Stations stationsToScan;
                } mob_scn_rsp;
            } peer;

            struct {
                Bit size;
            } magic;

        };



        /********** ScanningCallBackInterface ***********************************/
        class ScanningCallBackInterface
        {
        public:
            virtual ~ScanningCallBackInterface()
            {
            }

            virtual void
            resultScanning( const wimac::CIRMeasureInterface::MeasureValues& measureValuesOutput )
                = 0;

        };


        class Scanning :
            public virtual wns::ldk::FunctionalUnit,
            public wns::ldk::CommandTypeSpecifier< ScanningCommand >,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::tools::UpUnconnectable
        {
        public:
            // FUNConfigCreator interface realisation
            Scanning(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

            virtual void
            doWakeup();

        protected:
            std::list<wns::ldk::CompoundPtr> compoundQueue_;

            struct{
                std::string connectionManagerName;
                std::string cirMeasureProviderName;
                std::string connectionClassifierName;
                std::string newFrameProviderName;
                std::string pduWatchProviderName;

                service::ConnectionManager* connectionManager;
                wimac::CIRMeasureInterface* cirMeasureProvider;
                wns::ldk::FunctionalUnit* connectionClassifier;
                wns::ldk::fcf::NewFrameProvider* newFrameProvider;
                scheduler::PDUWatchProvider* pduWatchProvider;
            } friends_;

        };

        /********************* ScanningBS **********************************************/
        /**
         * @brief Scanning implementation for message exchange in the Base Station.
         * @author Markus Grauer <gra@comnets.rwth-aachen.de>
         *
         * \li ScanningBS reacts on an SCN_REQ( PM-CID, ScanDuration ) from ScanningSS
         *   with an MOB_SCN_RSP( PM-CID, ScanDuration )
         * \li The reaction is plain, without any logic
         *
         */
        class ScanningBS:
            public virtual wns::ldk::FunctionalUnit,
            public Scanning,
            public wns::Cloneable< ScanningBS >,
            public wns::ldk::fcf::NewFrameObserver,
            public scheduler::PDUWatchObserver
        {
        public:
            // FUNConfigCreator interface realisation
            ScanningBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

            virtual void
            doOnData(const wns::ldk::CompoundPtr& compound);

            virtual void
            onFUNCreated();

            // NewFrameObserver Interface
            virtual void
            messageNewFrame();

            // PDUWatch Interface
            virtual void
            notifyPDUWatch( wns::ldk::CommandPool commandPool );

        private:

            void doOnMOB_SCN_REQ(const wns::ldk::CompoundPtr& compound);

            std::list<wns::ldk::CommandPool> doMOB_SCN_REQ_;


            //Static values from PyConfig
            Bit mob_scn_rspPDUSize_;

        };


        /**
         * @brief Scanning implementation for message exchange in the Subscriber Station.
         * @author Markus Grauer <gra@comnets.rwth-aachen.de>
         *
         * \li Start ScanningSS  with function
         *     scan( scanDuration, cellIDsToScan, callBackInterface )
         * \li Sends an MOB_SCN_REQ( PM-CID, ScanDuration ) to ScanningBS
         * \li Receives an MOB_SCN_RSP( PM-CID, ScanDuration ) from ScanningBS
         * \li Start to scan and scan for "ScanDuration" * Frames
         *
         */
        class ScanningSS :
            public Scanning,
            public wns::Cloneable<ScanningSS>,
            public wns::ldk::fcf::NewFrameObserver
        {
        public:
            typedef std::list<wimac::Component*> CellsToScan;

            // FUNConfigCreator interface realisation
            ScanningSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);


            void
            start(service::scanningStrategy::Interface::Stations StationsToScan,
                  ScanningCallBackInterface* callBackInterface);

            virtual void
            doOnData(const wns::ldk::CompoundPtr& compound);


            virtual void
            onFUNCreated();

            // NewFrameObserver Interface
            virtual void
            messageNewFrame();


        private:

            void
            startScanning();

            void
            doOnMOB_SCN_RSP(const wns::ldk::CompoundPtr& compound);

            void
            resultScanning(CIRMeasureInterface::MeasureValues measureValues);



            struct{
                bool scan;
                service::scanningStrategy::Interface::Stations stationsToScan;
                service::scanningStrategy::Interface::Stations::const_iterator itStationsToScan;
                ConnectionIdentifier::Frames scanDuration;
                ConnectionIdentifier::Frames remainingScanDuration;
                wns::simulator::Time startTime;
                wns::simulator::Time stopTime;
            } scan_;


            ScanningCommand::TransactionID activeTransactionID_;
            ScanningCommand::TransactionID highestTransactionID_;

            ConnectionIdentifier::Frames remainTimerWaitingForRSP_;

            ScanningCallBackInterface* callBackInterface_;


            //Static values from PyConfig
            ConnectionIdentifier::Frames timerWaitingForRSP_;
            ConnectionIdentifier::Frames timerBetweenFChange_;
            Bit mob_scn_reqPDUSize_;

        };
    }}
#endif


