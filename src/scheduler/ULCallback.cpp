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
#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>
#include <boost/bind.hpp> 

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::scheduler::ULCallback,
    wimac::scheduler::Callback,
    "wimac.scheduler.ULCallback",
    wns::ldk::FUNConfigCreator );

using namespace wimac::scheduler;

ULCallback::ULCallback(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Callback(fun, config),
    fun_(fun),
    slotLength_(config.get<wns::simulator::Time>("slotLength")),
    tbCounter_(0),
    beamforming(config.get<bool>("beamforming"))
{}

void
ULCallback::callBack(wns::scheduler::SchedulingMapPtr schedulingMap)
{
    wimac::scheduler::RegistryProxyWiMAC* registry = dynamic_cast<wimac::scheduler::RegistryProxyWiMAC*>(colleagues.registry);
    bool isMaster = registry->getStationType(registry->getMyUserID()) == wns::service::dll::StationTypes::AP();
    if (isMaster && !beamforming)
        return;

    tbCounter_++;
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
                        /* Put estimated CQI in*/
                        it->estimatedCQI = iterPRB->getEstimatedCQI();
                        processPacket(*it, timeSlotPtr);
                    } // for (all scheduledCompounds)
                    iterPRB->clearScheduledCompounds();
                } // if there were compounds in this resource
            } // forall beams
        } // end for ( timeSlots )
    } // forall subChannels
}

//ULSlave processPacket, setting omnidirectional transmit phy access functor
void
ULCallback::processPacket(const wns::scheduler::SchedulingCompound & compound,
    wns::scheduler::SchedulingTimeSlotPtr& timeSlotPtr)
{
    simTimeType startTime = compound.startTime;
    simTimeType endTime = compound.endTime;
    wns::scheduler::UserID user = compound.userID;
    int userID = user.getNodeID();
    wns::service::phy::ofdma::PatternPtr pattern = compound.pattern;

    int fSlot = compound.subChannel;
    int timeSlot = compound.timeSlot;
    int beam = compound.spatialLayer; //beam;
    wns::Power txPower = compound.txPower;
    wns::service::phy::phymode::PhyModeInterfacePtr phyModePtr = compound.phyModePtr;


    simTimeType timeSlotOffset = timeSlot * slotLength_;
    startTime += timeSlotOffset;
    endTime += timeSlotOffset - Utilities::getComputationalAccuracyFactor();
    wns::scheduler::ChannelQualityOnOneSubChannel estimatedCQI = compound.estimatedCQI;

    double rate = phyModePtr->getDataRate();
    wns::ldk::CompoundPtr pdu  = compound.compoundPtr;
    simTimeType pduDuration = pdu->getLengthInBits() / rate;
    // TODO
    assure(pdu != wns::ldk::CompoundPtr(), "Invalid empty PDU");
    //assure(beam < maxBeams, "Too many beams");
    assure(endTime > startTime, "Scheduled PDU must end after it starts");
    //assure(endTime <= this->getDuration(), "PDU overun the maximum duration of the frame phase!");
    //assure(fSlot < freqChannels, "Invalid frequency channel");

#ifndef WNS_NO_LOGGING
    std::stringstream m;
    m <<     ":  direction: UL \n"
      << "        PDU scheduled for user (destination): " << colleagues.registry->getNameForUser(user) << "\n"
      << "        Frequency Slot: " << fSlot << "\n"
      << "        Time Slot: " << timeSlot << " slotLength: "<<slotLength_<< "\n"
      << "        StartTime:      " << startTime << "\n"
      << "        EndTime:        " << endTime<< "\n"
      << "        Beamforming:    " << beamforming << "\n"
      << "        Beam:           " << beam << "\n"
      << "        Tx Power:       " << txPower;
    LOG_INFO(fun_->getLayer()->getName(), m.str());
#endif

    PhyAccessFunc* func = 0;

     //in beamforming case (currently only in uplink master possible) the receive pattern need to be set
    if(compound.pattern != wns::service::phy::ofdma::PatternPtr())
    {
        assure(beamforming," set pattern without beamforming");
        LOG_INFO(fun_->getLayer()->getName(), " ULCallback::processPacket() create PatternSetterPhyAccessFunc " );
        PatternSetterPhyAccessFunc* patternFunc =
            new PatternSetterPhyAccessFunc;
        patternFunc->destination_ = user.getNode();
        patternFunc->patternStart_ = startTime;
        patternFunc->patternEnd_ = endTime;
        patternFunc->pattern_ = pattern;
        func = patternFunc;
    }
    else if(user.isValid())
    {
        LOG_INFO(fun_->getLayer()->getName(), " ULCallback::processPacket() create OmniUnicastPhyAccessFunc");
        OmniUnicastPhyAccessFunc* omniUnicastFunc = new OmniUnicastPhyAccessFunc;
        omniUnicastFunc->destination_ = user.getNode();
        omniUnicastFunc->transmissionStart_ = startTime;
        omniUnicastFunc->transmissionStop_ = endTime;
        omniUnicastFunc->subBand_ = fSlot;
        omniUnicastFunc->beam_ = beam;
        omniUnicastFunc->timeSlot_ = timeSlot;
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
    phyCommand->peer.estimatedCQI = estimatedCQI;
    phyCommand->magic.sourceComponent_ = wimacComponent;
    colleagues.harq->storeSchedulingTimeSlot(tbCounter_, timeSlotPtr);

    phyCommand->magic.schedulingTimeSlot = wns::scheduler::SchedulingTimeSlotPtr(
        new wns::scheduler::SchedulingTimeSlot(*timeSlotPtr));
  
    scheduledPDUs.push(pdu);
}

void ULCallback::deliverNow(wns::ldk::Connector* connector)
{
    LOG_INFO(fun_->getLayer()->getName(), " ULCallback::deliverNow() ");
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
        wimac::scheduler::RegistryProxyWiMAC* registry = dynamic_cast<wimac::scheduler::RegistryProxyWiMAC*>(colleagues.registry);
        bool isSlave = registry->getStationType(registry->getMyUserID()) == wns::service::dll::StationTypes::UT();
        if(isSlave)
        {
            frameOffsetDelayProbe_->put(compound, func->transmissionStart_ - lastScheduling_);
            transmissionDelayProbe_->put(compound, func->transmissionStop_ - func->transmissionStart_);
        }
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
