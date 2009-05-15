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

#ifndef WIMAC_SERVICES_SCANNINGSTRATEGY_SCANNINGSTRATEGY_HPP
#define WIMAC_SERVICES_SCANNINGSTRATEGY_SCANNINGSTRATEGY_HPP

#include <WIMAC/services/scanningStrategy/Interface.hpp>
#include <WIMAC/controlplane/Scanning.hpp>

#include <DLL/Layer2.hpp>

#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>

namespace wimac {
namespace service {
namespace scanningStrategy{

/**
* @brief Scanning Strategy Basic Class
* @author Markus Grauer <gra@comnets.rwth-aachen.de>
* -
* -
* -
*
*/
class ScanningStrategy
	: public Interface,
	  public wimac::controlplane::ScanningCallBackInterface,
	  public wns::ldk::fcf::NewFrameObserver

{
public:

	typedef ConnectionIdentifier::Frames Frames;
	typedef wimac::CIRMeasureInterface::MeasureValues MeasureValues;


	ScanningStrategy( VersusInterface* const versusUnit,
					  const dll::Layer2* layer,
					  const wns::pyconfig::View& config );

	virtual
	~ScanningStrategy();

	/// scanningStrategy::Interface Implementation
	virtual void
	controlRSP() = 0;

	/// scanningStrategy::Interface Implementation
	virtual void
	abortScanning() = 0;

	/// scanningStrategy::Interface Implementation
	virtual void
	setup(const Stations stationsToScan);

	/// scanningStrategy::Interface Implementation
	virtual void
	onFUNCreated();

	/// ScanningCallBackInterface implementation
	virtual void
	resultScanning(const wimac::CIRMeasureInterface::MeasureValues& measureValuesOutput);

	/// NewFrameObserver Interface
	virtual void
	messageNewFrame();


protected:

	virtual void
	result(const MeasureValues& measureValues) = 0;

	void
	startScanning(Stations stationsToScan);

	/// timer control function: start
	void
	timerStart(Frames time);

	/// timer control function: stop
	void
	timerStop();

	/// timer execute function
	virtual void
	timerExecute() = 0;

	Stations stationsToScan_;

	const dll::Layer2* layer_;
	VersusInterface* const versusUnit_;

	int remainRetries_;
	Frames timer_;

	// Static values from PyConfig
	const int retries_;
	const Frames framesBetweenScanning_;

	struct {
		std::string scanningProviderName;
		std::string newFrameProviderName;

		wimac::controlplane::ScanningSS* scanningProvider;
		wns::ldk::fcf::NewFrameProvider* newFrameProvider;
	} friends_;
};

}}} // scanningStrategy::service::wimac

#endif // WIMAC_SERVICES_SCANNINGSTRATEGY_SCANNINGSTRATEG_HPP


