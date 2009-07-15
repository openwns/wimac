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

#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>

#include <WNS/node/Node.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>
#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>

#include <DLL/StationManager.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/parameter/PHY.hpp>
#include <WIMAC/parameter/PHY.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

using namespace wimac;
using namespace wimac::scheduler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(RegistryProxyWiMAC, wns::scheduler::RegistryProxyInterface, "RegistryProxyWiMAC", wns::ldk::FUNConfigCreator);

RegistryProxyWiMAC::RegistryProxyWiMAC(wns::ldk::fun::FUN*, const wns::pyconfig::View& config)
	: //phyModeMapper(NULL),
	  phyModeMapper(wns::service::phy::phymode::PhyModeMapperInterface::getPhyModeMapper(config.getView("phyModeMapper"))),
	  queueSize(config.get<int>("queueSize")),
	  currentQoSFilter(ConnectionIdentifier::NoQoS),
	  powerUT(config.get("powerCapabilitiesUT")),
	  powerAP(config.get("powerCapabilitiesAP")),
	  powerFRS(config.get("powerCapabilitiesFRS"))
{
	//phyModeMapper.reset(wns::service::phy::phymode::createPhyModeMapper(config.getView("phyModeMapper")));// obsolete
}

wns::scheduler::UserID
RegistryProxyWiMAC::getUserForCID(wns::scheduler::ConnectionID cid) {
	assure(connManager, "No valid connection manager");

	// wimax-address of the station
	service::ConnectionManager::ConnectionIdentifierPtr wimacCIDPtr =
		connManager->getConnectionWithID( ConnectionIdentifier::CID(cid) );

	assure(wimacCIDPtr != service::ConnectionManager::ConnectionIdentifierPtr(), "Invalid CID Ptr");

	///\todo Differenzierung subscriber_ / baseStation
	ConnectionIdentifier::StationID peerStationId = 0;

	wns::service::dll::StationType stationType = layer2->getStationType();
	if ( stationType == wns::service::dll::StationTypes::AP() )
	{
		// I'm the BS, so get subscriber's StationID
		peerStationId = wimacCIDPtr->subscriberStation_;
	}
	else if ( stationType == wns::service::dll::StationTypes::UT() )
	{
		// I'm the BS, so get subscriber's StationID
		peerStationId = wimacCIDPtr->baseStation_;
	}
	else if ( stationType == wns::service::dll::StationTypes::FRS() )
	{
		if (wimacCIDPtr->baseStation_ ==  layer2->getID())
			peerStationId = wimacCIDPtr->subscriberStation_;
		else
			peerStationId = wimacCIDPtr->baseStation_;
	}
	else
	{
		assure(0, "Help, I don't know what to do...");
	}


	if (peerStationId == 0) //Broadcast
	{
		userId2StationId[NULL] = peerStationId;
		return NULL;
	}
	else  //Normal Compound
	{
		Component* peerLayer2 = dynamic_cast<wimac::Component*>
			( layer2->getStationManager()->getStationByID(peerStationId) );
		assure( peerLayer2, "Invalid peer layer pointer");
		assure( peerLayer2->getNode(), "No valid Node pointer in peer FUN");

		userId2StationId[peerLayer2->getNode()] = peerStationId;

		return peerLayer2->getNode();
	}
}

wns::service::dll::UnicastAddress
RegistryProxyWiMAC::getPeerAddressForCID(wns::scheduler::ConnectionID cid)
{
    wns::scheduler::UserID user = getUserForCID(cid);
    wns::service::dll::UnicastAddress peerAddress
      = layer2->getStationManager()->getStationByNode(user)->getDLLAddress();
    return peerAddress;
}

