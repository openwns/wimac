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

#ifndef WIMAC_COMPONENT_HPP
#define WIMAC_COMPONENT_HPP


#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/fun/Main.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/node/component/Component.hpp>

#include <DLL/Layer2.hpp>
#include <DLL/services/management/InterferenceCache.hpp>

#include <WIMAC/services/ConnectionManager.hpp>

#include <WIMAC/Logger.hpp>

namespace dll {
    class StationManager;
}

namespace wimac {

    namespace service {
        class ControlPlaneManagerInterface;
    }

    class FunctionalUnit;
    class CommandProxy;

    /**
     * \brief
     * Layer2 represents the layer2 instances, virtually being the jacket
     * for all functionalUnits.
     *
     * @todo replace getNumberOfQueuedPDUs with a better solution
     */
    class Component:
        public dll::Layer2
    {
        typedef ConnectionIdentifier::StationID StationID;
        typedef ConnectionIdentifier::QoSCategory QoSCategory;

    public:

        Component(wns::node::Interface*, const wns::pyconfig::View&);

        virtual ~Component();

        uint32_t getCellID();

        virtual void printName() { std::cout << getName() << std::endl; }

        service::ConnectionManager*
        getConnectionManagerMaster();

        //Used by class PseudoBWreqGenerator for BWreq shortcut.
        int getNumberOfQueuedPDUs(service::ConnectionManager::ConnectionIdentifiers cis);

        // ComponentInterface
        virtual void onNodeCreated();
        virtual void onWorldCreated();
        virtual void onShutdown();

    private:
        Component(const Component&);	// disallow copy constructor
        Component& operator=(const Component&); // disallow assignment

        virtual void
        doStartup();

        //Values form PyConfig
        StationID associateTo_;
        QoSCategory qosCategory_;
        simTimeType randomStartDelayMax_;
    };
}

#endif
