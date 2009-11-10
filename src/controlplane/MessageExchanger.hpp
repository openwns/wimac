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

#ifndef WIMAC_CONTROLPLANE_MESSAGEEXCHANGER_HPP
#define WIMAC_CONTROLPLANE_MESSAGEEXCHANGER_HPP


#include <WNS/pyconfig/View.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>

#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>

#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

#include <WIMAC/MACHeader.hpp>

#include <time.h>

namespace wimac{
    class ConnectionClassifier;

    namespace controlplane{



        /**
         *
         * @todo (gra): It seems to be better, if only one management message
         * Command exist for all ControlFUs. Members of the peer struct should
         * be only the ManagementMessageType and a Container with the specific
         * message informations.
         */
        class MessageExchangerCommand:
            public wns::ldk::Command
        {
        public:
            typedef int MessageType;
            typedef int StationID;

            struct ExchangeID{
                ExchangeID():
                    time(0),
                    funName()
                {}

                bool operator==(const ExchangeID eID) const
                {
                    return (time == eID.time) && (funName == eID.funName);
                }

                bool operator!=(const ExchangeID eID) const
                {
                    return (time != eID.time) || (funName != eID.funName);
                }

                time_t time;
                std::string funName;
            };

            MessageExchangerCommand()
            {
                peer.managementMessageType = -1;
                peer.exchangeID = ExchangeID();
                peer.peerID = -1;
                magic.size = 0;
            }

            virtual
            Bit getSize() const
            {
                return magic.size;
            }

            struct {} local;

            struct {
                int managementMessageType;
                ExchangeID exchangeID;
                StationID peerID;
            } peer;

            struct {
                Bit size;
            } magic;

        };

        class MessageExchangerCallBackInterface
        {
        public:
            virtual ~MessageExchangerCallBackInterface()
            {
            }

            virtual void
            resultMessageExchanger(std::string name, bool result) = 0;
        };


        /**
         * @brief MessageExchanger exchanges messages between two peers.
         */
        class MessageExchanger:
            public virtual wns::ldk::FunctionalUnit,
            public wns::ldk::CommandTypeSpecifier< MessageExchangerCommand >,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasDeliverer<>,
            public wns::Cloneable< MessageExchanger >,
            public wns::ldk::fcf::NewFrameObserver
        {
            typedef int Frames;
            typedef MessageExchangerCommand::MessageType MessageType;
            typedef std::map<MessageType,Bit> Messages;
            typedef ConnectionIdentifier::StationID StationID;
            typedef ConnectionIdentifier::CID CID;
            typedef ConnectionIdentifier::List ConnectionIdentifiers;
            typedef ConnectionIdentifier::Ptr ConnectionIdentifierPtr;

            struct PeerSet {
                PeerSet():
                    exchangeID( MessageExchangerCommand::ExchangeID() ),
                    remainTimerWaitingForReply(-1),
                    sendMessage()
                {}

                MessageExchangerCommand::ExchangeID exchangeID;
                Frames remainTimerWaitingForReply;
                Messages::const_iterator sendMessage;
            };

            typedef std::map<StationID, PeerSet> MessageExchanges;

        public:
            MessageExchanger(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& );
            ~MessageExchanger(){};

            void
            start(MessageExchangerCallBackInterface* callBackInterface);

            virtual bool
            doIsAccepting(const wns::ldk::CompoundPtr&) const
            {
                assure (0,
                        "wimac::MessageExchanger: doIsAcception is not in use! \n");
                return false;
            }

            virtual void
            doSendData(const wns::ldk::CompoundPtr&)
            {
                assure (0,
                        "wimac::MessageExchanger: doSendData is not in use! \n");
            }

            virtual void
            doOnData(const wns::ldk::CompoundPtr& compound);

            virtual void
            doWakeup();

            virtual void
            onFUNCreated();

            /**
             * @brief Callback of the NewFrameObserver.
             */
            virtual void
            messageNewFrame();


        private:
            void result(StationID peerID, bool result);

            wns::ldk::CompoundPtr createMessage(const StationID peerID, const MessageType messageType);

            StationID getPeerID(const CID cid) const;

            void clear();

            MessageExchanges messageExchanges_;

            MessageExchangerCallBackInterface* callBackInterface_;

            std::list<wns::ldk::CompoundPtr> compoundQueue_;

            wns::logger::Logger logger_;

            //Static values from PyConfig
            const int connectionType_;
            const Frames timerWaitingForReply_;
            Messages messages_;

            struct{
                std::string connectionManagerName;
                std::string connectionClassifierName;
                std::string newFrameProviderName;

                service::ConnectionManager* connectionManager;
                wimac::ConnectionClassifier* connectionClassifier;
                wns::ldk::fcf::NewFrameProvider* newFrameProvider;
            } friends_;
        };
    }}

#endif


