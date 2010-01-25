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
 * \file
 * \author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/services/handoverStrategy/BestStation.hpp>
#include <WIMAC/Component.hpp>

using namespace wimac;
using namespace wimac::service;
using namespace wimac::service::handoverStrategy;


STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::service::handoverStrategy::BestStation,
	wimac::service::handoverStrategy::Interface,
	"wimac.services.handoverStrategy.BestStation",
	wns::ldk::PyConfigCreator);



/********** BestStation ***************************************************/

BestStation::BestStation(const wns::pyconfig::View& config)
	: Interface(),
	  minCIR_(wns::Ratio()),
	  stationsDataSpace_()
{
	minCIR_.set_dB(config.get<double>("minCIR"));
}



void
BestStation::storeValues(const MeasureValues& measureValues)
{
	assure( !measureValues.empty(),
			"BestStation::handoverDecision: get no measureValues!\n");

	/********** Put measureValues into DataSpace **************/
	for (MeasureValues::const_iterator
			 it_mv = measureValues.begin(); it_mv != measureValues.end(); ++it_mv)
	{
#ifndef NDEBUG
		for (StationValues::iterator it_sDS = stationsDataSpace_.begin();
			 it_sDS != stationsDataSpace_.end(); ++it_sDS)
		{
		assure( (*it_sDS).stationID	!= (*it_mv).station->getID(),
				"BestStation::storeValues: Value for that StationID already exist!");
		assure( (*it_sDS).tune.frequency != (*it_mv).tune.frequency,
				"BestStation::storeValues: Value for that frequency already exist!");
		}
#endif // NDEBUG

		//append new element
		StationValue stationValue;

		stationValue.stationID = (*it_mv).station->getID();
		stationValue.tune = (*it_mv).tune;
		stationValue.cir.set_factor( (*it_mv).cir.get_factor() );

		stationsDataSpace_.push_back( stationValue );
	}
}//storeValues



BestStation::Stations
BestStation::decide(StationID /*compareStation*/)
{
	assure(!stationsDataSpace_.empty(),
		   "BestStation::decide: Can't decide without values in data space!");

	Stations targetStations;

	/********* Sort stationsDataSpace_ ***********************************/
	stationsDataSpace_.sort( CIRSorter() );

	// Return Stations, best is at beginning, because of sorting before
	for (StationValues::iterator it_sDS = stationsDataSpace_.begin();
		 it_sDS != stationsDataSpace_.end(); ++it_sDS)
	{
		if(it_sDS->cir >= minCIR_)
		{ // only return stations with a cir bigger than minCIR_
			Station station;
			station.id = (*it_sDS).stationID;
			station.tune = (*it_sDS).tune;
			targetStations.push_back( station );
		}
	}

	stationsDataSpace_.clear();
	return targetStations;

} //decide




