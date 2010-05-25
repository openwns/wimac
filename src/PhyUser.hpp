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


#ifndef WIMAC_PHYUSER_HPP
#define WIMAC_PHYUSER_HPP

#include <WNS/node/Node.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <WIMAC/GuiWriter.hpp>

#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Receptor.hpp>
#include <WNS/ldk/Connector.hpp>

#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/service/phy/ofdma/Notification.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/service/dll/Address.hpp>

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/PhyUserCommand.hpp>


#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>

namespace wns { namespace node{
    class Node;
}}

namespace wns { namespace ldk { namespace fcf {
    class FrameBuilder;
}}}

namespace wimac {

    class GuiWriter;
    namespace service {
        class ConnectionManager;
        class InterferenceCache;
    }

    class ConnectionClassifier;
    class Layer2;
    class InterferenceCache;

    /**
     * @brief The PhyUser receives all incoming compounds.
     *
     * @todo The PhyUser should not control the FrameBuilder. The PhyUser is a
     * passive element of the FUN.
     *
     * @ingroup frameConfigurationFramework
     */

    class PhyUser :
        public virtual wns::ldk::FunctionalUnit,
        public virtual wns::service::phy::ofdma::Handler,
        public wns::ldk::CommandTypeSpecifier< PhyUserCommand >,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<>,
        public wns::ldk::HasDeliverer<>,
        public wns::Cloneable<PhyUser>
    {
        enum States {initial, receiving, measuring};

    public:
        PhyUser(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
        PhyUser( const PhyUser& rhs );
		virtual ~PhyUser();

        // CompoundHandlerInterface
        virtual bool doIsAccepting(const wns::ldk::CompoundPtr& compound) const;
        virtual void doSendData(const wns::ldk::CompoundPtr& sdu);
        virtual void doOnData(const wns::ldk::CompoundPtr& compound);
        virtual void doWakeup();

        // Interface to lower layer (phy)
        /**
         * @name wns::service::phy::ofdma::Handler: Interface to lower
         * layer (OFDMAPhy)
         */
        virtual void
        onData(wns::SmartPtr<wns::osi::PDU>,
               wns::service::phy::power::PowerMeasurementPtr);

        // Handling of the services
        virtual void
        setNotificationService(wns::service::Service* phy);

        virtual wns::service::phy::ofdma::Notification*
        getNotificationService() const;

        virtual void
        setDataTransmissionService(wns::service::Service* phy);

        virtual wns::service::phy::ofdma::DataTransmission*
        getDataTransmissionService() const;

        virtual void
        setMACAddress(const wns::service::dll::UnicastAddress& address);

        void onFUNCreated();

    private:

        bool filter( const wns::ldk::CompoundPtr& compound);

        // point in time when last C/I measurement was written to
        // interference cache
        wns::simulator::Time cacheEntryTimeStamp;

        /**
         * @todo make max waiting time of 1.0 second configurable
         */
        const wns::simulator::Time maxAgeCacheEntry;

        struct{

            wns::probe::bus::ContextCollectorPtr interferenceSDMA;
            wns::probe::bus::ContextCollectorPtr carrierSDMA;
            wns::probe::bus::ContextCollectorPtr cirSDMA;
            wns::probe::bus::ContextCollectorPtr deltaPHYModeSDMA;
			wns::probe::bus::ContextCollectorPtr PHYModeSDMA;
            wns::probe::bus::ContextCollectorPtr deltaInterferenceSDMA;
            wns::probe::bus::ContextCollectorPtr deltaCarrierSDMA;
            wns::probe::bus::ContextCollectorPtr interferenceFrameHead;
            wns::probe::bus::ContextCollectorPtr cirFrameHead;
            wns::probe::bus::ContextCollectorPtr interferenceContention;
            wns::probe::bus::ContextCollectorPtr cirContention;
            wns::probe::bus::ContextCollectorPtr pathloss;
        } probes_;

        wns::simulator::Time safetyFraction;
        wns::events::scheduler::Interface* es;

        wns::service::dll::UnicastAddress address;
        wns::service::phy::ofdma::DataTransmission* dataTransmission;
        wns::service::phy::ofdma::Notification* notificationService;

        struct Friends {
            std::string interferenceCacheName;
            std::string connectionManagerName;
            std::string connectionClassifierName;

            wimac::Component* layer;
            service::InterferenceCache* interferenceCache;
            service::ConnectionManager* connectionManager;
            ConnectionClassifier* connectionClassifier;
        } friends_;


        wns::probe::bus::ContextCollectorPtr guiProbe_;

        GuiWriter* GuiWriter_;

    };
}


#endif
