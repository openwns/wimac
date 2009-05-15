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

#ifndef WIMAC_SERVICES_HANDOVERSTRATEGY_INTERFACE_HPP
#define WIMAC_SERVICES_HANDOVERSTRATEGY_INTERFACE_HPP


#include <WIMAC/CIRMeasureInterface.hpp>
#include <WNS/ldk/PyConfigCreator.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>

namespace wimac {
namespace service {
namespace handoverStrategy{

/**
* @brief Handover Strategy Inteface
* @author Markus Grauer <gra@comnets.rwth-aachen.de>
* -
* -
* -
*
*/
class Interface
{
public:
	typedef ConnectionIdentifier::StationID StationID;
	typedef CIRMeasureInterface::MeasureValues MeasureValues;

	struct Station{
		ConnectionIdentifier::StationID id;
		wns::service::phy::ofdma::Tune tune;
	};
    typedef std::list<Station> Stations;



	virtual
	~Interface(){}

	virtual void
	storeValues(const MeasureValues& measureValues) = 0;

	virtual Stations
	decide(const StationID compareStation) = 0;
};

typedef wns::ldk::PyConfigCreator<Interface> Creator;
typedef wns::StaticFactory<Creator> Factory;

}}} // handoverStrategy::service::wimac

#endif // WIMAC_SERVICES_LOGICHANDOVERRDECISION_HPP


