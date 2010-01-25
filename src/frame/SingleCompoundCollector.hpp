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

#ifndef WIMAC_FRAME_SINGLECOMPOUNDCOLLECTOR_H
#define WIMAC_FRAME_SINGLECOMPOUNDCOLLECTOR_H

#include <WNS/ldk/fcf/CompoundCollector.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>

#include <WIMAC/Component.hpp>

namespace wimac {
    class ConnectionClassifier;
    class PhyUser;
    namespace frame {

        /**
         * @brief A CompoundCollector that collects a single compound.
         */
        class SingleCompoundCollector :
            public wns::ldk::fcf::CompoundCollector,
            public wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::Cloneable<SingleCompoundCollector>
        {
        public:
            SingleCompoundCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config ) :
                wns::ldk::fcf::CompoundCollector( config ),
                wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>(fun),
                phyMode(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
                        (wns::service::phy::phymode::createPhyMode( config.getView("phyMode") ) ) )
            {
                component_ = dynamic_cast<Component*>( getFUN()->getLayer() );
                assure( component_, "the single compound collector needs to be part of the WiMAC layer" );
            }

            void doOnData( const wns::ldk::CompoundPtr& );
            void doSendData( const wns::ldk::CompoundPtr& );

            bool doIsAccepting( const wns::ldk::CompoundPtr& compound ) const;

            void doWakeup(){}

            void doStart(int);

            void doStartCollection(int){ accepting_ = true; getReceptor()->wakeup(); }
            void finishCollection(){ accepting_ = false; }
            wns::simulator::Time getCurrentDuration() const;

        protected:

            virtual void onFUNCreated();

        private:
            struct
            {
                wimac::ConnectionClassifier* classifier_;
                wimac::PhyUser* phyUser_;
            } friends_;

            Component* component_;

            wns::ldk::CompoundPtr compound_;
            bool accepting_;
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyMode;
        };
    }
}
#endif


