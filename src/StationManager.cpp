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
#include <WIMAC/StationManager.hpp>

#include <WNS/node/Node.hpp>
#include <WNS/Functor.hpp>
#include <WNS/Assure.hpp>

#include <WIMAC/Component.hpp>

using namespace wimac;

void
StationManager::registerStation(StationID id,
                                wns::service::dll::UnicastAddress adr,
                                Component* layer)
{
    assure(!macAdrLookup.knows(adr), "Station already registered.");
    macAdrLookup.insert(adr, layer);

    if (layerLookup.knows(id))
    {
        // id is already known -> station has more than one address
        wns::Exception e;
        e << "A station with id " << id << " is already registered using a different Component";
        throw e;
    }
    layerLookup.insert(id, layer);

    if(nodeLookup.knows(layer->getNode()))
    {
        wns::Exception e;
        e << "A station with this node is already registered using a different Component*";
        throw e;
    }
    nodeLookup.insert(layer->getNode(), layer);
}


Component*
StationManager::getStationByID(StationID id) const
{
    return layerLookup.find(id);
}

Component*
StationManager::getStationByNode(wns::node::Interface* node) const
{
    return nodeLookup.find(node);
}


wns::node::Interface*
StationManager::getNodeByID(StationID id) const
{
    return this->getStationByID(id)->getNode();
}


Component*
StationManager::getStationByMAC(wns::service::dll::UnicastAddress adr) const
{
    return macAdrLookup.find(adr);
}


