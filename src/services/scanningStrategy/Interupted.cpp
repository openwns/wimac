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

/**
 * \file
 * \author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/services/scanningStrategy/Interupted.hpp>
#include <WIMAC/services/scanningStrategy/VersusInterfaceLayerConfigCreator.hpp>

using namespace wimac;
using namespace wimac::service;
using namespace wimac::service::scanningStrategy;


STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::service::scanningStrategy::Interupted,
	wimac::service::scanningStrategy::Interface,
	"wimac.services.scanningStrategy.Interupted",
	VersusInterfaceLayerConfigCreator);


/********** Interupted ***************************************************/

Interupted::Interupted(VersusInterface* const versusUnit,
					   const dll::Layer2* layer,
					   const wns::pyconfig::View& config)
	: ScanningStrategy(versusUnit, layer, config),
	  oldNextStationToScan_(),
	  nextStationToScan_(),
	  measuredValues_(),
	  maxNumberOfStationsToScan_(config.get<int>("maxNumberOfStationsToScan")),
	  framesBetweenSubScanning_(config.get<int>("framesBetweenSubScanning"))
{
	oldNextStationToScan_ = stationsToScan_.end();
	nextStationToScan_ = stationsToScan_.end();
}

void
Interupted::controlRSP()
{
	LOG_INFO( layer_->getName(),
			  ": Calling controlRSP()");

	remainRetries_ = -1;
	measuredValues_.clear();
	this->scan();
}



void
Interupted::setup(const Stations stationsToScan)
{
	ScanningStrategy::setup(stationsToScan);
	nextStationToScan_ = stationsToScan_.end();
	this->timerStart(framesBetweenScanning_);
}



/************* Private Functions *********************************************/

void
Interupted::result(const MeasureValues& measuredValues)
{
	if( measuredValues.empty() )
	{ // Scanning failed
		nextStationToScan_= oldNextStationToScan_; // go to previous subScanning

		this->scan();
		return;
	}

	// Store measured values
	MeasureValues mV = measuredValues;
	measuredValues_.merge(mV);

	if(nextStationToScan_ != stationsToScan_.end())
	{ // Scanning isn't finished, continue scanning after a break;
		this->timerStart(framesBetweenSubScanning_);
		return;
	}

	this->timerStart(framesBetweenScanning_);
	MeasureValues result = measuredValues_;
	measuredValues_.clear();
	versusUnit_->scanningStrategyResult(result);
}



void
Interupted::timerExecute()
{
	if(nextStationToScan_ == stationsToScan_.end())
	{
		this->timerStart(framesBetweenScanning_);
		versusUnit_->scanningStrategyControlREQ();
	} else
	{
		this->scan();
	}
}



void
Interupted::scan()
{
	if(timer_ >= 0)
		this->timerStop();

	/** Scan only the allowed max number of stations at one scan process **/
	Stations stationsToScan;

	if(nextStationToScan_ == stationsToScan_.end())
		nextStationToScan_ = stationsToScan_.begin();

	oldNextStationToScan_ = nextStationToScan_;

	for(int i = 0; i != maxNumberOfStationsToScan_; ++i)
	{
		if(nextStationToScan_ == stationsToScan_.end())
			break;

		stationsToScan.push_back(*nextStationToScan_);
		++nextStationToScan_;
	}
	/*********************************************************************/

	this->startScanning(stationsToScan);
}