wns::scheduler::ConnectionVector
RegistryProxyWiMAC::getConnectionsForUser(const wns::scheduler::UserID user) {
	wns::scheduler::ConnectionVector connections;

	// getUserForCid is called first, so I can remember the mapping there

	assure(userId2StationId.find(user) != userId2StationId.end(), "User not yet registered");
	ConnectionIdentifier::StationID toDestination = userId2StationId[user];

	// the ConnectionManager returns the CIDs ordered with the basic connections
	// coming first, thus I can simply return the cids in this order.
	ConnectionIdentifier::List CIDs = connManager->getOutgoingConnections(toDestination);
	ConnectionIdentifier::List incomings = connManager->getIncomingConnections(toDestination);

	CIDs.insert( CIDs.end(), incomings.begin(), incomings.end());

	LOG_INFO("getOutgoingConnections returned ", CIDs.size(), " connections:");

	for (ConnectionIdentifier::List::const_iterator it = CIDs.begin();
		 it != CIDs.end();
		 ++it)
	{
		LOG_INFO("Connection ID: ", (*it)->getID());
	}

	for (service::ConnectionManager::ConnectionIdentifiers::iterator iter = CIDs.begin();
		 iter != CIDs.end(); ++iter)
		connections.push_back(wns::scheduler::ConnectionID((*iter)->getID()));

	// the convention is that connections to be prioritized have to come first
	return connections;
}

float
RegistryProxyWiMAC::getMinTPForCID(wns::scheduler::ConnectionID /* cid */) {
	///\todo add QoS
	return 0.0;
}


float
RegistryProxyWiMAC::getMaxDelayForCID(wns::scheduler::ConnectionID /* cid */) {
	///\todo add QoS
	return 0.0;
}

wns::Ratio
RegistryProxyWiMAC::getMinSIRForCID(wns::scheduler::ConnectionID /* cid */) {
	///\todo add QoS
	return wns::Ratio::from_dB(-1000.0);
}

wns::scheduler::ConnectionID
RegistryProxyWiMAC::getCIDforPDU(const wns::ldk::CompoundPtr& compound) {
	assure(friends.classifier, "Need a classifier as friend, please set first");
	wns::ldk::ClassifierCommand* command =
		dynamic_cast<wns::ldk::ClassifierCommand*>(friends.classifier->getCommand(compound->getCommandPool()));

	return command->peer.id;

}

void
RegistryProxyWiMAC::setFriends( const wns::ldk::CommandTypeSpecifierInterface* _classifier)
{
	friends.classifier = const_cast<wns::ldk::CommandTypeSpecifierInterface*>(_classifier);
}

void
RegistryProxyWiMAC::setFUN(const wns::ldk::fun::FUN* _fun) {
	fun = const_cast<wns::ldk::fun::FUN*>(_fun);

	assure(fun, "RegistryProxyWiMAC needs a FUN");

	layer2 = dynamic_cast<wimac::Component*>(fun->getLayer());
	assure(layer2, "RegistryProxyWiMAC could not get Layer2 from FUN");

	LOG_INFO("RegistryProxy::setFUN called in station ", layer2->getID(), " ");

	connManager = layer2->getManagementService<service::ConnectionManager>("connectionManager");
	assure(connManager, "RegistryProxyWiMAC needs a Connection Manager");
}

std::string
RegistryProxyWiMAC::getNameForUser(const wns::scheduler::UserID user)
{
	// I was asked for my name
	if (user == getMyUserID())
		return fun->getLayer()->getName();

	// I was asked for broadcast id
	if (user == 0)
		return "Broadcast";

	// same workaround as for getConnectionsForUser
	if (userId2StationId.find(user) == userId2StationId.end())
		return std::string("not yet known");

	Component* peerLayer2 = dynamic_cast<wimac::Component*>( layer2->getStationManager()->getStationByID(userId2StationId[user]) );
	assure(peerLayer2, "Could not get peer layer for user");

	return peerLayer2->getName();
}

wns::service::phy::phymode::PhyModeMapperInterface*
RegistryProxyWiMAC::getPhyModeMapper() const
{
	//return phyModeMapper.get();
	assure (phyModeMapper != NULL, "phyModeMapper==NULL");
	return phyModeMapper;
}

wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
RegistryProxyWiMAC::getBestPhyMode(const wns::Ratio& sinr)
{
	return getPhyModeMapper()->getBestPhyMode(sinr);
}

wns::scheduler::UserID
RegistryProxyWiMAC::getMyUserID()
{
	return layer2->getNode();
}

simTimeType
RegistryProxyWiMAC::getOverhead(int /* numBursts */) {
	simTimeType retVal = 0.0;

	assure(0, "Please insert the calculation here");

	return retVal;
}

wns::CandI RegistryProxyWiMAC::estimateTxSINRAt(const wns::scheduler::UserID user){

	// lookup the results reported by the receiving subscriber station in the
	// local cache
	wns::Power interference =
		layer2->getManagementService<dll::services::management::InterferenceCache>("interferenceCache")->getAveragedInterference(user);
	wns::Power carrier =
		layer2->getManagementService<dll::services::management::InterferenceCache>("interferenceCache")->getAveragedCarrier(user);

	return wns::CandI(carrier, interference);
}

wns::CandI RegistryProxyWiMAC::estimateRxSINROf(const wns::scheduler::UserID user){

	// lookup the results previously reported by us to the remote side
	dll::services::management::InterferenceCache* remoteCache =
		layer2->
		getStationManager()->
		getStationByNode(user)->
		getManagementService<dll::services::management::InterferenceCache>("interferenceCache");

	wns::Power carrier = remoteCache->getAveragedCarrier(getMyUserID());
	wns::Power interference = remoteCache->getAveragedInterference(getMyUserID());

	return wns::CandI(carrier, interference);
}

wns::Power
RegistryProxyWiMAC::estimateInterferenceStdDeviation(const wns::scheduler::UserID user) {
	return layer2->getManagementService<dll::services::management::InterferenceCache>("interferenceCache")->getInterferenceDeviation(user);
}

wns::scheduler::Bits
RegistryProxyWiMAC::getQueueSizeLimitPerConnection() {
	return queueSize;

}

wns::service::dll::StationType
RegistryProxyWiMAC::getStationType(const wns::scheduler::UserID user)
{
	wns::service::dll::StationType stationType = layer2->getStationManager()->getStationByNode(user)->getStationType();
	return stationType;
	/*
	std::map<wns::scheduler::UserID, ConnectionIdentifier::StationID>::const_iterator station =
		userId2StationId.find(user);
	assure(station != userId2StationId.end(), "User not yet registered");

	Component* userLayer2 =
		dynamic_cast<wimac::Component*>( layer2->getStationManager()->getStationByID(station->second) );

	std::string stationType = wns::service::dll::StationTypes::toString( userLayer2->getStationType() );
	if( stationType == "AP" )
	{
		return wns::scheduler::BaseStation;
	}
	else if( stationType == "FRS" )
	{
		return wns::scheduler::RelayStation;
	}
	else if( stationType == "UT" )
	{
		return wns::scheduler::SubscriberStation;
	}
	else if( stationType == "RT" )
	{
		return wns::scheduler::RemoteStation;
	}
	else
	{
			assure(0, "unknown station type");
	}
	return -1;
	*/
}

wns::scheduler::UserSet
RegistryProxyWiMAC::filterListening( wns::scheduler::UserSet users )
{
	wns::scheduler::UserSet result;

	for (wns::scheduler::UserSet::const_iterator it = users.begin();
		 it != users.end(); ++it)
	{
		wimac::ConnectionIdentifier::StationID uID;
		uID = layer2->getStationManager()->getStationByNode(*it)->getID();

		service::ConnectionManager::ConnectionIdentifierPtr ci (
			connManager->getBasicConnectionFor( uID ) );

		assure( ci->valid_, "RegistryProxyWiMAC::filterListening: no ConnectionIdentifier exist for this user!");

		if (ci->ciNotListening_ == 0 )
		{
			result.insert(*it);
		}
	}
	return result;
}

