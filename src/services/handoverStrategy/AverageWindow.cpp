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

#include <WIMAC/services/handoverStrategy/AverageWindow.hpp>


using namespace wimac;
using namespace wimac::service;
using namespace wimac::service::handoverStrategy;


STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::service::handoverStrategy::AverageWindow,
	wimac::service::handoverStrategy::Interface,
	"wimac.services.handoverStrategy.AverageWindow",
	wns::ldk::PyConfigCreator);


/********** AverageWindow ***************************************************/

AverageWindow::AverageWindow(const wns::pyconfig::View& config)
	: Interface(),
	  stationsDataSpace_(),
	  windowForAverage_( config.get<int>("windowForAverage") )
{
}


void
AverageWindow::storeValues(const MeasureValues& measureValues)
{
	assure( !measureValues.empty(),
			"AverageWindow::handoverDecision: get no measureValues!\n");

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
				(*it_sDS).mValues.push_back( (*it_mv) ); //add to element
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
	 		stationValue.mValues.push_back( *it_mv );

			stationsDataSpace_.push_back( stationValue );
		}

	}
} //storeValues




AverageWindow::Stations
AverageWindow::decide(StationID compareStation)
{
	this->doAverage();

	/********* Sort stationsDataSpace_ ***********************************/
	stationsDataSpace_.sort( CIRSorter() );

	return this->decide_(compareStation);

} //decide





/************* Private Functions *********************************************/


void
AverageWindow::doAverage()
{
	/********* Cut list of each element in DataSpace to maximum lenght ***/

	for (StationValues::iterator it_sDS = stationsDataSpace_.begin();
		 it_sDS != stationsDataSpace_.end(); ++it_sDS)
	{
		while( (*it_sDS).mValues.size() > static_cast<size_t>(windowForAverage_) )
		{
			(*it_sDS).mValues.pop_front();
		}
	}


    /********* Calculate the average of each element *********************/

	for (StationValues::iterator it_sDS = stationsDataSpace_.begin();
			 it_sDS != stationsDataSpace_.end(); ++it_sDS)
		{
			wns::Ratio sumCIR;

			for (std::list<CIRMeasureInterface::MValue>::iterator it_mV = (*it_sDS).mValues.begin();
				 it_mV != (*it_sDS).mValues.end(); ++it_mV)
			{
				sumCIR += (*it_mV).cir;
			}

			(*it_sDS).averageCIR.set_factor( sumCIR.get_factor() / (*it_sDS).mValues.size() );
		}

} //doAverage



AverageWindow::Stations
AverageWindow::decide_(const StationID compareStation)
{
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
			"AverageWindow::handoverDecision: No Station in list we are associated to! \n");


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




