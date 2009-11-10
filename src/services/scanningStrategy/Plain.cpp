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


/**
 * @file
 * @author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/services/scanningStrategy/Plain.hpp>
#include <WIMAC/services/scanningStrategy/VersusInterfaceLayerConfigCreator.hpp>
#include <WIMAC/Component.hpp>

using namespace wimac;
using namespace wimac::service;
using namespace wimac::service::scanningStrategy;


STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::scanningStrategy::Plain,
    wimac::service::scanningStrategy::Interface,
    "wimac.services.scanningStrategy.Plain",
    VersusInterfaceLayerConfigCreator);



Plain::Plain(VersusInterface* versusUnit,
             Component* layer,
             const wns::pyconfig::View& config):
    ScanningStrategy(versusUnit, layer, config)
{
}



void
Plain::controlRSP()
{
    LOG_INFO( layer_->getName(),
              ": Calling controlRSP()");

    remainRetries_ = -1;
    this->scan();
}



void
Plain::setup(const Stations stationsToScan)
{
    ScanningStrategy::setup(stationsToScan);
    this->timerStart(framesBetweenScanning_);
}



void
Plain::result(const MeasureValues& measuredValues)
{
    if( measuredValues.empty() )
    { // Scanning failed
        this->scan();
        return;
    }

    this->timerStart(framesBetweenScanning_);
    versusUnit_->scanningStrategyResult(measuredValues);
}



void
Plain::timerExecute()
{
    this->timerStart(framesBetweenScanning_);
    versusUnit_->scanningStrategyControlREQ();
}



void
Plain::scan()
{
    if(timer_ >= 0)
        this->timerStop();
    this->startScanning(stationsToScan_);
}


