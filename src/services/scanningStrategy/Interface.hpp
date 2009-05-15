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

#ifndef WIMAC_SERVICES_SCANNINGSTRATEGY_INTERFACE_HPP
#define WIMAC_SERVICES_SCANNINGSTRATEGY_INTERFACE_HPP


#include <WNS/ldk/PyConfigCreator.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>

#include <WIMAC/CIRMeasureInterface.hpp>
#include <WIMAC/services/scanningStrategy/VersusInterfaceLayerConfigCreator.hpp>

namespace wimac {
namespace service {
namespace scanningStrategy{

/**
* @brief Scanning Strategy Inteface
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

	struct Station{
		ConnectionIdentifier::StationID id;
		wns::service::phy::ofdma::Tune tune;
	};
    typedef std::list<Station> Stations;


	virtual
	~Interface(){}

	virtual void
	controlRSP() = 0;

	virtual void
	abortScanning() = 0;

	virtual void
	setup(const Stations stationToScan) = 0;

	virtual void
	onFUNCreated() = 0;
};

typedef VersusInterfaceLayerConfigCreator<Interface> Creator;
typedef wns::StaticFactory<Creator> Factory;


/**
* @brief Versus Scanning Strategy Inteface
* @author Markus Grauer <gra@comnets.rwth-aachen.de>
* -
* -
* -
*
*/
class VersusInterface
{
public:
	typedef CIRMeasureInterface::MeasureValues MeasureValues;


	virtual
	~VersusInterface(){}

	virtual void
	scanningStrategyControlREQ() = 0;

	virtual void
	scanningStrategyResult(const MeasureValues& mValues) = 0;
};

}}} // scanningStrategy::service::wimac

#endif // WIMAC_SERVICES_SCANNINGSTRATEGY_INTERFACE_HPP


