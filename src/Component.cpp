/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
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

#include <cstdlib>
#include <sstream>

#include <boost/bind.hpp>

#include <WNS/rng/RNGen.hpp>
#include <WNS/ldk/fcf/FrameBuilder.hpp>
#include <WNS/ldk/buffer/Buffer.hpp>
#include <WNS/ldk/fun/Main.hpp>
#include <WNS/ldk/utils.hpp>
#include <WNS/ldk/Group.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WNS/service/phy/ofdma/Handler.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/service/dll/StationTypes.hpp>

#include <WIMAC/ConnectionKey.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/StationManager.hpp>
#include <WIMAC/UpperConvergence.hpp>
#include <WIMAC/helper/ContextProvider.hpp>


using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(Component,
                                     wns::node::component::Interface,
                                     "wimac.Component",
                                     wns::node::component::ConfigCreator);


Component::Component(wns::node::Interface* node, const wns::pyconfig::View& config) :
    wns::node::component::Component(node, config),
    stationType_(wns::service::dll::StationTypes::fromString(config.get<std::string>("stationType"))),
    id_(config.get<unsigned int>("stationID")),
    address_(config.get<wns::service::dll::UnicastAddress>("address"))
{
    LOG_INFO( "Creating station ", node->getName(), " with station ID ", id_,
              " and station type ", wns::service::dll::StationTypes::toString(stationType_) );

    // build FUN
    fun_ = new wns::ldk::fun::Main(this);
    wns::ldk::configureFUN(fun_, config.get<wns::pyconfig::View>("fun"));

    // fire up control and management Services
    { // do control services
        for (int ii = 0; ii<config.len("controlServices"); ++ii){
            wns::pyconfig::View controlServiceView = config.get("controlServices",ii);
            std::string serviceName = controlServiceView.get<std::string>("serviceName");
            std::string creatorName = controlServiceView.get<std::string>("__plugin__");
            wns::ldk::ControlServiceCreator* serviceCreator = wns::ldk::ControlServiceFactory::creator(creatorName);
            wns::ldk::ControlServiceInterface* service = serviceCreator->create(getCSR(), controlServiceView);
            addControlService(serviceName, service);
            LOG_INFO(" Registered Control Service: " , serviceName);
        }
        // do management services @todo pab: get rid of copy and paste patterns
        for (int ii = 0; ii<config.len("managementServices"); ++ii){
            wns::pyconfig::View managementServiceView = config.get("managementServices",ii);
            std::string serviceName = managementServiceView.get<std::string>("serviceName");
            std::string creatorName = managementServiceView.get<std::string>("__plugin__");
            wns::ldk::ManagementServiceCreator* serviceCreator = wns::ldk::ManagementServiceFactory::creator(creatorName);
            wns::ldk::ManagementServiceInterface* service = serviceCreator->create(getMSR(), managementServiceView);
            addManagementService(serviceName, service);
            LOG_INFO(" Registered Management Service: ", serviceName);
        }
    }

    getNode()->getContextProviderCollection().
        addProvider(wns::probe::bus::contextprovider::Callback
                    ("MAC.CellId", boost::bind(&wimac::Component::getCellID, this ) ) );
    getNode()->getContextProviderCollection().
        addProvider(wns::probe::bus::contextprovider::Callback
                    ("MAC.Id", boost::bind(&wimac::Component::getID, this ) ) );
    getNode()->getContextProviderCollection().
        addProvider(wns::probe::bus::contextprovider::Callback
                    ("MAC.Ring", boost::bind(&wimac::Component::getRing, this ) ) );
    getNode()->getContextProviderCollection().
        addProvider(wns::probe::bus::contextprovider::Callback
                    ("MAC.StationType", boost::bind(&wimac::Component::getStationType, this ) ) );
    getNode()->getContextProviderCollection().addProvider(
        wimac::helper::contextprovider::SourceAddress(fun_, 
            config.get<std::string>("upperConvergenceName")));    
    getNode()->getContextProviderCollection().addProvider(
        wimac::helper::contextprovider::TargetAddress(fun_, 
            config.get<std::string>("upperConvergenceName")));
    getNode()->getContextProviderCollection().addProvider(
        wimac::helper::contextprovider::CallbackCommandContextProvider(
            fun_,
            config.get<std::string>("upperConvergenceName"),
            "MAC.CellId",
            &wimac::Component::getCellID));
    getNode()->getContextProviderCollection().addProvider(
        wimac::helper::contextprovider::CallbackCommandContextProvider(
            fun_,
            config.get<std::string>("upperConvergenceName"),
            "MAC.Id",
            &wimac::Component::getID));
    getNode()->getContextProviderCollection().addProvider(
        wimac::helper::contextprovider::CallbackCommandContextProvider(
            fun_,
            config.get<std::string>("upperConvergenceName"),
            "MAC.Ring",
            &wimac::Component::getRing));
    getNode()->getContextProviderCollection().addProvider(
        wimac::helper::contextprovider::CallbackCommandContextProvider(
            fun_,
            config.get<std::string>("upperConvergenceName"),
            "MAC.StationType",
            &wimac::Component::getStationType));

    // global station registry
    TheStationManager::getInstance()->registerStation(id_, address_, this);

    if(getStationType() == wns::service::dll::StationTypes::AP())
        ring_ = config.get<unsigned int>("ring");

}

