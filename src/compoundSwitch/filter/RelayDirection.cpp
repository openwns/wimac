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

#include <WIMAC/compoundSwitch/filter/RelayDirection.hpp>
#include <WIMAC/relay/RelayMapper.hpp>
#include <DLL/compoundSwitch/CompoundSwitch.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::compoundSwitch::filter::RelayDirection,
	dll::compoundSwitch::Filter,
	"wimac.compoundSwitch.filter.RelayDirection",
	dll::compoundSwitch::CompoundSwitchConfigCreator);

using namespace wimac::compoundSwitch::filter;

RelayDirection::RelayDirection(dll::compoundSwitch::CompoundSwitch* compoundSwitch, wns::pyconfig::View& config) :
	dll::compoundSwitch::Filter(compoundSwitch, config)
{
	direction_ = config.get<int>("direction");
}

void
RelayDirection::onFUNCreated()
{
	mapper_ = dynamic_cast<wimac::relay::RelayMapper*>
		(compoundSwitch_->findFUNFriend("relayMapper"));
	assureNotNull(mapper_);
}

bool
RelayDirection::filter( const wns::ldk::CompoundPtr& compound) const
{
	wimac::relay::RelayMapperCommand* command =
		mapper_->getCommand(compound->getCommandPool());
	return command->peer.direction_ == direction_;
}


