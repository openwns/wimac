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

#include <WIMAC/services/DeadStationDetect.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

#include <WNS/ldk/FunctionalUnit.hpp>
#include <DLL/Layer2.hpp>


using namespace wimac::service;


STATIC_FACTORY_REGISTER_WITH_CREATOR(
 	wimac::service::DeadStationDetect,
  	wns::ldk::ManagementServiceInterface,
 	"wimac.services.DeadStationDetect",
  	wns::ldk::MSRConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::service::DSDSensor,
	wns::ldk::FunctionalUnit,
	"wimac.services.DSDSensor",
	wns::ldk::FUNConfigCreator);



/********** ControlPlaneManager ***************************************************/
DeadStationDetect::DeadStationDetect( wns::ldk::ManagementServiceRegistry* msr,
									  const wns::pyconfig::View& config )
 	: ManagementService( msr ),
	  NewFrameObserver(config.get<std::string>("__plugin__")),
	  timer_(-1),
	  stationLastActive_(),
	  checkInterval_(config.get<int>("checkInterval")),
	  timeToLive_(config.get<simTimeType>("timeToLive")),
 	  friends_(),
	  logger_("WIMAC","service::DeadStationDetect")
{
	timer_ = checkInterval_;

	friends_.connectionManagerName = config.get<std::string>("connectionManager");
	friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");

	friends_.connectionManager = NULL;
	friends_.newFrameProvider = NULL;
}



void
DeadStationDetect::notifyActivityFor(CID cid)
{
	ConnectionIdentifier::Ptr ci;
	ConnectionIdentifier::StationID stationID;

	// get ConnectionIdentifier for that CID
	ci = friends_.connectionManager->getConnectionWithID(cid);
	assure(ci,
		   "DeadStationDetect::notifyActivityFor: Didn't get ConnectionIdentifier from ConnectionManager for that CID!");


	// get peer stations for that CID
	stationID = static_cast<dll::Layer2*>(getMSR()->getLayer())->getID();
	if( stationID == ci->subscriberStation_ )
		stationID = ci->baseStation_;
	else if( stationID == ci->baseStation_)
		stationID = ci->subscriberStation_;


	this->setStationLastActive(stationID);
}



void
DeadStationDetect::messageNewFrame()
{
	if ( timer_ == 0 )
	{
		timer_= checkInterval_; // Restart timer_
		this->timerExecute();
	}

   	if( timer_ > 0 )
		--timer_;

}



void
DeadStationDetect::onMSRCreated()
{
	friends_.connectionManager = static_cast<dll::Layer2*>( getMSR()->getLayer() )
		->getManagementService<service::ConnectionManager>
		(friends_.connectionManagerName);

	friends_.newFrameProvider = static_cast<dll::Layer2*>( getMSR()->getLayer() )
		->getFUN()->findFriend<wns::ldk::fcf::NewFrameProvider*>
		(friends_.newFrameProviderName);

	friends_.newFrameProvider->attachObserver(this);
}



/*** Private Functions ***********/

void
DeadStationDetect::setStationLastActive(const StationID stationID)
{
	stationLastActive_[stationID] = wns::simulator::getEventScheduler()->getTime();
}



