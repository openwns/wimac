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

#include <WIMAC/scheduler/Callback.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/PhyUserCommand.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp> 
#include <WNS/ldk/Layer.hpp> 


using namespace wimac::scheduler;

Callback::Callback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config)
{
    colleagues.registry = 0;
    friends_.phyUser = fun->findFriend<wimac::PhyUser*>("phyUser");
    assureNotNull(friends_.phyUser);

    wns::probe::bus::ContextProviderCollection* cpcParent = 
        &fun->getLayer()->getContextProviderCollection();

    wns::probe::bus::ContextProviderCollection cpc(cpcParent);

    frameOffsetDelayProbe_ = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(cpc, config.get<std::string>(
            "frameOffsetDelayProbeName")));

    transmissionDelayProbe_ = wns::probe::bus::ContextCollectorPtr(
        new wns::probe::bus::ContextCollector(cpc, config.get<std::string>(
            "transmissionDelayProbeName")));

}


void Callback::setColleagues(wns::scheduler::RegistryProxyInterface* registry)
{
    colleagues.registry = registry;
}


