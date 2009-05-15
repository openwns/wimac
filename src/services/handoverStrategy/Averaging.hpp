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

#ifndef WIMAC_SERVICES_HANDOVERSTRATEGY_AVERAGING_HPP
#define WIMAC_SERVICES_HANDOVERSTRATEGY_AVERAGING_HPP


#include <WIMAC/services/handoverStrategy/Interface.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>

namespace wimac {
namespace service {
namespace handoverStrategy {

/**
* @brief Averaging handover strategy
* @author Markus Grauer <gra@comnets.rwth-aachen.de>
* -
* -
* -
*
*/
class Averaging
	: public Interface
{
	struct StationValue{
		ConnectionIdentifier::StationID stationID;
		wns::service::phy::ofdma::Tune tune;
		wns::Ratio averageCIR;
	};

	typedef std::list<StationValue> StationValues;


	struct CIRSorter : public std::binary_function<StationValue,StationValue,bool>
	{
		bool operator()(const StationValue& lhs,const StationValue& rhs) const
			{
				return lhs.averageCIR > rhs.averageCIR;
			}
	};

public:

	Averaging(const wns::pyconfig::View&);

	~Averaging(){};

    /// HandoverStrategyInterface implementation
	virtual void
	storeValues(const MeasureValues& measureValues);

    /// HandoverStrategyInterface implementation
	virtual Stations
	decide(const StationID compareStation);

private:

	void doAverage();

	StationValues stationsDataSpace_;

	// Static values from PyConfig
	const double alpha_;
};


}}} //handoverStrategy::service::wimac

#endif // WIMAC_SERVICES_LOGICHANDOVERRDECISION_HPP


