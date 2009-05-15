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

#ifndef WIMAC_SERVICES_DEADSTATIONDETECT_HPP
#define WIMAC_SERVICES_DEADSTATIONDETECT_HPP

#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>
#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/SuspendableInterface.hpp>
#include <WNS/ldk/SuspendSupport.hpp>

#include <WIMAC/ConnectionIdentifier.hpp>


namespace wimac {

class ConnectionClassifier;

namespace service {

class ConnectionManager;

/************************* Notify interface *********************/
class DeadStationDetectNotifyInterface
{
public:
	typedef ConnectionIdentifier::CID CID;

	virtual
	~DeadStationDetectNotifyInterface()
		{
		}

	virtual void
	notifyActivityFor(CID cid) = 0;
};




/********************** DeadStationDetect ************************/

/**
	 * @brief DeadStationDetect
	 *
	 *
     *
	 */

class DeadStationDetect
	: public wns::ldk::ManagementService,
	  public DeadStationDetectNotifyInterface,
	  public wns::ldk::fcf::NewFrameObserver
{
	typedef ConnectionIdentifier::StationID StationID;
	typedef ConnectionIdentifier::Frames Frames;
	typedef std::map<StationID, simTimeType> MapStationTime;

public:
	DeadStationDetect(wns::ldk::ManagementServiceRegistry* msr,
					  const  wns::pyconfig::View& config);

	~DeadStationDetect()
		{
			friends_.newFrameProvider->detachObserver(this);
		};

	/// DeadStationDetect notify interface implementation
	virtual void
	notifyActivityFor(const CID cid);

	/// NewFrameObserver interface implementation
	virtual void
	messageNewFrame();

	void
	onMSRCreated();


private:
	/// Update SimTime to an existing stationLastActive_ entry or create a new one
    /// with current SimTime
	void
	setStationLastActive(const StationID stationID);

	/// Adjust stationLastActive_ Map with stations stored in the
	/// ConnectionManager
	void
	adjustStations();

	/// Delete all ConnectionIdentifer for stations,
	/// with their last activity older than deltaTime
	void
	deleteDeadStations(const simTimeType deltaTime);

	/// Execute on timer_ has run out.
	void
	timerExecute();

	int timer_;
	MapStationTime stationLastActive_;

	// Static values from PyConfig
	const int checkInterval_;
	const simTimeType timeToLive_;

	struct{
		std::string connectionManagerName;
		std::string newFrameProviderName;

		wimac::service::ConnectionManager* connectionManager;
		wns::ldk::fcf::NewFrameProvider* newFrameProvider;
	} friends_;

	wns::logger::Logger logger_;
};



/***************** DSDSensor Functiona Unit ***********************/
/**
 * @brief DSDSensor is the sensor for DeadStationDetect management service
 *
 *
 *
 */
class DSDSensor
	: virtual public wns::ldk::FunctionalUnit,
	  public wns::ldk::CommandTypeSpecifier<>,
	  public wns::ldk::HasReceptor<>,
	  public wns::ldk::HasConnector<>,
      public wns::ldk::HasDeliverer<>,
	  public wns::ldk::Processor< DSDSensor>,
	  public wns::Cloneable<DSDSensor>
{
public:
	// FUNConfigCreator interface realisation
	DSDSensor(wns::ldk::fun::FUN* fuNet, const wns::pyconfig::View& config);

	~DSDSensor(){};

	virtual void onFUNCreated();


private:
	/// Procesor interface implementation
	virtual void
	processIncoming(const wns::ldk::CompoundPtr& compound);

    /// Procesor interface implementation
	virtual void
	processOutgoing(const wns::ldk::CompoundPtr&);


    struct {
		std::string connectionClassifierName;
		std::string deadStationDetectName;

		ConnectionClassifier* connectionClassifier;
		DeadStationDetectNotifyInterface* deadStationDetect;
	} friends_;
};

}} // service::wimac
#endif // WIMAC_SERVICES_DEADSTATIONDEDECT_HPP


