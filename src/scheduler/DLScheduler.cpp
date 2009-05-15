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

#include <WIMAC/scheduler/DLScheduler.hpp>
#include <WIMAC/Classifier.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/frame/ContentionCollector.hpp>
#include <DLL/StationManager.hpp>
#include <WNS/ldk/Classifier.hpp>
#include <WNS/service/dll/StationTypes.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::frame::MultiCompoundBSDLScheduler,
	wns::ldk::FunctionalUnit,
	"wimac.frame.MultiCompoundBSDLScheduler",
	wns::ldk::FUNConfigCreator );

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::frame::SSDLScheduler,
	wimac::scheduler::SchedulerInterface,
	"wimac.frame.SSDLScheduler",
	wns::ldk::FUNConfigCreator );

using namespace wimac::frame;

MultiCompoundBSDLScheduler::MultiCompoundBSDLScheduler( wns::ldk::fun::FUN* fun,
														const wns::pyconfig::View& config ) :
	ContentionCollector( fun, config )
{
}

void MultiCompoundBSDLScheduler::onFUNCreated()
{
	wns::ldk::fcf::CompoundCollector::onFUNCreated();

	connectionManager_ =
		getFUN()->getLayer()->getManagementService<service::ConnectionManager>( "connectionManager" );

	phyUser_ = getFUN()->findFriend<wimac::PhyUser*>("phyUser");
	assure( phyUser_, "PhyUser not of type wimac::PhyUser" );
	ContentionCollector::onFUNCreated();
}

SSDLScheduler::SSDLScheduler( wns::ldk::fun::FUN* /*fun*/, const wns::pyconfig::View& config) :
	dlMapRetrieverName_( config.get<std::string>("dlMapRetrieverName") )
{}