wns::scheduler::UserSet
RegistryProxyWiMAC::filterQoSbased( wns::scheduler::UserSet users )
{
	assure(connManager, "No valid connection manager");

	wns::scheduler::UserSet results, tmpResults;
	results.clear(); tmpResults.clear();



	for (wns::scheduler::UserSet::const_iterator it = users.begin();
		 it != users.end(); ++it)
	{
		wns::scheduler::UserID user = *it;

		assure(userId2StationId.find(user) != userId2StationId.end(), "User not yet registered");
		ConnectionIdentifier::StationID toDestination = userId2StationId[user];

		ConnectionIdentifier::List cids = connManager->getAllCIForSS(toDestination);

		for (service::ConnectionManager::ConnectionIdentifiers::const_iterator cidIter = cids.begin();
			 cidIter != cids.end(); ++cidIter)
		{
			if ((*cidIter)->qos_ == currentQoSFilter)
			{
				results.insert(user);
				LOG_INFO("Included user with QoS=", currentQoSFilter, "  with cidPtr ", **cidIter);
			}

			if (   (currentQoSFilter ==  ConnectionIdentifier::Signaling)
				&& ((*cidIter)->qos_ != ConnectionIdentifier::Signaling) )
			{
				tmpResults.insert(user);
			}

		}
	}


	//We would to catch users with only Signaling Connections
	if(currentQoSFilter == ConnectionIdentifier::Signaling)
		for(wns::scheduler::UserSet::const_iterator it = tmpResults.begin();
			it != tmpResults.end(); ++it)
			results.erase(*it);


	return results;
}

wns::scheduler::UserSet
RegistryProxyWiMAC::filterReachable( wns::scheduler::UserSet users )
{
	users = filterListening(users);

	if (currentQoSFilter != ConnectionIdentifier::NoQoS)
		return filterQoSbased(users);

	// else
	return users;
}

wns::scheduler::ConnectionSet
RegistryProxyWiMAC::filterReachable(wns::scheduler::ConnectionSet connections)
{
       return connections;
}

wns::scheduler::PowerMap
RegistryProxyWiMAC::calcULResources(const wns::scheduler::UserSet& /*users*/, uint32_t /*rapResources*/) const
{
	throw wns::Exception("called un-implemented method RegistryProxyWiMAC::calcULResources");
	return wns::scheduler::PowerMap();
}

wns::scheduler::UserSet
RegistryProxyWiMAC::getActiveULUsers() const
{
	throw wns::Exception("called un-implemented method RegistryProxyWiMAC::getActiveULUsers");
	return wns::scheduler::UserSet();
}

int
RegistryProxyWiMAC::getTotalNumberOfUsers(wns::scheduler::UserID user) const
{
/*	service::ConnectionManagerInterface::ConnectionIdentifiers conns = connManager->getAllBasicConnections();
	int count = 0;
	for (service::ConnectionManagerInterface::ConnectionIdentifiers::iterator it = conns.begin();
		it != conns.end();
		++it )
	{
		if (layer2->getStationManager()->getStationByID((*it)->subscriberStation_)->getNode() == user)
		{
			++count;
		}
	}
	assure(count>0, "No basic connection for user " << user->getName() << " found.");
	LOG_INFO("getTotalNumberOfUsers(): ", count, " users");
	return count;*/
	return 1;
}

void
RegistryProxyWiMAC::switchFilterTo(int qos)
{
	currentQoSFilter = qos;
}

wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
RegistryProxyWiMAC::getChannelQualities4UserOnUplink(wns::scheduler::UserID /*user*/, int)
{
  return wns::scheduler::ChannelQualitiesOnAllSubBandsPtr(); // fake (empty)
}

wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
RegistryProxyWiMAC::getChannelQualities4UserOnDownlink(wns::scheduler::UserID /*user*/, int)
{
  return wns::scheduler::ChannelQualitiesOnAllSubBandsPtr(); // fake (empty)
}

wns::scheduler::PowerCapabilities
RegistryProxyWiMAC::getPowerCapabilities(const wns::scheduler::UserID user) const
{
	wns::service::dll::StationType stationType;
	if (user!=NULL) { // peer known
	  stationType = layer2->getStationManager()->getStationByNode(user)->getStationType();
	} else { // peer unknown. assume peer=UT
	  stationType = wns::service::dll::StationTypes::UT();
	}
	if ( stationType == wns::service::dll::StationTypes::UT() )
		return powerUT; // this may be variable in the future (different userTerminal classes)
	else if ( stationType == wns::service::dll::StationTypes::AP() )
		return powerAP;
	else if ( stationType == wns::service::dll::StationTypes::FRS() )
		return powerFRS;
	else
		assure(false, "oops, don't know other station ("<<user->getName()<<") stationType="<<stationType);
	return wns::scheduler::PowerCapabilities();
}

wns::scheduler::PowerCapabilities
RegistryProxyWiMAC::getPowerCapabilities() const
{
	// get my own station type
	int stationType = layer2->getStationType();
	if ( stationType == wns::service::dll::StationTypes::UT() )
		return powerUT; // this may be variable in the future (different userTerminal classes)
	else if ( stationType == wns::service::dll::StationTypes::AP() )
		return powerAP;
	else if ( stationType == wns::service::dll::StationTypes::FRS() )
		return powerFRS;
	else
		assure(false, "oops, don't know stationType="<<stationType);
	return wns::scheduler::PowerCapabilities();
}

// This is QoS/priority related
int
RegistryProxyWiMAC::getNumberOfQoSClasses()
{
	return wns::service::qos::NUMBEROFQOSCLASSES;
}

// This is QoS/priority related
int
RegistryProxyWiMAC::getNumberOfPriorities()
{
  return 1; // TODO: no QoS yet.
}

//const wns::service::phy::phymode::PhyModeInterfacePtr
//RegistryProxyWiMAC::getPhyMode(wns::scheduler::ConnectionID /*cid*/)
//{
//       return wns::service::phy::phymode::PhyModeInterfacePtr(); // empty
//}

// This is QoS/priority related
wns::scheduler::ConnectionList&
RegistryProxyWiMAC::getCIDListForPriority(int /*priority*/)
{
       wns::scheduler::ConnectionList connList;
       wns::scheduler::ConnectionID cid = 0;
       connList.push_front(cid);
       wns::scheduler::ConnectionList& c = connList;
       return c; // no good idea to return ptr/ref to temporary object
}


// gets the cids in a set, because the strategy can better handle sorted list of
// cids (a set implicit sorts the cids)
wns::scheduler::ConnectionSet
RegistryProxyWiMAC::getConnectionsForPriority(int priority)
{
	wns::scheduler::ConnectionSet result;

	//MESSAGE_SINGLE(NORMAL, logger, "getConnectionsforPriority("<<priority<<")");
	assure(priority>=0 && priority<getNumberOfPriorities(),"invalid priority "<<priority);
	/*
	for ( wns::scheduler::ConnectionList::iterator iter = connectionsForPriority[priority].begin();
		  iter != connectionsForPriority[priority].end(); ++iter)
	{
	  ConnectionID cid = *iter;
	  result.insert(cid);
	  MESSAGE_SINGLE(NORMAL, logger, "getConnectionsforPriority(): added cid=" << cid);
	}
	*/
	return result; // TODO: QOS for WiMAX
}

int
RegistryProxyWiMAC::getPriorityForConnection(wns::scheduler::ConnectionID /*cid*/)
{
       return 0;
}

bool
RegistryProxyWiMAC::getDL() const
{
       return true;
}

bool
RegistryProxyWiMAC::getCQIAvailable() const
{
       return false;
}
