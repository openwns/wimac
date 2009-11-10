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

#include <WNS/service/nl/Address.hpp>
#include <WNS/service/dll/ProtocolNumber.hpp>
#include <WNS/ReferenceModifier.hpp>
#include <WNS/ReferenceModifier.hpp>


#include <WIMAC/UpperConvergence.hpp>
#include <WIMAC/RANG.hpp>
#include <WIMAC/Logger.hpp>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    UpperConvergence,
    wns::ldk::FunctionalUnit,
    "wimac.UpperConvergence",
    wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    NoUpperConvergence,
    wns::ldk::FunctionalUnit,
    "wimac.NoUpperConvergence",
    wns::ldk::FUNConfigCreator);


UpperConvergence::UpperConvergence(wns::ldk::fun::FUN* fun, const wns::pyconfig::View&) :
    wns::ldk::CommandTypeSpecifier<UpperCommand>(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<wns::ldk::FirstServeConnector>(),
    wns::ldk::HasDeliverer<>(),
    sourceMACAddress_(),
    dataHandler_(NULL),
    rang_(NULL)
{}

void
UpperConvergence::onFUNCreated()
{
}


//*************
/*
void 
wns::service::dll::DataTransmission<Address>::sendData(
	const Address&,
	const wns::osi::PDUPtr&,
	wns::service::dll::protocolNumber,
	wns::service::dll::FlowID)

	 [with Address = wns::service::dll::UnicastAddress]
*/


void
UpperConvergence::sendData(
    const wns::service::dll::UnicastAddress& peer,
    const wns::SmartPtr<wns::osi::PDU>& pdu,
    wns::service::dll::protocolNumber protocol,
    wns::service::dll::FlowID _dllFlowID)
{
    pdu->setPDUType(protocol);

    LOG_INFO(getFUN()->getName(),
             ": doSendData() called in convergence::Upper, target DLLAddress: " ,
             peer);

    wns::ldk::CompoundPtr compound(new wns::ldk::Compound(getFUN()->createCommandPool(), pdu));

    activateCommand(compound->getCommandPool());

    UpperCommand* sgc = getCommand(compound->getCommandPool());

    sgc->peer.targetMACAddress = peer;
    sgc->peer.sourceMACAddress = sourceMACAddress_;
	sgc->local.dllFlowID = _dllFlowID;

    if(this->getConnector()->hasAcceptor(compound))
    {
        this->wns::ldk::FunctionalUnit::sendData(compound);
    }
    else
    {
        LOG_INFO("Dropped Outgoing Compound because DLL cannot handle it.");
    }
}

void
UpperConvergence::setMACAddress(const wns::service::dll::UnicastAddress& sourceMACAddress)
{
    sourceMACAddress_ = sourceMACAddress;
}

wns::service::dll::UnicastAddress
UpperConvergence::getMACAddress() const
{
    return sourceMACAddress_;
}

void
UpperConvergence::doOnData(const wns::ldk::CompoundPtr& compound)
{
    LOG_INFO( getFUN()->getName(),
              ": doOnData(), forwarding to upper Component (IP) ");

    if(dataHandler_ != NULL)
        dataHandler_->onData(compound->getData());
    else if (rang_ != NULL)
    {
        UpperCommand* myCommand = getCommand(compound->getCommandPool());
        rang_->onData(compound->getData(),
            myCommand->peer.sourceMACAddress,
            this);
    }
    else
        assure(false, "No data handler registered");   

    LOG_TRACE(getFUN()->getName(), ": Compound backtrace",
              compound->dumpJourney());
}

wns::ldk::CommandPool*
UpperConvergence::createReply(const wns::ldk::CommandPool* original) const
{
    wns::ldk::CommandPool* reply = getFUN()->createCommandPool();

    UpperCommand* originalCommand = getCommand(original);
    UpperCommand* replyCommand = activateCommand(reply);

    replyCommand->peer.sourceMACAddress = sourceMACAddress_;
    replyCommand->peer.targetMACAddress =
        originalCommand->peer.sourceMACAddress;

    return reply;
}


bool
UpperConvergence::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
    return getConnector()->hasAcceptor(compound);
}

