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

#include <WIMAC/scheduler/ULCallback.hpp>
#include <WNS/ldk/Layer.hpp>
#include <WNS/scheduler/RegistryProxyInterface.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/Logger.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/PhyUserCommand.hpp>
#include <WIMAC/PhyUser.hpp>


STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::scheduler::ULCallback,
	wimac::scheduler::Callback,
	"wimac.scheduler.ULCallback",
	wns::ldk::FUNConfigCreator );

using namespace wimac::scheduler;

ULCallback::ULCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& /*config*/) :
	Callback(fun),
	fun_(fun)
{}

void
ULCallback::callBack(wns::scheduler::MapInfoEntryPtr mapInfoEntry)
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
  simTimeType pduPointer = 0.0;

  // iterate over all compounds in list:
  for (wns::scheduler::CompoundList::iterator iter=compounds.begin(); iter!=compounds.end(); ++iter)
  {
	wns::ldk::CompoundPtr pdu = *iter;
    simTimeType pduDuration = pdu->getLengthInBits() / rate;;


	assure(pdu != wns::ldk::CompoundPtr(), "Invalid PDU");

#ifndef WNS_NO_LOGGING
	std::stringstream m;
	m <<     ":  direction: UL \n"
	  << "        PDU scheduled for user: " << colleagues.registry->getNameForUser(user) << "\n"
	  << "        Frequency Slot: " << fSlot << "\n"
	  << "        StartTime:      " << pduPointer << "\n"
	  << "        EndTime:        " << pduPointer + pduDuration << "\n"
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


	//only in beamforming case receive pattern need to be set
	PatternSetterPhyAccessFunc* patternFunc =
		new PatternSetterPhyAccessFunc;
	patternFunc->destination_ = user;
	patternFunc->patternStart_ = pduPointer;
	patternFunc->patternEnd_ = pduPointer + pduDuration;
	patternFunc->pattern_ = pattern;

	// set PhyUser command
	wimac::PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>(
		fun_->getProxy()->activateCommand( pdu->getCommandPool(), friends_.phyUser ) );

	phyCommand->local.pAFunc_.reset( patternFunc );

	phyCommand->peer.destination_ = user;
	wimac::Component* wimacComponent = dynamic_cast<wimac::Component*>(fun_->getLayer());
	phyCommand->peer.cellID_ = wimacComponent->getCellID();
	phyCommand->peer.source_ = wimacComponent->getNode();
	phyCommand->peer.phyModePtr = phyModePtr;
	//	(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
	//	 (dynamic_cast<const wns::service::phy::phymode::PhyModeInterface*>(phyMode.clone())));

	phyCommand->peer.measureInterference_ = true; //measureInterference;
	phyCommand->peer.estimatedCandI_ = estimatedCandI;
	phyCommand->magic.sourceComponent_ = wimacComponent;

	/// @todo enable pduWatch
	//this->pduWatch(pdu);  // Watch for special compounds to inform its observer

	scheduledPDUs.push(pdu);
  
    pduPointer += pduDuration;
  }
}

void ULCallback::deliverNow(wns::ldk::Connector* connector)
{

	wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();

	//if(beamforming)
	//{
		while( !scheduledPDUs.empty())
		{
			wns::ldk::CompoundPtr compound =
				scheduledPDUs.front();
			PhyUserCommand* phyUserCommand =
				friends_.phyUser->getCommand( compound->getCommandPool() );

			PatternSetterPhyAccessFunc* func =
				dynamic_cast<PatternSetterPhyAccessFunc*>(phyUserCommand->local.pAFunc_.get());

			func->patternStart_ += now;
			func->patternEnd_ += now;

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
		//}
}