void
Component::doStartup()
{
}

unsigned int
Component::getRing() const
{
    if ( getStationType() == wns::service::dll::StationTypes::AP() )
    {
        return ring_;
    } 
    else
    {
        ConnectionIdentifier::Ptr ci;

        ci = getManagementService<service::ConnectionManager>
            ("connectionManager")->getConnectionWithID(0);

        if( ci )
        {
            Component* associatedWith = dynamic_cast<wimac::Component*>
                ( TheStationManager::getInstance()->getStationByID(ci->baseStation_) );
            assure(associatedWith, "Station is not associated with a WiMAC station");

            return associatedWith->getRing() + 1;
        }
        return 0;
    }
}

unsigned int
Component::getCellID() const
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
                ( TheStationManager::getInstance()->getStationByID(ci->baseStation_) );
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
     std::string transServiceName = getConfig().get<std::string>("phyDataTransmission");
     std::string notifyServiceName = getConfig().get<std::string>("phyNotification");

    // set services in PhyUser to communicate with lower layer
    PhyUser* phyUser = getFUN()->findFriend<PhyUser*>("phyUser");
    phyUser->setMACAddress( getMACAddress() );

    if(!getConfig().isNone("upperConvergenceName"))
    {
        std::string upperConvergenceName = getConfig().get<std::string>("upperConvergenceName");
        UpperConvergence* upperConvergence =
            getFUN()->findFriend<UpperConvergence*>(upperConvergenceName);

        // register UpperConvergence as the DLL DataTransmissionService
        addService(getConfig().get<std::string>("dataTransmission"), upperConvergence);
        addService(getConfig().get<std::string>("notification"), upperConvergence);
        addService(getConfig().get<std::string>("flowEstablishmentAndRelease"), upperConvergence);
        upperConvergence->setMACAddress(address_);
    }

	phyUser->setDataTransmissionService(
			getService<wns::service::phy::ofdma::DataTransmission*>( transServiceName ) );
	phyUser->setNotificationService(
			getService<wns::service::phy::ofdma::Notification*>( notifyServiceName ) );

    getFUN()->onFUNCreated();
    getMSR()->onMSRCreated();
    getCSR()->onCSRCreated();

    // Start the Framebuilder and set it to pause state for synchronizing the
    // periodically event of all stations. 
    wns::ldk::fcf::FrameBuilder* frameBuilder =
        getFUN()->findFriend<wns::ldk::fcf::FrameBuilder*>("frameBuilder");
    frameBuilder->start();
}

void
Component::onWorldCreated()
{
}

void
Component::onShutdown()
{
}

int
Component::getNumberOfQueuedPDUs(ConnectionIdentifiers cis)
{
    int queuedPDUs = 0;
    for(ConnectionIdentifiers::const_iterator conn = cis.begin();
        conn != cis.end();
        ++conn)
    {
        assure((*conn)->direction_ != ConnectionIdentifier::Downlink,
               "Component::getNumberOfQueuedPDUs(...) works for uplink PDUs only");
        wns::ldk::FlowSeparator* bufferSep =
            getFUN()->findFriend<wns::ldk::FlowSeparator*>("bufferSep");
        wns::ldk::ConstKeyPtr key(new ConnectionKey((*conn)->cid_));
        wns::ldk::Group* group =
            dynamic_cast<wns::ldk::Group*>(bufferSep->getInstance(key));
        if(group)
        {
            wns::ldk::buffer::Buffer* buffer =
                group->getSubFUN()->findFriend<wns::ldk::buffer::Buffer*>("buffer");
            assure(buffer, "Cannot find buffer in subFUN");
            queuedPDUs += buffer->getSize();
        }
    }
    if (getFUN()->knowsFunctionalUnit("upRelayInject"))
    {
        wns::ldk::buffer::Buffer* relayInject =
            getFUN()->findFriend<wns::ldk::buffer::Buffer*>("upRelayInject");
        queuedPDUs += relayInject->getSize();
        LOG_INFO("Added ", relayInject->getSize(),
                 " PDUs to the number of total PDUs waiting for beeing transmitted");
    }
    LOG_INFO( getName(), " with station ID ", id_,
              " and station type ", wns::service::dll::StationTypes::toString(stationType_),
              " has queued PDUS ", queuedPDUs );
    return queuedPDUs;
}

void
Component::doVisit(wns::probe::bus::IContext& context) const
{
    contextProviders_.fillContext(context);
}

wns::ldk::fun::Main*
Component::getFUN()
{
    return fun_;
}

std::string
Component::getName() const
{
    return getNode()->getName();
}