void
UpperConvergence::doSendData(const wns::ldk::CompoundPtr& compound)
{
    getConnector()->getAcceptor(compound)->sendData(compound);
}

void
UpperConvergence::doWakeup()
{
    getReceptor()->wakeup();
}

// UTUpperConvergence::UTUpperConvergence(wns::ldk::fun::FUN* fun,const wns::pyconfig::View& config) :
//     UpperConvergence(fun,config),
//     wns::ldk::Forwarding<UTUpperConvergence>(),
//     wns::Cloneable<UTUpperConvergence>()
// {}

// void
// UTUpperConvergence::processIncoming(const wns::ldk::CompoundPtr& compound)
// {
//     MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
//     m << ": doOnData(), forwarding to upper Component (IP) ";
//     MESSAGE_END();

//     dataHandlerRegistry.find(wns::service::dll::protocolNumberOf(compound->getData()))->onData(compound->getData());

//     MESSAGE_BEGIN(VERBOSE, logger, m, getFUN()->getName());
//     m << ": Compound backtrace"
//       << compound->dumpJourney(); // JOURNEY
//     MESSAGE_END();

// }

// void
// UTUpperConvergence::registerHandler(wns::service::dll::protocolNumber protocol,
//                                     wns::service::dll::Handler* dh)
// {
//     assureNotNull(dh);
//     dataHandlerRegistry.insert(protocol, dh);

//     MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
//     m << ": UTUpperConv registered dataHandler for protocol number " << protocol;
//     MESSAGE_END();
// }

void UpperConvergence::registerHandler(wns::service::dll::protocolNumber,
                                       wns::service::dll::Handler* handler)
{
    assureNotNull(handler);

    // Quick hack neded to call right function in onData
    rang_ = dynamic_cast<RANG*>(handler);
    if(rang_ == NULL)
        dataHandler_ = handler;

    LOG_INFO("Registering data handler");
}

// APUpperConvergence::APUpperConvergence(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
//     UpperConvergence(fun,config),
//     wns::ldk::Forwarding<APUpperConvergence>(),
//     wns::Cloneable<APUpperConvergence>(),
//     dataHandler(NULL)
// {}

// void
// APUpperConvergence::processIncoming(const wns::ldk::CompoundPtr& compound)
// {
//     MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
//     m << ": doOnData(), forwarding to RANG";
//     MESSAGE_END();
//     assure(dataHandler, "no data handler set");

//     // as opposed to the UT upper convergence, we have to tell the RANG who we
//     // are and where the Packet comes from.
//     UpperCommand* myCommand = getCommand(compound->getCommandPool());
//     dataHandler->onData(compound->getData(),
//                         myCommand->peer.sourceMACAddress,
//                         this);

//     MESSAGE_BEGIN(VERBOSE, logger, m, getFUN()->getName());
//     m << ": Compound backtrace"
//       << compound->dumpJourney(); // JOURNEY
//     MESSAGE_END();

// }

// void
// APUpperConvergence::registerHandler(wns::service::dll::protocolNumber /*protocol*/,
//                                     wns::service::dll::Handler* dh)
// {
//     // Upper layer protocol demultiplexing is done in RANG. Therefore we
//     // ignore the protocol field.
//     assure(dh, "APUpperConvergence failed to register wns::service::dll::Handler");
//     dataHandler = dh;

//     MESSAGE_BEGIN(NORMAL, logger, m, getFUN()->getName());
//     m << ": APUpperConv registered dataHandler";
//     MESSAGE_END();
// }

// bool
// APUpperConvergence::hasRANG()
// {
//     if (dataHandler)
//         return true;
//     return false;
// }

// wimac::RANG*
// APUpperConvergence::getRANG()
// {
//     assure(dataHandler, "RANG hasn't been set");
//     return dataHandler;
// }

