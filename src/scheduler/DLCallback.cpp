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

#include <WIMAC/scheduler/DLCallback.hpp>

#include <WNS/scheduler/RegistryProxyInterface.hpp>
#include <WNS/scheduler/MapInfoEntry.hpp>
#include <WNS/scheduler/SchedulingMap.hpp>
#include <WNS/scheduler/strategy/Strategy.hpp>
#include <WNS/ldk/Layer.hpp>
//#include <WNS/ldk/CompoundPtr.hpp>

#include <WIMAC/Logger.hpp>

#include <WIMAC/Component.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/PhyUserCommand.hpp>

#include <boost/bind.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
				     wimac::scheduler::DLCallback,
				     wimac::scheduler::Callback,
				     "wimac.scheduler.DLCallback",
				     wns::ldk::FUNConfigCreator );

using namespace wimac::scheduler;

DLCallback::DLCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Callback(fun, config),
    fun_(fun),
    beamforming(config.get<bool>("beamforming")),
    slotLength_(config.get<wns::simulator::Time>("slotLength"))
{}

void DLCallback::deliverNow(wns::ldk::Connector* connector)
{
    LOG_INFO(fun_->getLayer()->getName(), " DLCallback::deliverNow() ");
    wns::simulator::Time now = wns::simulator::getEventScheduler()->getTime();

    while (!scheduledPDUs.empty())
    {
        wns::ldk::CompoundPtr compound = scheduledPDUs.front();
        PhyUserCommand* phyUserCommand =
            friends_.phyUser->getCommand( compound->getCommandPool() );


        PhyAccessFunc* func =
            dynamic_cast<PhyAccessFunc*>(phyUserCommand->local.pAFunc_.get());

        func->transmissionStart_ += now;
        func->transmissionStop_ += now;

        frameOffsetDelayProbe_->put(compound, func->transmissionStart_ - lastScheduling_);
        transmissionDelayProbe_->put(compound, func->transmissionStop_ - func->transmissionStart_);

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

void
DLCallback::callBack(wns::scheduler::SchedulingMapPtr schedulingMap)
{
    lastScheduling_ = wns::simulator::getEventScheduler()->getTime();    

    for(wns::scheduler::SubChannelVector::iterator iterSubChannel = schedulingMap->subChannels.begin();
        iterSubChannel != schedulingMap->subChannels.end(); ++iterSubChannel)
    {
        wns::scheduler::SchedulingSubChannel& subChannel = *iterSubChannel;
        for(wns::scheduler::SchedulingTimeSlotPtrVector::iterator iterTimeSlot = 
            subChannel.temporalResources.begin(); 
            iterTimeSlot != subChannel.temporalResources.end(); ++iterTimeSlot)
        {
            wns::scheduler::SchedulingTimeSlotPtr timeSlotPtr = *iterTimeSlot;
            for( wns::scheduler::PhysicalResourceBlockVector::iterator iterPRB = 
                timeSlotPtr->physicalResources.begin();
                iterPRB != timeSlotPtr->physicalResources.end(); ++iterPRB)
            {
                if ( iterPRB->hasScheduledCompounds() )
                {
                    wns::scheduler::ScheduledCompoundsList::const_iterator it;
                    
                    for(it = iterPRB->scheduledCompoundsBegin();
                        it != iterPRB->scheduledCompoundsEnd();
                        it++)
                    { // for every compound in subchannel:
                        processPacket(*it, timeSlotPtr);
                    } // for (all scheduledCompounds)
                    iterPRB->clearScheduledCompounds();
                } // if there were compounds in this resource
            } // forall beams
        } // end for ( timeSlots )
    } // forall subChannels
}

void
DLCallback::processPacket(const wns::scheduler::SchedulingCompound & compound,
    const wns::scheduler::SchedulingTimeSlotPtr& timeSlotPtr)
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
    endTime += timeSlotOffset;

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

    wns::scheduler::ChannelQualityOnOneSubChannel estimatedCQI = compound.estimatedCQI;
    
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
    m <<     ":  direction: DL \n"
    << "        PDU scheduled for user: " << colleagues.registry->getNameForUser(user) << "\n"
    << "        Frequency Slot: " << fSlot << "\n"
    << "        Time Slot: " << timeSlot <<" slotLength: "<<slotLength_<<"\n"
    << "        StartTime:      " << startTime<< "\n"
    << "        EndTime:        " << endTime<< "\n"
    << "        Beamforming:    " << beamforming << "\n"
    << "        Beam:           " << beam << "\n"
    << "        Tx Power:       " << txPower;
    LOG_INFO(fun_->getLayer()->getName(), m.str());
#endif

    PhyAccessFunc* func = 0;

    if(beamforming && (pattern != wns::service::phy::ofdma::PatternPtr()))
    {
        LOG_INFO(fun_->getLayer()->getName(), " DLCallback::processPacket() create BeamformingPhyAccessFunc");
        BeamformingPhyAccessFunc* sdmaFunc = new BeamformingPhyAccessFunc;
        sdmaFunc->destination_ = user.getNode();
        sdmaFunc->transmissionStart_ = startTime;
        sdmaFunc->transmissionStop_ =
        startTime + pduDuration - Utilities::getComputationalAccuracyFactor();
        sdmaFunc->subBand_ = fSlot;
        sdmaFunc->pattern_ = pattern;
        sdmaFunc->requestedTxPower_ = txPower;
        func = sdmaFunc;
    }
    else if(user.isValid())
    {
        LOG_INFO(fun_->getLayer()->getName(), " DLCallback::processPacket() create OmniUnicastPhyAccessFunc");
        OmniUnicastPhyAccessFunc* omniUnicastFunc = new OmniUnicastPhyAccessFunc;
        omniUnicastFunc->destination_ = user.getNode();
        omniUnicastFunc->transmissionStart_ = startTime;
        omniUnicastFunc->transmissionStop_ =
        startTime + pduDuration - Utilities::getComputationalAccuracyFactor();
        omniUnicastFunc->subBand_ = fSlot;
        omniUnicastFunc->requestedTxPower_ = txPower;
        func = omniUnicastFunc;
    }
    else
    {
        LOG_INFO(fun_->getLayer()->getName(), " ULMasterCallback::deliverNow() create BroadcastPhyAccessFunc");
        BroadcastPhyAccessFunc* broadcastFunc = new BroadcastPhyAccessFunc;
        broadcastFunc->transmissionStart_ = startTime;
        broadcastFunc->transmissionStop_ =
        startTime + pduDuration - Utilities::getComputationalAccuracyFactor();
        broadcastFunc->subBand_ = fSlot;
        func = broadcastFunc;
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
    phyCommand->peer.estimatedCQI = estimatedCQI;
    phyCommand->magic.sourceComponent_ = wimacComponent;

    scheduledPDUs.push(pdu);
}

