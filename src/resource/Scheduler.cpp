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
#include <WIMAC/resource/Scheduler.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>

#include <WIMAC/resource/Job.hpp>
#include <WIMAC/resource/Mapper.hpp>
#include <WIMAC/resource/strategy/FCFS.hpp>


typedef wimac::resource::Scheduler<wimac::resource::Job,
                                   wimac::resource::strategy::FCFS<wimac::resource::Job>,
                                   wimac::resource::Mapper> DefaultScheduler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    DefaultScheduler,
    wimac::scheduler::Interface,
    "wimac.resource.Scheduler",
    wns::ldk::FUNConfigCreator);

