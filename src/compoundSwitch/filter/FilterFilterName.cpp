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

#include <WIMAC/compoundSwitch/filter/FilterFilterName.hpp>

#include <WIMAC/compoundSwitch/CompoundSwitch.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/Assure.hpp>
#include <WNS/logger/Logger.hpp>

#include <WNS/ldk/Command.hpp>


using namespace wimac;
using namespace wimac::compoundSwitch;
using namespace wimac::compoundSwitch::filter;


STATIC_FACTORY_REGISTER_WITH_CREATOR(FilterFilterName,
									 Filter,
									 "wimac.compoundSwitch.filter.FilterFilterName",
									 CompoundSwitchConfigCreator);

FilterFilterName::FilterFilterName(CompoundSwitch* compoundSwitch,
							 wns::pyconfig::View& config) :
	Filter(compoundSwitch, config),
	friends_()
{
} // Filter



FilterFilterName::~FilterFilterName()
{
} // ~Filter



void
FilterFilterName::onFUNCreated()
{
}



bool
FilterFilterName::filter(const wns::ldk::CompoundPtr& compound) const
{
    if( compoundSwitch_->getCommand( compound->getCommandPool() )
		->local.filterName == getName() )
		return true;

	return false;
}






