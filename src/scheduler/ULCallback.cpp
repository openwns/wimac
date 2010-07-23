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
#include <WIMAC/Utilities.hpp>

#include <boost/bind.hpp> 

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::scheduler::ULMasterCallback,
    wimac::scheduler::Callback,
    "wimac.scheduler.ULMasterCallback",
    wns::ldk::FUNConfigCreator );

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::scheduler::ULSlaveCallback,
    wimac::scheduler::Callback,
    "wimac.scheduler.ULSlaveCallback",
    wns::ldk::FUNConfigCreator );


using namespace wimac::scheduler;
ULMasterCallback::ULMasterCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config):
    ULCallback(fun, config),
    beamforming(config.get<bool>("beamforming"))
{}

ULSlaveCallback::ULSlaveCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config):
ULCallback(fun, config)
{}

ULCallback::ULCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Callback(fun, config),
    fun_(fun),
    slotLength_(config.get<wns::simulator::Time>("slotLength"))
{}

void
ULCallback::callBack(wns::scheduler::SchedulingMapPtr schedulingMap)
{
    LOG_INFO(fun_->getLayer()->getName(), " ULCallback::callBack(): ");
    lastScheduling_ = wns::simulator::getEventScheduler()->getTime();

    //send only own bursts?
    for(wns::scheduler::SubChannelVector::iterator iterSubChannel = 
            schedulingMap->subChannels.begin();
        iterSubChannel != schedulingMap->subChannels.end(); 
        ++iterSubChannel)
    {
        wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
        for(wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = 
                subChannel.temporalResources.begin();
            iterTimeSlot != subChannel.temporalResources.end(); 
            ++iterTimeSlot)
        {
            wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;
            for(wns::scheduler::PhysicalResourceBlockVector::iterator iterPRB = 
                    timeSlotPtr->physicalResources.begin();
                iterPRB != timeSlotPtr->physicalResources.end(); 
                ++iterPRB)
            {
                if ( iterPRB->hasScheduledCompounds() )
                {
                    wns::scheduler::ScheduledCompoundsList::const_iterator it;
                    
                    for(it = iterPRB->scheduledCompoundsBegin();
                        it != iterPRB->scheduledCompoundsEnd();
                        it++)
                    { // for every compound in subchannel:
                        processPacket(*it);
                    } // for (all scheduledCompounds)
                    iterPRB->clearScheduledCompounds();
                } // if there were compounds in this resource
            } // forall beams
        } // end for ( timeSlots )
    } // forall subChannels
}

// ULMaster processPacket, seting receive beamforming pattern
void
ULMasterCallback::processPacket(const wns::scheduler::SchedulingCompound & compound)
{
    simTimeType startTime = compound.startTime;
    simTimeType endTime = compound.endTime;
    wns::scheduler::UserID user = compound.userID;
    int userID = user.getNodeID();
    int fSlot = compound.subChannel;
    int timeSlot = compound.timeSlot;
    int beam = compound.spatialLayer; //beam;
    wns::Power txPower = compound.txPower;
    wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = compound.phyModePtr;
    wns::service::phy::ofdma::PatternPtr pattern = compound.pattern;

    simTimeType timeSlotOffset = timeSlot * slotLength_;
    startTime += timeSlotOffset;
    endTime += timeSlotOffset - Utilities::getComputationalAccuracyFactor();

    if(scheduleStartProbe_->hasObservers())
    {
        // Probe userID for start and stop to get nice sample and hold curve
        wns::simulator::getEventScheduler()->schedule(
        boost::bind(&Callback::probeScheduleStart, this, timeSlot, fSlot, beam, userID), startTime);
    }
    if(scheduleStopProbe_->hasObservers())
    {
        wns::simulator::getEventScheduler()->schedule(
        boost::bind(&Callback::probeScheduleStop, this, timeSlot, fSlot, beam, userID), endTime);
    }

    /* @TODO: enable measuring of SINR estimation error*/
    wns::CandI estimatedCandI = wns::CandI();
    //wns::CandI estimatedCandI = compound.estimatedCandI;
    double rate = phyModePtr->getDataRate();

//  mapInfoEntry->start += timeSlot * slotLength_;
//  mapInfoEntry->end += timeSlot * slotLength_;

  //simTimeType pduPointer = mapInfoEntry->start;

  // iterate over all compounds in list:
  //for (wns::scheduler::CompoundList::iterator iter=compounds.begin(); iter!=compounds.end(); ++iter)
  //{
    wns::ldk::CompoundPtr pdu = compound.compoundPtr;
    simTimeType pduDuration = pdu->getLengthInBits() / rate;;

    assure(pdu != wns::ldk::CompoundPtr(), "Invalid PDU");

#ifndef WNS_NO_LOGGING
    std::stringstream m;
    m <<     ":  direction: UL master \n"
    << "        PDU scheduled for user (destination): " << colleagues.registry->getNameForUser(user) << "\n"
    << "        Frequency Slot: " << fSlot << "\n"
    << "        Time Slot: " << timeSlot << " slotLength: "<<slotLength_<< "\n"
    << "        StartTime:      " << startTime << "\n"
    << "        EndTime:        " << endTime << "\n"
    << "        Beamforming:    " << beamforming << "\n"
    << "        Beam:           " << beam << "\n"
    << "        Tx Power:       " << txPower;
    LOG_INFO(fun_->getLayer()->getName(), m.str());
#endif
//	pduCount++;


    //only in beamforming case receive pattern need to be set
    LOG_INFO(fun_->getLayer()->getName(), " DLCallback::processPacket() create PatternSetterPhyAccessFuncFunc");
    PatternSetterPhyAccessFunc* patternFunc =
        new PatternSetterPhyAccessFunc;
    patternFunc->destination_ = user.getNode();
    patternFunc->patternStart_ = startTime;
    patternFunc->patternEnd_ = endTime;
    patternFunc->pattern_ = pattern;

    // set PhyUser command
    wimac::PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>(
        fun_->getProxy()->activateCommand( pdu->getCommandPool(), friends_.phyUser ) );

    phyCommand->local.pAFunc_.reset( patternFunc );

    phyCommand->peer.destination_ = user.getNode();
    wimac::Component* wimacComponent = dynamic_cast<wimac::Component*>(fun_->getLayer());
    phyCommand->peer.cellID_ = wimacComponent->getCellID();
    phyCommand->peer.source_ = wimacComponent->getNode();
    phyCommand->peer.phyModePtr = phyModePtr;
    //	(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
    //	 (dynamic_cast<const wns::service::phy::phymode::PhyModeInterface*>(phyMode.clone())));

    phyCommand->peer.measureInterference_ = true; //measureInterference;
    phyCommand->peer.estimatedCandI_ = estimatedCandI;
    phyCommand->magic.sourceComponent_ = wimacComponent;
}

