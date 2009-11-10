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


#include <WIMAC/scheduler/PriorityScheduler.hpp>
#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/parameter/PHY.hpp>
#include <WNS/scheduler/queue/QueueInterface.hpp>
#include <WNS/scheduler/strategy/StrategyInterface.hpp>
#include <sstream>
#include <iostream>
#include <fstream>

#include <WIMAC/scheduler/Callback.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::scheduler::PriorityScheduler,
	wimac::scheduler::Interface,
	"wimac.scheduler.PriorityScheduler",
        wimac::FUConfigCreator );

using namespace wimac::scheduler;

PriorityScheduler::PriorityScheduler(wns::ldk::FunctionalUnit* parent, const wns::pyconfig::View& config)
	: Scheduler(parent, config)
{
}

// void
// PriorityScheduler::callBack( unsigned int fSlot, wns::simulator::Time startTime, wns::simulator::Time endTime, wns::scheduler::UserID user,
// 							 const wns::ldk::CompoundPtr& pdu, float cidColor, unsigned int beam,
// 							 wns::service::phy::ofdma::PatternPtr pattern, wns::scheduler::MapInfoEntryPtr burst,
// 							 wns::scheduler::PHYmode phyMode, bool measureInterference, wns::Power requestedTxPower,
// 							 wns::CandI estimatedCandI)
// {
// 	// here we have to handle the offset resulting from running the scheduling
// 	// in several phases

// 	startTime += offsetInSlot;
// 	endTime += offsetInSlot;

// 	assure(endTime <= this->getDuration(), "Timing inconsistency");
// 	assure(startTime < endTime, "Timing inconsistency");


// 	if (endTime > usedSlotDuration)
// 		usedSlotDuration = endTime;

// // the rest can be handled by the normal callBack function
// 	Scheduler::callBack(fSlot,
// 						startTime,
// 						endTime,
// 						user,
// 						pdu,
// 						cidColor,
// 						beam,
// 						pattern,
// 						burst,
// 						phyMode,
// 						measureInterference,
// 						requestedTxPower,
// 						estimatedCandI);
// }


void
//PriorityScheduler::startCollection(int)
PriorityScheduler::startScheduling()
{
	usedSlotDuration = 0.0;
	offsetInSlot = 0.0;

	if (plotFrames)
		setupPlotting();

	// in case I am an UL scheduler, wakeup my BW request generator
	//getReceptor()->wakeup();


	/****************** Broadcast Phase ****************************************/
	// if we have broadcast pdus in the queue, schedule them first;
	// usedSlotDuration gets adapted
	handleBroadcast();

	MESSAGE_BEGIN(NORMAL, logger, m, colleagues.registry->getNameForUser(colleagues.registry->getMyUserID()));
	m << " Used "
	  << offsetInSlot
	  << " of slot time for Broadcast phase";
	MESSAGE_END();

	// the following scheduling phases must not schedule into the already used
	// beginning of the slot; start with next OFDM symbol
	//offsetInSlot = ceil(usedSlotDuration / this->getFrameBuilder()->getSymbolDuration())*this->getFrameBuilder()->getSymbolDuration();
	offsetInSlot = ceil(usedSlotDuration / parameter::ThePHY::getInstance()->getSymbolDuration())* parameter::ThePHY::getInstance()->getSymbolDuration();
	usedSlotDuration = offsetInSlot;
	colleagues.callback->setOffset(offsetInSlot);

	if( (double(this->getDuration()) - double(usedSlotDuration)) < parameter::ThePHY::getInstance()->getSymbolDuration() )
		return; // No more space left for further scheduling


    /****************** 1 Phase ***********************************************/
	// first serve users with only signaling CIDs
	colleagues.registry->switchFilterTo(ConnectionIdentifier::Signaling);
	colleagues.strategy->startScheduling(freqChannels, maxBeams,
										 double(this->getDuration()) - double(usedSlotDuration), colleagues.callback);

	MESSAGE_BEGIN(NORMAL, logger, m, colleagues.registry->getNameForUser(colleagues.registry->getMyUserID()));
	m << " Used "
	  << offsetInSlot
	  << " of slot time for Broadcast+Signaling phases";
	MESSAGE_END();

	// the following scheduling phases must not schedule into the already used
	// beginning of the slot; start with next OFDM symbol
	offsetInSlot = ceil(usedSlotDuration / parameter::ThePHY::getInstance()->getSymbolDuration())*parameter::ThePHY::getInstance()->getSymbolDuration();
	usedSlotDuration = offsetInSlot;
	colleagues.callback->setOffset(offsetInSlot);

	if( (double(this->getDuration()) - double(usedSlotDuration)) < parameter::ThePHY::getInstance()->getSymbolDuration() )
		return; // No more space left for further scheduling


    /****************** 2 Phase ***********************************************/
	// first serve the priority users
	colleagues.registry->switchFilterTo(ConnectionIdentifier::rtPS);
	colleagues.strategy->startScheduling(freqChannels, maxBeams,
										 double(this->getDuration()) - double(usedSlotDuration), colleagues.callback);

	MESSAGE_BEGIN(NORMAL, logger, m, colleagues.registry->getNameForUser(colleagues.registry->getMyUserID()));
	m << " Used "
	  << offsetInSlot
	  << " of slot time for Broadcast+Signaling+Priority phases";
	MESSAGE_END();

	// the following scheduling phases must not schedule into the already used
	// beginning of the slot; start with next OFDM symbol
	offsetInSlot = ceil(usedSlotDuration / parameter::ThePHY::getInstance()->getSymbolDuration())*parameter::ThePHY::getInstance()->getSymbolDuration();
	usedSlotDuration = offsetInSlot;
	colleagues.callback->setOffset(offsetInSlot);

	if( (double(this->getDuration()) - double(usedSlotDuration)) < parameter::ThePHY::getInstance()->getSymbolDuration() )
		return; // No more space left for further scheduling


    /****************** 3 Phase ***********************************************/
    // afterwards, serve the rest
	colleagues.registry->switchFilterTo(ConnectionIdentifier::BE);
	colleagues.strategy->startScheduling(freqChannels, maxBeams,
										 double(this->getDuration()) - double(usedSlotDuration), colleagues.callback);

	MESSAGE_BEGIN(NORMAL, logger, m, colleagues.registry->getNameForUser(colleagues.registry->getMyUserID()));
	m << " Used "
	  << offsetInSlot
	  << " of slot time for Broadcast+Signaling+Priority+BestEffort phases";
	MESSAGE_END();


}




