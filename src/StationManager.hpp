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
#ifndef WIMAC_STATIONMANAGER_HPP
#define WIMAC_STATIONMANAGER_HPP

#include <cstdlib>
#include <vector>

#include <WNS/logger/Logger.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/Singleton.hpp>
#include <WNS/container/Registry.hpp>
#include <WNS/service/dll/Address.hpp>

namespace wns { namespace node {
        class Interface;
}}

namespace wimac {

    class Component;

    typedef unsigned int StationID;

    /**
     * @brief The StationManager is an information database for all DLLs in the
     * simulator.
     *
     * \li Upon simulation startup, the RANG Node has to be registered at the
     *   Stationmanager.
     * \li After creation, each Component instance also registers at the
     * stationManager.
     *
     * @todo On the long run, we should get rid of the stationIDs here and only
     * work on wns::service::dll::Address
     */
    class StationManager
    {
        typedef wns::container::Registry<StationID, Component*> LayerLookup;
        typedef wns::container::Registry<wns::node::Interface*, Component*> NodeLookup;
        typedef wns::container::Registry<wns::service::dll::UnicastAddress, Component*> MACAdrLookup;

    public:
        /**
         * @brief Register a station (Layer2) after its creation.
         */
        void registerStation(StationID id,
                             wns::service::dll::UnicastAddress adr,
                             Component* layer);

        /** @name Lookup methods */
        //@{
        Component* getStationByMAC(wns::service::dll::UnicastAddress mac) const;
        Component* getStationByID(StationID) const;
        Component* getStationByNode(wns::node::Interface*) const;
        wns::node::Interface* getNodeByID(StationID) const;
        //@}

    private:
        MACAdrLookup macAdrLookup;
        LayerLookup layerLookup;
        NodeLookup nodeLookup;
    };

    typedef wns::SingletonHolder<StationManager> TheStationManager;
}


#endif


