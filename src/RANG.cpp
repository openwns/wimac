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
#include <WIMAC/RANG.hpp>

#include <WNS/node/component/FQSN.hpp>
#include <WNS/module/Base.hpp>

#include <WIMAC/UpperConvergence.hpp>

#include <sstream>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(RANG,
                                     wns::node::component::Interface,
                                     "wimac.RANG",
                                     wns::node::component::ConfigCreator);

RANG::RANG(wns::node::Interface* node, const wns::pyconfig::View& _config) :
    wns::node::component::Component(node, _config),
    config(_config),
    accessPointLookup(),
    logger(config.get("logger"))
{
}

void
RANG::doStartup()
{
    addService(config.get<std::string>("dataTransmission"), this);
    addService(config.get<std::string>("notification"), this);
}

void
RANG::onData(const wns::SmartPtr<wns::osi::PDU>& _data,
		wns::service::dll::FlowID)//****
{
    assure(dataHandlerRegistry.knows(wns::service::dll::protocolNumberOf(_data)), "no data handler set");
    dataHandlerRegistry.find(wns::service::dll::protocolNumberOf(_data))->onData(_data);
}

void
RANG::onData(const wns::SmartPtr<wns::osi::PDU>& _data,
             wns::service::dll::UnicastAddress _sourceMACAddress,
             wns::service::dll::UnicastDataTransmission* _ap,
             wns::service::dll::FlowID _dllFlowID)//****
{
    updateAPLookUp(_sourceMACAddress, _ap);

    MESSAGE_BEGIN(NORMAL, logger, m,"Receiving incoming data from MAC Address: ");
    m << _sourceMACAddress;
    MESSAGE_END();

    assure(dataHandlerRegistry.knows(wns::service::dll::protocolNumberOf(_data)), "no data handler set");
    dataHandlerRegistry.find(wns::service::dll::protocolNumberOf(_data))->onData(_data);
}

void
RANG::sendData(
    const wns::service::dll::UnicastAddress& _peer,
    const wns::SmartPtr<wns::osi::PDU>& pdu,
    wns::service::dll::protocolNumber protocol,
    wns::service::dll::FlowID _dllFlowID)//****
{
    wns::service::dll::UnicastDataTransmission* ap = NULL;
    if( knowsAddress(_peer) )
    {
        ap = getAccessPointFor(_peer);
    } else
    {
        MESSAGE_BEGIN(NORMAL, logger, m, "No valid access point found. Dropping packet to ");
        m << _peer;
        MESSAGE_END();
        return;
    }
    MESSAGE_BEGIN(NORMAL, logger, m, getName());
    m << ": doSendData(), RANG forwarding to convergence::Upper\n";
    m << "target is ";
    m << _peer;
    MESSAGE_END();

    ap->sendData(_peer, pdu, protocol);
}

void
RANG::registerHandler(wns::service::dll::protocolNumber protocol, wns::service::dll::Handler* dh)
{
    assure(dh, "no data handler set");
    assure(!dataHandlerRegistry.knows(protocol), "data handler already registered");
    dataHandlerRegistry.insert(protocol, dh);
}

void
RANG::onNodeCreated()
{
}


void
RANG::onWorldCreated()
{
    int numAPs = config.len("dllDataTransmissions");
    assure( numAPs == config.len("dllNotifications"),
            "mismatch in number of DataTransmission / Notification services");
    // browse through the list of connected APs
    for (int i=0; i<numAPs; ++i)
    {
        wns::node::component::FQSN dataTransmission(config.get<wns::pyconfig::View>("dllDataTransmissions",i));
        wns::node::component::FQSN notification(config.get<wns::pyconfig::View>("dllNotifications",i));

        UpperConvergence* dataTransmissionService =
            getRemoteService<UpperConvergence*>(dataTransmission);
        UpperConvergence* notificationService =
            getRemoteService<UpperConvergence*>(notification);

        wns::service::dll::UnicastAddress macAdr = dataTransmissionService->getMACAddress();

        notificationService->registerHandler(wns::service::dll::TUNNEL, this);
        accessPointLookup.insert(macAdr, dataTransmissionService);

        MESSAGE_BEGIN(NORMAL, logger, m, "Added AP");
        m << i+1 << " with MAC Adr.: " << macAdr << " to RANG!";
        MESSAGE_END();
    }
}

bool
RANG::knowsAddress(wns::service::dll::UnicastAddress _sourceMACAddress)
{
    return accessPointLookup.knows(_sourceMACAddress);
}

wns::service::dll::UnicastDataTransmission*
RANG::getAccessPointFor(wns::service::dll::UnicastAddress _sourceMACAddress)
{
    assure(knowsAddress(_sourceMACAddress), "No valid access point found.");
    return accessPointLookup.find(_sourceMACAddress);
}

// Modified Handler Interface for APs
void
RANG::updateAPLookUp( wns::service::dll::UnicastAddress _sourceMACAddress,
                      wns::service::dll::UnicastDataTransmission* _ap)
{
    // keep the MAC Address table up to date
    if (knowsAddress(_sourceMACAddress))
        accessPointLookup.erase(_sourceMACAddress);

    accessPointLookup.insert(_sourceMACAddress, _ap);

    MESSAGE_BEGIN(NORMAL, logger, m, "updated APLookup for UT with MAC Adr.: ");
    m << _sourceMACAddress << " (AP with MAC Adr.: "
      << dynamic_cast<wimac::UpperConvergence*>(_ap)->getMACAddress() << ").";
    MESSAGE_END();
}

void
RANG::removeAddress(wns::service::dll::UnicastAddress _sourceMACAddress,
                    wns::service::dll::UnicastDataTransmission* _ap)
{
    if (knowsAddress(_sourceMACAddress))
    {
        if (getAccessPointFor(_sourceMACAddress) == _ap)
        {
            accessPointLookup.erase(_sourceMACAddress);

            MESSAGE_BEGIN(NORMAL, logger, m,"removed UT with MAC Adr.: ");
            m << _sourceMACAddress << " from APLookup.";
            MESSAGE_END();
        }
        else
        {
            throw wns::Exception("AP unrelated to UT tried to remove its routing entry!");
        }
    }
    else
        throw wns::Exception("Tried to remove unknown UT address from RANG APlookup table!");
}

void
RANG::onShutdown()
{
}