//ULSlave processPacket, setting omnidirectional transmit phy access functor
void
ULSlaveCallback::processPacket(const wns::scheduler::SchedulingCompound & compound)
{
    simTimeType startTime = compound.startTime;
    simTimeType endTime = compound.endTime;
    wns::scheduler::UserID user = compound.userID;
    int userID = user.getNodeID();
    int fSlot = compound.subChannel;
    int timeSlot = compound.timeSlot;
    int beam = compound.spatialLayer; //beam;
    wns::Power txPower = compound.txPower;
    wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = compound.phyModePtr;
    wns::service::phy::ofdma::PatternPtr pattern = compound.pattern;

    simTimeType timeSlotOffset = timeSlot * slotLength_;
    startTime += timeSlotOffset;
    endTime += timeSlotOffset - Utilities::getComputationalAccuracyFactor();
    //TODO:bmw
    //wns::CandI estimatedCandI = compound.estimatedCandI;
    wns::CandI estimatedCandI = wns::CandI();
    //pdu->startTime += timeSlot * slotLength_;
    //pdu->endTime += timeSlot * slotLength_;

    double rate = phyModePtr->getDataRate();
    wns::ldk::CompoundPtr pdu  = compound.compoundPtr;
    simTimeType pduDuration = pdu->getLengthInBits() / rate;
    // TODO
    assure(pdu != wns::ldk::CompoundPtr(), "Invalid empty PDU");
    //assure(beam < maxBeams, "Too many beams");
    //assure(endTime > startTime, "Scheduled PDU must end after it starts");
    //assure(endTime <= this->getDuration(), "PDU overun the maximum duration of the frame phase!");
    //assure(fSlot < freqChannels, "Invalid frequency channel");

#ifndef WNS_NO_LOGGING
    std::stringstream m;
    m <<     ":  direction: UL slave \n"
      << "        PDU scheduled for user (destination): " << colleagues.registry->getNameForUser(user) << "\n"
      << "        Frequency Slot: " << fSlot << "\n"
      << "        Time Slot: " << timeSlot << " slotLength: "<<slotLength_<< "\n"
      << "        StartTime:      " << startTime << "\n"
      << "        EndTime:        " << endTime<< "\n"
      //<< "        Beamforming:    " << beamforming << "\n"
      << "        Beam:           " << beam << "\n"
      << "        Tx Power:       " << txPower;
    LOG_INFO(fun_->getLayer()->getName(), m.str());
#endif

    PhyAccessFunc* func = 0;

    if(user.isValid())
    {
        LOG_INFO(fun_->getLayer()->getName(), " ULCallback::processPacket() create OmniUnicastPhyAccessFunc");
        OmniUnicastPhyAccessFunc* omniUnicastFunc = new OmniUnicastPhyAccessFunc;
        omniUnicastFunc->destination_ = user.getNode();
        omniUnicastFunc->transmissionStart_ = startTime;
        omniUnicastFunc->transmissionStop_ = endTime;
        omniUnicastFunc->subBand_ = fSlot;
        omniUnicastFunc->requestedTxPower_ = txPower;
        func = omniUnicastFunc;
    }else
    {
        throw wns::Exception( "destination address missing, UT is not supposed to broadcast" );
    }


    // set PhyUser command
    wimac::PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>(
        fun_->getProxy()->activateCommand( pdu->getCommandPool(), friends_.phyUser ) );

    phyCommand->local.pAFunc_.reset( func );

    phyCommand->local.pAFunc_->phyMode_ = phyModePtr;

    phyCommand->peer.destination_ = user.getNode();
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
}

void ULMasterCallback::deliverNow(wns::ldk::Connector* connector)
{
    assure(false, "ULMaster must not deliver packets");
}

void ULSlaveCallback::deliverNow(wns::ldk::Connector* connector)
{
    LOG_INFO(fun_->getLayer()->getName(), " ULSlaveCallback::deliverNow() ");
    wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();

    while (!scheduledPDUs.empty())
    {
        wns::ldk::CompoundPtr compound =
            scheduledPDUs.front();
        PhyUserCommand* phyUserCommand =
            friends_.phyUser->getCommand(compound->getCommandPool());

        PhyAccessFunc* func =
            dynamic_cast<PhyAccessFunc*>(phyUserCommand->local.pAFunc_.get());

        func->transmissionStart_ += now;
        func->transmissionStop_ += now;

        frameOffsetDelayProbe_->put(compound, func->transmissionStart_ - lastScheduling_);
        transmissionDelayProbe_->put(compound, func->transmissionStop_ - func->transmissionStart_);

        if(connector->hasAcceptor(scheduledPDUs.front()))
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
