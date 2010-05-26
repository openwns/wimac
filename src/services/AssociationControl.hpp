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

#ifndef WIMAC_SERVICES_ASSOCIATIONCONTROL_H
#define WIMAC_SERVICES_ASSOCIATIONCONTROL_H

#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WNS/StaticFactory.hpp>
#include <WNS/PowerRatio.hpp>
#include <WNS/service/phy/power/PowerMeasurement.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/StationManager.hpp>
#include <WIMAC/services/IChannelQualityObserver.hpp>

#include <boost/bind.hpp>

namespace wimac { namespace service {

        class ConnectionManager;
        class AssociationControl :
            public wns::ldk::ControlService,
            public IChannelQualityObserver
        {
        public:
            AssociationControl( wns::ldk::ControlServiceRegistry* csr,
                               wns::pyconfig::View& config );


            void
            createRecursiveConnection( ConnectionIdentifier::CID basicCID,
                                       ConnectionIdentifier::CID primaryCID,
                                       ConnectionIdentifier::CID downlinkTransportCID,
                                       ConnectionIdentifier::CID uplinkTransportCID,
                                       ConnectionIdentifier::StationID remote,
                                       int qosCategory);
            void onCSRCreated();

            void associateTo( wimac::StationID destination,
                              ConnectionIdentifier::QoSCategory category);

            void
            storeMeasurement(StationID source, 
                const wns::service::phy::power::PowerMeasurementPtr&);

        private:
            virtual void 
            doOnCSRCreated() = 0;

            virtual void
            doStoreMeasurement(StationID source, 
                const wns::service::phy::power::PowerMeasurementPtr&) = 0;

            struct {
                wimac::service::ConnectionManager* connectionManager;
            } friends_;

        };

    namespace associationcontrol {

        class IDecideBest
        {
        public:
            typedef wns::Creator<IDecideBest> Creator;
            typedef wns::StaticFactory<Creator> Factory;

            virtual void
            put(StationID st, const wns::service::phy::power::PowerMeasurementPtr& p) = 0;
            
            virtual StationID
            getBest()= 0;

            virtual bool
            isInitialized() = 0;
        };

        template <typename T>
        class DecideBest :
            public IDecideBest
        {
        public:
            DecideBest() :
                samples_(0),
                initialized_(false){};

            void
            put(StationID st, const wns::service::phy::power::PowerMeasurementPtr& p)
            {
                samples_++;
                doPut(st, p);
            };

            StationID
            getBest(){return bestStation_;};

            virtual bool
            isInitialized(){return initialized_;};

            virtual
            ~DecideBest()
            {}
        private:
            virtual void 
            doPut(StationID, const wns::service::phy::power::PowerMeasurementPtr&) = 0;

        protected:
            StationID bestStation_;
            T bestValue_;
            unsigned int samples_;
            bool initialized_;
        };

        class BestPathloss :
            public DecideBest<wns::Ratio>
        {
            virtual void
            doPut(StationID st, const wns::service::phy::power::PowerMeasurementPtr& p)
            {
                if(!initialized_ || p->getPathLoss() < bestValue_)
                {
                    initialized_ = true;
                    bestValue_ = p->getPathLoss();
                    bestStation_ = st;
                }
            };            
        };

        class BestRxPower :
            public DecideBest<wns::Power>
        {
            virtual void
            doPut(StationID st, const wns::service::phy::power::PowerMeasurementPtr& p)
            {
                if(!initialized_ || p->getRxPower() > bestValue_)
                {
                    initialized_ = true;
                    bestValue_ = p->getRxPower();
                    bestStation_ = st;
                }
            };            
        };

        class BestSINR :
            public DecideBest<wns::Ratio>
        {
            virtual void
            doPut(StationID st, const wns::service::phy::power::PowerMeasurementPtr& p)
            {
                if(!initialized_ || p->getSINR() > bestValue_)
                {
                    initialized_ = true;
                    bestValue_ = p->getSINR();
                    bestStation_ = st;
                }
            };            
        };

        /** @brief Associate to node ID provided by PyConfig
         */
        class Fixed :
            public AssociationControl
        {
        public:
            Fixed(wns::ldk::ControlServiceRegistry* csr,
                                wns::pyconfig::View& config );
            virtual
            ~Fixed();
    
        private:
                virtual void
                doOnCSRCreated();

                virtual void
                doStoreMeasurement(StationID source, 
                    const wns::service::phy::power::PowerMeasurementPtr&);

                wimac::StationID associatedWithID_;
    
        };

        /** @brief Associate to best BS decided by strategy at a given time
         */
        class BestAtGivenTime :
            public AssociationControl
        {
        public:
            BestAtGivenTime(wns::ldk::ControlServiceRegistry* csr,
                                wns::pyconfig::View& config );
            virtual
            ~BestAtGivenTime();
    
        private:
                virtual void
                doOnCSRCreated();

                void
                doStoreMeasurement(StationID source, 
                    const wns::service::phy::power::PowerMeasurementPtr&);

                void
                associateNow();

                wns::simulator::Time decisionTime_;
                IDecideBest* deciderStrategy_;
        };
    }

    }
}

#endif


