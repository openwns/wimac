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

#include <WIMAC/ProbeStartStop.hpp>

#include <WNS/ldk/Layer.hpp>

#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/utils.hpp>

#include <iostream>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(ProbeStartStop,
				     wns::ldk::FunctionalUnit,
				     "wimac.ProbeStartStop",
				     wns::ldk::FUNConfigCreator);

ProbeStartStop::ProbeStartStop(wns::ldk::fun::FUN* fun,
			       const wns::pyconfig::View& config) :
	Probe(),
	wns::ldk::CommandTypeSpecifier<ProbeStartStopCommand>(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<>(),
	wns::ldk::Forwarding<ProbeStartStop>(),
	wns::Cloneable<ProbeStartStop>(),
	ProbeStartStopInterface(),
	EventObserver(config.get<std::string>("name")),

	probing_(false),

	cumulatedIncomingBits_(0.0),
	cumulatedIncomingCompounds_(0.0),
	cumulatedOutgoingBits_(0.0),
	cumulatedOutgoingCompounds_(0.0),

	logger_("WIMAC", config.get<std::string>("name")),
	probe_(),
	friends_(),
	eventReset_(config.get<std::string>("eventReset")),
	eventStart_(config.get<std::string>("eventStart")),
	eventStop_(config.get<std::string>("eventStop"))

{
    wns::probe::bus::ContextProviderCollection& cpc =
        fun->getLayer()->getContextProviderCollection();

	probe_.incomingBits = wns::probe::bus::collector(
		cpc,
		config,
		"probeIncomingBitsName");
	probe_.incomingCompounds = wns::probe::bus::collector(
		cpc,
		config,
		"probeIncominCompoundsName");
	probe_.outgoingBits = wns::probe::bus::collector(
		cpc,
		config,
		"probeOutgoingBitsName");
	probe_.outgoingCompounds = wns::probe::bus::collector(
		cpc,
		config,
		"probeOutgoingCompoundsName");
	probe_.aggregatedBits = wns::probe::bus::collector(
		cpc,
		config,
		"probeAggregatedBitsName");
	probe_.aggregatedCompounds = wns::probe::bus::collector(
		cpc,
		config,
		"probeAggregatedCompoundsName");

	if(!config.isNone("eventStartStopSubjectName"))
	{
		friends_.eventStartStopSubjectName
			= config.get<std::string>("eventStartStopSubjectName");
		friends_.eventStartStopSubjectType
			= config.get<std::string>("eventStartStopSubjectType");
	} else
	{
		friends_.eventStartStopSubjectName = "";
		friends_.eventStartStopSubjectType = "";
	}
	friends_.eventStartStopSubject = NULL;

	this->reset();
}



ProbeStartStop::~ProbeStartStop()
{
	if(this->getEventSubject())
		friends_.eventStartStopSubject->detachObserver(this);
}



void
ProbeStartStop::processOutgoing(const wns::ldk::CompoundPtr& compound)
{
	if(!probing_)   // Probing is disabled
		return;

	// Add command for aggregated probe
	ProbeStartStopCommand* pssCommand;
	pssCommand = activateCommand(compound->getCommandPool());
        pssCommand->magic.probingFU = this;

	Bit commandPoolSize;
	Bit dataSize;
	this->calculateSizes(compound->getCommandPool(),
			     commandPoolSize,
			     dataSize);
	const int32_t compoundSize = commandPoolSize + dataSize;

	MESSAGE_SINGLE(NORMAL, logger_,
		       getFUN()->getName()
		       << ":  outgoing"
		       << " compounds size " << compoundSize);

	cumulatedOutgoingBits_ += compoundSize;
	cumulatedOutgoingCompounds_ += 1;

	wns::ldk::Forwarding<ProbeStartStop>::processOutgoing(compound);
}



void
ProbeStartStop::processIncoming(const wns::ldk::CompoundPtr& compound)
{
	if(!probing_)   // Probing is disabled
		return;


	Bit commandPoolSize;
	Bit dataSize;
	this->calculateSizes(compound->getCommandPool(),
			     commandPoolSize,
			     dataSize);
	const int32_t compoundSize = commandPoolSize + dataSize;

	MESSAGE_SINGLE(NORMAL, logger_,
		       getFUN()->getName()
		       << ": incoming"
		       << " compounds size = " << compoundSize);

	cumulatedIncomingBits_ += compoundSize;
	cumulatedIncomingCompounds_ += 1;

	// put aggregated
	ProbeStartStopCommand* pssCommand = getCommand(compound->getCommandPool());
        pssCommand->magic.probingFU->cumulatedAggregatedBits_ += compoundSize;
	pssCommand->magic.probingFU->cumulatedAggregatedCompounds_ += 1;


	wns::ldk::Forwarding<ProbeStartStop>::processIncoming(compound);
}



void
ProbeStartStop::event(const std::string event)
{
	MESSAGE_SINGLE(NORMAL, logger_,
		       getFUN()->getName() << ": receiving event: " << event);


	if(event == eventReset_)
		this->reset();
	else if(event == eventStart_)
		this->start();
	else if(event == eventStop_)
		this->stop();
	else
	{	std::stringstream ss;
		ss << "ProbeStartStop::event: Get unknown event: " << event;
		throw wns::Exception(ss.str());
	}
}



void
ProbeStartStop::onFUNCreated()
{
	if(!(friends_.eventStartStopSubjectName == ""))
	{
		if(friends_.eventStartStopSubjectType == "functionalUnit")
			friends_.eventStartStopSubject = getFUN()
				->findFriend<EventSubject*>
				(friends_.eventStartStopSubjectName);
		else if(friends_.eventStartStopSubjectType == "managementService")
			friends_.eventStartStopSubject
				= getFUN()->getLayer()
				->getManagementService<EventSubject>
				(friends_.eventStartStopSubjectName);
		else if(friends_.eventStartStopSubjectType == "controlService")
			friends_.eventStartStopSubject
				= getFUN()->getLayer()
				->getControlService<EventSubject>
				(friends_.eventStartStopSubjectName);
		else
			assure(0, "ProbeStartStop::onFUNCreated: Unkown eventStartStopSubjectType!");

		friends_.eventStartStopSubject->attachObserver(this);
	}
}



/*************** privat functions **********************************************/
void
ProbeStartStop::reset()
{
	MESSAGE_SINGLE(NORMAL,logger_, getFUN()->getName() << ": Stop Probing without putting and reset the measured values!");

	probing_ = false;

	cumulatedIncomingBits_ = 0.0;
	cumulatedIncomingCompounds_ = 0.0;
	cumulatedOutgoingBits_ = 0.0;
	cumulatedOutgoingCompounds_ = 0.0;
	cumulatedAggregatedBits_ = 0.0;
	cumulatedAggregatedCompounds_ = 0.0;
}



void
ProbeStartStop::start()
{
	MESSAGE_SINGLE(NORMAL,logger_, getFUN()->getName() << ": Start Probing!");

	probing_ = true;

	cumulatedIncomingBits_ = 0.0;
	cumulatedIncomingCompounds_ = 0.0;
	cumulatedOutgoingBits_ = 0.0;
	cumulatedOutgoingCompounds_ = 0.0;
	cumulatedAggregatedBits_ = 0.0;
	cumulatedAggregatedCompounds_ = 0.0;
}



void
ProbeStartStop::stop()
{
	MESSAGE_SINGLE(NORMAL,logger_, getFUN()->getName() << ": Stop Probing!");

	probing_ = false;

	{
		probe_.incomingBits->put(cumulatedIncomingBits_);
		MESSAGE_SINGLE(NORMAL,logger_,
			       getFUN()->getName()
			       << ": Put to Probe: IncomingCompounds = "
			       << cumulatedIncomingBits_);
	}

	{
		probe_.incomingCompounds->put(cumulatedIncomingCompounds_);
		MESSAGE_SINGLE(NORMAL,logger_,
			       getFUN()->getName()
			       << ": Put to Probe: IncomingCompounds = "
			       << cumulatedIncomingCompounds_);
	}

	{
		probe_.outgoingBits->put(cumulatedOutgoingBits_);
		MESSAGE_SINGLE(NORMAL,logger_,
			       getFUN()->getName()
			       << ": Put to Probe: OutgoingBits =  "
			       << cumulatedOutgoingBits_);
	}

	{
		probe_.outgoingCompounds->put(cumulatedOutgoingCompounds_);
		MESSAGE_SINGLE(NORMAL,logger_,
			       getFUN()->getName()
			       << ": Put to Probe: OutgoingCompounds = "
			       << cumulatedOutgoingCompounds_);
	}

	{
		probe_.aggregatedBits->put(cumulatedAggregatedBits_);
		MESSAGE_SINGLE(NORMAL,logger_,
			       getFUN()->getName()
			       << ": Put to Probe: AggregatedBits = "
			       << cumulatedAggregatedBits_);
	}

	{
		probe_.aggregatedCompounds->put(cumulatedAggregatedCompounds_);
		MESSAGE_SINGLE(NORMAL,logger_,
			       getFUN()->getName()
			       << ": Put to Probe: AggregatedCompounds = "
			       << cumulatedAggregatedCompounds_);
	}

	cumulatedIncomingBits_ = 0.0;
	cumulatedIncomingCompounds_ = 0.0;
	cumulatedOutgoingBits_ = 0.0;
	cumulatedOutgoingCompounds_ = 0.0;
	cumulatedAggregatedBits_ = 0.0;
	cumulatedAggregatedCompounds_ = 0.0;
}
