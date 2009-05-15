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

#include <WIMAC/WiMAC.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/parameter/PHY.hpp>

#include <DLL/StationManager.hpp>
#include <WNS/pyconfig/Parser.hpp>
#include <WNS/pyconfig/View.hpp>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(WiMAC, wns::module::Base, "wimac", wns::PyConfigViewCreator);

WiMAC::WiMAC(const wns::pyconfig::View& _pyConfigView) :
	wns::module::Module<WiMAC>(_pyConfigView)
{
	LOG_INFO( "creating module: WiMAC" );
//	version = wns::VersionInformation(BUILDVINFO);
	// Register a service if this module provides a service
	//MODULE_REGISTER_SERVICE(SomeService, "SomeService", ProjNameModule::getSomeService);
}

void WiMAC::configure()
{
    wns::pyconfig::View pyco = getPyConfigView().getView("logger");
    WiMACLogger::getInstance()->configure(pyco);

    // not available (and not needed) in testing mode
    if (!getPyConfigView().isNone("parametersPHY"))
    {
        pyco = getPyConfigView().getView("parametersPHY");
        parameter::ThePHY::getInstance()->init(pyco);
    }
}

dll::StationManager*
WiMAC::getStationManager()
{
	return dll::TheStationManager::getInstance();
}

void WiMAC::startUp()
{
}

void WiMAC::shutDown()
{
}
