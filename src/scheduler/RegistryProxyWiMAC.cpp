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
#include <WNS/service/qos/QoSClasses.hpp>
#include <WNS/service/dll/StationTypes.hpp>

#include <WIMAC/StationManager.hpp>
#include <WIMAC/Classifier.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/parameter/PHY.hpp>
#include <WIMAC/parameter/PHY.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/services/InterferenceCache.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/frame/ULMapCollector.hpp>


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
	  powerFRS(config.get("powerCapabilitiesFRS")),
      numberOfPriorities(config.get<int>("numberOfPriorities")),
          isDL_(config.get<bool>("isDL")),
          mapHandler(0)
{
	//phyModeMapper.reset(wns::service::phy::phymode::createPhyModeMapper(config.getView("phyModeMapper")));// obsolete
}

wns::scheduler::UserID
RegistryProxyWiMAC::getUserForCID(wns::scheduler::ConnectionID cid) {
	assure(connManager, "No valid connection manager");

	// wimax-address of the station
	ConnectionIdentifierPtr wimacCIDPtr =
		connManager->getConnectionWithID( ConnectionIdentifier::CID(cid) );

	assure(wimacCIDPtr != ConnectionIdentifierPtr(), "Invalid CID Ptr");

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
        wns::scheduler::UserID bcast;
        bcast.setBroadcast();
		userId2StationId[bcast] = peerStationId;
        
		return bcast;
	}
	else  //Normal Compound
	{
		Component* peerLayer2 = dynamic_cast<wimac::Component*>
			( TheStationManager::getInstance()->getStationByID(peerStationId) );
		assure( peerLayer2, "Invalid peer layer pointer");
		assure( peerLayer2->getNode(), "No valid Node pointer in peer FUN");

		userId2StationId[wns::scheduler::UserID(peerLayer2->getNode())] = peerStationId;

		return wns::scheduler::UserID(peerLayer2->getNode());
	}
}

