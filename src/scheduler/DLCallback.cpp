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
#include <WIMAC/scheduler/DLCallback.hpp>

#include <WNS/scheduler/RegistryProxyInterface.hpp>
#include <WNS/scheduler/MapInfoEntry.hpp>
#include <WNS/ldk/Layer.hpp>
#include <WIMAC/Logger.hpp>

#include <WIMAC/Utilities.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/PhyUserCommand.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::scheduler::DLCallback,
	wimac::scheduler::Callback,
	"wimac.scheduler.DLCallback",
	wns::ldk::FUNConfigCreator );

using namespace wimac::scheduler;

DLCallback::DLCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
	Callback(fun),
	fun_(fun),
	beamforming(config.get<bool>("beamforming"))
{}

void
DLCallback::callBack(wns::scheduler::MapInfoEntryPtr mapInfoEntry)
{
  simTimeType startTime = mapInfoEntry->start;
  simTimeType endTime = mapInfoEntry->end;
  wns::scheduler::UserID user = mapInfoEntry->user;
  int userID = user->getNodeID();
  int fSlot = mapInfoEntry->subBand;
  int beam = mapInfoEntry->beam;
  wns::Power txPower = mapInfoEntry->txPower;
  wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = mapInfoEntry->phyModePtr;
  wns::service::phy::ofdma::PatternPtr pattern = mapInfoEntry->pattern;
  wns::CandI estimatedCandI = mapInfoEntry->estimatedCandI;
  std::list<wns::ldk::CompoundPtr> compounds = mapInfoEntry->compounds;

  double rate = phyModePtr->getDataRate();

  assure(compounds.size() > 0, "Empty compound list");
  simTimeType pduEndTime = (*(compounds.begin()))->getLengthInBits() / rate;
  simTimeType pduStartTime = 0.0;

  // iterate over all compounds in list:
  for (wns::scheduler::CompoundList::iterator iter=compounds.begin(); iter!=compounds.end(); ++iter)
  {
    wns::ldk::CompoundPtr pdu = *iter;

  // TODO
	assure(pdu != wns::ldk::CompoundPtr(), "Invalid PDU");
// 	assure(beam < maxBeams, "Too many beams");
// 	assure(endTime > startTime, "Scheduled PDU must end after it starts");
// 	assure(endTime <= this->getDuration(), "PDU overun the maximum duration of the frame phase!");
// 	assure(fSlot < freqChannels, "Invalid frequency channel");

#ifndef WNS_NO_LOGGING
	std::stringstream m;
	m <<     ":  direction: DL \n"
	  << "        PDU scheduled for user: " << colleagues.registry->getNameForUser(user) << "\n"
	  << "        Frequency Slot: " << fSlot << "\n"
	  << "        StartTime:      " << pduStartTime << "\n"
	  << "        EndTime:        " << pduEndTime << "\n"
//	  << "        Beamforming:    " << beamforming << "\n"
	  << "        Beam:           " << beam << "\n"
	  << "        Tx Power:       " << txPower;
	LOG_INFO(fun_->getLayer()->getName(), m.str());
#endif

//	pduCount++;

	/// @todo enable frame plotting again
// 	if (plotFrames) {
// 		// substr(19,2) should deliver the "subscriber station xy"

// 		std::string printID;

// 		if (uplink || measureInterference)
// 			printID = colleagues.registry->getNameForUser(user);
// 		else
// 			printID = std::string("");
// 		*plotFiles[fSlot] << startTime << "\t" << endTime << "\t"
// 						  << float(beam) << "\t" << cidColor/2.0 << "\t\"" << printID << "\"\n";

// 	}

	// DEBUG: Don't use beamfoarming. For debuging
// 	if(!beamforming)
// 		pattern = wns::service::phy::ofdma::PatternPtr();

	PhyAccessFunc* func = 0;

	if(beamforming && (pattern != wns::service::phy::ofdma::PatternPtr()))
	{
		BeamformingPhyAccessFunc* sdmaFunc = new BeamformingPhyAccessFunc;
		sdmaFunc->destination_ = user;
		sdmaFunc->transmissionStart_ = pduStartTime;
		sdmaFunc->transmissionStop_ =
			pduEndTime - Utilities::getComputationalAccuracyFactor();
		sdmaFunc->subBand_ = fSlot;
		sdmaFunc->pattern_ = pattern;
		sdmaFunc->requestedTxPower_ = txPower;
		func = sdmaFunc;
	}
	else if(user != NULL)
	{
		OmniUnicastPhyAccessFunc* omniUnicastFunc = new OmniUnicastPhyAccessFunc;
		omniUnicastFunc->destination_ = user;
		omniUnicastFunc->transmissionStart_ = pduStartTime;
		omniUnicastFunc->transmissionStop_ =
			pduEndTime - Utilities::getComputationalAccuracyFactor();
		omniUnicastFunc->subBand_ = fSlot;
		func = omniUnicastFunc;
	}else
	{
		BroadcastPhyAccessFunc* broadcastFunc = new BroadcastPhyAccessFunc;
		broadcastFunc->transmissionStart_ = pduStartTime;
		broadcastFunc->transmissionStop_ =
			pduEndTime - Utilities::getComputationalAccuracyFactor();
		broadcastFunc->subBand_ = fSlot;
		func = broadcastFunc;
	}


	// set PhyUser command
	wimac::PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>(
		fun_->getProxy()->activateCommand( pdu->getCommandPool(), friends_.phyUser ) );

	phyCommand->local.pAFunc_.reset( func );

	//wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyModePtr
	//	(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
	//	 (dynamic_cast<const wns::service::phy::phymode::PhyModeInterface*>(phyMode.clone())));

	phyCommand->local.pAFunc_->phyMode_ = phyModePtr;


	phyCommand->peer.destination_ = user;
	wimac::Component* wimacComponent = dynamic_cast<wimac::Component*>(fun_->getLayer());
	phyCommand->peer.cellID_ = wimacComponent->getCellID();
	phyCommand->peer.source_ = wimacComponent->getNode();
	phyCommand->peer.phyModePtr = phyModePtr;
	phyCommand->peer.measureInterference_ = true; // measureInterference;
	phyCommand->peer.estimatedCandI_ = estimatedCandI;
	phyCommand->magic.sourceComponent_ = wimacComponent;

	/// @todo enable pduWatch again
	//this->pduWatch(pdu);  // Watch for special compounds to inform its observer

	scheduledPDUs.push(pdu);
    simTimeType oldEnd = pduEndTime;
    pduEndTime = pduEndTime +  pdu->getLengthInBits() / rate;
    pduStartTime = oldEnd;
  }
  
}

void DLCallback::deliverNow(wns::ldk::Connector* connector)
{
	simTimeType now = wns::simulator::getEventScheduler()->getTime();

	while (!scheduledPDUs.empty())
	{
		wns::ldk::CompoundPtr compound =
			scheduledPDUs.front();
		PhyUserCommand* phyUserCommand =
			friends_.phyUser->getCommand( compound->getCommandPool() );


		PhyAccessFunc* func =
			dynamic_cast<PhyAccessFunc*>(phyUserCommand->local.pAFunc_.get());

		func->transmissionStart_ += now;
		func->transmissionStop_ += now;

		if ( connector->hasAcceptor(scheduledPDUs.front() ) )
		{
			connector->getAcceptor(scheduledPDUs.front())->sendData(scheduledPDUs.front());
			scheduledPDUs.pop();
		}
		else
		{
			throw wns::Exception( "Lower FU is not accepting scheduled PDU but is supposed to do so" );
		}
	}
}



