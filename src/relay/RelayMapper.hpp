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
 * @author Karsten Klagges <kks@comnets.rwth-aachen.de>
 */
#ifndef WIMAC_RELAYMAPPER_HPP
#define WIMAC_RELAYMAPPER_HPP

#include <WNS/ldk/ldk.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>

namespace wimac {

    namespace service {
        class ConnectionManager;
    }

    class ConnectionClassifier;
    class ACKSwitch;
    namespace relay {


        class RelayMapperCommand :
            public wns::ldk::Command
        {
        public:
            /**
             * @brief Specifies the direction of a Compound.
             *
             * Down direction goes from BS to RS to SS.
             * Up direction goes from SS to RS to BS.
             */
            enum Direction {
                Down,
                Up
            };
            struct {} local;
            struct {
                int direction_;
            } peer;
            struct {} magic;
        };

        /**
         * @brief A FunctionalUnit that rewrites CIDs regarding a
         * internally stored map.
         */
        class RelayMapper :
            public virtual wns::ldk::ProcessorInterface,
            public wns::ldk::CommandTypeSpecifier<RelayMapperCommand>
        {
        public:
            RelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

        };

        /**
         * @brief A dummy implementation of the RelayMapper for BSs.
         */
        class BSRelayMapper :
            public RelayMapper,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::ldk::Processor<BSRelayMapper>,
            public wns::Cloneable<BSRelayMapper>
        {
        public:
            BSRelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

            void onFUNCreated() {}

            /**
             * @brief  Mandatory reimplementation from Processor.
             */
            void processIncoming( const wns::ldk::CompoundPtr& ) {}

            /**
             * @brief Mandatory reimplementation from Processor.
             */
            void processOutgoing( const wns::ldk::CompoundPtr& );
        };

        /**
         * @brief Implemenattion of the RelayMapper in the RS.
         */
        class RSRelayMapper :
            public RelayMapper,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::ldk::Processor<RSRelayMapper>,
            public wns::Cloneable<RSRelayMapper>
        {
        public:
            RSRelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

            void onFUNCreated();

            /**
             * @brief Mandatory reimplementation from Processor.
             */
            void processIncoming( const wns::ldk::CompoundPtr& compound );

            /**
             * @briefMandatory reimplementation from Processor.
             */
            void processOutgoing( const wns::ldk::CompoundPtr& );

            /**
             * @brief One mapping that is stored in the RelayMapper.
             */
            class RelayMapping
            {
            public:
                RelayMapping(): upperConnection_(0), lowerConnection_(0) {}

                RelayMapping( ConnectionIdentifier::CID upperConnection,
                              ConnectionIdentifier::CID lowerConnection ) :
                    upperConnection_(upperConnection),
                    lowerConnection_(lowerConnection){}

                bool operator==( const RelayMapping& rhs ) const;
                bool operator!=( const RelayMapping& rhs ) const
                {
                    return !operator==( rhs );
                }
            private:
                ConnectionIdentifier::CID upperConnection_;
                ConnectionIdentifier::CID lowerConnection_;

                friend class RSRelayMapper;
            };

        private:
            wns::ldk::FunctionalUnit* downRelayInject_;
            wns::ldk::FunctionalUnit* upRelayInject_;
            wimac::ConnectionClassifier* classifier_;
            service::ConnectionManager* connectionManager_;
            wimac::ACKSwitch* ackSwitch_;

            /**
             * @brief The functional unit for which the upper commands shall be copied.
             */
            wns::ldk::FunctionalUnit* copyThreshold_;
            std::string copyThresholdName_;

        public:
            RelayMapping findMapping( const ConnectionIdentifier::CID& id ) const;

            void addMapping( const RelayMapping& mapping );

        private:
            typedef std::list<RelayMapping> Mappings;
            Mappings mappings_;
        };

        class SSRelayMapper :
            public RelayMapper,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::ldk::Processor<SSRelayMapper>,
            public wns::Cloneable<SSRelayMapper>
        {
        public:
            SSRelayMapper( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

            void onFUNCreated(){}

            /**
             * @brief Mandatory reimplementation from Processor.
             */
            void processIncoming( const wns::ldk::CompoundPtr& ) {}

            /**
             * @brief Mandatory reimplementation from Processor.
             */
            void processOutgoing( const wns::ldk::CompoundPtr& );
        };
    }
}
#endif


