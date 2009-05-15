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

#ifndef WIMAC_SERVICES_SCANNINGSTRATEGY_INTERUPTED_HPP
#define WIMAC_SERVICES_SCANNINGSTRATEGY_INTERUPTED_HPP


#include <WIMAC/services/scanningStrategy/ScanningStrategy.hpp>


namespace wimac {
namespace service {
namespace scanningStrategy{

/**
* @brief Interupted Scanning Strategy
* @author Markus Grauer <gra@comnets.rwth-aachen.de>
* -
* -
* -
*
*/
class Interupted
	: public ScanningStrategy


{
public:
	Interupted( VersusInterface* const versusUnit,
				const dll::Layer2* layer,
				const wns::pyconfig::View& config);

	virtual
	~Interupted(){}

	/// ScanningStrategy Interface implementation
	virtual void
	controlRSP();

	/// ScanningStrategy Interface implementation
	virtual void
	abortScanning()
		{
			assure(0, "Interupted::abortScanning: This functionality is not supported!");
		}

    /// scanningStrategy::Interface Implementation
	virtual void
	setup(const Stations stationsToScan);


private:

	/// ScanningStrategy Interface implementation
	virtual void
	result(const MeasureValues& measureValuesOutput);

	void
	timerExecute();

	void
	scan();

	Stations::const_iterator oldNextStationToScan_;
	Stations::const_iterator nextStationToScan_;
	CIRMeasureInterface::MeasureValues measuredValues_;

	// Static values from PyConfig
	const int maxNumberOfStationsToScan_;
	const ConnectionIdentifier::Frames framesBetweenSubScanning_;
};

}}} // scanningStrategy::service::wimac

#endif // WIMAC_SERVICES_SCANNINGSTRATEGY_INTERRUPTED_HPP


