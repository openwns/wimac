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

#include <WIMAC/Component.hpp>

#include <boost/bind.hpp>

#include <WNS/rng/RNGen.hpp>

#include <WNS/ldk/fcf/FrameBuilder.hpp>
#include <WNS/ldk/buffer/Buffer.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WIMAC/ConnectionKey.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>

#include <DLL/StationManager.hpp>

#include <WIMAC/Logger.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/services/ControlPlaneManager.hpp>
#include <WIMAC/services/ControlPlaneManagerSimple.hpp>
#include <WIMAC/services/DeadStationDetect.hpp>


#include <cstdlib>
#include <sstream>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(Component,
									 wns::node::component::Interface,
									 "wimac.Component",
									 wns::node::component::ConfigCreator);



class StartControlPlaneManagerSS
{
public:
	StartControlPlaneManagerSS(wimac::service::ControlPlaneManagerInterface* cpm,
							   ConnectionIdentifier::StationID associateTo,
							   ConnectionIdentifier::QoSCategory qosCategory) :
		controlPlaneManagerSS_(cpm),
		associateTo_(associateTo),
		qosCategory_(qosCategory)
	{}

	virtual void execute()
	{
		controlPlaneManagerSS_->start(associateTo_, qosCategory_);
	}

private:
	service::ControlPlaneManagerInterface* controlPlaneManagerSS_;
	ConnectionIdentifier::StationID associateTo_;
	ConnectionIdentifier::QoSCategory qosCategory_;
};


Component::Component(wns::node::Interface* _node, const wns::pyconfig::View& _config) :
    dll::Layer2(_node, _config, NULL),
        //wimac::WithStationType(_config),
    associateTo_(dll::Layer2::invalidStationID),
	qosCategory_(ConnectionIdentifier::NoQoS),
	randomStartDelayMax_(_config.get<simTimeType>("randomStartDelayMax"))
{
	LOG_INFO( "Creating station ", _node->getName(), " with station ID ", stationID,
			  " and station type ", type );

	if ( !_config.isNone("associateTo") )
		associateTo_ = _config.get<StationID>("associateTo");

	// get qosCategory
	std::string qosCategory = config.get<std::string>("qosCategory");
	if (qosCategory == "Signaling"){
		qosCategory_ = ConnectionIdentifier::Signaling;
	}else if (qosCategory == "UGS"){
		qosCategory_ = ConnectionIdentifier::UGS;
	}else if (qosCategory == "rtPS"){
		qosCategory_ = ConnectionIdentifier::rtPS;
	}else if (qosCategory ==  "nrtPS"){
		qosCategory_ = ConnectionIdentifier::nrtPS;
	}else if (qosCategory ==  "BE"){
		qosCategory_ = ConnectionIdentifier::BE;
	}else{
		qosCategory_ = ConnectionIdentifier::NoQoS;
	}


}

void
Component::doStartup()
{
    dll::Layer2::doStartup();
    getNode()->getContextProviderCollection().
        addProvider(wns::probe::bus::contextprovider::Callback("MAC.CellId", boost::bind(&wimac::Component::getCellID, this ) ) );
}

Component::~Component()
{
}


uint32_t
Component::getCellID()
{
	if ( getStationType() == wns::service::dll::StationTypes::AP() )
	{
		return getID();
	} else
	{
		ConnectionIdentifier::Ptr ci;

		ci = getManagementService<service::ConnectionManager>
			("connectionManager")->getConnectionWithID(0);

		if( ci )
		{
			Component* associatedWith = dynamic_cast<wimac::Component*>
				( getStationManager()->getStationByID(ci->baseStation_) );
			assure(associatedWith, "Station is not associated with a WiMAC station");

			return associatedWith->getCellID();
		}
		return 0;
	}
}


void
Component::onNodeCreated()
{
	// get service names of the lower layer
	std::string transServiceName = config.get<std::string>("phyDataTransmission");
	std::string notifyServiceName = config.get<std::string>("phyNotification");

	// set services in PhyUser to communicate with lower layer
	PhyUser* phyUser = getFUN()->findFriend<PhyUser*>("phyUser");
	phyUser->setDataTransmissionService(
			getService<wns::service::phy::ofdma::DataTransmission*>( transServiceName ) );
	phyUser->setNotificationService(
			getService<wns::service::phy::ofdma::Notification*>( notifyServiceName ) );
	phyUser->setMACAddress( address );

	fun->onFUNCreated();
	getMSR()->onMSRCreated();
	getCSR()->onCSRCreated();

	// Start the Framebuilder and set it to pause state for synchronising the
	// periodically event of all stations. Afterwards the PhyUser controls
	// the FrameBuilder by the startMeasuring and stopMeasuring methods.
	wns::ldk::fcf::FrameBuilder* frameBuilder = getFUN()->findFriend<wns::ldk::fcf::FrameBuilder*>("frameBuilder");
	frameBuilder->start();
	frameBuilder->pause();

	// Set PhyUser receiving for AP
	if( getStationType() == wns::service::dll::StationTypes::AP() )
	{
		phyUser->startMeasuring();
		phyUser->stopMeasuring();
	}

}

void
Component::onWorldCreated()
{
}

void
Component::onShutdown()
{
}

service::ConnectionManager*
Component::getConnectionManagerMaster()
{
	Component* associatedWith = dynamic_cast<wimac::Component*>
		( dll::TheStationManager::getInstance()->getStationByID(associateTo_) );

	if ( !associatedWith ) {
		std::stringstream error;
		error << getMSR()->getLayer()->getName() << " has not found the station it is associated with";
		throw wns::Exception( error.str() );
	}

	LOG_TRACE(getName(),
			 ": associatedWith: ", associatedWith->getStationType());

	if ( associatedWith->getStationType() == wns::service::dll::StationTypes::AP() )
		return associatedWith
			->getManagementService<service::ConnectionManager>("connectionManager");
	else
		return associatedWith
			->getConnectionManagerMaster();
}

int
Component::getNumberOfQueuedPDUs(service::ConnectionManager::ConnectionIdentifiers cis)
{
	int queuedPDUs = 0;
	for(service::ConnectionManager::ConnectionIdentifiers::const_iterator conn = cis.begin();
		conn != cis.end();
		++conn)
	 {
		 assure((*conn)->direction_ != ConnectionIdentifier::Downlink,
				"Component::getNumberOfQueuedPDUs(...) works for uplink PDUs only");
		 wns::ldk::FlowSeparator* bufferSep = getFUN()->findFriend<wns::ldk::FlowSeparator*>("bufferSep");
		 wns::ldk::ConstKeyPtr key(new ConnectionKey((*conn)->cid_));
		 wns::ldk::buffer::Buffer* buffer = dynamic_cast<wns::ldk::buffer::Buffer*>(bufferSep->getInstance(key));
		 if(buffer)
			 queuedPDUs += buffer->getSize();
	 }
	if (getFUN()->knowsFunctionalUnit("upRelayInject"))
	{
		wns::ldk::buffer::Buffer* relayInject =
			getFUN()->findFriend<wns::ldk::buffer::Buffer*>("upRelayInject");
		queuedPDUs += relayInject->getSize();
		LOG_INFO("Added ", relayInject->getSize(),
				 " PDUs to the number of total PDUs waiting for beeing transmitted");
	}
	LOG_INFO( getName(), " with station ID ", stationID,
			  " and station type ", type, " has queued PDUS ", queuedPDUs );
	return queuedPDUs;
}
