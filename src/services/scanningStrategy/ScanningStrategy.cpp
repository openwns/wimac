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

#include <WIMAC/services/scanningStrategy/ScanningStrategy.hpp>


using namespace wimac;
using namespace wimac::service;
using namespace wimac::service::scanningStrategy;


/********** ScanningStrategy ***************************************************/

ScanningStrategy::ScanningStrategy(VersusInterface* const versusUnit,
								   const dll::Layer2* layer,
								   const wns::pyconfig::View& config)
	: Interface(),
	  ScanningCallBackInterface(),
	  NewFrameObserver(config.get<std::string>("__plugin__")),
	  layer_(layer),
	  versusUnit_(versusUnit),
	  remainRetries_(-1),
	  timer_(-1),
	  retries_(config.get<int>("retries")),
	  framesBetweenScanning_(config.get<ConnectionIdentifier::Frames>
							 ("framesBetweenScanning"))
{
	friends_.scanningProviderName = config.get<std::string>("scanningProvider");
	friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");

	friends_.scanningProvider = NULL;
	friends_.newFrameProvider = NULL;
}

ScanningStrategy::~ScanningStrategy()
{
	if(timer_ >= 0)
		this->timerStop();
}

void
ScanningStrategy::setup(const Stations stationsToScan)
{
	assure( !stationsToScan.empty(),
		    "ScanningStrategy::setup: List stationsToScan is empty!" );

	stationsToScan_ = stationsToScan;
}

void
ScanningStrategy::onFUNCreated()
{
	friends_.scanningProvider = layer_->getFUN()
		->findFriend<controlplane::ScanningSS*>
		(friends_.scanningProviderName);

	friends_.newFrameProvider = layer_->getFUN()
		->findFriend<wns::ldk::fcf::NewFrameProvider*>
		(friends_.newFrameProviderName);
}



void
ScanningStrategy::resultScanning(const wimac::CIRMeasureInterface::MeasureValues& measureValuesOutput)
{
	if(!measureValuesOutput.empty())
		remainRetries_ = -1;

	this->result(measureValuesOutput);
}



void
ScanningStrategy::messageNewFrame()
{
	if ( timer_ == 0 )
	{
		this->timerStop();
		this->timerExecute();
	}

   	if( timer_ > 0 )
		--timer_;
}



/************* Private Functions *********************************************/

void
ScanningStrategy::startScanning(Stations stationsToScan)
{
	if ( remainRetries_ < 0 )
		remainRetries_ = retries_;

	if( remainRetries_ == 0 )
	{
		LOG_INFO(layer_->getName(), ": Scanning failed ", retries_, " times.");

		remainRetries_ = -1;
		this->timerStart(framesBetweenScanning_);
		versusUnit_->scanningStrategyResult(CIRMeasureInterface::MeasureValues());
		return;
	}

	remainRetries_--;

	friends_.scanningProvider->start(stationsToScan, this);
}



void
ScanningStrategy::timerStart(Frames time)
{
	timer_ = time;
	friends_.newFrameProvider->attachObserver(this);
}



void
ScanningStrategy::timerStop()
{
		friends_.newFrameProvider->detachObserver(this);
		timer_ = -1;
}




