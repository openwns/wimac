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

#ifndef WIMAC_SERVICES_CONTROLPLANEMANAGER_HPP
#define WIMAC_SERVICES_CONTROLPLANEMANAGER_HPP

#include <WIMAC/services/ControlPlaneManagerInterface.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/EventSubjectObserver.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/services/Dissociating.hpp>
#include <WIMAC/services/Associating.hpp>
#include <WIMAC/services/handoverStrategy/Interface.hpp>
#include <WIMAC/services/scanningStrategy/Interface.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <WNS/probe/bus/ContextProvider.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>

namespace wimac { namespace service {

class ConnectionManager;


/**
	 * @brief ControlPlaneManagerSS search peridicaly for better base station and do the
	 *        handover by using:
	 *
     * - Scanning provider
	 * - Handover provider
     * - Ranging provider
	 * - Regristration provider
     * - SetupConnectionProvider
     *
	 */

class ControlPlaneManagerSS
	: public ControlPlaneManagerInterface,
	  public wns::ldk::ManagementService,
	  public scanningStrategy::VersusInterface,
	  public AssociatingCallBackInterface,
	  public DissociatingCallBackInterface,
	  public wimac::EventSubject
{
	enum State{
		InitialScanning,
		Scanning,
		Associating,
		Dissociating,
		Associated,
		Dissociated
	};

	enum ScanningMode{
		Initial,
		Main
	};

public:
	typedef CIRMeasureInterface::MeasureValues MeasureValues;


	ControlPlaneManagerSS(wns::ldk::ManagementServiceRegistry* msr,
						  const  wns::pyconfig::View& config);

	~ControlPlaneManagerSS()
		{
			delete strategyHandover_;
			strategyHandover_ = NULL;
			delete strategyBestStation_;
			strategyBestStation_ = NULL;
			delete scanningStrategy_;
			scanningStrategy_ = NULL;
			delete associating_;
			associating_= NULL;
			delete dissociating_;
			dissociating_ = NULL;
		};

	/// ControlPlaneManagerInterface implementation
	void
	start(StationID associateTo, QoSCategory QoSCategory);

	/// scanningStrategy::VersusInterface implementation
	virtual void
	scanningStrategyControlREQ();

	/// scanningStrategy::VersusInterface implementation
	virtual void
	scanningStrategyResult(const MeasureValues& measuredValues);

	/// AssociatingCallBackInterface implementation
	virtual void
	resultAssociating(const bool result, const double failure);

	/// DissociatingCallBackInterface implementation
	virtual void
	resultDissociating( const handoverStrategy::Interface::Stations
						newBaseStations);

	/// ControlPlaneManagerInterface implementation
	void
	onMSRCreated();



private:
	void
	stateInitinalScanning(const MeasureValues& measureValues);

	void
	stateScanning(const MeasureValues& measureValues);

	void
	doNextStep(const State state);

	void
	setScanningStrategy(const ScanningMode mode);


	State state_;
	QoSCategory qosCategory_;

	handoverStrategy::Interface::Stations targetBaseStations_;
	handoverStrategy::Interface::Stations ackBaseStations_;

	scanningStrategy::Interface* scanningStrategy_;

	//Probes
	simTimeType startTimeHO_;
	bool handover_;
	double failure_;
	wns::probe::bus::contextprovider::Variable probeAssociatedToContextProvider_;
	wns::probe::bus::contextprovider::Variable probeQoSCategoryContextProvider_;
	wns::probe::bus::ContextCollectorPtr probeHandoverDuration_;
	wns::probe::bus::ContextCollectorPtr probeFailure_;

	service::Associating* associating_;
	service::Dissociating* dissociating_;

	// Static values from PyConfig
	handoverStrategy::Interface* strategyHandover_;
	handoverStrategy::Interface* strategyBestStation_;

	const wns::pyconfig::View configScanningStrategyInitial_;
	const wns::pyconfig::View configScanningStrategyMain_;

	scanningStrategy::Interface::Stations stationsToScan_;
	wns::service::phy::ofdma::Tune tuneSiding_;


	struct{
		std::string connectionManagerName;
		std::string phyUserName;

		wimac::service::ConnectionManager* connectionManager;
		wimac::PhyUser* phyUser;
	} friends_;

};


}} // service::wimac

#endif // WIMAC_SERVICES_CONTROLPLANEMANAGER_HPP


