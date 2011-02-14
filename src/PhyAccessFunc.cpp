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

#include <WIMAC/PhyAccessFunc.hpp>

#include <WIMAC/PhyUser.hpp>
#include <WIMAC/Logger.hpp>

using namespace wimac;

void StopTransmission::operator()()
{
    LOG_INFO(phyUser_->getFUN()->getName(), " stop transmission");
    phyUser_->getNotificationService()->enableReception();
    phyUser_->getDataTransmissionService()->stopTransmission( compound_, subBand_ );
}

void StartTransmission::operator()()
{
    LOG_INFO(phyUser_->getFUN()->getName(), " start transmission to ", dstStation_->getName());
    assure(phyMode_,"invalid PhyMode");
    phyUser_->getNotificationService()->disableReception();
    phyUser_->getDataTransmissionService()->startBroadcast( compound_, subBand_, requestedTxPower_, phyMode_ );
}

void StartBroadcastTransmission::operator()()
{
    LOG_INFO(phyUser_->getFUN()->getName(), " start broadcast transmission");
    wns::Power defaultTxPower = phyUser_->getDataTransmissionService()->getMaxPowerPerSubband();
    assure(phyMode_,"invalid PhyMode");
    phyUser_->getNotificationService()->disableReception();
    phyUser_->getDataTransmissionService()->startBroadcast( compound_, subBand_, defaultTxPower, phyMode_ );
}

void StartBeamformingTransmission::operator()()
{
    assure(dstStation_ != NULL, " destination is required for beamforming transmission ");
    assure(pattern_ != wns::service::phy::ofdma::PatternPtr(), " pattern is required for beamforming transmission");
    assure(phyMode_,"invalid PhyMode");
    LOG_INFO(phyUser_->getFUN()->getName(), ": starts beamforming transmission to: ", dstStation_->getName());
    phyUser_->getNotificationService()->disableReception();
    phyUser_->getDataTransmissionService()->startTransmission(compound_, dstStation_, subBand_, pattern_, requestedTxPower_, phyMode_ );
}

void SetPattern::operator()()
{
    LOG_INFO(phyUser_->getFUN()->getName(), " set pattern to ", dstStation_->getName());
    phyUser_->getDataTransmissionService()->insertReceivePattern( dstStation_, pattern_ );
}

void RemovePattern::operator()()
{
    phyUser_->getDataTransmissionService()->removeReceivePattern(dstStation_);
}

void
BroadcastPhyAccessFunc::operator()(wimac::PhyUser* phyUser, const wns::ldk::CompoundPtr& compound )
{
    assureNotNull(phyMode_.getPtr());
    StartBroadcastTransmission start ( phyUser, compound, phyMode_ );
    wns::simulator::getEventScheduler()->schedule( start, transmissionStart_ );

    StopTransmission stop ( phyUser, compound );
    wns::simulator::getEventScheduler()->schedule( stop, transmissionStop_);
}

void OmniUnicastPhyAccessFunc::operator()( PhyUser* pu, const wns::ldk::CompoundPtr& compound )
{
    assureNotNull(phyMode_.getPtr());
    StartTransmission start ( pu, compound, destination_, phyMode_, subBand_,  requestedTxPower_);

    wns::simulator::getEventScheduler()
        ->schedule( start,  transmissionStart_ );


    StopTransmission stop ( pu, compound );

    wns::simulator::getEventScheduler()
        ->schedule( stop, transmissionStop_ );
}

void BeamformingPhyAccessFunc::operator()( PhyUser* pu, const wns::ldk::CompoundPtr& compound )
{
    assure( destination_ != NULL, " destination is required for beamforming transmission ");
    assure(pattern_ != wns::service::phy::ofdma::PatternPtr(), " pattern is required for beamforming transmission");
    assure(subBand_ >= 0," negative Subband is a not valid value");
    assureNotNull(phyMode_.getPtr());

    StartBeamformingTransmission start ( pu, compound, destination_,
                                         pattern_, subBand_,
                                         requestedTxPower_, phyMode_ );
    wns::simulator::getEventScheduler()
        ->schedule( start,  transmissionStart_ );

    StopTransmission stop ( pu, compound );
    wns::simulator::getEventScheduler()
        ->schedule( stop, transmissionStop_ );
}

void PatternSetterPhyAccessFunc::operator()( PhyUser* pu, const wns::ldk::CompoundPtr& )
{
    patternStart_ = transmissionStart_;
    patternEnd_ = transmissionStop_;
    SetPattern setter ( pu, destination_, pattern_);
    wns::simulator::getEventScheduler()
        ->schedule( setter, patternStart_ );
    
    //race condition: removing before reading by approximalty 1.6e-5 [s] 
    /*RemovePattern remover (pu, destination_);
    wns::simulator::getEventScheduler()
        ->schedule( remover, patternEnd_);*/
}
