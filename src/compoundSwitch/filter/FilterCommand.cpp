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

#include <WIMAC/compoundSwitch/filter/FilterCommand.hpp>

#include <WIMAC/compoundSwitch/CompoundSwitch.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/Assure.hpp>
#include <WNS/logger/Logger.hpp>

#include <WNS/ldk/Command.hpp>


using namespace wimac;
using namespace wimac::compoundSwitch;
using namespace wimac::compoundSwitch::filter;


STATIC_FACTORY_REGISTER_WITH_CREATOR(FilterCommand,
									 Filter,
									 "wimac.compoundSwitch.filter.FilterCommand",
									 CompoundSwitchConfigCreator);

FilterCommand::FilterCommand(CompoundSwitch* compoundSwitch,
							 wns::pyconfig::View& config) :
	Filter(compoundSwitch, config),
	friends_()
{
	friends_.commandProviderName = config.get<std::string>("commandProvider");

	friends_.commandProvider = NULL;
} // Filter



FilterCommand::~FilterCommand()
{
} // ~Filter



void
FilterCommand::onFUNCreated()
{
	friends_.commandProvider = compoundSwitch_->findFUNFriend(friends_.commandProviderName);
}



bool
FilterCommand::filter(const wns::ldk::CompoundPtr& compound) const
{
    if( compoundSwitch_->getCommandProxy()->commandIsActivated( compound->getCommandPool(), friends_.commandProvider ) )
		return true;

	return false;
}






