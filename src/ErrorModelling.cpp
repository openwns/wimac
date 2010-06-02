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


/**
 * \file
 * \author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/ErrorModelling.hpp>
#include <WIMAC/CIRProvider.hpp>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::ErrorModelling,
    wns::ldk::FunctionalUnit,
    "wimac.ErrorModelling",
    wns::ldk::FUNConfigCreator);


ErrorModelling::ErrorModelling(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::fu::Plain<ErrorModelling, ErrorModellingCommand>(fun),
    wns::ldk::Forwarding<ErrorModelling>(),
    CIRProviderName_(config.get<std::string>("cirProvider")),
    PHYModeProviderName_( config.get<std::string>("phyModeProvider")),
    friends()
{
}

void
ErrorModelling::processOutgoing(const wns::ldk::CompoundPtr& compound)
{
    activateCommand(compound->getCommandPool());

    LOG_INFO( getFUN()->getName(), " ErrorModelling: Outgoing Compound passed!" );
}


void
ErrorModelling::processIncoming(const wns::ldk::CompoundPtr& compound)
{
    wns::Ratio cir = dynamic_cast<CIRProviderCommand*>
        (friends.CIRProvider->getCommand(compound->getCommandPool()))->getCIR();

    const wns::service::phy::phymode::PhyModeInterface*	phyModePtr = 
        dynamic_cast<wimac::PhyModeProviderCommand*>(
            friends.PHYModeProvider->getCommand(compound->getCommandPool()))
                ->getPhyModePtr();

    int blocksize = compound->getLengthInBits();
    double mib = phyModePtr->getSINR2MIB(cir);
    double per = phyModePtr->getMI2PER(mib, blocksize); 

    //  Output
    this->getCommand(compound->getCommandPool())->local.per = per;

    LOG_INFO( getFUN()->getName(),
              ": doOnData!  CIR=",cir.get_dB()," PHY Mode=", phyModePtr->getString(),
              " ; PER=",getCommand( compound->getCommandPool() )->local.per);
}

void
ErrorModelling::onFUNCreated()
{
    friends.CIRProvider = getFUN()->findFriend<FunctionalUnit*>(CIRProviderName_);
    assure(friends.CIRProvider,
           "ErrorModelling requires a PHYUser friend with name '"
           + CIRProviderName_ + "' \n");
    friends.PHYModeProvider = getFUN()->findFriend<FunctionalUnit*>(PHYModeProviderName_);
    assure(friends.PHYModeProvider,
           "ErrorModelling requires a PHYModeProvider friend with name '"
           + PHYModeProviderName_ + "' \n");
}