void
DeadStationDetect::adjustStations()
{
	ConnectionIdentifier::StationID ownID = -1, stationID = -1;
	ConnectionManager::ConnectionIdentifiers cis;

	ownID = static_cast<dll::Layer2*>(getMSR()->getLayer())->getID();
	cis = friends_.connectionManager->getAllConnections();


	// Add new stations from ConnectionManager to stationLastActive_ map
	for( ConnectionManager::ConnectionIdentifiers::const_iterator cisIT = cis.begin();
		 cisIT != cis.end(); cisIT++)
	{
		// Ranging CID 0 mustn't check
		if( (*cisIT)->cid_ == 0 )
			continue;

		// get peer stationID
		if( ownID == (*cisIT)->subscriberStation_ )
			stationID = (*cisIT)->baseStation_;
		else if( ownID == (*cisIT)->baseStation_)
			stationID = (*cisIT)->subscriberStation_;

		// If stationId doesn't exist in stationLastActive_ map, add it with current
		// SimTime
		MapStationTime::const_iterator slaIT = stationLastActive_.end();
		slaIT = stationLastActive_.find(stationID);
		if(slaIT == stationLastActive_.end())
		{
			this->setStationLastActive(stationID);
		}
	}


	// Remove station from stationLastActive_ map, which doesn't exist in the
    // ConnectionManager
	for(MapStationTime::iterator it = stationLastActive_.begin();
		it != stationLastActive_.end(); )
	{
		MapStationTime::iterator cpIt = it;
		it++;

		ConnectionManager::ConnectionIdentifiers::const_iterator cisIT = cis.begin();
		for( ;cisIT != cis.end(); cisIT++)
		{
			if(   (*cisIT)->baseStation_ == cpIt->first
			  || (*cisIT)->subscriberStation_ == cpIt->first)
				break;
		}

		if( cisIT == cis.end() )
			stationLastActive_.erase(cpIt->first);
	}

}


void
DeadStationDetect::deleteDeadStations(const simTimeType deltaSimTime)
{
	simTimeType currentSimTime = wns::simulator::getEventScheduler()->getTime();

	for(MapStationTime::iterator it = stationLastActive_.begin();
		it != stationLastActive_.end(); )
	{
		MapStationTime::iterator cpIt = it;
		it++;

		MESSAGE_SINGLE(NORMAL, logger_,
					   static_cast<dll::Layer2*>(getMSR()->getLayer())->getName()
					   << ": Station:" << cpIt->first
					   <<"   time since last activity: " << (currentSimTime - cpIt->second)
					   <<"   TimeToLive: " << deltaSimTime);


		if( (currentSimTime - cpIt->second) >= deltaSimTime )
		{
			friends_.connectionManager->deleteConnectionsForBS(cpIt->first);
			friends_.connectionManager->deleteConnectionsForSS(cpIt->first);
			stationLastActive_.erase(cpIt->first);
		}
	}
}



void
DeadStationDetect::timerExecute()
{
	MESSAGE_SINGLE(NORMAL, logger_,
				   static_cast<dll::Layer2*>(getMSR()->getLayer())->getName()
				   << ": Checking for dead stations");
	this->adjustStations();
	this->deleteDeadStations(timeToLive_);
}


/***************** DSDSensor Functiona Unit ***********************/

DSDSensor::DSDSensor(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config)
	: wns::ldk::FunctionalUnit(),
	  wns::ldk::CommandTypeSpecifier<>(fun),
	  wns::ldk::HasReceptor<>(),
	  wns::ldk::HasConnector<>(),
	  wns::ldk::HasDeliverer<>(),
	  wns::ldk::Processor< DSDSensor>(),
	  wns::Cloneable<DSDSensor>(),
	  friends_()
{
	friends_.connectionClassifierName
		= config.get<std::string>("connectionClassifier");
	friends_.deadStationDetectName
		= config.get<std::string>("deadStationDetect");

	friends_.connectionClassifier = NULL;
	friends_.deadStationDetect = NULL;
}



void
DSDSensor::onFUNCreated()
{
	friends_.connectionClassifier = getFUN()->findFriend<ConnectionClassifier*>
		(friends_.connectionClassifierName);

	friends_.deadStationDetect = static_cast<dll::Layer2*>(getFUN()->getLayer())
		->getManagementService<service::DeadStationDetectNotifyInterface>
		(friends_.deadStationDetectName);
}



/*** Private Functions *********/

void
DSDSensor::processIncoming(const wns::ldk::CompoundPtr& compound)
{
	wns::ldk::ClassifierCommand* cCommand;
    cCommand = static_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

	friends_.deadStationDetect->notifyActivityFor(cCommand->peer.id);
}



void
DSDSensor::processOutgoing(const wns::ldk::CompoundPtr& /*compound*/)
{
/* // Only do on incomming compounds

  wns::ldk::ClassifierCommand* cCommand;
	cCommand = static_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

	friends_.deadStationDetect->notifyActivityFor(cCommand->peer.id);
*/
}