wns::service::dll::UnicastAddress
RegistryProxyWiMAC::getPeerAddressForCID(wns::scheduler::ConnectionID cid)
{
    wns::scheduler::UserID user = getUserForCID(cid);
    wns::service::dll::UnicastAddress peerAddress
      = TheStationManager::getInstance()->getStationByNode(user.getNode())->getDLLAddress();
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

	for (ConnectionIdentifiers::iterator iter = CIDs.begin();
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
        
        wns::service::dll::StationType stationType = layer2->getStationType();
        if(stationType == wns::service::dll::StationTypes::UT()) {
            mapHandler = fun->findFriend< wimac::frame::MapHandlerInterface*>("ulmapcollector");
            assure( mapHandler, "mapcollector not of type wimac::scheduler::MapHandler");
        }
}

std::string
RegistryProxyWiMAC::getNameForUser(const wns::scheduler::UserID user)
{
	// I was asked for my name
	if (user == getMyUserID())
		return fun->getLayer()->getName();

	// I was asked for broadcast id
	if (user.isBroadcast())
		return "Broadcast";

	// same workaround as for getConnectionsForUser
	if (userId2StationId.find(user) == userId2StationId.end())
		return std::string("not yet known");

	Component* peerLayer2 = dynamic_cast<wimac::Component*>( TheStationManager::getInstance()->getStationByID(userId2StationId[user]) );
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
	return wns::scheduler::UserID(layer2->getNode());
}

simTimeType
RegistryProxyWiMAC::getOverhead(int /* numBursts */) {
	simTimeType retVal = 0.0;

	assure(0, "Please insert the calculation here");

	return retVal;
}

wns::scheduler::ChannelQualityOnOneSubChannel
RegistryProxyWiMAC::estimateTxSINRAt(const wns::scheduler::UserID user, int slot){
    wns::service::dll::StationType stationType = layer2->getStationType();
    if(stationType != wns::service::dll::StationTypes::UT()) {
	// lookup the results reported by the receiving subscriber station in the
        // local cache
	wns::Power interference =
		layer2->getManagementService<service::InterferenceCache>("interferenceCache")
            ->getAveragedInterference(user.getNode(), slot);
	wns::Ratio pathloss =
		layer2->getManagementService<service::InterferenceCache>("interferenceCache")
            ->getAveragedPathloss(user.getNode(), slot);
        wns::Power carrier =
        layer2->getManagementService<service::InterferenceCache>("interferenceCache")
            ->getAveragedCarrier(user.getNode(), slot);
        return wns::scheduler::ChannelQualityOnOneSubChannel(pathloss, interference, carrier);
    } else {
        //lookup results signaled by the BS-master through the MAP
        return mapHandler->getEstimatedCQI();
    }
}

wns::scheduler::ChannelQualityOnOneSubChannel
RegistryProxyWiMAC::estimateRxSINROf(const wns::scheduler::UserID user, int slot){

	// lookup the results previously reported by us to the remote side
	service::InterferenceCache* remoteCache =
		TheStationManager::getInstance()->
		getStationByNode(user.getNode())->
		getManagementService<service::InterferenceCache>("interferenceCache");

	wns::Ratio pathloss = remoteCache->getAveragedPathloss(getMyUserID().getNode(), slot);
    wns::Power interference = remoteCache->getAveragedInterference(getMyUserID().getNode(), slot);
    wns::Power carrier = remoteCache->getAveragedCarrier(getMyUserID().getNode(), slot);

    return wns::scheduler::ChannelQualityOnOneSubChannel(pathloss, interference, carrier);
}

wns::scheduler::Bits
RegistryProxyWiMAC::getQueueSizeLimitPerConnection() {
	return queueSize;

}

wns::service::dll::StationType
RegistryProxyWiMAC::getStationType(const wns::scheduler::UserID user)
{
	wns::service::dll::StationType stationType = TheStationManager::getInstance()->getStationByNode(user.getNode())->getStationType();
	return stationType;
	/*
	std::map<wns::scheduler::UserID, ConnectionIdentifier::StationID>::const_iterator station =
		userId2StationId.find(user);
	assure(station != userId2StationId.end(), "User not yet registered");

	Component* userLayer2 =
		dynamic_cast<wimac::Component*>( TheStationManager::getInstance()->getStationByID(station->second) );

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
		uID = TheStationManager::getInstance()->getStationByNode(it->getNode())->getID();

		ConnectionIdentifierPtr ci (
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

		for (ConnectionIdentifiers::const_iterator cidIter = cids.begin();
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
RegistryProxyWiMAC::filterReachable(wns::scheduler::UserSet users, const int /*frameNr*/)
{
	users = filterListening(users);

	if (currentQoSFilter != ConnectionIdentifier::NoQoS)
		return filterQoSbased(users);

	// else
	return users;
}

wns::scheduler::UserSet
RegistryProxyWiMAC::filterReachable(wns::scheduler::UserSet users)
{
    return filterReachable(users,0);
}

wns::scheduler::ConnectionSet
RegistryProxyWiMAC::filterReachable(wns::scheduler::ConnectionSet connections, const int /*frameNr*/, bool /*useHARQ*/)
{
       return connections;
}

wns::scheduler::PowerMap
RegistryProxyWiMAC::calcULResources(const wns::scheduler::UserSet& /*users*/, unsigned long int /*rapResources*/) const
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
RegistryProxyWiMAC::getTotalNumberOfUsers(wns::scheduler::UserID user)
{
/*	ConnectionIdentifiers conns = connManager->getAllBasicConnections();
	int count = 0;
	for (ConnectionIdentifiers::iterator it = conns.begin();
		it != conns.end();
		++it )
	{
		if (TheStationManager::getInstance()->getStationByID((*it)->subscriberStation_)->getNode() == user)
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
	if (user.isValid()) { // peer known
	  stationType = TheStationManager::getInstance()->getStationByNode(user.getNode())->getStationType();
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
		assure(false, "oops, don't know other station ("<<user.getNode()->getName()<<") stationType="<<stationType);
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
    return ConnectionIdentifier::MaxQoSCategory + 1;
}

// This is QoS/priority related
int
RegistryProxyWiMAC::getNumberOfPriorities()
{
  return getNumberOfQoSClasses();
}


wns::scheduler::ConnectionList&
RegistryProxyWiMAC::getCIDListForPriority(int priority)
{
    assure(false, "Not implemented");
}


// gets the cids in a set, because the strategy can better handle sorted list of
// cids (a set implicit sorts the cids)
wns::scheduler::ConnectionSet
RegistryProxyWiMAC::getConnectionsForPriority(int priority)
{
    wns::scheduler::ConnectionSet result;

    assure(priority >= 0 && priority < getNumberOfPriorities(),
        "invalid priority " << priority);

    /* The priority is directly mapped to the QoS class number */

    ConnectionIdentifier::List CIDs = 
        connManager->getAllDataConnections(ConnectionIdentifier::Downlink, 
            ConnectionIdentifier::QoSCategory(priority));

    for (ConnectionIdentifiers::iterator iter = CIDs.begin();
            iter != CIDs.end(); ++iter)
        result.insert(wns::scheduler::ConnectionID((*iter)->getID()));
    
    if(isDL_) {
        CIDs = connManager->getAllDataConnections(ConnectionIdentifier::Downlink, 
            ConnectionIdentifier::QoSCategory(priority));
    } else {
        CIDs = connManager->getAllDataConnections(ConnectionIdentifier::Uplink, 
            ConnectionIdentifier::QoSCategory(priority));
    }
    
    for (ConnectionIdentifiers::iterator iter = CIDs.begin();
            iter != CIDs.end(); ++iter)
        result.insert(wns::scheduler::ConnectionID((*iter)->getID()));

    return result; 
}

int
RegistryProxyWiMAC::getPriorityForConnection(wns::scheduler::ConnectionID cid)
{
    ConnectionIdentifierPtr ci = connManager->getConnectionWithID(cid);
    assure(ci != NULL, "Unknown CID");

    return int(ci->qos_);
}

bool
RegistryProxyWiMAC::getDL() const
{
    LOG_INFO("RegistryProxy::getDL called in station ", layer2->getID(), " isDL: ", isDL_);
    return isDL_;
}

bool
RegistryProxyWiMAC::getCQIAvailable() const
{
       return false;
}

wns::Ratio
RegistryProxyWiMAC::getEffectiveUplinkSINR(const wns::scheduler::UserID sender, 
    const std::set<unsigned int>& scs, 
    const wns::Power& txPower)
{
    assure(false, "Not implemented, adapt WiMAC interference cache or use DLLBase ICache.");
}

wns::Ratio
RegistryProxyWiMAC::getEffectiveDownlinkSINR(const wns::scheduler::UserID receiver, 
    const std::set<unsigned int>& scs, 
    const wns::Power& txPower)
{
    assure(false, "Not implemented, adapt WiMAC interference cache or use DLLBase ICache.");
}
