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

#include <WIMAC/services/handoverStrategy/Averaging.hpp>


using namespace wimac;
using namespace wimac::service;
using namespace wimac::service::handoverStrategy;


STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::service::handoverStrategy::Averaging,
	wimac::service::handoverStrategy::Interface,
	"wimac.services.handoverStrategy.Averaging",
	wns::ldk::PyConfigCreator);


/********** Averaging ***************************************************/

Averaging::Averaging(const wns::pyconfig::View& config)
	: Interface(),
	  stationsDataSpace_(),
	  alpha_(config.get<double>("alpha"))
{
}



void
Averaging::storeValues(const MeasureValues& measureValues)
{
	assure( !measureValues.empty(),
			"Averaging::handoverDecision: get no measureValues!\n");

	/********** Put measureValues into DataSpace **************/
	for (MeasureValues::const_iterator it_mv = measureValues.begin();
		 it_mv != measureValues.end(); ++it_mv)
	{
		//LOG_INFO( layer_->getFUN()->getName(), ": store MValues   StationID:",(*it_mv).station->getID(),
		//		  " CIR:",(*it_mv).cir);


		bool inserted = false;

		for (StationValues::iterator it_sDS = stationsDataSpace_.begin();
			 it_sDS != stationsDataSpace_.end(); ++it_sDS)
		{
			if(   ( (*it_sDS).stationID	== (*it_mv).station->getID() )
			   && ( (*it_sDS).tune.frequency == (*it_mv).tune.frequency )
			   )
			{
				(*it_sDS).averageCIR.set_factor(
					(1-alpha_)*(*it_sDS).averageCIR.get_factor() + alpha_*(*it_mv).cir.get_factor()
					); //add to element
				inserted = true;
			}
		}

		if (inserted == false)
		{
            //append new element
			StationValue stationValue;

			stationValue.stationID = (*it_mv).station->getID();
			stationValue.tune = (*it_mv).tune;
			stationValue.averageCIR.set_factor( (*it_mv).cir.get_factor() );

			stationsDataSpace_.push_back( stationValue );
		}
	}
} //storeValues



Averaging::Stations
Averaging::decide(StationID compareStation)
{
	// Sort stationsDataSpace for cir average
	stationsDataSpace_.sort( CIRSorter() );


	Stations targetStations;


	// get mValues in stationDataSpace_ for associatedToStation
	StationValues::iterator it_associateToStation = stationsDataSpace_.begin();
	for (; it_associateToStation != stationsDataSpace_.end(); ++it_associateToStation)
	{
		if( (*it_associateToStation).stationID == compareStation )
		{
			break;
		}
	}
	assure( it_associateToStation != stationsDataSpace_.end(),
			"Averaging::handoverDecision: No Station in list we are associated to! \n");


	// Search in StationDataSpace for better Stations
	for (StationValues::iterator it_sDS = stationsDataSpace_.begin();
		 it_sDS != stationsDataSpace_.end(); ++it_sDS)
	{

		//LOG_INFO("               StationID:",(*it_sDS).stationID,
		//		 " averageCIR:",(*it_sDS).averageCIR);

		if ( (*it_sDS).averageCIR > (*it_associateToStation).averageCIR )
		{
			Station station;
			station.id = (*it_sDS).stationID;
			station.tune = (*it_sDS).tune;
			targetStations.push_back( station );
		}

	}

	return targetStations;
} //decide




